#pragma once
#include "pch.h"
#include "CeresSolver.h"
#include "SmoothTypes.h"
#define _MIN(x, y) ((x)<(y)?(x):(y))
#define _MAX(x, y) ((x)<(y)?(y):(x))


enum ParamSpace {
	param_diffuse = 0x1,
	param_specular = 0x2,
	param_roughness = 0x4,
	param_normal = 0x8,
};
DEFINE_ENUM_FLAG_OPERATORS(ParamSpace)


enum class NormalOptMode {
	raw_normal,
	raw_normal2D,
	heightmap,
	heightmap2018,
	heightmap2020,
};


enum class DifferenceMode {
	forward,
	forward2,
	central
};


enum class Param {
	diffuseR, diffuseG, diffuseB, specular, roughness, height, sphere, MAX, diffuse
};


class AppearanceSolver : public CeresSolver
{
	struct DomainIter;

	struct Domain {
		int sx = 0;
		int sy = 0;
		int ex = 0;
		int ey = 0;
		const AppearanceSolver& solver;

		void set(int startX, int startY, int width, int height)
		{
			sx = _MAX(startX, 1);
			sy = _MAX(startY, 1);
			ex = _MIN(startX + width, solver.width - 1);
			ey = _MIN(startY + height, solver.height - 1);
		}

		Domain(const AppearanceSolver& solver) : solver(solver) {}
		
		int area() const 
			{ return (ex - sx) * (ey - sy); }
		
		DomainIter begin()  const 
			{ return DomainIter{ sx, sy, *this }; }
		
		DomainIter end() const 
			{ return DomainIter{ sx, ey, *this }; }
	} 
	domain = Domain(*this);

	struct DomainIter {
		int j, i;
		const Domain& domain;

		int operator*() const
			{ return i * domain.solver.width + j; }
		
		bool operator!=(const DomainIter& iter2) const 
			{ return j != iter2.j || i != iter2.i; }

		void operator++() 
		{
			if (++j >= domain.ex)
			{
				j = domain.sx;
				++i;
			}
		}
	};

	struct PathInfo {
		std::string inDir;
		std::string inCameraInfo		= "CameraInfo.json";
		std::string inLightSampleInfo	= "LightSampleInfo.json";
		std::string inPositionMap		= "posMap";
		std::string inNormalMap			= "norMap";
		std::string inViewMaps			= "viewMap$";
		std::string inWeightMaps		= "weightMap$";
		std::string inShadowMaps		= "shadowMap$";

		std::string outDir;
		std::string outFormat			= ".jpg";
		std::string outDiffuseMap		= "diffuse";
		std::string outSrgbDiffuseMap	= "diffuse_srgb";
		std::string outSpecularMap		= "specualr";
		std::string outNormalMap		= "normal";
		std::string outHeightMap		= "height";
		std::string outRoughnessMap		= "roughness";
		std::string loggingfile			= "OupLog.txt";
	} pathInfo;

	struct ProblemOptions {
		NormalOptMode normalMode{ NormalOptMode::raw_normal };
		DifferenceMode diffMode{ DifferenceMode::forward2 };
		bool bActiveShadow = true;
		
		double viewWeightMin = 1e-4;
		double viewWeightBias = 1.0;
		int zeroRadius = 3;

		ParamSpace params{};
		bool constantSpecular = false;	
		bool constantRoughness = false;

		double channelWeight[3] = { 1.0, 1.0, 1.0 };

		std::map<
			std::pair<Param, SmoothType>, 
			std::pair<double, double> > smoothWeightAndExp;

		double base[(int)Param::MAX];
		std::map<Param, std::pair<double, double>> bounds;

	} problemOpts;

	struct RecordOptions {
		bool writeVisibility = false;
		bool recordIterSeparately = true;
		std::set<int> viewIdices;
		double maxSpecular = 1.5;
		double maxRoughness = 0.8;
		double maxHeight = 1.0;
		double maxView = 1.0;
	} recordOpts;

	enum SolverState {
		invalidInputData,
		invalidSolution,
		invalidHeight,
		invalidTBN,
		invalidProblem,
		solvable
	} solverState = invalidInputData;

	void changeState(SolverState newState) {
		solverState = _MIN(solverState, newState);
	}

public:
	const double* getMapValue(Param param, int x, int y=0) const {
		switch (param) {
		case Param::diffuse:
			return &diffuseMap[3*(x + y*dy)][0];
		case Param::specular:
			return &specularMap[x + y*dy];
		case Param::roughness:
			return &roughnessMap[x + y*dy];
		case Param::height:
			return &heightMap[x + y*dy];
		case Param::sphere:
			return &sphereMap[2*(x + y*dy)][0];
		}
	}

	void  writeVisibilityAtNextRun() {
		recordOpts.writeVisibility = true;
	}

	void setRecordViewIndices(std::set<int> viewIdices) {
		recordOpts.viewIdices = std::move(viewIdices);
	}

	void setOutputDirectory(std::string outputDir) {
		pathInfo.outDir = (outputDir.back() == '/' || outputDir.back() == '\\') ?
			std::move(outputDir) : std::move(outputDir + '/');
	}

	void setInputDirectory(std::string inputDir) {
		pathInfo.inDir = (inputDir.back() == '/' || inputDir.back() == '\\') ?
			std::move(inputDir) : std::move(inputDir + '/');
		changeState(invalidInputData);
	}

	void setDomain(int startX, int startY, int width, int height) {
		domain.set(startX, startY, width, height);
		changeState(invalidSolution);
	}

	void setActiveShadow(bool bActive) {
		if (problemOpts.bActiveShadow == bActive)
			return;
		problemOpts.bActiveShadow = bActive;
	}

	void setNormalMode(NormalOptMode norMode) {
		if (problemOpts.normalMode == norMode)
			return;
		problemOpts.normalMode = norMode;
		changeState(invalidHeight);
	}

	void setDifferenceMode(DifferenceMode diffMode) {
		if (problemOpts.diffMode == diffMode)
			return;
		problemOpts.diffMode = diffMode;
		changeState(invalidTBN);
	}

	void setDisabledCameras(std::set<int> viewIdices) {
		disabledCameras = std::move(viewIdices);
		changeState(invalidProblem);
	}

	void setViewWeightMin(double weightMin) {
		if (problemOpts.viewWeightMin == weightMin)
			return;
		problemOpts.viewWeightMin = weightMin;
		changeState(invalidProblem);
	}

	void setViewWeightBias(double weightBias) {
		if (problemOpts.viewWeightBias == weightBias)
			return;
		problemOpts.viewWeightBias = weightBias;
		changeState(invalidProblem);
	}

	void setZeroRadius(int radius) {
		if (problemOpts.zeroRadius == radius)
			return;
		problemOpts.zeroRadius = radius;
		changeState(invalidProblem);
	}

	// From here, set** methods can be used to obtain single multi-phase solution.
	void setParamSpace(ParamSpace params) {
		if (problemOpts.params == params)
			return;
		problemOpts.params = params;
		changeState(invalidProblem);
	}

	void setConstantSpecular(bool bConstant) {
		if (problemOpts.constantSpecular == bConstant)
			return;
		problemOpts.constantSpecular = bConstant;
		changeState(invalidProblem);
	}

	void setConstantRoughness(bool bConstant) {
		if (problemOpts.constantRoughness == bConstant)
			return;
		problemOpts.constantRoughness = bConstant;
		changeState(invalidProblem);
	}

	void setChannelWeight(Eigen::Vector3d weight) {
		if (problemOpts.channelWeight[0] == weight[0] &&
			problemOpts.channelWeight[1] == weight[1] &&
			problemOpts.channelWeight[2] == weight[2])
			return;
		problemOpts.channelWeight[0] = weight[0];
		problemOpts.channelWeight[1] = weight[1];
		problemOpts.channelWeight[2] = weight[2];
		changeState(invalidProblem);
	}

	void setSmoothCostBase(Param param, double base) {
		if(problemOpts.base[(int)param] == base)
			return;
		problemOpts.base[(int)param] = base;
		changeState(invalidProblem);
	}

	void setSmoothCost(Param param, SmoothType type, double weight, double exp = 1.0) {
		if (param == Param::diffuse) {
			setSmoothCost(Param::diffuseR, type, weight, exp);
			setSmoothCost(Param::diffuseG, type, weight, exp);
			setSmoothCost(Param::diffuseB, type, weight, exp);
			return;
		}

		if (auto it = problemOpts.smoothWeightAndExp.find({ param, type }); 
			it != problemOpts.smoothWeightAndExp.end())
		{
			if(it->second.first == weight && it->second.second == exp)
				return;
			if(weight == 0.0)
				problemOpts.smoothWeightAndExp.erase(it);
			else
				it->second = {weight, exp};
			changeState(invalidProblem);
			return;
		}

		problemOpts.smoothWeightAndExp[{param, type}] = {weight, exp};
		changeState(invalidProblem);
		return;
	}

	void setSmoothCost(Param param, SmoothType type, Eigen::Vector3d weight, double exp = 1.0) {
		if (param == Param::diffuse) {
			setSmoothCost(Param::diffuseR, type, weight[0], exp);
			setSmoothCost(Param::diffuseG, type, weight[1], exp);
			setSmoothCost(Param::diffuseB, type, weight[2], exp);
		}
		else throw;
	}

	void setSmoothCost(Param param, SmoothType type, Eigen::Vector3d weight, Eigen::Vector3d exp) {
		if (param == Param::diffuse) {
			setSmoothCost(Param::diffuseR, type, weight[0], exp[0]);
			setSmoothCost(Param::diffuseG, type, weight[1], exp[1]);
			setSmoothCost(Param::diffuseB, type, weight[2], exp[2]);
		}
		else throw;
	}

	void setBound(Param param, double minValue = DBL_MAX, double maxValue = DBL_MIN) {
		if (param == Param::diffuse) {
			setBound(Param::diffuseR, minValue, maxValue);
			setBound(Param::diffuseG, minValue, maxValue);
			setBound(Param::diffuseB, minValue, maxValue);
			return;
		}

		if (auto it = problemOpts.bounds.find(param);
			it != problemOpts.bounds.end())
		{
			if(it->second.first == minValue && it->second.second == maxValue)
				return;
			if(minValue == DBL_MAX && maxValue == DBL_MIN)
				problemOpts.bounds.erase(it);
			else
				it->second = {minValue, maxValue};
			changeState(invalidProblem);
			return;
		}

		problemOpts.bounds[param] = {minValue, maxValue};
		changeState(invalidProblem);
	}
	// Until here, set** methods can be used to obtain single multi-phase solution.

	ceres::CallbackReturnType operator()(const ceres::IterationSummary& summary) override;
	AppearanceSolver(int width, int height);
	void invalidateSolution() { 
		changeState(invalidSolution); 
	}
	void run(int maxIter) {
		setMaxNumIteration(maxIter);
		run();
	}
	void run();

private:
	// Remember that below functions can be called only in run() !!
	bool loadInputData(std::ostream& log = std::cout);
	void resetSolution();
	void computeTBNMatrix();
	void createProblem();

	void constructNormal();
	void constructView();
	void writeVisibilityImage();
	void printConfigurations();

	template<typename T>
	Eigen::Vector<T, 3> evaluate(
		int viewIdx,
		int pixelIdx,
		const Eigen::Vector<T, 3>& diffuse,
		const T& specular,
		const T& roughness,
		const Eigen::Vector<T, 3>& N) const;

	class AccuracyCost {
		friend AppearanceSolver;
		AppearanceSolver& solver;
		const int v = -1;
		const int p = -1;
		const double weight;
		AccuracyCost(AppearanceSolver& solver, int viewIdx, int pixelIdx, double weight)
			: solver(solver), v(viewIdx), p(pixelIdx), weight(weight) {}
	public:
		template<typename... Params> bool operator()(Params* ... params) const;
	};

	bool isEnabled(int cameraId) const {
		return disabledCameras.find(cameraId) == disabledCameras.end();
	}

	bool isImaging(int cameraId) const {
		return recordOpts.viewIdices.find(cameraId) != recordOpts.viewIdices.end();
	}

	bool isValidPixel(const auto& viewMap, int p) const {
		int r = problemOpts.zeroRadius;

		int px = p % width;
		int py = p / width;
		int sx = _MAX(0, px - r);
		int sy = _MAX(0, py - r);
		int ex = _MIN(width  - 1, px + r);
		int ey = _MIN(height - 1, py + r);

		for (int i = sy; i <= ey; ++i)
			for (int j = sx; j <= ex; ++j)
		{
			if (viewMap[i * width + j].isZero())
				return false;
		}
		return true;
	}

private:
	struct LightSample {
		Eigen::Vector3d position{};
		Eigen::Vector3d normal{};
		Eigen::Vector3d emittance{};
		double area{};
	};

	struct ViewData {
		int								cameraId;
		Eigen::Vector3d					cameraPos{};
		std::vector<double>				weightMap;
		std::vector<Eigen::Vector3d>	trgViewMap;
		std::vector<Eigen::Vector3d>	viewMap;
		std::vector<Eigen::Vector3d>	errorMap;
	};

	inline static const double defaultSpecular = 1.0;
	inline static const double defaultRoughness = 0.2;
	inline static const int shadowPackSize = 32;
	
	const int height = 0;
	const int width = 0;
	const int& dx = 1;
	const int& dy = width;
	
	std::set<int>					disabledCameras;

	std::vector<Eigen::Vector3d>	diffuseMap;
	std::vector<double>				specularMap;
	std::vector<double>				roughnessMap;
	std::vector<double>				heightMap;
	std::vector<Eigen::Vector3d>	normalMap;
	std::vector<Eigen::Vector2d>	sphereMap;

	std::vector<LightSample>		lightSamples;
	std::vector<Eigen::Vector3d>	positionMap;
	std::vector<Eigen::Vector3d>	geoNormalMap;
	std::vector<ViewData>			views;
	std::vector<std::vector<uint>>	shadowMaps;
	std::vector<Eigen::Matrix3d>	tbnMap;

	bool							bUseShadow = false;
	int								iterCount = -1;
	clock_t							lastTime;
};

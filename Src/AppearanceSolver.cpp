#include "pch.h"
#include "AppearanceSolver.h"
#include "utils.h"
#include <random>
#include <filesystem>


template <typename _SrcType, int _srcComp, int _trgComp = _srcComp>
struct ReadInfo {
	using SrcType = _SrcType;
	constexpr static int srcComp = _srcComp;
	constexpr static int trgComp = _trgComp;
};


void AppearanceSolver::printConfigurations()
{
	for (auto& view : views)
	{
		if (isEnabled(view.cameraId) && isImaging(view.cameraId))
		{
			writeImage(pathInfo.outDir + "view" + std::to_string(view.cameraId) + "_1000" + ".jpg",
				(double*)view.trgViewMap.data(), width, height, 3, 0.0, recordOpts.maxView);
		}
	}

	FILE* fp = fopen((pathInfo.outDir + pathInfo.loggingfile).c_str(), /*iterCount < 0 ? "w" :*/ "a");
	const auto& opt = problemOpts;

	fprintf(fp, "\n\n\n");
	fprintf(fp, "Domain              :  %d, %d, %d, %d\n", domain.sx, domain.sy, domain.ex - domain.sx, domain.ey - domain.sy);
	if (!disabledCameras.empty())
	{
		fprintf(fp, "Disabled Camera Ids :  ");
		for (int id : disabledCameras)
			fprintf(fp, "%d, ", id);
		fseek(fp, -2, SEEK_CUR);
		fprintf(fp, "\n");
	}
	fprintf(fp, "View weight min     :  %g\n", opt.viewWeightMin);
	fprintf(fp, "View weight bias    :  %g\n", opt.viewWeightBias);
	fprintf(fp, "Zero radius         :  %d\n", opt.zeroRadius);
	fprintf(fp, "Channel weight      :  [%g, %g, %g]\n", opt.channelWeight[0], opt.channelWeight[1], opt.channelWeight[2]);

	if (opt.params & ParamSpace::param_normal)
	{
		if (opt.normalMode == NormalOptMode::raw_normal)
			fprintf(fp, "NormalMode          :  raw\n");
		else if (opt.normalMode == NormalOptMode::raw_normal2D)
			fprintf(fp, "NormalMode          :  raw2D\n");
		else if (opt.normalMode == NormalOptMode::heightmap2018)
			fprintf(fp, "NormalMode          :  heightmap2018\n");
		else if (opt.normalMode == NormalOptMode::heightmap2020)
			fprintf(fp, "NormalMode          :  heightmap2020\n");
		else if (opt.normalMode == NormalOptMode::heightmap)
			fprintf(fp, "NormalMode          :  heightmap\n");

		if (opt.diffMode == DifferenceMode::forward)
			fprintf(fp, "DifferenceMode      :  forward\n");
		else if (opt.diffMode == DifferenceMode::forward2)
			fprintf(fp, "DifferenceMode      :  forward2\n");
		else if (opt.diffMode == DifferenceMode::central)
			fprintf(fp, "DifferenceMode      :  central\n");
	}
	fprintf(fp, "\n");

	static std::unordered_map<Param, std::string> paramToString = {
		{Param::diffuse,   "Diffuse"},
		{Param::specular,  "Specular"},
		{Param::roughness, "Roughness"},
		{Param::height,    "Height"},
		{Param::sphere,    "Sphere"}
	};

	if(opt.params & ParamSpace::param_diffuse)
	{
		fprintf(fp, "[Paramerter : %s]\n", paramToString[Param::diffuse].c_str());

		double lb[3] = {DBL_MAX, DBL_MAX, DBL_MAX};
		double ub[3] = {DBL_MIN, DBL_MIN, DBL_MIN};
		bool lbUsed = false;
		bool ubUsed = false;
	
		for (int k = 0; k < 3; ++k)
		{
			if (auto it = opt.bounds.find((Param)k); it != opt.bounds.end())
			{
				if (it->second.first != DBL_MAX)
				{
					lb[k] = it->second.first;
					lbUsed = true;
				}
				if (it->second.second != DBL_MIN)
				{
					ub[k] = it->second.second;
					ubUsed = true;
				}
			}
		}
	
		if (lbUsed)
		{
			if(lb[0] == lb[1] && lb[1] == lb[2])
				fprintf(fp, "Lower bound         :  %g\n", lb[0]);
			else
				fprintf(fp, "Lower bound         :  [%g, %g, %g]\n", lb[0], lb[1], lb[2]);
		}

		if (ubUsed)
		{
			if(ub[0] == ub[1] && ub[1] == ub[2])
				fprintf(fp, "Upper bound         :  %g\n", ub[0]);
			else
				fprintf(fp, "Upper bound         :  [%g, %g, %g]\n", ub[0], ub[1], ub[2]);
		}

		struct WeightExp {
			double weight[3] = {0.0, 0.0, 0.0};
			double exp[3]    = {1.0, 1.0, 1.0};
		};
		std::map<SmoothType, WeightExp> typeToWeight;
		{
			auto begin = opt.smoothWeightAndExp.lower_bound({Param::diffuseR, SmoothType::MIN});
			auto end   = opt.smoothWeightAndExp.upper_bound({Param::diffuseB, SmoothType::MAX});
			for (auto it = begin; it != end; ++it) 
			{
				auto& [param, type] = it->first;
				auto& [weight, exp] = it->second;
				typeToWeight[type].weight[(int)param - (int)Param::diffuseR] = weight;
				typeToWeight[type].exp   [(int)param - (int)Param::diffuseR] = exp;
			}
		}

		for (auto& [type, value] : typeToWeight) 
		{
			std::string weightStr = std::format("{:g}", value.weight[0]);
			if(value.weight[0] != value.weight[1] || value.weight[1] != value.weight[2])
				weightStr = std::format("[{:g}, {:g}, {:g}]", value.weight[0], value.weight[1], value.weight[2]);

			std::string expStr = std::format("{:g}", value.exp[0]);
			if(value.exp[0] != value.exp[1] || value.exp[1] != value.exp[2])
				weightStr = std::format("[{:g}, {:g}, {:g}]", value.exp[0], value.exp[1], value.exp[2]);

			fprintf(fp, std::format("SmoothCost_{:<7}  :  weight={},  exponet={}\n", toString(type), weightStr, expStr).c_str());
		}
		fprintf(fp, "\n");
	}

	auto printParam = [&opt, fp](Param param, bool isConstant=false) {
		fprintf(fp, "[Paramerter : %s]\n", paramToString[param].c_str());

		if (auto it = opt.bounds.find(param); it != opt.bounds.end()) {
			if (it->second.first  != DBL_MAX) fprintf(fp, "Lower bound         :  %g\n", it->second.first);
			if (it->second.second != DBL_MIN) fprintf(fp, "Upper bound         :  %g\n", it->second.second);
		}

		if (isConstant) {
			fprintf(fp, "bConstant           :  true\n");
			return;
		}

		auto begin = opt.smoothWeightAndExp.lower_bound({param, SmoothType::MIN});
		auto end   = opt.smoothWeightAndExp.upper_bound({param, SmoothType::MAX});
		for (auto it = begin; it != end; ++it) {
			SmoothType type = it->first.second;
			auto& [weight, exp] = it->second;
			std::string str = std::format("SmoothCost_{:<7}  :  weight={:g},  exponet={:g}", toString(type), weight, exp);
			if(useBase(type))
				str += std::format(",  base={:g}\n", opt.base[(int)param]);
			else
				str += '\n';
			fprintf(fp, str.c_str());
		}

		fprintf(fp, "\n");
	};

	if(opt.params & ParamSpace::param_specular)
		printParam(Param::specular, opt.constantSpecular);
	if(opt.params & ParamSpace::param_roughness)
		printParam(Param::roughness, opt.constantRoughness);
	if(opt.params & ParamSpace::param_normal)
		if(opt.normalMode == NormalOptMode::raw_normal2D)
			printParam(Param::sphere);
		else if(opt.normalMode != NormalOptMode::raw_normal)
			printParam(Param::height);

	if (bUseShadow)
		fprintf(fp, "Shadow              :  On\n");
	else
		fprintf(fp, "Shadow              :  Off\n");

	

	fprintf(fp, "Max Iterations      :  %d\n\n", solverOptions.max_num_iterations);
	fprintf(fp, "-------------Now, Solve!-------------\n");
	fclose(fp);
}


ceres::CallbackReturnType AppearanceSolver::operator()(const ceres::IterationSummary& summary)
{
	printf("%d'th iteration has passed, and %d seconds has passed during the iteration.\n", ++iterCount, (clock() - lastTime) / CLOCKS_PER_SEC);

	if(iterCount%2 == 1)
	{
		std::string iter = recordOpts.recordIterSeparately ? ("_" + std::to_string(iterCount)) : "";

		auto makePath = [&](std::string&& name, std::string&& name2="") {
			return pathInfo.outDir + name + iter + name2 + ".jpg";
		};

		if (problemOpts.params & ParamSpace::param_diffuse)
		{
			writeImage(makePath("predicted_diffuse_srgb"),
				(double*)diffuseMap.data(), width, height, 3, 0.0, 1.0, true);
			writeImage(makePath("predicted_diffuse"),
				(double*)diffuseMap.data(), width, height, 3, 0.0, 1.0, false);
		}

		if (problemOpts.params & ParamSpace::param_specular)
		{
			if (!problemOpts.constantSpecular)
			{
				writeImage(makePath("predicted_specular"),
					specularMap.data(), width, height, 1, 0.0, recordOpts.maxSpecular);
			}
		}

		if (problemOpts.params & ParamSpace::param_roughness)
		{
			if (!problemOpts.constantRoughness)
			{
				writeImage(makePath("predicted_roughness"),
					roughnessMap.data(), width, height, 1, 0.0, recordOpts.maxRoughness);
			}
		}

		if (problemOpts.params & ParamSpace::param_normal)
		{
			if (problemOpts.normalMode != NormalOptMode::raw_normal)
			{
				if (problemOpts.normalMode != NormalOptMode::raw_normal2D)
					writeImage(makePath("predicted_height"),
						heightMap.data(), width, height, 1, -recordOpts.maxHeight, recordOpts.maxHeight);
				constructNormal();
			}
			writeImage(makePath("predicted_normal"),
				(double*)normalMap.data(), width, height, 3, -1.0, 1.0);
		}

		constructView();

		for (auto& view : views)
		{
			if (isEnabled(view.cameraId) && isImaging(view.cameraId))
			{
				writeImage(makePath("view" + std::to_string(view.cameraId)),
					(double*)view.viewMap.data(), width, height, 3, 0.0, recordOpts.maxView);
				writeImage(makePath("view" + std::to_string(view.cameraId) + "err"),
					(double*)view.errorMap.data(), width, height, 3, 0.0, recordOpts.maxView * 0.1);
			}
		}
	}

	FILE* fp = fopen((pathInfo.outDir + pathInfo.loggingfile).c_str(), "a");
	fprintf(fp, "Iter %d :\n", iterCount);
	if (problemOpts.constantSpecular)  fprintf(fp, "\tConstantSpecular    :  %lf\n", specularMap[0]);
	if (problemOpts.constantRoughness) fprintf(fp, "\tConstantRoughness   :  %lf\n", roughnessMap[0]);
	fclose(fp);

	lastTime = clock();
	return ceres::CallbackReturnType::SOLVER_CONTINUE;
}


AppearanceSolver::AppearanceSolver(int width, int height)
	: width(width), height(height) 
{
	diffuseMap.resize(width * height, { 0.0, 0.0, 0.0 });
	specularMap.resize(width * height, defaultSpecular);
	roughnessMap.resize(width * height, defaultRoughness);
	heightMap.resize(width * height, 0.0);						
	sphereMap.resize(width * height, { 0.0, 0.0 });
	normalMap.resize(width * height);
	tbnMap.resize(width * height);
	
	problemOpts.base[(int)Param::diffuseR] = 0.0;
	problemOpts.base[(int)Param::diffuseG] = 0.0;
	problemOpts.base[(int)Param::diffuseB] = 0.0;
	problemOpts.base[(int)Param::specular] = defaultSpecular;
	problemOpts.base[(int)Param::roughness] = defaultRoughness;
	problemOpts.base[(int)Param::height] = 0.0;
	problemOpts.base[(int)Param::sphere] = 0.0;

	domain.set(0, 0, width, height);
	setMaxNumIteration(21);
	setFunctionTolerance(1e-7);
}


bool AppearanceSolver::loadInputData(std::ostream& log)
{
	printf("Loading input data...\n");
	
	lightSamples.clear();
	positionMap.clear();
	geoNormalMap.clear();
	views.clear();
	shadowMaps.clear();

	auto clear = [](auto& container) {
		container.clear();
		container.shrink_to_fit();
	};

	auto readBinary = [&](auto staticInfo, std::string& fileName, auto& out) {
		try
		{
			using Info = std::decay_t<decltype(staticInfo)>;
			readBinaryImage<Info::SrcType, Info::srcComp, Info::trgComp>(pathInfo.inDir + fileName, width, height, out);
			log << "-> The corresponding map is generated from the file: " << fileName << std::endl;
			return true;
		}
		catch (std::runtime_error& e)
		{
			log << "-> " << e.what() << ": " << pathInfo.inPositionMap << std::endl;
			return false;
		}
	};
	

	bool success = true;
	int numLightSamples = 0;

	if (std::ifstream lightFile(pathInfo.inDir + pathInfo.inLightSampleInfo); !lightFile)
	{
		log << "-> Failed to read file: " << pathInfo.inLightSampleInfo << std::endl;
		success = false;
	}
	else
	{
		auto sampleInfo = json::parse(lightFile)["Samples"];
		numLightSamples = (int) sampleInfo.size();
		lightSamples.reserve(numLightSamples);
		for (const auto& sample : sampleInfo)
		{
			LightSample l;
			for (int i = 0; i < 3; ++i) {
				l.position[i] = sample["position"][i];
				l.normal[i] = sample["normal"][i];
				l.emittance[i] = sample["emittance"][i];
			}
			l.area = sample["area"];
			lightSamples.push_back(l);
		}
	}

	if (!success)
	{
		lightSamples.clear();
		return false;
	}
	else
	{
		int numMaps = (numLightSamples + shadowPackSize - 1) / shadowPackSize;
		size_t pos = pathInfo.inShadowMaps.find_last_of('$');
		shadowMaps.resize(numMaps);

		for (int i = 0; i < numMaps; ++i)
		{
			std::string inShadowMap = pathInfo.inShadowMaps;
			inShadowMap.replace(pos, 1, std::to_string(i + 1));
			if (!readBinary(ReadInfo<uint, 1>(), inShadowMap, shadowMaps[i]))
			{
				log << "---> All Shadow maps are being discarded !!" << std::endl;
				clear(shadowMaps);
				break;
			}
		}
	}

	success &= readBinary(ReadInfo<float, 4, 3>(), pathInfo.inPositionMap, positionMap);
	success &= readBinary(ReadInfo<float, 4, 3>(), pathInfo.inNormalMap, geoNormalMap);

	if (!success)
	{
		clear(lightSamples);
		clear(positionMap);
		clear(geoNormalMap);
		return false;
	}

	int nFailed = 0;

	if (std::ifstream cameraFile(pathInfo.inDir + pathInfo.inCameraInfo); !cameraFile)
	{
		log << "-> Failed to read file: " << pathInfo.inCameraInfo << std::endl;
	}
	else
	{
		auto cameraInfo = json::parse(cameraFile);
		size_t vPos = pathInfo.inViewMaps.find_last_of('$');
		size_t wPos = pathInfo.inWeightMaps.find_last_of('$');

		assert(cameraInfo["Coordinates"] == "Unreal");
		
		for (const auto& camera : cameraInfo["Poses"])
		{
			ViewData viewData;
			viewData.cameraId = camera["id"];
			viewData.cameraPos = {
				-(double)camera["position"][0],
				(double)camera["position"][2],
				(double)camera["position"][1] };

			std::string inViewMap = pathInfo.inViewMaps;
			inViewMap.replace(vPos, 1, std::to_string(viewData.cameraId));
			if (!readBinary(ReadInfo<float, 4, 3>(), inViewMap, viewData.trgViewMap))
			{
				nFailed++;
				continue;
			}

			std::string inWeightMap = pathInfo.inWeightMaps;
			inWeightMap.replace(wPos, 1, std::to_string(viewData.cameraId));
			if (!readBinary(ReadInfo<float, 1>(), inWeightMap, viewData.weightMap))
			{
				nFailed++;
				continue;
			}

			viewData.viewMap.resize(width * height, Eigen::Vector3d(0.0, 0.0, 0.0));
			viewData.errorMap.resize(width * height, Eigen::Vector3d(0.0, 0.0, 0.0));

			views.push_back(std::move(viewData));
		}
	}

	if (views.size() == 0)
	{
		clear(lightSamples);
		clear(positionMap);
		clear(geoNormalMap);
		return false;
	}

	if (nFailed > 0)
		log << "[WARNING] " << nFailed << " captured images cannot be read." << std::endl;
	
	normalMap = geoNormalMap;

	return true;
}


void AppearanceSolver::resetSolution()
{
	std::mt19937 rng;
	std::uniform_real_distribution<double> distrb(0.0, 1.0);

	for (int p : domain)
	{
		diffuseMap[p] = { distrb(rng), distrb(rng), distrb(rng) };
		specularMap[p] = defaultSpecular;
		roughnessMap[p] = defaultRoughness;
		heightMap[p] = 0.0;
		sphereMap[p] = Eigen::Vector2d(0.0, 0.0);
		normalMap[p] = geoNormalMap[p];
	}

	specularMap[0] = defaultSpecular;
	roughnessMap[0] = defaultRoughness;

	iterCount = -1;
}


void AppearanceSolver::run()
{
	if (!std::filesystem::exists(pathInfo.outDir))
	{
		std::filesystem::create_directories(pathInfo.outDir);
	}

	if (solverState <= invalidInputData) {
		if (!loadInputData())
		{
			printf("The \'solverState\' is \'invalidInputData\', but failed to load input data!!\n");
			return;
		}
	}

	if (solverState <= invalidSolution) {
		resetSolution();
		solverState = invalidTBN;
	}

	if (solverState <= invalidHeight) {
		for (int p : domain) 
			heightMap[p] = 0.0;
	}

	if (solverState <= invalidTBN)
		computeTBNMatrix();

	if (solverState <= invalidProblem)
		createProblem();

	recordOpts.maxHeight = (problemOpts.normalMode == NormalOptMode::heightmap2018) ? 3.0 : 0.1;
	bUseShadow = problemOpts.bActiveShadow && shadowMaps.size() > 0;
	solverState = solvable;

	if (recordOpts.writeVisibility) {
		writeVisibilityImage();
		recordOpts.writeVisibility = false;
	}

	printConfigurations();

	lastTime = clock();
	CeresSolver::solve();
	--iterCount;

	FILE* fp = fopen((pathInfo.outDir + pathInfo.loggingfile).c_str(), "a");
	fprintf(fp, "-------All Iteration Complete!-------\n\n");
	fclose(fp);

	if (problemOpts.constantSpecular || problemOpts.constantRoughness)
	{
		for (int p : domain)
		{
			if (problemOpts.constantSpecular)
				specularMap[p] = specularMap[0];

			if (problemOpts.constantRoughness)
				roughnessMap[p] = roughnessMap[0];
		}
	}
}


void AppearanceSolver::computeTBNMatrix()
{
	if (problemOpts.normalMode == NormalOptMode::raw_normal)
		return;

	printf("TBN matrix computation...\n");
	
	int nInvalid = 0;
	
	for (int p : domain)
	{
		Eigen::Vector3d N = geoNormalMap[p];
		if (N.norm() < 0.8)
		{
			++nInvalid;
			continue;
		}

		if (problemOpts.normalMode == NormalOptMode::raw_normal2D)
		{
			Eigen::Vector3d T = positionMap[p + dx] - positionMap[p];
			Eigen::Vector3d B = N.cross(T).normalized();
			T = B.cross(N).normalized();
			tbnMap[p] << T, B, N;
		}
		else
		{
			Eigen::Vector3d T;
			Eigen::Vector3d B;
			if (problemOpts.diffMode == DifferenceMode::forward) {
				T = positionMap[p] - positionMap[p - dx];
				B = positionMap[p] - positionMap[p - dy];
			}
			else if (problemOpts.diffMode == DifferenceMode::forward2) {
				T = positionMap[p + dx] - positionMap[p];
				B = positionMap[p + dy] - positionMap[p];
			}
			else if (problemOpts.diffMode == DifferenceMode::central) {
				T = 0.5 * (positionMap[p + dx] - positionMap[p - dx]);
				B = 0.5 * (positionMap[p + dy] - positionMap[p - dy]);
			}

			if (problemOpts.normalMode == NormalOptMode::heightmap) {
				tbnMap[p] << T, B, N;
			}
			else if (problemOpts.normalMode == NormalOptMode::heightmap2018) {
				T = N.cross(T).cross(N).normalized();
				B = N.cross(B).cross(N).normalized();
				tbnMap[p] << T, B, N;
			}
			else if (problemOpts.normalMode == NormalOptMode::heightmap2020) {
				tbnMap[p] << T, B, (T.norm() * B.norm()) * N;
			}
		}
	}

	if (nInvalid > 0)
		printf("Invaild normals are found : [%d / %d]\n", nInvalid, domain.area());
}


void AppearanceSolver::constructNormal()
{
	if (problemOpts.normalMode == NormalOptMode::raw_normal)
		return;

	for (int p : domain)
	{
		Eigen::Matrix3d& tbnMap = this->tbnMap[p];

		if (problemOpts.normalMode == NormalOptMode::raw_normal2D) {
			double U = sphereMap[p][0];
			double V = sphereMap[p][1];

			normalMap[p] = tbnMap.col(0) * (sin(U) * cos(V)) +
						   tbnMap.col(1) * (sin(U) * sin(V)) +
						   tbnMap.col(2) * (cos(U));
		}
		else
		{
			double du, dv;
			if (problemOpts.diffMode == DifferenceMode::forward) {
				du = heightMap[p] - heightMap[p - dx];
				dv = heightMap[p] - heightMap[p - dy];
			}
			else if (problemOpts.diffMode == DifferenceMode::forward2) {
				du = heightMap[p + dx] - heightMap[p];
				dv = heightMap[p + dy] - heightMap[p];
			}
			else {
				du = 0.5 * (heightMap[p + dx] - heightMap[p - dx]);
				dv = 0.5 * (heightMap[p + dy] - heightMap[p - dy]);
			}
				
			if (problemOpts.normalMode == NormalOptMode::heightmap) {
				normalMap[p] = (tbnMap.col(0) + du * tbnMap.col(2)) .cross
							   (tbnMap.col(1) + dv * tbnMap.col(2));
			}
			else {
				normalMap[p] = (tbnMap * Eigen::Vector3d(-du, -dv, 1.0));
			}
		}

		normalMap[p].normalize();
	}
}


void AppearanceSolver::constructView()
{
	for (auto& view : views)
	{
		if (isEnabled(view.cameraId))
		{
			for (int p : domain)
			{
				if (isValidPixel(view.trgViewMap, p))
				{
					//views[v].viewMap[p] = evaluate(v, p, diffuseMap[p], specularMap[p], roughnessMap[p], normalMap[p]);
					view.errorMap[p] = (view.viewMap[p] - view.trgViewMap[p]).cwiseAbs();
				}
			}
		}
	}
}


void AppearanceSolver::writeVisibilityImage()
{
	uint64* totalVis = new uint64[width * height]();
	double(*visColor)[3] = new double[width * height][3]();
	double* visCount = new double[width * height]();
	double* visTotWeight = new double[width * height]();

	int numViews = std::accumulate(views.begin(), views.end(), 0,
		[this](int tot, const auto& view) { return tot + (isEnabled(view.cameraId) ? 1 : 0); });
	double inc = 1.0 / numViews;

	for (int p = 0; p < height * width; ++p)
	{
		for (int v = 0; v < views.size(); ++v)
		{
			if (isEnabled(views[v].cameraId) && 
				isValidPixel(views[v].trgViewMap, p) &&
				views[v].weightMap[p] > problemOpts.viewWeightMin)
			{
				visCount[p] += inc;
				totalVis[p] |= ((uint64)1u) << (v % 64);
				visTotWeight[p] += std::pow(views[v].weightMap[p], problemOpts.viewWeightBias);
			}
		}
	}

	double maxTotalWeight = std::accumulate(visTotWeight, visTotWeight + width * height, 0.0,
		[](double _max, double now) { return _MAX(_max, now); });

	for (int p = 0; p < height * width; ++p)
	{
		if (visTotWeight[p] == 0.0)
			visTotWeight[p] = 1.0;
		else
			visTotWeight[p] /= maxTotalWeight;
	}

	std::unordered_map<uint64, Eigen::Vector3d> color;
	for (int p = 0; p < height * width; ++p)
	{
		*(Eigen::Vector3d*)visColor[p] = color.try_emplace( totalVis[p], 
			0.5 * Eigen::Vector3d::Random() + Eigen::Vector3d::Constant(0.5) ) .first->second;
	}

	writeImage(pathInfo.outDir + "visibilityCategory.jpg", (double*)visColor, width, height, 3);
	writeImage(pathInfo.outDir + "visibilityCount.jpg", visCount, width, height, 1);
	writeImage(pathInfo.outDir + "visibilityTotalWeight.jpg", visTotWeight, width, height, 1);

	delete[] totalVis;
	delete[] visColor;
	delete[] visCount;
	delete[] visTotWeight;
}
#include "pch.h"
#include "AppearanceSolver.h"
#include "AccuracyCost.h"
#include "utils.h"


struct PDiffuse { using seq = std::index_sequence<1, 1, 1>; };
struct PSpecular { using seq = std::index_sequence<1>; };
struct PRoughness { using seq = std::index_sequence<1>; };
template<NormalOptMode norMode, DifferenceMode diffMode = DifferenceMode::forward2>
struct PNormal {
	constexpr static auto get() {
		if constexpr (norMode == NormalOptMode::raw_normal)
			return std::index_sequence <3>();
		else if constexpr (norMode == NormalOptMode::raw_normal2D)
			return std::index_sequence <1, 1>();
		else if constexpr (diffMode == DifferenceMode::central)
			return std::index_sequence <1, 1, 1, 1>();
		else
			return std::index_sequence <1, 1, 1>();
	};
	using seq = decltype(get());
};

template <size_t... ints1, size_t... ints2>
constexpr auto operator,(std::index_sequence<ints1...>, std::index_sequence<ints2...>)
{
	return std::index_sequence<ints1..., ints2...>();
}

template <typename... Params>
constexpr auto enumerate()
{
	return (..., Params::seq());
}

template <typename CostFtn, size_t... ints>
inline constexpr auto get_cost_function(CostFtn* ftn, std::index_sequence<ints...>)
{
	return new ceres::AutoDiffCostFunction<CostFtn, 3, ints...>(ftn);
}

template <typename CostFtn>
inline ceres::CostFunction* makeAutoDiffCostFunction(
	CostFtn* tcf, 
	ParamSpace params, 
	NormalOptMode norMode, 
	DifferenceMode diffMode)
{
	ceres::CostFunction* costFtn = nullptr;

	using m1 = NormalOptMode;
	using m2 = DifferenceMode;

	switch (params)
	{
	case param_diffuse:
		costFtn = get_cost_function(tcf, enumerate<PDiffuse>());
		break;
	case param_specular:
		costFtn = get_cost_function(tcf, enumerate<PSpecular>());
		break;
	case param_diffuse | param_specular:
		costFtn = get_cost_function(tcf, enumerate<PDiffuse, PSpecular>());
		break;
	case param_roughness:
		costFtn = get_cost_function(tcf, enumerate<PRoughness>());
		break;
	case param_diffuse | param_roughness:
		costFtn = get_cost_function(tcf, enumerate<PDiffuse, PRoughness>());
		break;
	case param_specular | param_roughness:
		costFtn = get_cost_function(tcf, enumerate<PSpecular, PRoughness>());
		break;
	case param_diffuse | param_specular | param_roughness:
		costFtn = get_cost_function(tcf, enumerate<PDiffuse, PSpecular, PRoughness>());
		break;

	case param_normal:
		if (norMode == m1::raw_normal)
			costFtn = get_cost_function(tcf, enumerate < PNormal<m1::raw_normal> >());
		else if (norMode == m1::raw_normal2D)
			costFtn = get_cost_function(tcf, enumerate < PNormal<m1::raw_normal2D> >());
		else if (diffMode == m2::central)
			costFtn = get_cost_function(tcf, enumerate < PNormal<m1::heightmap, m2::central> >());
		else
			costFtn = get_cost_function(tcf, enumerate < PNormal<m1::heightmap> >());
		break;

	case param_diffuse | param_normal:
		if (norMode == m1::raw_normal)
			costFtn = get_cost_function(tcf, enumerate < PDiffuse, PNormal<m1::raw_normal> >());
		else if (norMode == m1::raw_normal2D)
			costFtn = get_cost_function(tcf, enumerate < PDiffuse, PNormal<m1::raw_normal2D> >());
		else if (diffMode == m2::central)
			costFtn = get_cost_function(tcf, enumerate < PDiffuse, PNormal<m1::heightmap, m2::central> >());
		else
			costFtn = get_cost_function(tcf, enumerate < PDiffuse, PNormal<m1::heightmap> >());
		break;

	case param_specular | param_normal:
		if (norMode == m1::raw_normal)
			costFtn = get_cost_function(tcf, enumerate < PSpecular, PNormal<m1::raw_normal> >());
		else if (norMode == m1::raw_normal2D)
			costFtn = get_cost_function(tcf, enumerate < PSpecular, PNormal<m1::raw_normal2D> >());
		else if (diffMode == m2::central)
			costFtn = get_cost_function(tcf, enumerate < PSpecular, PNormal<m1::heightmap, m2::central> >());
		else
			costFtn = get_cost_function(tcf, enumerate < PSpecular, PNormal<m1::heightmap> >());
		break;

	case param_diffuse | param_specular | param_normal:
		if (norMode == m1::raw_normal)
			costFtn = get_cost_function(tcf, enumerate < PDiffuse, PSpecular, PNormal<m1::raw_normal> >());
		else if (norMode == m1::raw_normal2D)
			costFtn = get_cost_function(tcf, enumerate < PDiffuse, PSpecular, PNormal<m1::raw_normal2D> >());
		else if (diffMode == m2::central)
			costFtn = get_cost_function(tcf, enumerate < PDiffuse, PSpecular, PNormal<m1::heightmap, m2::central> >());
		else
			costFtn = get_cost_function(tcf, enumerate < PDiffuse, PSpecular, PNormal<m1::heightmap> >());
		break;

	case param_roughness | param_normal:
		if (norMode == m1::raw_normal)
			costFtn = get_cost_function(tcf, enumerate < PRoughness, PNormal<m1::raw_normal> >());
		else if (norMode == m1::raw_normal2D)
			costFtn = get_cost_function(tcf, enumerate < PRoughness, PNormal<m1::raw_normal2D> >());
		else if (diffMode == m2::central)
			costFtn = get_cost_function(tcf, enumerate < PRoughness, PNormal<m1::heightmap, m2::central> >());
		else
			costFtn = get_cost_function(tcf, enumerate < PRoughness, PNormal<m1::heightmap> >());
		break;

	case param_diffuse | param_roughness | param_normal:
		if (norMode == m1::raw_normal)
			costFtn = get_cost_function(tcf, enumerate < PDiffuse, PRoughness, PNormal<m1::raw_normal> >());
		else if (norMode == m1::raw_normal2D)
			costFtn = get_cost_function(tcf, enumerate < PDiffuse, PRoughness, PNormal<m1::raw_normal2D> >());
		else if (diffMode == m2::central)
			costFtn = get_cost_function(tcf, enumerate < PDiffuse, PRoughness, PNormal<m1::heightmap, m2::central> >());
		else
			costFtn = get_cost_function(tcf, enumerate < PDiffuse, PRoughness, PNormal<m1::heightmap> >());
		break;

	case param_specular | param_roughness | param_normal:
		if (norMode == m1::raw_normal)
			costFtn = get_cost_function(tcf, enumerate < PSpecular, PRoughness, PNormal<m1::raw_normal> >());
		else if (norMode == m1::raw_normal2D)
			costFtn = get_cost_function(tcf, enumerate < PSpecular, PRoughness, PNormal<m1::raw_normal2D> >());
		else if (diffMode == m2::central)
			costFtn = get_cost_function(tcf, enumerate < PSpecular, PRoughness, PNormal<m1::heightmap, m2::central> >());
		else
			costFtn = get_cost_function(tcf, enumerate < PSpecular, PRoughness, PNormal<m1::heightmap> >());
		break;

	case param_diffuse | param_specular | param_roughness | param_normal:
		if (norMode == m1::raw_normal)
			costFtn = get_cost_function(tcf, enumerate < PDiffuse, PSpecular, PRoughness, PNormal<m1::raw_normal> >());
		else if (norMode == m1::raw_normal2D)
			costFtn = get_cost_function(tcf, enumerate < PDiffuse, PSpecular, PRoughness, PNormal<m1::raw_normal2D> >());
		else if (diffMode == m2::central)
			costFtn = get_cost_function(tcf, enumerate < PDiffuse, PSpecular, PRoughness, PNormal<m1::heightmap, m2::central> >());
		else
			costFtn = get_cost_function(tcf, enumerate < PDiffuse, PSpecular, PRoughness, PNormal<m1::heightmap> >());
		break;
	}

	return costFtn;
}


void AppearanceSolver::createProblem()
{
	if (problem)
		delete problem;

	problem = new ceres::Problem;
	const auto& opt = problemOpts;
	
	int count = 0;

	for (int p : domain)
	{
		double* const x_diff = diffuseMap[p].data();
		double* const x_spec = !opt.constantSpecular ? &specularMap[p] : &specularMap[0];
		double* const x_r = !opt.constantRoughness ? &roughnessMap[p] : &roughnessMap[0];
		double* const x_h = &heightMap[p];
		double* const x_sh = sphereMap[p].data();
		double* const x_nor = normalMap[p].data();

		std::vector<double*> mutable_parameters;
		mutable_parameters.reserve(maxParams);

		if (opt.params & ParamSpace::param_diffuse)
		{
			mutable_parameters.push_back(x_diff + 0);
			mutable_parameters.push_back(x_diff + 1);
			mutable_parameters.push_back(x_diff + 2);
		}

		if (opt.params & ParamSpace::param_specular)
		{
			mutable_parameters.push_back(x_spec);
		}

		if (opt.params & ParamSpace::param_roughness)
		{
			mutable_parameters.push_back(x_r);
		}

		if (opt.params & ParamSpace::param_normal)
		{
			if (opt.normalMode == NormalOptMode::raw_normal)
			{
				mutable_parameters.push_back(x_nor);
			}
			else if (opt.normalMode == NormalOptMode::raw_normal2D)
			{
				mutable_parameters.push_back(x_sh + 0);
				mutable_parameters.push_back(x_sh + 1);
			}
			else
			{
				if (opt.diffMode == DifferenceMode::forward)
				{
					mutable_parameters.push_back(x_h);
					mutable_parameters.push_back(x_h - dx);
					mutable_parameters.push_back(x_h - dy);
				}
				else if (opt.diffMode == DifferenceMode::forward2)
				{
					mutable_parameters.push_back(x_h);
					mutable_parameters.push_back(x_h + dx);
					mutable_parameters.push_back(x_h + dy);
				}									 
				else								 
				{									 
					mutable_parameters.push_back(x_h - dx);
					mutable_parameters.push_back(x_h + dx);
					mutable_parameters.push_back(x_h - dy);
					mutable_parameters.push_back(x_h + dy);
				}
			}
		}

		double total_weight = 0.0;
		std::vector<double> frameWeights(views.size(), 0.0);
		
		for (int v = 0; v < views.size(); v++)
		{
			if (isEnabled(views[v].cameraId) && 
				isValidPixel(views[v].trgViewMap, p) &&
				views[v].weightMap[p] > opt.viewWeightMin)
			{
				frameWeights[v] = ceres::pow(views[v].weightMap[p], opt.viewWeightBias);
				total_weight += frameWeights[v];
			}
		}

		if (total_weight == 0.0)
			continue;

		for (int v = 0; v < views.size(); v++)
		{
			if (frameWeights[v] > 0.0)
			{
				ceres::CostFunction* cost_function = makeAutoDiffCostFunction(
					new AccuracyCost(*this, v, p, ceres::sqrt(frameWeights[v] / total_weight)),
					opt.params,
					opt.normalMode,
					opt.diffMode);

				problem->AddResidualBlock(cost_function, nullptr, mutable_parameters);
			}
		}

		for (auto& [param_type, weight_exp] : opt.smoothWeightAndExp)
		{
			auto& [param, smoothType] = param_type;
			auto& [weight, exp] = weight_exp;
			if(weight <= 0.0) continue;
			
			double* x_param = nullptr;
			int dx = this->dx;
			int dy = this->dy;
			switch (param) 
			{
			case Param::diffuseR:	
				if(!(opt.params & ParamSpace::param_diffuse)) continue;
				x_param = x_diff + 0; dx*=3; dy*=3;	
				break;
			case Param::diffuseG:
				if(!(opt.params & ParamSpace::param_diffuse)) continue;
				x_param = x_diff + 1; dx*=3; dy*=3;	
				break;
			case Param::diffuseB:	
				if(!(opt.params & ParamSpace::param_diffuse)) continue;
				x_param = x_diff + 2; dx*=3; dy*=3;	
				break;
			case Param::specular:	
				if(!(opt.params & ParamSpace::param_specular) || opt.constantSpecular) continue;
				x_param = x_spec;					
				break;
			case Param::roughness:	
				if(!(opt.params & ParamSpace::param_roughness) || opt.constantRoughness) continue;
				x_param = x_r;						
				break;
			case Param::height:		
				if(!(opt.params & ParamSpace::param_normal) || opt.normalMode==NormalOptMode::raw_normal || opt.normalMode==NormalOptMode::raw_normal2D) continue;
				x_param = x_h;						
				break;
			case Param::sphere:		
				if(!(opt.params & ParamSpace::param_normal) || opt.normalMode!=NormalOptMode::raw_normal2D) continue;
				x_param = x_sh;		  dx*=2; dy*=2;	
				break;
			}

			addSmoothCost(smoothType, problem, x_param, dx, dy, weight, exp, useBase(smoothType) ? opt.base[(int)param] : 0.0);
		}

		for (auto& [param, bound] : opt.bounds)
		{
			auto& [lb, ub] = bound;
			double* x_param = nullptr;
			switch (param) 
			{
			case Param::diffuseR:	
				if(!(opt.params & ParamSpace::param_diffuse)) continue;
				x_param = x_diff + 0; 	
				break;
			case Param::diffuseG:	
				if(!(opt.params & ParamSpace::param_diffuse)) continue;
				x_param = x_diff + 1; 	
				break;
			case Param::diffuseB:	
				if(!(opt.params & ParamSpace::param_diffuse)) continue;
				x_param = x_diff + 2; 	
				break;
			case Param::specular:	
				if(!(opt.params & ParamSpace::param_specular)) continue;
				x_param = x_spec;		
				break;
			case Param::roughness:	
				if(!(opt.params & ParamSpace::param_roughness)) continue;
				x_param = x_r;			
				break;
			case Param::height:		
				if(!(opt.params & ParamSpace::param_normal)|| opt.normalMode==NormalOptMode::raw_normal || opt.normalMode==NormalOptMode::raw_normal2D) continue;
				x_param = x_h;			
				break;
			case Param::sphere:		
				if(!(opt.params & ParamSpace::param_normal) || opt.normalMode!=NormalOptMode::raw_normal2D) continue;
				x_param = x_sh;			
				break;
			}

			if (lb != DBL_MAX) problem->SetParameterLowerBound(x_param, 0, lb);
			if (ub != DBL_MIN) problem->SetParameterUpperBound(x_param, 0, ub);
		}

		void printProgress(int, int);
		if(count++ % 100 == 0) 
			printProgress(count, domain.area());
	}

	printf("\n");
}


#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60
void printProgress(int count, int total)
{
	int val = count * 100 / total;
	int lpad = count * PBWIDTH / total;
	int rpad = PBWIDTH - lpad;
	printf("\r%3d%% [%.*s%*s] %d/%d", val, lpad, PBSTR, rpad, "", count, total);
	fflush(stdout);
}


#include "pch.h"
#include "SmoothTypes.h"


template<SmoothType type>
struct Desc {};


template<>
struct Desc<SmoothType::zero> {
	constexpr static uint maxResidual = 1;
	constexpr static uint numParams = 1;

	static std::vector<double*> makeParams(double* x_param, int dx, int dy)
	{
		return {x_param};
	}

	template <int k, typename T>
	static T residuals(T* x[])
	{
		return *x[0];
	}
};


template<>
struct Desc<SmoothType::one> {
	constexpr static uint maxResidual = 2;
	constexpr static uint numParams = 3;

	static std::vector<double*> makeParams(double* x_param, int dx, int dy)
	{
		return {x_param, x_param + dx, x_param + dy};
	}

	template <int k, typename T>
	static T residuals(T* x[])
	{
		if constexpr (k==0)
			return *x[1] - *x[0];
		else if(k==1)
			return *x[2] - *x[0];
	}
};


template<>
struct Desc<SmoothType::two> {
	constexpr static uint maxResidual = 2;
	constexpr static uint numParams = 5;

	static std::vector<double*> makeParams(double* x_param, int dx, int dy)
	{
		return {               x_param - dy,
			x_param - dx     , x_param     , x_param + dx     ,
							   x_param + dy                    };
	}

	template <int k, typename T>
	static T residuals(T* x[])
	{
		if constexpr (k==0)
			return *x[0] + *x[4] - 2.0 * *x[2];
		else if(k==1)
			return *x[1] + *x[3] - 2.0 * *x[2];
	}
};


template<>
struct Desc<SmoothType::three> {
	constexpr static uint maxResidual = 1;
	constexpr static uint numParams = 5;

	static std::vector<double*> makeParams(double* x_param, int dx, int dy)
	{
		return {               x_param - dy,
			x_param - dx     , x_param     , x_param + dx     ,
							   x_param + dy                    };
	}

	template <int k, typename T>
	static T residuals(T* x[])
	{
		return *x[0] + *x[1] + *x[3] + *x[4] - 4.0 * *x[2];
	}
};


template<>
struct Desc<SmoothType::four> {
	constexpr static uint maxResidual = 1;
	constexpr static uint numParams = 9;

	static std::vector<double*> makeParams(double* x_param, int dx, int dy)
	{
		return std::vector<double*>{
			x_param - dx - dy, x_param - dy, x_param + dx - dy,
			x_param - dx     , x_param     , x_param + dx     ,
			x_param - dx + dy, x_param + dy, x_param + dx + dy};
	}

	template <int k, typename T>
	static T residuals(T* x[])
	{
		return *x[0] + *x[1] + *x[2] + *x[3] + *x[5] + *x[6] + *x[7] + *x[8] - 8.0 * *x[4];
	}
};


template<>
struct Desc<SmoothType::five> {
	constexpr static uint maxResidual = 1;
	constexpr static uint numParams = 9;

	static std::vector<double*> makeParams(double* x_param, int dx, int dy)
	{
		return std::vector<double*>{
			x_param - dx - dy, x_param - dy, x_param + dx - dy,
			x_param - dx     , x_param     , x_param + dx     ,
			x_param - dx + dy, x_param + dy, x_param + dx + dy};
	}

	template <int k, typename T>
	static T residuals(T* x[])
	{
		return (1.0/9.0) * (*x[0] + *x[1] + *x[2] + *x[3] + *x[4] + *x[5] + *x[6] + *x[7] + *x[8]);
	}
};





template<typename T, typename... Ts>
struct _Head { using type = std::decay_t<T>; };


template <size_t... j>
constexpr auto make_ones(std::index_sequence<j...>)
{
	return std::index_sequence< (j, 1)... >();
}


template <typename D, size_t... k, size_t... j>
void _addSmoothCost(ceres::Problem* problem, double* x_param, int dx, int dy, double weight, double exp, double base,  
	std::index_sequence<k...>, std::index_sequence<j...>) 
{
	auto cost = [weight, exp, base] <typename... Params> (Params* ... params)
	{
		using T = typename _Head<Params...>::type;
		T* x[] = { ((T*)(params))... };
		T* residual = x[sizeof...(params) - 1];
		( (residual[k] = weight * ceres::pow(D::residuals<k>(x) - base, exp)), ... );
		return true;
	};
	using F = decltype(cost);

	problem->AddResidualBlock(
		new ceres::AutoDiffCostFunction<F, D::maxResidual, j...>(new F(cost)),
		nullptr, 
		D::makeParams(x_param, dx, dy) );
}


template <SmoothType type>
void _addSmoothCost(ceres::Problem* problem, double* x_param, int dx, int dy, double weight, double exp, double base=0.0)
{
	_addSmoothCost< Desc<type> >(
		problem, x_param, dx, dy, weight, exp, base,
		std::make_index_sequence< Desc<type>::maxResidual >{},
		make_ones(std::make_index_sequence< Desc<type>::numParams >{}) 
	);
}


#define KV(key) {key, _addSmoothCost<key>}
static std::unordered_map<SmoothType, void(*)(ceres::Problem*, double*, int, int, double, double, double)> 
addSmoothCostFamily = {
	KV(SmoothType::zero),
	KV(SmoothType::one),
	KV(SmoothType::two),
	KV(SmoothType::three),
	KV(SmoothType::four),
	KV(SmoothType::five),
};


void addSmoothCost(SmoothType type, ceres::Problem* problem, double* x_param, int dx, int dy, double weight, double exp, double base)
{
	addSmoothCostFamily[type](problem, x_param, dx, dy, weight, exp, base);
}
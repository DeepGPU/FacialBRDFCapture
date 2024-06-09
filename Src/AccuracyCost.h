#pragma once
#include "pch.h"
#include "AppearanceSolver.h"
#include "BRDFs.h"


template<typename T, typename... Ts>
struct _Head { using type = std::decay_t<T>; };


template<typename... Params> inline
bool AppearanceSolver::AccuracyCost::operator()(Params* ... params) const
{
	using T = typename _Head<Params...>::type;
	using Vec3 = Eigen::Vector<T, 3>;
	using Vec2 = Eigen::Vector<T, 2>;
	using ceres::cos;
	using ceres::sin;

	T* x[] = { ((T*)(params))... };
	int id = 0;
	const auto& opt = solver.problemOpts;
	auto& view = solver.views[v];

	Vec3 diffuse;
	if (opt.params & ParamSpace::param_diffuse)
	{
		diffuse = Vec3(*x[id + 0], *x[id + 1], *x[id + 2]);
		id += 3;
	}
	else
	{
		diffuse = solver.diffuseMap[p].cast<T>();
	}

	T specular = (opt.params & ParamSpace::param_specular) ?
		*x[id++] : T(solver.specularMap[p]);

	T roughness = (opt.params & ParamSpace::param_roughness) ?
		*x[id++] : T(solver.roughnessMap[p]);

	Vec3 N;
	if (!(opt.params & ParamSpace::param_normal))
	{
		N = solver.normalMap[p].cast<T>();
	}
	else
	{
		Eigen::Matrix<T, 3, 3> tbnMat = solver.tbnMap[p].cast<T>();

		if (opt.normalMode == NormalOptMode::raw_normal)
		{
			N = Eigen::Map<const Vec3>(x[id++]);
		}

		else if (opt.normalMode == NormalOptMode::raw_normal2D)
		{
			T U = *x[id++];
			T V = *x[id++];

			N = tbnMat.col(0) * (sin(U) * cos(V)) +
				tbnMat.col(1) * (sin(U) * sin(V)) +
				tbnMat.col(2) * (cos(U));
		}
		
		else 
		{
			T du, dv;
			if (opt.diffMode == DifferenceMode::forward) {
				du = *x[id] - *x[id + 1];
				dv = *x[id] - *x[id + 2];
				id += 3;
			}
			else if (opt.diffMode == DifferenceMode::forward2) {
				du = *x[id + 1] - *x[id];
				dv = *x[id + 2] - *x[id];
				id += 3;
			}
			else {
				du = 0.5 * (*x[id + 1] - *x[id + 0]);
				dv = 0.5 * (*x[id + 3] - *x[id + 2]);
				id += 4;
			}
			
			if (opt.normalMode == NormalOptMode::heightmap) {
				N = (tbnMat.col(0) + du * tbnMat.col(2)) .cross
					(tbnMat.col(1) + dv * tbnMat.col(2));
			}
			else {
				N = (tbnMat * Vec3(-du, -dv, T(1.0)));
			}
			
			N.normalize();
		}
	}

	Vec3 radiance = solver.evaluate(v, p, diffuse, specular, roughness, N);
	
	if constexpr (!std::is_fundamental_v<T>)
	{
		if(radiance.isZero())
			view.viewMap[p] = Eigen::Vector3d(0.0, 10000.0, 0.0);
		else
			view.viewMap[p] = Eigen::Vector3d(radiance[0].a, radiance[1].a, radiance[2].a);
	}

	x[id][0] = opt.channelWeight[0] * weight * (view.trgViewMap[p][0] - radiance[0]);
	x[id][1] = opt.channelWeight[1] * weight * (view.trgViewMap[p][1] - radiance[1]);
	x[id][2] = opt.channelWeight[2] * weight * (view.trgViewMap[p][2] - radiance[2]);

	return true;
}


template<typename T> inline
Eigen::Vector<T, 3> AppearanceSolver::evaluate(
	int viewIdx,
	int pixelIdx,
	const Eigen::Vector<T, 3>& diffuse,
	const T& specular,
	const T& roughness,
	const Eigen::Vector<T, 3>& N) const
{
	using T3 = Eigen::Vector<T, 3>;
	Eigen::Vector3d P = positionMap[pixelIdx];
	Eigen::Vector3d V = (views[viewIdx].cameraPos - P).normalized();

	T3 radiance = { T(0.0), T(0.0), T(0.0) };

	for (int i = 0; i < lightSamples.size(); ++i)
	{
		const LightSample& sample = lightSamples[i];
		int arrIdx = i / shadowPackSize;
		int bitIdx = i % shadowPackSize;
		bool visible = bUseShadow ? (shadowMaps[arrIdx][pixelIdx] & (1u << bitIdx)) : true;
		if (!visible)
			continue;

		Eigen::Vector3d L = sample.position - P;
		double inv_r = 1.0 / L.norm();
		L *= inv_r;

		T cos_i = L.dot(N);
		if (cos_i <= 0)
			continue;

		double cos_j = -(L.dot(sample.normal));
		if (cos_j <= 0)
			continue;
		
		T3 brdf = (1.0 / PI) * diffuse + T3::Constant(specular * brdf_ggx(N, L, V, roughness));

		radiance += sample.emittance.cwiseProduct(brdf) * (cos_i * (cos_j * inv_r * inv_r * sample.area));
	}

	return radiance;
}

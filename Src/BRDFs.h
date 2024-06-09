#pragma once
#include "pch.h"


static const double skin_ind = 1.5;
static const double skin_ref = ((1 - skin_ind) * (1 - skin_ind)) / ((1 + skin_ind) * (1 + skin_ind));


template<typename T, typename U>
inline T brdf_ggx(
	Eigen::Vector<T, 3> N,
	Eigen::Vector<U, 3> L,
	Eigen::Vector<U, 3> V,
	T roughness)
{
	Eigen::Vector<U, 3> H = (L + V).normalized();
	U F = skin_ref + (1.0 - skin_ref) * pow(1.0 - H.dot(V), 5.0);

	T r2 = roughness * roughness;
	T cos2 = H.dot(N); cos2 *= cos2;
	T D = r2 + (1.0 - cos2) / cos2; D = r2 / (PI * cos2 * cos2 * D * D);

	T NV = N.dot(V);
	T NVNV = NV * NV;
	T lambda1 = 0.5 * (-1.0 + sqrt(1.0 + r2 * (1.0 - NVNV) / NVNV));

	T NL = N.dot(L);
	T NLNL = NL * NL;
	T lambda2 = 0.5 * (-1.0 + sqrt(1.0 + r2 * (1.0 - NLNL) / NLNL));

	T G = 1.0 / (1.0 + lambda1 + lambda2);

	return (D * G * F) / (4.0 * NV * NL);
}
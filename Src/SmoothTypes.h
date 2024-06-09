#pragma once
#include "pch.h"


enum class SmoothType {
	MIN,
	zero = MIN,
	one,
	two,
	three,
	four,
	five,

	MAX
};


inline std::string toString(SmoothType type)
{
	static std::unordered_map<SmoothType, std::string> typeNames = {
		{SmoothType::zero,  "zero"},
		{SmoothType::one,   "one"},
		{SmoothType::two,   "two"},
		{SmoothType::three, "three"},
		{SmoothType::four,  "four"},
		{SmoothType::five,  "five"},
	};
	return typeNames[type];
}


inline bool useBase(SmoothType type)
{
	return type == SmoothType::zero || type == SmoothType::five;
}

void addSmoothCost(SmoothType, ceres::Problem*, double*, int, int, double, double, double);
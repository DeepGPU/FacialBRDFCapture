#pragma once
#include "pch.h"


template<typename SrcType, uint srcComp, uint trgComp = srcComp, typename TrgType>
void readBinaryImage(std::string filename, uint width, uint height, std::vector<TrgType>& out);


void writeImage(
	const std::string& filename, 
	const double* src, 
	int width, 
	int height, 
	int channels, 
	double min = 0.0,
	double max = 1.0,
	bool srgb = false);
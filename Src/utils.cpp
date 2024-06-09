#include "pch.h"
#include "utils.h"
#include <fstream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"



template<typename SrcType, uint srcComp, uint trgComp, typename TrgType>
void readBinaryImage(std::string filename, uint width, uint height, std::vector<TrgType>& out)
{
	static_assert(srcComp >= trgComp);
	uint64 srcDataSize = height * width * srcComp * sizeof(SrcType);

	SrcType* data = new SrcType[height * width * srcComp];
	{
		FILE* fp = fopen(filename.c_str(), "rb");
		if (!fp)
			throw std::runtime_error("File open error");

		
		fseek(fp, 0, SEEK_END);
		if (ftell(fp) != srcDataSize)
			throw std::runtime_error("File data match error");

		fseek(fp, 0, SEEK_SET);
		fread(data, 1, srcDataSize, fp);
		fclose(fp);
	}

	out.clear();
	out.shrink_to_fit();

	if constexpr (std::is_fundamental_v<TrgType>)
	{
		out.resize(height * width * trgComp);
	}
	else
	{
		static_assert(sizeof(TrgType) == sizeof(TrgType::value_type) * trgComp);
		out.resize(height * width);
	}

	auto foo = [](auto* x) {
		if constexpr (std::is_fundamental_v<TrgType>)
			return (TrgType*)x;
		else
			return (typename TrgType::value_type*)x;
	};

	SrcType* src = data;
	auto* trg = foo(out.data());
	using EleType = std::decay_t<decltype(*trg)>;

	if constexpr (std::is_same_v<SrcType, EleType> && srcComp == trgComp)
	{
		memcpy(trg, src, srcDataSize);
	}
	else
	{
		for (uint i = 0; i < height * width; ++i, src += srcComp, trg += trgComp)
		{
			for (uint j = 0; j < trgComp; ++j)
				trg[j] = (EleType)src[j];
		}
	}

	delete[] data;
}
template void readBinaryImage<float, 4, 3>(std::string filename, uint width, uint height, std::vector<Eigen::Vector3d>& out);
template void readBinaryImage<float, 4, 3>(std::string filename, uint width, uint height, std::vector<double>& out);
template void readBinaryImage<float, 1>(std::string filename, uint width, uint height, std::vector<double>& out);
template void readBinaryImage<uint, 1>(std::string filename, uint width, uint height, std::vector<uint>& out);


void writeImage(
	const std::string& filename, 
	const double* src, 
	int width, 
	int height, 
	int channels,
	double min,
	double max,
	bool srgb)
{
	constexpr double gamma = 1.0 / 2.2;
	constexpr double nearlyOne = 1.0 - 1e-8;
	auto encode = [&](const double& x) {
		double _x = std::clamp((x - min) / (max - min), 0.0, nearlyOne);
		return srgb ? std::pow(_x, gamma) : _x;
	};
	
	uint8_t* pixels = new uint8_t[width * height * channels];

	for (int i = 0; i < height; ++i)
	{
		for (int j = 0; j < width; ++j)
		{
			int idx = (i * width + j) * channels;
			if (channels == 3)
			{
				pixels[idx + 0] = (uint8_t) (256.0 * encode(src[idx + 0]));
				pixels[idx + 1] = (uint8_t) (256.0 * encode(src[idx + 1]));
				pixels[idx + 2] = (uint8_t) (256.0 * encode(src[idx + 2]));
			}
			else
			{
				pixels[idx + 0] = (uint8_t)(256.0 * encode(src[idx + 0]));
			}
		}
	}

	stbi_write_jpg(filename.c_str(), width, height, channels, pixels, 100);

	delete[] pixels;
}

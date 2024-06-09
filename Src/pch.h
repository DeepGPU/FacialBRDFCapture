#pragma once


#define GLOG_NO_ABBREVIATED_SEVERITIES
#pragma warning(disable:4996)
#pragma warning(disable:4819)
#pragma warning(disable:26451)
#pragma warning(disable:26812)
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS

//This is added at 2024/06/09//////////////////////
#define GLOG_USE_GLOG_EXPORT
#include <Windows.h>
///////////////////////////////////////////////////
#include <ceres/ceres.h>
#include <glog/logging.h>
#include <Eigen/Dense>

#include <array>
#include <vector>
#include <set>
#include <map>
#include <unordered_map>

#include <string>
#include <format>
#include <type_traits>
#include <functional>
#include <fstream>
#include <filesystem>
#include <time.h>
#include "debug.h"
#include "json.hpp"
namespace fs = std::filesystem;
using json = nlohmann::json;


#define yaml_str(x) x.as<std::string>()
#define yaml_int(x) x.as<int>()
#define yaml_double(x) x.as<double>()


#define saturate(x) __min(1.0f, __max(0.0f, x))
#define saturateT(x) __min(T(1.0f), __max(T(0.00001f), x))

#define PI 3.14159265358979323846


typedef wchar_t wchar;
typedef unsigned char uint8;
typedef unsigned int uint;
typedef unsigned long long uint64;


// num of light samples
static const int kSamples = 2048;


#pragma once
#define NS_PATH "C:/Users/nicol/source/repos/noisyEngine/noisyEngine/Source/"
#define CONFIG_FILE "config.yaml"
#define NS_MATERIAL_FILE_EXTENSION ".nsmat"

#ifndef NDEBUG
#define USE_IMGUI
#else
#define USE_IMGUI
#endif // !NDEBUG

#ifdef USE_IMGUI
#define NS_TEXTURE_VIEW_STORE_POINTER
#endif

//when false allow to save almost 200 bytes on each geometric object3d but this remove access to the translation, scaling and rotation matrix
#define NS_GEOMETRIC_OBJECT3D_STORE_ALL_MATRICES false


//macros to make sintax faster and more readable
#define dout ns::Debug::get()
#define newl '\n'

#include <glm/glm.hpp>

namespace ns{
	using LengthType = float;
	using HeightType = float;
	using PartitionType = size_t;
	using ChunkPartitionType = glm::vec<2, PartitionType>;
	using MapLengthType = glm::vec<2, LengthType>;
	using GridPositionType = glm::ivec2;

	constexpr ChunkPartitionType defaultSize = glm::vec<2, PartitionType>(32);
	constexpr unsigned int maximunRenderDistance = 100;

	//simple cast to LengthType
	constexpr LengthType operator""_lt(long double number) {
		return static_cast<LengthType>(number);
	}

	//simple cast to HeightType
	constexpr HeightType operator""_ht(long double number) {
		return static_cast<HeightType>(number);
	}
}

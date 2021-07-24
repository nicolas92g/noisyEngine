#pragma once
#define NS_PATH "C:/Users/nicol/source/repos/noisyEngine/noisyEngine/Source/"
#define CONFIG_FILE "config.yaml"

#ifndef NDEBUG
#define USE_IMGUI
#endif // !NDEBUG

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

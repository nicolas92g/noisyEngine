#pragma once
#include <configNoisy.hpp>
#include <array>

namespace ns {
	class HeightMapGenerator
	{
	public:
		//HeightMapGenerator();

		
	protected:


	};

	HeightType simplexNoise(MapLengthType location);
	int fastfloor(LengthType value);
}

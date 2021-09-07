#pragma once
#include <configNoisy.hpp>
#include <array>
#include <vector>

namespace ns::Plane {
	class HeightMapGenerator
	{
	public:
		struct Octave {
			LengthType frequency;
			LengthType amplitude;
			LengthType offset;
			bool ridged;
		};

		struct Settings {
			std::vector<Octave> octaves;
			double exponent;
		};

		HeightMapGenerator(const Settings& settings);
		const Settings& settings() const;
		HeightType operator()(const MapLengthType& pos) const;

#		ifndef NDEBUG
		size_t callCounter() const;
#		endif // !NDEBUG

	protected:
		const Settings settings_;
		HeightType inversedAmplitudeRect_;

#		ifndef NDEBUG
		mutable size_t numberOfComputation_;  
#		endif // !NDEBUG
	};

	HeightType simplexNoise(const MapLengthType& location);
	HeightType ridgedSimplexNoise(const MapLengthType& location);
	int fastfloor(LengthType value);
}

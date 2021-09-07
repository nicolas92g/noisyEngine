#include "HeightMapGenerator.h"
#include "gcem.hpp"

ns::Plane::HeightMapGenerator::HeightMapGenerator(const Settings& settings)
	:
	settings_(settings)
{
#		ifndef NDEBUG
		numberOfComputation_ = 0;
#		endif // !NDEBUG

		inversedAmplitudeRect_ = 0;
		for (const auto& octave : settings_.octaves) {
			inversedAmplitudeRect_ += octave.amplitude;
		}

		inversedAmplitudeRect_ = 1 / inversedAmplitudeRect_;
}

const ns::Plane::HeightMapGenerator::Settings& ns::Plane::HeightMapGenerator::settings() const
{
	return settings_;
}

ns::HeightType ns::Plane::HeightMapGenerator::operator()(const ns::MapLengthType& pos) const
{
#		ifndef NDEBUG
		numberOfComputation_++;
#		endif // !NDEBUG

	HeightType ret = 0;

	for (const auto& octave : settings_.octaves) {
		if(octave.ridged)
			ret += ridgedSimplexNoise(pos * octave.frequency + octave.offset) * octave.amplitude;
		else
			ret += simplexNoise(pos * octave.frequency + octave.offset) * octave.amplitude;
	}

	ret *= inversedAmplitudeRect_;

	return pow(ret, settings_.exponent);
}

#ifndef NDEBUG

size_t ns::Plane::HeightMapGenerator::callCounter() const
{
	return numberOfComputation_;
}

#endif // !NDEBUG


//SIMPLEX NOISE
// 
//global constants

constexpr short p[256] = { 151,160,137,91,90,15,
	131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
	190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
	88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
	77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
	102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
	135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
	5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
	223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
	129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
	251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
	49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
	138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180 };

constexpr std::array<int, 512> fillPerm()
{
    std::array<int, 512> ret;
    for (size_t i = 0; i < ret.size(); i++)
    {
        ret[i] = p[i & 255];
    }
    return ret;
}
const std::array<int, 512> perm = fillPerm();

int grad3[12][3] = 
	{ {1,1,0},{-1,1,0},{1,-1,0},{-1,-1,0}, {1,0,1},{-1,0,1},{1,0,-1},{-1,0,-1}, {0,1,1},{0,-1,1},{0,1,-1},{0,-1,-1} };

//functions

int ns::Plane::fastfloor(LengthType x)
{
	return (x > 0) ? static_cast<int>(x) : static_cast<int>(x - 1);
}

ns::LengthType dot(int g[], ns::LengthType x, ns::LengthType y) {
	return g[0] * x + g[1] * y;
}

ns::HeightType ns::Plane::simplexNoise(const MapLengthType& in)
{
	LengthType n0, n1, n2;

	static constexpr LengthType F2 = .5 * (gcem::sqrt(3.0) - 1.0);
	const LengthType s = (in.x + in.y) * F2;
	const int i = fastfloor(in.x + s);
	const int j = fastfloor(in.y + s);

	static constexpr LengthType G2 = (3._lt - gcem::sqrt(3._lt)) / 6._lt;
	const LengthType t = (i + j) * G2;
	const LengthType X0 = i - t;
	const LengthType Y0 = j - t;
	const LengthType x0 = in.x - X0;
	const LengthType y0 = in.y - Y0;

	int i1, j1;
	if (x0 > y0) {
		i1 = 1;
		j1 = 0;
	}
	else {
		i1 = 0;
		j1 = 1;
	}

	const LengthType x1 = x0 - i1 + G2;
	const LengthType y1 = y0 - j1 + G2;
	const LengthType x2 = x0 - 1._lt + 2._lt * G2;
	const LengthType y2 = y0 - 1._lt + 2._lt * G2;

	const int ii = i & 255;
	const int jj = j & 255;
	const int gi0 = perm[static_cast<size_t>(ii) + perm[jj]] % 12;
	const int gi1 = perm[static_cast<size_t>(ii) + i1 + perm[static_cast<size_t>(jj) + j1]] % 12;
	const int gi2 = perm[static_cast<size_t>(ii) + 1 + perm[static_cast<size_t>(jj) + 1]] % 12;

	LengthType t0 = 0.5_lt - x0 * x0 - y0 * y0;
	if (t0 < 0.0_lt)
		n0 = 0.0_lt;
	else {
		t0 *= t0;
		n0 = t0 * t0 * dot(grad3[gi0], x0, y0);
	}

	LengthType t1 = 0.5_lt - x1 * x1 - y1 * y1;
	if (t1 < 0.0_lt)
		n1 = 0.0_lt;
	else {
		t1 *= t1;
		n1 = t1 * t1 * dot(grad3[gi1], x1, y1);
	}

	LengthType t2 = 0.5_lt - x2 * x2 - y2 * y2;
	if (t2 < 0) 
		n2 = 0.0;
	else {
		t2 *= t2;
		n2 = t2 * t2 * dot(grad3[gi2], x2, y2);
	}

	return 70._ht * (static_cast<ns::HeightType>(n0) + n1 + n2);
}

ns::HeightType ns::Plane::ridgedSimplexNoise(const MapLengthType& location)
{
	return 2.0_ht * (abs(0.5_ht - simplexNoise(location)));
}

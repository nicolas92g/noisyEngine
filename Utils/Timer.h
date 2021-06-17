#pragma once
#include <chrono>
#include <string>
#include <iostream>
#include <array>

namespace ns {
	class Timer
	{
	public:
		Timer(const std::string& name);
		~Timer();
		void stop();
		void abort();

	protected:
		std::chrono::time_point<std::chrono::steady_clock> start;
		std::string name;
		bool hasBeenStoped;
	};

#define PERFORMANCE_MAX_OF_FRAME 1000
	class performanceBench {
	public:
		performanceBench();
		~performanceBench();
		void recordFrame();
		uint16_t getFps() const;

	protected:
		std::array<int, PERFORMANCE_MAX_OF_FRAME> framerates;
		std::array<int, PERFORMANCE_MAX_OF_FRAME> frameratesMeans;
		int cursor;
		int cursorMeans;

		int fps;
		std::chrono::steady_clock::time_point time;
	};
}

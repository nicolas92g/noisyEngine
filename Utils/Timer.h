#pragma once
#include <chrono>
#include <string>
#include <iostream>
#include <array>

#define TIMER(call) \
					Timer t;\
					call;\
					t.stop();\
						

namespace ns {
	/**
	 * @brief allow to record the timing of a scope easily 
	 */
	class Timer
	{
	public:
		/**
		 * @brief start the timer
		 * \param name
		 */
		Timer(const std::string& name = "timer");
		/**
		 * @brief end the timer and log the time
		 */
		~Timer();
		/**
		 * @brief cancel the effect of the destructor and log the time at the moment when it is called
		 */
		void stop();
		/**
		 * @brief abort the timer
		 */
		void abort();
	protected:
		std::chrono::time_point<std::chrono::steady_clock> start;
		std::string name;
		bool hasBeenStoped;
	};

#define PERFORMANCE_MAX_OF_FRAME 1000
	/**
	 * @brief allow to calculate the mean framerate of an appplication
	 * (old stuff, may not work so don't trust this)
	 */
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

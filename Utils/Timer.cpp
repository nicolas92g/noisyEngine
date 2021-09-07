#include "Timer.h"

#include <Utils/DebugLayer.h>

using namespace std::chrono;
using namespace std::chrono_literals;

ns::Timer::Timer(const std::string& name)
{
	start = high_resolution_clock::now();
	this->name = name;
	hasBeenStoped = false;
}

ns::Timer::~Timer()
{
	stop();
}

void ns::Timer::stop()
{

	auto stop = high_resolution_clock::now();

	auto duration = duration_cast<nanoseconds>(stop - start);

	if (hasBeenStoped) return;
	
	dout << name << " took " << std::to_string((double)duration.count() * 1e-6) << " ms\n";
	hasBeenStoped = true;
}

void ns::Timer::abort()
{
	hasBeenStoped = true;
}

ns::performanceBench::performanceBench() : framerates({0}), frameratesMeans({-1})
{
	cursor = 0;
	cursorMeans = 0;
	fps = -1;
	time = high_resolution_clock::now();

	for (auto& means : frameratesMeans) means = -1;
	for (auto& fps : framerates) fps = 0;

}

ns::performanceBench::~performanceBench()
{
	size_t sum(0);
	size_t number(0);

	for (const auto& means : frameratesMeans) {
		if (means == -1) break;
		sum += means;
		number++;
	}

	if (!number) return;

	Debug::get() << "average framerates is : " << sum / number << " fps\n";
}

void ns::performanceBench::recordFrame()
{
	auto delta = duration_cast<nanoseconds>(high_resolution_clock::now() - time);
	time = high_resolution_clock::now();
	framerates[cursor] = (uint16_t)(1.0 / ((double)delta.count() * 1e-9));
	cursor++;

	if (cursor > PERFORMANCE_MAX_OF_FRAME - 1) {
		cursor = 0;
		size_t sum(0);

		for (const auto& fps : framerates)
			sum += fps;
		
		frameratesMeans[cursorMeans] = sum / PERFORMANCE_MAX_OF_FRAME;
		cursorMeans++;

		if (cursorMeans > PERFORMANCE_MAX_OF_FRAME - 1) {
			cursorMeans = 0;
		}
	}
}

uint16_t ns::performanceBench::getFps() const
{
	return framerates[cursor];
}

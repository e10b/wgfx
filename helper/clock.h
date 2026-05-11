#pragma once

#include <chrono>

class Clock {
public:
	Clock();
	float restart();
	float getElapsedTime();

private:
	std::chrono::high_resolution_clock::time_point startTime_;
};

#include "clock.h"

Clock::Clock() {
    restart();
}

float Clock::restart() {
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = now - startTime_;
    startTime_ = now;
    return elapsed.count();
}

float Clock::getElapsedTime() {
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = now - startTime_;
    return elapsed.count();
}
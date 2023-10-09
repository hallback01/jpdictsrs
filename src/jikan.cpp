#include <chrono>
#include "jikan.h"

int64_t Jikan::epoch_time_seconds() {
    int64_t sec = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    return sec;
}
#pragma once

#include "image.hpp"

// Result of a multithreaded inversion run.
struct InvertStats {
    unsigned int threadsUsed = 0;  // number of worker threads actually spawned
    double elapsedMs = 0.0;        // wall-clock time spent inverting (ms)
};

// Inverts every colour channel of the image in place (value -> 255 - value),
// leaving any alpha channel untouched.
//
// The pixel range is split into `threadCount` contiguous chunks, each handed to
// its own std::thread. After all workers finish they are joined back into the
// calling (main) thread before the function returns.
//
// threadCount == 0 means "use std::thread::hardware_concurrency()".
InvertStats invertImageMultithreaded(Image& image, unsigned int threadCount = 0);

#include "inverter.hpp"

#include "logger.hpp"

#include <chrono>
#include <string>
#include <thread>
#include <vector>

namespace {

// Inverts the colour channels of pixels in the half-open range [startPixel, endPixel).
// Runs on its own worker thread.
void invertRange(unsigned char* data, std::size_t startPixel, std::size_t endPixel,
                 int channels, unsigned int threadIndex) {
    Logger::instance().debug(
        "worker #" + std::to_string(threadIndex) + " inverting pixels [" +
        std::to_string(startPixel) + ", " + std::to_string(endPixel) + ")  (" +
        std::to_string(endPixel - startPixel) + " px)");

    // Channel layout: grayscale(1), grayscale+alpha(2), RGB(3), RGBA(4).
    // Alpha is the last channel of the 2- and 4-channel variants; keep it as is.
    const bool hasAlpha = (channels == 2 || channels == 4);
    const int colorChannels = hasAlpha ? channels - 1 : channels;

    for (std::size_t p = startPixel; p < endPixel; ++p) {
        unsigned char* px = data + p * static_cast<std::size_t>(channels);
        for (int c = 0; c < colorChannels; ++c) {
            px[c] = static_cast<unsigned char>(255 - px[c]);
        }
    }

    Logger::instance().debug("worker #" + std::to_string(threadIndex) + " done");
}

}  // namespace

InvertStats invertImageMultithreaded(Image& image, unsigned int threadCount) {
    InvertStats stats;

    const std::size_t pixels = image.pixelCount();
    if (image.data() == nullptr || pixels == 0) {
        Logger::instance().warn("invert: image is empty, nothing to do");
        return stats;
    }

    if (threadCount == 0) {
        threadCount = std::thread::hardware_concurrency();
        if (threadCount == 0) {
            threadCount = 1;  // hardware_concurrency() may report 0
        }
    }
    // Never spawn more threads than there are pixels to process.
    if (static_cast<std::size_t>(threadCount) > pixels) {
        threadCount = static_cast<unsigned int>(pixels);
    }

    Logger::instance().info(
        "Inverting " + std::to_string(image.width()) + "x" +
        std::to_string(image.height()) + " image (" + std::to_string(image.channels()) +
        " channels, " + std::to_string(pixels) + " px) using " +
        std::to_string(threadCount) + " thread(s)");

    const auto t0 = std::chrono::steady_clock::now();

    std::vector<std::thread> workers;
    workers.reserve(threadCount);

    // Distribute pixels as evenly as possible: the first `remainder` threads
    // each take one extra pixel.
    const std::size_t base = pixels / threadCount;
    const std::size_t remainder = pixels % threadCount;
    std::size_t cursor = 0;
    for (unsigned int i = 0; i < threadCount; ++i) {
        const std::size_t count = base + (i < remainder ? 1 : 0);
        const std::size_t start = cursor;
        const std::size_t end = cursor + count;
        cursor = end;
        workers.emplace_back(invertRange, image.data(), start, end, image.channels(), i);
    }

    Logger::instance().info(
        "Spawned " + std::to_string(workers.size()) +
        " worker thread(s); joining them back into the main thread...");

    // Merge every worker back into this (main) thread.
    for (auto& worker : workers) {
        worker.join();
    }

    const auto t1 = std::chrono::steady_clock::now();
    stats.threadsUsed = threadCount;
    stats.elapsedMs = std::chrono::duration<double, std::milli>(t1 - t0).count();

    Logger::instance().info(
        "All workers joined. Inversion finished in " +
        std::to_string(stats.elapsedMs) + " ms");

    return stats;
}

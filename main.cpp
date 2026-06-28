#include "image.hpp"
#include "inverter.hpp"
#include "logger.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

void printUsage(const char* prog) {
    std::cout
        << "inverted_photo - multithreaded image inverter\n\n"
        << "Usage:\n"
        << "  " << prog << " <input> <output> [options]\n\n"
        << "Options:\n"
        << "  --threads N    number of worker threads (default: all CPU cores)\n"
        << "  --log FILE     write the log to FILE (default: invert.log)\n"
        << "  --verbose, -v  verbose (DEBUG) logging, incl. per-thread ranges\n"
        << "  --quiet, -q    only warnings and errors\n"
        << "  --help, -h     show this help and exit\n\n"
        << "Input formats:  PNG, JPG, BMP, TGA, PSD, GIF, HDR, PIC, PNM\n"
        << "Output formats: PNG, JPG, BMP, TGA (selected by file extension)\n";
}

}  // namespace

int main(int argc, char** argv) {
    std::string logPath = "invert.log";
    unsigned int threads = 0;  // 0 => all cores
    LogLevel level = LogLevel::INFO;
    std::vector<std::string> positional;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "--verbose" || arg == "-v") {
            level = LogLevel::DEBUG;
        } else if (arg == "--quiet" || arg == "-q") {
            level = LogLevel::WARN;
        } else if (arg == "--threads") {
            if (i + 1 >= argc) {
                std::cerr << "error: --threads requires a value\n";
                return 2;
            }
            threads = static_cast<unsigned int>(std::strtoul(argv[++i], nullptr, 10));
        } else if (arg == "--log") {
            if (i + 1 >= argc) {
                std::cerr << "error: --log requires a value\n";
                return 2;
            }
            logPath = argv[++i];
        } else if (!arg.empty() && arg[0] == '-') {
            std::cerr << "error: unknown option '" << arg << "'\n";
            return 2;
        } else {
            positional.push_back(arg);
        }
    }

    if (positional.size() < 2) {
        printUsage(argv[0]);
        return positional.empty() ? 0 : 2;
    }
    const std::string input = positional[0];
    const std::string output = positional[1];

    // Make sure the parent directories for the log and the output file exist.
    std::error_code ec;
    if (const auto p = fs::path(logPath).parent_path(); !p.empty()) {
        fs::create_directories(p, ec);
    }
    if (const auto p = fs::path(output).parent_path(); !p.empty()) {
        fs::create_directories(p, ec);
    }

    Logger& log = Logger::instance();
    log.init(logPath, level);
    log.info("=== inverted_photo started ===");
    log.info("input='" + input + "' output='" + output + "' log='" + logPath + "'");

    Image image;
    log.info("Loading image: " + input);
    if (!image.load(input)) {
        log.error("Aborting: could not load input image");
        return 1;
    }
    log.info("Loaded " + std::to_string(image.width()) + "x" +
             std::to_string(image.height()) + " image, channels=" +
             std::to_string(image.channels()));

    const InvertStats stats = invertImageMultithreaded(image, threads);

    log.info("Saving inverted image: " + output);
    if (!image.save(output)) {
        log.error("Aborting: could not save output image");
        return 1;
    }

    log.info("Done. threads=" + std::to_string(stats.threadsUsed) +
             " time=" + std::to_string(stats.elapsedMs) + "ms output='" + output + "'");
    log.info("=== inverted_photo finished ===");
    return 0;
}

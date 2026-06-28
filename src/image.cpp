#include "image.hpp"

#include "logger.hpp"
#include "stb_image.h"
#include "stb_image_write.h"

#include <algorithm>
#include <cctype>

namespace {

std::string lowerExtension(const std::string& path) {
    const auto dot = path.find_last_of('.');
    if (dot == std::string::npos) {
        return "";
    }
    std::string ext = path.substr(dot + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return ext;
}

}  // namespace

Image::~Image() {
    reset();
}

void Image::reset() {
    if (data_) {
        stbi_image_free(data_);
        data_ = nullptr;
    }
    width_ = height_ = channels_ = 0;
}

Image::Image(Image&& other) noexcept
    : data_(other.data_),
      width_(other.width_),
      height_(other.height_),
      channels_(other.channels_) {
    other.data_ = nullptr;
    other.width_ = other.height_ = other.channels_ = 0;
}

Image& Image::operator=(Image&& other) noexcept {
    if (this != &other) {
        reset();
        data_ = other.data_;
        width_ = other.width_;
        height_ = other.height_;
        channels_ = other.channels_;
        other.data_ = nullptr;
        other.width_ = other.height_ = other.channels_ = 0;
    }
    return *this;
}

bool Image::load(const std::string& path) {
    reset();
    data_ = stbi_load(path.c_str(), &width_, &height_, &channels_, 0);
    if (!data_) {
        const char* reason = stbi_failure_reason();
        Logger::instance().error("Failed to load image '" + path +
                                 "': " + (reason ? reason : "unknown error"));
        return false;
    }
    return true;
}

bool Image::save(const std::string& path) const {
    if (!data_) {
        Logger::instance().error("save: no image data to write");
        return false;
    }

    const std::string ext = lowerExtension(path);
    const int stride = width_ * channels_;
    int ok = 0;

    if (ext == "png") {
        ok = stbi_write_png(path.c_str(), width_, height_, channels_, data_, stride);
    } else if (ext == "jpg" || ext == "jpeg") {
        ok = stbi_write_jpg(path.c_str(), width_, height_, channels_, data_, 95);
    } else if (ext == "bmp") {
        ok = stbi_write_bmp(path.c_str(), width_, height_, channels_, data_);
    } else if (ext == "tga") {
        ok = stbi_write_tga(path.c_str(), width_, height_, channels_, data_);
    } else {
        Logger::instance().error("Unsupported output extension '" + ext +
                                 "'. Use one of: png, jpg, bmp, tga.");
        return false;
    }

    if (!ok) {
        Logger::instance().error("Failed to write image '" + path + "'");
        return false;
    }
    return true;
}

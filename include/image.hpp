#pragma once

#include <cstddef>
#include <string>

// Owns an 8-bit-per-channel image buffer loaded/saved through stb_image.
// Pixels are stored row-major, interleaved channels: data[(y*width + x)*ch + c].
class Image {
public:
    Image() = default;
    ~Image();

    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;
    Image(Image&& other) noexcept;
    Image& operator=(Image&& other) noexcept;

    // Loads any format stb_image supports (PNG/JPG/BMP/TGA/PSD/GIF/HDR/PIC/PNM).
    bool load(const std::string& path);

    // Writes PNG/JPG/BMP/TGA, chosen by the output file extension.
    bool save(const std::string& path) const;

    unsigned char* data() { return data_; }
    const unsigned char* data() const { return data_; }

    int width() const { return width_; }
    int height() const { return height_; }
    int channels() const { return channels_; }

    std::size_t pixelCount() const {
        return static_cast<std::size_t>(width_) * static_cast<std::size_t>(height_);
    }
    std::size_t sizeBytes() const { return pixelCount() * static_cast<std::size_t>(channels_); }

private:
    void reset();

    unsigned char* data_ = nullptr;
    int width_ = 0;
    int height_ = 0;
    int channels_ = 0;
};

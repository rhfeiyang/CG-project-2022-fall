#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <stb_image_write.h>

#include "image.h"

ImageRGB::ImageRGB(int width, int height)
        : resolution(width, height) {
    data.resize(width * height * 3);
}

float ImageRGB::getAspectRatio() const {
    return static_cast<float>(resolution.x()) / static_cast<float>(resolution.y());
}

Vec2i ImageRGB::getResolution() const {
    return resolution;
}

void ImageRGB::setPixel(int x, int y, const Vec3f &value) {
    data[3 * (x + resolution.x() * y)] = utils::gammaCorrection(value.x());
    data[3 * (x + resolution.x() * y) + 1] = utils::gammaCorrection(value.y());
    data[3 * (x + resolution.x() * y) + 2] = utils::gammaCorrection(value.z());
}

void ImageRGB::writeImgToFile(const std::string &file_name) {
    stbi_flip_vertically_on_write(true);
    stbi_write_png(file_name.c_str(), resolution.x(), resolution.y(), 3, data.data(), 0);
}


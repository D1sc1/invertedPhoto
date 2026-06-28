// Single translation unit that compiles the stb single-header implementations.
// Kept separate so the (third-party) implementation can be built with warnings
// disabled while the rest of the project is built with -Wall -Wextra / -W4.
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

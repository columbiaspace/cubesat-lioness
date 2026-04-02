#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int main(int argc, char *argv[]) {
    std::string inputFile = "image.png";

    if (argc >= 2) {
        inputFile = argv[1];
    }


}

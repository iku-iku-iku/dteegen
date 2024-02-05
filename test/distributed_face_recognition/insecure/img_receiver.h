#pragma once
#include "../secure/embedding.h"
#include <array>
int img_recorder(std::array<char, IMG_SIZE> arr, int id);

int img_verifier(std::array<char, IMG_SIZE> arr);

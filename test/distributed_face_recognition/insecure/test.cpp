
#define CATCH_CONFIG_MAIN
#include "../secure/embedding.h"
#include "TEE-Capability/distributed_tee.h"
#include "catch.hpp"
#include "retinanet.h"
#include "stb_image.h"

unsigned char *detect_face_and_load(const char *img_path)
{
    detect_face(img_path);  // assume cropped face will be saved to image.png
    int width, height, channels;
    unsigned char *image_data =
        stbi_load("image.png", &width, &height, &channels, 0);

    if (image_data == NULL) {
        printf("Fail to load image.png\n");
        return NULL;
    }

    return image_data;
}

TEST_CASE("Face Recognition", "Integration test")
{
    auto ctx = init_distributed_tee_context({.side = SIDE::Client,
                                             .mode = MODE::Transparent,
                                             .name = "face_recognition",
                                             .version = "1.0"});

    auto record = [&](const std::string &img_path, int person_id) {
        printf("Recording: %s with person ID: %d\n", img_path.c_str(),
               person_id);
        auto image_data = detect_face_and_load(img_path.c_str());
        // std::array<char, IMG_SIZE> arr;
        // memcpy(arr.data(), image_data, IMG_SIZE);
        stbi_image_free(image_data);

        // int sealed_data_len =
        //     call_remote_secure_function(ctx, img_recorder, arr, person_id);
        int sealed_data_len = img_recorder((char *)image_data, person_id);
        REQUIRE(sealed_data_len > 0);
    };

    auto verify = [&](const std::string &img_path, int expected_id) {
        printf("Verifying: %s with expected_id: %d\n", img_path.c_str(),
               expected_id);
        auto image_data = detect_face_and_load(img_path.c_str());
        // std::array<char, IMG_SIZE> arr;
        // memcpy(arr.data(), image_data, IMG_SIZE);
        stbi_image_free(image_data);

        // int person_id = call_remote_secure_function(ctx, img_verifier, arr);
        int person_id = img_verifier((char *)image_data);
        REQUIRE(person_id == expected_id);
    };

    // verify("trump1.jpg", -1);
    // verify("trump2.jpg", -1);
    // verify("biden1.jpg", -1);
    // verify("biden2.jpg", -1);

    record("trump1.jpg", 1);

    verify("trump1.jpg", 1);
    verify("trump2.jpg", 1);
    verify("biden1.jpg", -1);
    verify("biden2.jpg", -1);

    record("biden1.jpg", 2);

    verify("trump1.jpg", 1);
    verify("trump2.jpg", 1);
    verify("biden1.jpg", 2);
    verify("biden2.jpg", 2);

    destroy_distributed_tee_context(ctx);
}

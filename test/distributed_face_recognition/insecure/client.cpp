#include "file.h"
#include "CLI11.hpp"
#include "TEE-Capability/distributed_tee.h"
#include "../secure/embedding.h"
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

int main(int argc, char **argv)
{
    (void)read_file;
    (void)write_file;
    (void)get_emb_list;
    CLI::App app{"face recognition client cli"};
    app.require_subcommand(1);

    auto record = app.add_subcommand("record", "Record a new person entry");
    std::string img_path;
    int person_id;
    record->add_option("img_path", img_path, "Path to the image file")
        ->required();
    record->add_option("person_id", person_id, "Person ID")->required();

    auto verify = app.add_subcommand("verify", "Verify a person");
    std::string img_to_verify_path;
    verify
        ->add_option("img_path", img_to_verify_path,
                     "Path to the image file to verify")
        ->required();

    CLI11_PARSE(app, argc, argv);

    auto ctx = init_distributed_tee_context({.side = SIDE::Client,
                                             .mode = MODE::Transparent,
                                             .name = "face_recognition",
                                             .version = "1.0"});
    if (*record) {
        printf("Recording: %s with person ID: %d", img_path.c_str(), person_id);
        auto image_data = detect_face_and_load(img_path.c_str());
        int res = img_recorder((char*)image_data, person_id);
        printf("Record successfully. Embedding length: %d\n", res);
    }
    else if (*verify) {
        printf("Verifying: %s", img_to_verify_path.c_str());
        auto image_data = detect_face_and_load(img_to_verify_path.c_str());
        int res = img_verifier((char*)image_data);
        if (res == -1) {
            printf("Not valid person\n");
        }
        else {
            printf("Valid person. Person ID: %d\n", res);
        }
    }

    destroy_distributed_tee_context(ctx);
}

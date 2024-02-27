#include <sstream>
#include <string>

#include "../secure/llm.h"
#include "file_stub.h"
#define USED(x) ((void)(x))

std::string construct_prompt(std::string user_prompt)
{
    std::stringstream ss;
    ss << R"(<|im_start|>system:
This is a normal conversation between user and assistant.<|im_end|>
<|im_start|>user:
)";
    ss << user_prompt;
    ss << R"(<|im_end|>
<|im_start|>assistant:
)";

    return ss.str();
}

#define DEFAULT_PROMPT "Who are you?"

int main(int argc, char **argv)
{
    USED(read_file);
    std::string prompt;
    if (argc == 1) {
        prompt = construct_prompt(DEFAULT_PROMPT);
    }
    else if (argc == 2) {
        prompt = construct_prompt(argv[1]);
    }
    else {
        printf("Usage: %s [prompt] or %s\n", argv[0], argv[0]);
    }

    /* char user_input[] = "<user_prompt>who are you<model_prompt>\n"; */
    llm_inference((char *)prompt.c_str(), prompt.size());
}

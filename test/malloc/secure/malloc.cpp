#include "malloc.h"

#include <stdlib.h>

#include <sstream>
#include <unordered_map>

#include "TEE-Capability/common.h"

struct Object
{
    Object() { eapp_print("Object::Object()\n"); }
};

Object obj;

char zero_buf[4096 * 10];

extern char __bss_start;
extern char __bss_end;

void zero_bss()
{
    for (char* p = &__bss_start; p < &__bss_end; p++) {
        *p = 0;
    }
}
std::unordered_map<int, const char*> g_map = {
    {0, "HELLO"}, {1, "OK"},   {2, "WORLD"},
    {3, "PPP"},   {4, "QQQ"},  {5, "RRR"},
    {6, "SSS"},   {7, "TTT"},  {8, "UUU"},
    {9, "VVV"},   {10, "WWW"}, {11, "XXX"},
    {12, "YYY"},  {13, "ZZZ"}, {14, "AAA"},
    {15, "BBB"},  {16, "CCC"}, {17, "DDD"},
    {18, "EEE"},  {19, "FFF"}, {20, "GGG"},
    {21, "HHH"},  {22, "III"}, {23, "JJJ"},
    {24, "KK"},   {25, "PPP"}, {26, "DDD"},
    {27, "AAA"},  {28, "BBB"}, {29, "CCC"},
    {30, "DDD"},  {31, "EEE"}, {32, "FFF"},
    {33, "GGG"},  {34, "HHH"}, {35, "III"},
    {36, "JJJ"},  {37, "LLL"}, {38, "QQQ"},
    {39, "RRR"}

};

int test_malloc()
{
    // zero_bss();
    eapp_print("TEST: %s\n", "malloc");
    std::stringstream ss;
    ss << "OK" << std::endl;
    eapp_print("%s\n", ss.str().c_str());
    eapp_print("END\n");
    for (const auto& kv : g_map) {
        if (kv.second == 0) {
            eapp_print("kv.second == 0\n");
            return 0;
        }
        eapp_print("kv.second = %s\n", kv.second);
    }

    void* ptr;
    posix_memalign(&ptr, 16, 100);
    eapp_print("ptr = %p\n", ptr);
    for (int i = 0; i < sizeof(zero_buf); i++) {
        if (zero_buf[i] != 0) {
            eapp_print("zero_buf[%d] != 0\n", i);
            return 0;
        }
    }
    for (int i = 1024; i < 100 * 1024 * 1024; i += 1024) {
        std::stringstream ss;
        char* p = (char*)malloc(i);
        ss << "malloc " << i << " bytes, bytes[0] = [0x" << std::hex
           << (int)p[0] << "]\n";
        free(p);
        eapp_print("%s\n", ss.str().c_str());
    }
    return 0;
}

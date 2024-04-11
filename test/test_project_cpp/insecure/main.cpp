extern "C"
{
#include <miracl/miracl.h>  // 确保包含正确的头文件路径
#include <miracl/mirdef.h>  // 根据你的MIRACL版本和配置调整
}
#include <cstring>
#include <iostream>

#include "../secure/add.h"
#include "num.h"

#define SM2_WORDSIZE 8
#define SM2_NUMBITS 256
#define SM2_NUMWORD (SM2_NUMBITS / SM2_WORDSIZE)
void str_to_point(std::string str, epoint *a)
{
    char mem[MR_BIG_RESERVE(2)] = {0};
    big x, y;
    x = mirvar_mem(mem, 0);
    y = mirvar_mem(mem, 1);
    bytes_to_big(SM2_NUMWORD, str.c_str(), x);
    bytes_to_big(SM2_NUMWORD, str.c_str() + SM2_NUMWORD, y);

    epoint_set(x, y, 0, a);
}

std::string point_to_str(epoint *a)
{
    char mem[MR_BIG_RESERVE(2)] = {0};
    big x, y;
    x = mirvar_mem(mem, 0);
    y = mirvar_mem(mem, 1);

    epoint_get(a, x, y);

    char buffer[2 * SM2_NUMWORD];
    big_to_bytes(SM2_NUMWORD, x, buffer, 1);

    big_to_bytes(SM2_NUMWORD, y, buffer + SM2_NUMWORD, 1);

    return std::string(buffer, buffer + 2 * SM2_NUMWORD);
}

const char SM2_p[32] = {0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff,
                        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                        0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
                        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

const char SM2_a[32] = {0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff,
                        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                        0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
                        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc};

const char SM2_b[32] = {0x28, 0xe9, 0xfa, 0x9e, 0x9d, 0x9f, 0x5e, 0x34,
                        0x4d, 0x5a, 0x9e, 0x4b, 0xcf, 0x65, 0x09, 0xa7,
                        0xf3, 0x97, 0x89, 0xf5, 0x15, 0xab, 0x8f, 0x92,
                        0xdd, 0xbc, 0xbd, 0x41, 0x4d, 0x94, 0x0e, 0x93};

const char SM2_Gx[32] = {0x32, 0xc4, 0xae, 0x2c, 0x1f, 0x19, 0x81, 0x19,
                         0x5f, 0x99, 0x04, 0x46, 0x6a, 0x39, 0xc9, 0x94,
                         0x8f, 0xe3, 0x0b, 0xbf, 0xf2, 0x66, 0x0b, 0xe1,
                         0x71, 0x5a, 0x45, 0x89, 0x33, 0x4c, 0x74, 0xc7};

const char SM2_Gy[32] = {0xbc, 0x37, 0x36, 0xa2, 0xf4, 0xf6, 0x77, 0x9c,
                         0x59, 0xbd, 0xce, 0xe3, 0x6b, 0x69, 0x21, 0x53,
                         0xd0, 0xa9, 0x87, 0x7c, 0xc6, 0x2a, 0x47, 0x40,
                         0x02, 0xdf, 0x32, 0xe5, 0x21, 0x39, 0xf0, 0xa0};

const char SM2_n[32] = {0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff,
                        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                        0x72, 0x03, 0xdf, 0x6b, 0x21, 0xc6, 0x05, 0x2b,
                        0x53, 0xbb, 0xf4, 0x09, 0x39, 0xd5, 0x41, 0x23};

std::pair<std::string, std::string> get_key_pair()
{
    miracl *mip = mirsys(128, 16);
    irand(time(0));

    char mem[MR_BIG_RESERVE(7)] = {0};
    char mem_point[MR_ECP_RESERVE(2)] = {0};

    epoint *g, *pub;
    big p, a, b, n, gx, gy, pri;
    p = mirvar_mem(mem, 0);
    a = mirvar_mem(mem, 1);
    b = mirvar_mem(mem, 2);
    n = mirvar_mem(mem, 3);
    gx = mirvar_mem(mem, 4);
    gy = mirvar_mem(mem, 5);
    pri = mirvar_mem(mem, 6);

    bytes_to_big(SM2_NUMWORD, SM2_a, a);
    bytes_to_big(SM2_NUMWORD, SM2_b, b);
    bytes_to_big(SM2_NUMWORD, SM2_p, p);
    bytes_to_big(SM2_NUMWORD, SM2_Gx, gx);
    bytes_to_big(SM2_NUMWORD, SM2_Gy, gy);
    bytes_to_big(SM2_NUMWORD, SM2_n, n);

    g = epoint_init_mem(mem_point, 0);

    pub = epoint_init_mem(mem_point, 1);
    ecurve_init(a, b, p, MR_PROJECTIVE);

    if (!epoint_set(gx, gy, 0, g)) {
        {};
    }

    auto make_prikey = [n](big x) {
        bigrand(n, x);  // generate a big random number 0<=x<n
    };

    make_prikey(pri);
    ecurve_mult(pri, g, pub);

    auto get_key = [](big b) {
        char buffer[SM2_NUMWORD];
        big_to_bytes(SM2_NUMWORD, b, buffer, 1);
        return std::string(buffer, buffer + SM2_NUMWORD);
    };

    auto res = std::make_pair(get_key(pri), point_to_str(pub));
    mirexit();
    printf("OK\n");
    return res;
}

std::string make_shared_key(std::string pri_key, std::string in_pub_key)
{
    for (int i = 0; i < pri_key.size(); i++) printf("%02x ", pri_key[i]);
    printf("\n");
    for (int i = 0; i < in_pub_key.size(); i++) printf("%02x ", in_pub_key[i]);
    printf("\n");
    miracl *mip = mirsys(128, 16);
    char mem[MR_BIG_RESERVE(1)] = {0};
    char mem_point[MR_ECP_RESERVE(2)] = {0};

    big pri, a, b, p;
    pri = mirvar_mem(mem, 0);
    a = mirvar_mem(mem, 1);
    b = mirvar_mem(mem, 2);
    p = mirvar_mem(mem, 3);
    bytes_to_big(SM2_NUMWORD, SM2_a, a);
    bytes_to_big(SM2_NUMWORD, SM2_b, b);
    bytes_to_big(SM2_NUMWORD, SM2_p, p);
    ecurve_init(a, b, p, MR_PROJECTIVE);

    epoint *pub = epoint_init_mem(mem_point, 0);
    str_to_point(std::move(in_pub_key), pub);

    epoint *shared = epoint_init_mem(mem_point, 1);
    bytes_to_big(SM2_NUMWORD, pri_key.c_str(), pri);
    ecurve_mult(pri, pub, shared);

    auto res = point_to_str(shared);
    printf("OK\n");
    mirexit();
    return res;
}
int main(int argc, char *argv[])
{
    (void)get_num;

    std::cout << add(1, 2) << std::endl;
    std::cout << addf(1.1, 2.2) << std::endl;

    std::cout << pi() << std::endl;
    std::cout << get_e() << std::endl;
    return 0;
}

#include "add.h"

#include <map>
#include <string>
#include <vector>

#include "../insecure/num.h"
#include "TEE-Capability/common.h"

namespace {
class Object
{
   public:
    Object(const char *obj_str) : obj_str_(obj_str)
    {
        eapp_print("OBJECT CREATED: %s\n", obj_str_.c_str());
    }
    std::size_t len() { return obj_str_.size(); }

   private:
    std::string obj_str_;
};
static Object s("global static object ctor");

static std::map<const char *, Object> vec = {{"A", "static obj in vec ctor 1"},
                                             {"B", "static obj in vec ctor 2"}};

int add_internal(int x, int y)
{
    eapp_print("s.len: %d\n", s.len());

    return (x + y) % s.len();
}
}  // namespace
extern void (*__init_array_start[])(void);
extern void (*__init_array_end[])(void);
void manual_init_array(void)
{
    for (void (**p)() = __init_array_start; p < __init_array_end; ++p) {
        eapp_print("CALL INIT ARRAY: %p\n", *p);
        (*p)();
    }
    eapp_print("CALL INIT ARRAY DONE\n");
}

int add(int x, int y) { return add_internal(x, y); }

float addf(float x, float y)
{
#ifdef __TEE
    manual_init_array();
#endif
    static Object local_static_obj("local static object ctor");
    return (x + y) * get_num();
}

void print_num(int num) { eapp_print("%d", num); }
bool check_nan(float f)
{
    unsigned int *bits = (unsigned int *)&f;

    // NaN的特点是指数域全为1，尾数域不全为0
    return (((*bits) & 0x7F800000) == 0x7F800000) &&
           (((*bits) & 0x007FFFFF) != 0);
}
void print_float(float num)
{
    // check num is nan
    if (check_nan(num)) {
        eapp_print("nan");
        return;
    }
    print_num((int)(num * 10000));
}
float pi()
{
    float res = 0;
    int a = 1;
    for (int i = 0; i < 100000000; i++) {
        res += 4.0f / (float)a;
        a += 2;
        res -= 4.0f / (float)a;
        a += 2;
    }
    return res;
}
float get_e()
{
    float res = 0;
    float a = 1;
    for (int i = 1; i < 100000000; i++) {
        res += 1 / a;
        a *= i;
    }
    return res;
}

// NOTE: this file is just used to generated libjustworkaound, don't use it
// directly!
// gcc -c justworkaround.cc -o justworkaround.o
// ar rcs libjustworkaround.a justworkaround.o
// mv libjustworkaround.a secure_include_lib/riscv64
#ifdef __cplusplus
extern "C" {
#endif

typedef long unsigned int size_t;

struct dl_phdr_info {};
struct locale_t {};

__attribute__((weak)) void *__dso_handle = 0;

__attribute__((weak)) int dl_iterate_phdr(
    int (*callback)(struct dl_phdr_info *info, size_t size, void *data),
    void *data) {
  return 0;
}

__attribute__((weak)) void __fprintf_chk(...) {}
__attribute__((weak)) void __sprintf_chk(...) {}

__attribute__((weak)) void __calloc() {}

__attribute__((weak)) unsigned long long strtoull_l(const char *__restrict s,
                                                    char **__restrict ptr,
                                                    int base,
                                                    struct locale_t loc) {
  return 0;
}

__attribute__((weak)) void *strtoll_l(void) { return 0; }

__attribute__((weak)) void *__libc_fatal(void) { return 0; }

__attribute__((weak)) void *__libc_longjmp(void) { return 0; }

__attribute__((weak)) void *calloc(long unsigned int x, long unsigned int y) {
  return 0;
}

#ifdef __cplusplus
}
#endif

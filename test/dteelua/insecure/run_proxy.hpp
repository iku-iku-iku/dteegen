#include "../secure/lua_rt.h"
#include <iostream>
#include <string>
extern "C" {
#include "lauxlib.h"
#include "lstate.h"
#include "lua.h"
#include "lualib.h"
}

template <typename T> void push(lua_State *L, T arg);

template <> void push<int>(lua_State *L, int arg) { lua_pushinteger(L, arg); }

template <> void push<double>(lua_State *L, double arg) {
  lua_pushnumber(L, arg);
}

template <> void push<std::string>(lua_State *L, std::string arg) {
  lua_pushstring(L, arg.c_str());
}

template <typename... Args> void push_args(lua_State *L, Args... args) {
  (push(L, args), ...);
}

template <typename RetType, typename... Args>
RetType exec_in_lua(std::string script, Args... args) {
  lua_State *L = luaL_newstate();
  luaL_openlibs(L);

  auto oldp = reinterpret_cast<const char *>(L->top.p);
  push_args(L, args...);
  auto newp = reinterpret_cast<const char *>(L->top.p);
  std::cout << "DIFF" << (char *)newp - (char *)oldp << std::endl;

  const char *stacked_params = oldp;
  int stacked_params_len = newp - oldp;
  int arg_num = sizeof...(args);

  RetType res;
  if constexpr (std::is_same_v<RetType, float>) {
    res = exec_script_return_float((char *)script.c_str(), script.size(),
                                   (char *)stacked_params, stacked_params_len,
                                   arg_num);
  } else if constexpr (std::is_same_v<RetType, int>) {
    res = exec_script_return_int((char *)script.c_str(), script.size(),
                                 (char *)stacked_params, stacked_params_len,
                                 arg_num);
  }
  lua_close(L);
  return res;
}

inline int add(int a, int b) {
  std::string script = R"(
        function f(x, y)
            return x + y
        end
    )";

  int result = exec_in_lua<int>(script, a, b);
  return result;
}

inline int mul(int a, int b) {
  std::string script = R"(
        function f(x, y)
            return x * y
        end
    )";

  int result = exec_in_lua<int>(script, a, b);
  return result;
}

inline int run_proxy(std::string script) { return exec_in_lua<int>(script); }

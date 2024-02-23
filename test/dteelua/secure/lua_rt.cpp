
#include "lua_rt.h"
#include <cstring>
#include <iostream>
#include <string>
extern "C" {
#include "lauxlib.h"
#include "lstate.h"
#include "lua.h"
#include "lualib.h"
}

// 助手函数：从 Lua 获取返回值
template <typename RetType> RetType get(lua_State *L);

template <> int get<int>(lua_State *L) { return lua_tointeger(L, -1); }

template <> float get<float>(lua_State *L) { return lua_tonumber(L, -1); }

template <> double get<double>(lua_State *L) { return lua_tonumber(L, -1); }

template <> std::string get<std::string>(lua_State *L) {
  return lua_tostring(L, -1);
}

template <typename RetType>
RetType exec_script_return(const char *script, int script_len,
                           const char *stacked_params, int stacked_params_len,
                           int arg_num) {

  lua_State *L = luaL_newstate();
  luaL_openlibs(L);
  if (luaL_dostring(L, script) != LUA_OK) {
    lua_close(L);
    return RetType();
  }
  lua_getglobal(L, "f");
  memcpy(L->top.p, stacked_params, stacked_params_len);
  L->top.p += arg_num;

  if (lua_pcall(L, arg_num, 1, 0) != LUA_OK) {
    std::cerr << "Lua error: " << lua_tostring(L, -1) << std::endl;
    lua_close(L);
    return 0;
  }

  RetType result = get<RetType>(L);
  return result;
}

float exec_script_return_float(char *script, int script_len,
                               char *stacked_params, int stacked_params_len,
                               int arg_num) {
  return exec_script_return<float>(script, script_len, stacked_params,
                                   stacked_params_len, arg_num);
}

int exec_script_return_int(char *script, int script_len, char *stacked_params,
                           int stacked_params_len, int arg_num) {
  return exec_script_return<int>(script, script_len, stacked_params,
                                 stacked_params_len, arg_num);
}

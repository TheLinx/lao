#include <stdio.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "ao/ao.h"

struct luaobject {
    enum {
        UNKNOWN = 0,
        DEVICE,
        INFO,
        OPTION,
        SAMPLE_FORMAT,
    } type;
    union {
      void *pointer;
    } data;
};

static int l_test(lua_State* L)
{
  puts(luaL_checkstring(L, 1));
  return 0;
}

static const luaL_Reg lao [] = {
  {"test", l_test},
  {NULL, NULL}
};

int luaopen_lao(lua_State* L)
{
  luaL_register(L, "lao", lao);
  return 1;
}
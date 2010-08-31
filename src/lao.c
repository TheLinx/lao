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

static int l_initialize(lua_State* L)
{
  ao_initialize();
  return 0;
}
static int l_shutdown(lua_State* L)
{
  ao_shutdown();
  return 0;
}

static int l_default_driver_id(lua_State* L)
{
  int default_driver;
  default_driver = ao_default_driver_id();
  lua_pushinteger(L, default_driver);
  return 1;
}

static int l_new_device(lua_State* L)
{
  size_t nbytes;
  luaobject *obj;
  ao_device device;

  nbytes = sizeof(luaobject);
  obj = (luaobject *)lua_newuserdata(L, nbytes);

  obj->pointer = *device;
  obj->type = DEVICE;

  return 1;
}

static const luaL_Reg ao [] = {
  {"test", l_test},
  {"initialize", l_initialize},
  {"shutdown", l_shutdown},
  {"defaultDriverId", l_default_driver_id},
  {"newDevice" , l_new_device},
  {NULL, NULL}
};

int luaopen_ao(lua_State* L)
{
  luaL_register(L, "ao", ao);
  return 1;
}
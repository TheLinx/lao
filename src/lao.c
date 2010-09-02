#include <stdio.h>
#include <string.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "ao/ao.h"

typedef struct {
    enum {
        UNKNOWN = 0,
        O_LIVE,
        O_FILE
    } type;
    union {
      void *pointer;
    } data;
} device; //use a typedef so we don't need the struct keyword

static int l___gc(lua_State *L)
{
  device *dev = (device*) lua_touserdata(L, 1);
  free(dev->data.pointer);
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
  int default_driver = ao_default_driver_id();
  lua_pushinteger(L, default_driver);
  return 1;
}

static int l_open_live(lua_State* L)
{
  size_t nbytes;
  device *dev;
  ao_device *devi = (ao_device*) malloc(sizeof(ao_device)); //we need to free this on gc

  nbytes = sizeof(device);
  dev = (device *)lua_newuserdata(L, nbytes);
  luaL_getmetatable(L, "ao.device");
  lua_setmetatable(L, -2);

  memset(dev, 0, nbytes); //clear it before using

  dev->data.pointer = (void*) devi;
  dev->type = O_LIVE;

  return 1;
}

static const luaL_Reg ao [] = {
  {"initialize", l_initialize},
  {"shutdown", l_shutdown},
  {"defaultDriverId", l_default_driver_id},
  {"openLive", l_open_live},
  {NULL, NULL}
};

int luaopen_ao(lua_State* L)
{
  luaL_newmetatable(L, "ao.device");
  lua_pushstring(L, "__gc");
  lua_pushcfunction(L, l___gc);
  lua_settable(L, -3);
  luaL_register(L, "ao", ao);
  return 1;
}

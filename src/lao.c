#include <stdio.h>
#include <string.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "ao/ao.h"

typedef struct luaobject {
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
} luaobject; //use a typedef so we don't need the struct keyword

static int l___gc(lua_State *L)
{
  luaobject *o = (luaobject*) lua_touserdata(L, 1);
  switch(o->type)
  {
  case DEVICE:
    //close the device
    ao_close((ao_device *)o->data.pointer);
    break;
  }
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
  ao_device *device = (ao_device*) malloc(sizeof(ao_device)); //we need to free this on gc

  nbytes = sizeof(luaobject);
  obj = (luaobject *)lua_newuserdata(L, nbytes);
  luaL_getmetatable(L, "ao.object");
  lua_setmetatable(L, -2);

  memset(obj, 0, nbytes); //clear it before using

  obj->data.pointer = (void*) device;
  obj->type = DEVICE;

  return 1;
}

static const luaL_Reg ao [] = {
  {"initialize", l_initialize},
  {"shutdown", l_shutdown},
  {"defaultDriverId", l_default_driver_id},
  {"newDevice" , l_new_device},
  {NULL, NULL}
};

int luaopen_ao(lua_State* L)
{
  luaL_newmetatable(L, "ao.object");
  lua_pushstring(L, "__gc");
  lua_pushcfunction(L, l___gc);
  lua_settable(L, -3);
  luaL_register(L, "ao", ao);
  return 1;
}

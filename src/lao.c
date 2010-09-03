#include <stdio.h>
#include <string.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "ao/ao.h"

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
static int l___gc(lua_State* L)
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

struct ao_sample_format table2sampleformat(lua_State* L, int index)
{
  if(lua_type(L, index) != LUA_TTABLE)
    luaL_error(L, "that's not a table!");
  ao_sample_format fmt;
  const char *byte_format;
  lua_pushstring(L, "bits");
  lua_gettable(L, index);
  fmt.bits = luaL_checkint(L, -1);
  lua_pop(L, 1);
  lua_pushstring(L, "channels");
  lua_gettable(L, index);
  fmt.channels = luaL_checkint(L, -1);
  lua_pop(L, 1);
  lua_pushstring(L, "rate");
  lua_gettable(L, index);
  fmt.rate = luaL_checkint(L, -1);
  lua_pop(L, 1);
  lua_pushstring(L, "byte_format");
  lua_gettable(L, index);
  byte_format = luaL_checkstring(L, -1);
  lua_pop(L, 1);
  if (!strcmp(byte_format, "little"))
    fmt.byte_format = AO_FMT_LITTLE;
  else if (!strcmp(byte_format, "big"))
    fmt.byte_format = AO_FMT_BIG;
  else if (!strcmp(byte_format, "native"))
    fmt.byte_format = AO_FMT_NATIVE;
  else
    luaL_error(L, "not a valid byte format");
  return fmt;
}
struct ao_option table2option(lua_State* L, int index)
{
  ao_option opt;
  if (!lua_isnil(L, index))
  {
    lua_pushnil(L);
    while (lua_next(L, index) != 0)
    {
      const char *key = luaL_checkstring(L, -2);
      const char *val = luaL_checkstring(L, -1);
      ao_append_option(&opt, key, val);
      lua_pop(L, 1);
    }
  }
  return opt;
}

static int l_open_live(lua_State* L)
{
  int driver_id = luaL_checkint(L, 1);
  struct ao_sample_format fmt = table2sampleformat(L, 2);
  struct ao_option opt = table2option(L, 3);
  size_t nbytes;

  nbytes = sizeof(ao_device*);
  ao_device **dev = (ao_device **)lua_newuserdata(L, nbytes);
  luaL_getmetatable(L, "ao.device");
  lua_setmetatable(L, -2);

  memset(dev, 0, nbytes); //clear it before using

  ao_device *tdev = ao_open_live(driver_id, &fmt, NULL);
  *dev = tdev;

  return 1;
}

static int l_play(lua_State *L)
{
  ao_device *dev = *((ao_device **) lua_touserdata(L, 1));
  int num_bytes = luaL_checkint(L, 3);
  const char *samples = luaL_checklstring(L, 2, (size_t *) &num_bytes);
  int result = ao_play(dev, (char *)samples, (uint_32)num_bytes);
  lua_pushnumber(L, result);
  return 1;
}

static int l_close_device(lua_State* L)
{
  ao_device *dev = *((ao_device **) lua_touserdata(L, 1));
  ao_close(dev);
  return 0;
}

static const luaL_Reg ao [] = {
  {"initialize", l_initialize},
  {"shutdown", l_shutdown},
  {"defaultDriverId", l_default_driver_id},
  {"openLive", l_open_live},
  {"__gc", l___gc},
  {NULL, NULL}
};

int luaopen_ao(lua_State* L)
{
  ao_initialize();
  luaL_newmetatable(L, "ao.device");
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  lua_pushstring(L, "__gc");
  lua_pushcfunction(L, l_close_device);
  lua_settable(L, -3);
  lua_pushstring(L, "close");
  lua_pushcfunction(L, l_close_device);
  lua_settable(L, -3);
  lua_pushstring(L, "play");
  lua_pushcfunction(L, l_play);
  lua_settable(L, -3);
  luaL_register(L, "ao", ao);
  return 1;
}
#include <stdio.h>
#include <string.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "ao/ao.h"

/* -- Library Setup/Teardown -- */
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

/* --  Device Setup/Playback/Teardown -- */
// backend stuff
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
  lua_pushstring(L, "byteFormat");
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
  struct ao_option *opt;
  if (!lua_isnoneornil(L, index))
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
  return *opt;
}
//actual functions
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

  ao_device *tdev = ao_open_live(driver_id, &fmt, &opt);
  if (!tdev)
  {
    switch (errno)
    {
    case AO_ENODRIVER:
      luaL_error(L, "no such driver");
      break;
    case AO_ENOTFILE:
      luaL_error(L,  "not a live-type driver");
      break;
    case AO_EBADOPTION:
      luaL_error(L,  "a valid option-key has an invalid value");
      break;
    case AO_EOPENFILE:
      luaL_error(L,  "cannot open the device");
      break;
    case AO_EFAIL:
    default:
      luaL_error(L, "something went wrong");
      break;
    }
  }
  else
    *dev = tdev;

  return 1;
}
static int l_open_file(lua_State* L)
{
  int driver_id = luaL_checkint(L, 1);
  const char *filename = luaL_checkstring(L, 2);
  if (!lua_isboolean(L, 3))
    luaL_error(L, "bad argument #3 to 'openFile' (boolean expected)");
  int overwrite = lua_toboolean(L, 3);
  struct ao_sample_format fmt = table2sampleformat(L, 4);
  struct ao_option opt = table2option(L, 5);
  size_t nbytes;

  nbytes = sizeof(ao_device*);
  ao_device **dev = (ao_device **)lua_newuserdata(L, nbytes);
  luaL_getmetatable(L, "ao.device");
  lua_setmetatable(L, -2);

  memset(dev, 0, nbytes); //clear it before using

  ao_device *tdev = ao_open_file(driver_id, filename, overwrite, &fmt, &opt);
  if (!tdev)
  {
    switch (errno)
    {
    case AO_ENODRIVER:
      luaL_error(L, "no such driver");
      break;
    case AO_ENOTFILE:
      luaL_error(L, "not a file-type driver");
      break;
    case AO_EBADOPTION:
      luaL_error(L, "a valid option-key has an invalid value");
      break;
    case AO_EOPENFILE:
      luaL_error(L, "cannot open the file");
      break;
    case AO_EFILEEXISTS:
      luaL_error(L, "file already exists");
      break;
    case AO_EFAIL:
    default:
      luaL_error(L, "something went wrong");
      break;
    }
  }
  else
    *dev = tdev;

  return 1;
}
static int l_play(lua_State *L)
{
  ao_device *dev = *((ao_device **) lua_touserdata(L, 1));
  int num_bytes = luaL_checkint(L, 3);
  const char *samples = luaL_checklstring(L, 2, (size_t *) &num_bytes);
  int result = ao_play(dev, (char *)samples, (uint_32)num_bytes);
  lua_pushboolean(L, result);
  return 1;
}
static int l_close_device(lua_State* L)
{
  ao_device *dev = *((ao_device **) lua_touserdata(L, 1));
  int result = ao_close(dev);
  lua_pushboolean(L, result);
  return 0;
}

/* -- Driver Information -- */
//backend stuff
void info2luaTable(lua_State* L, ao_info* inf)
{
  int i;
  lua_newtable(L);
  lua_pushstring(L, "type");
  switch (inf->type)
  {
  case AO_TYPE_LIVE:
    lua_pushstring(L, "live");
    break;
  case AO_TYPE_FILE:
    lua_pushstring(L, "file");
    break;
  }
  lua_settable(L, -3);
  lua_pushstring(L, "name");
  lua_pushstring(L, inf->name);
  lua_settable(L, -3);
  lua_pushstring(L, "shortName");
  lua_pushstring(L, inf->short_name);
  lua_settable(L, -3);
  lua_pushstring(L, "comment");
  if (inf->comment)
    lua_pushstring(L, inf->comment);
  else
    lua_pushnil(L);
  lua_settable(L, -3);
  lua_pushstring(L, "preferredByteFormat");
  switch (inf->preferred_byte_format)
  {
  case AO_FMT_LITTLE:
    lua_pushstring(L, "little");
    break;
  case AO_FMT_BIG:
    lua_pushstring(L, "big");
    break;
  case AO_FMT_NATIVE:
    lua_pushstring(L, "native");
    break;
  }
  lua_settable(L, -3);
  lua_pushstring(L, "priority");
  lua_pushinteger(L, inf->priority);
  lua_settable(L, -3);
  lua_pushstring(L, "options");
  lua_newtable(L);
  for (i = 0; i < inf->option_count; i++)
  {
    lua_pushinteger(L, i+1);
    lua_pushstring(L, inf->options[i]);
    lua_settable(L, -3);
  }
  lua_settable(L, -3);
}
//actual functions
static int l_driver_id(lua_State* L)
{
  const char *driver = luaL_checkstring(L, 1);
  int driverId = ao_driver_id((char *)driver);
  if (driverId == -1)
    lua_pushnil(L);
  else
    lua_pushinteger(L, driverId);
  return 1;
}
static int l_default_driver_id(lua_State* L)
{
  int default_driver = ao_default_driver_id();
  if (default_driver == -1)
    lua_pushnil(L);
  else
    lua_pushinteger(L, default_driver);
  return 1;
}
static int l_driver_info(lua_State *L)
{
  int driver_id = luaL_checkint(L, 1);
  ao_info *inf = ao_driver_info(driver_id);
  if (!inf)
  {
    lua_pushnil(L);
    lua_pushstring(L, "invalid device id");
    return 2;
  }
  else
  {
    info2luaTable(L, inf);
    return 1;
  }
}
static int l_driver_info_list(lua_State *L)
{
  int count, i, driverid;
  ao_info **infa = ao_driver_info_list(&count);
  lua_newtable(L);
  for (i = 0; i < count; i++)
  {
    driverid = ao_driver_id(infa[i]->short_name);
    lua_pushinteger(L, driverid);
    info2luaTable(L, infa[i]);
    lua_settable(L, -3);
  }
  return 1;
}
static int l_file_extension(lua_State *L)
{
  int driverId = luaL_checkint(L, 1);
  char *ext = ao_file_extension(driverId);
  lua_pushstring(L, ext);
  return 1;
}

/* -- Miscellaneous -- */
static int l_is_big_endian(lua_State *L)
{
  lua_pushboolean(L, ao_is_big_endian());
  return 1;
}

/* -- Lua Stuff -- */
static const luaL_Reg ao [] = {
  {"initialize", l_initialize},
  {"shutdown", l_shutdown},
  {"openLive", l_open_live},
  {"openFile", l_open_file},
  {"driverId", l_driver_id},
  {"defaultDriverId", l_default_driver_id},
  {"driverInfo", l_driver_info},
  {"driverInfoList", l_driver_info_list},
  //{"fileExtension", l_file_extension},
  {"isBigEndian", l_is_big_endian},
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
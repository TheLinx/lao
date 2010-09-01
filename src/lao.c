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
    free(o->data.pointer);
    break;
  }
}

static int l_sample_format__index(lua_State* L)
{
  luaobject *o = (luaobject*) lua_touserdata(L, 1);
  const char *key = lua_tostring(L, 2);
  ao_sample_format *fmt = (ao_sample_format *)o->data.pointer;
  if (!strcmp(key, "bits"))
    lua_pushnumber(L, (double)fmt->bits);
  else if (!strcmp(key, "rate"))
    lua_pushnumber(L, (double)fmt->rate);
  else if (!strcmp(key, "channels"))
    lua_pushnumber(L, (double)fmt->channels);
  else if (!strcmp(key, "byte_format"))
  {
    switch(fmt->byte_format)
    {
    case 0:
      lua_pushstring(L, "");
      break;
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
  }
  //else if (!strcmp(key, "matrix"))
  //  lua_pushstring(L, fmt->matrix);
  else
    lua_pushnil(L);
  return 1;
}
static int l___index(lua_State* L)
{
  luaobject *o = (luaobject *) lua_touserdata(L, 1);
  switch(o->type)
  {
  case SAMPLE_FORMAT:
    return l_sample_format__index(L);
    break;
  }
  return 0;
}

static int l_sample_format__newindex(lua_State* L)
{
  luaobject *o = (luaobject*) lua_touserdata(L, 1);
  const char *key = lua_tostring(L, 2);
  ao_sample_format *fmt = (ao_sample_format *)o->data.pointer;
  if (!strcmp(key, "bits"))
  {
    int newVal = luaL_checkint(L, 3);
    fmt->bits = newVal;
  }
  else if (!strcmp(key, "rate"))
  {
    int newVal = luaL_checkint(L, 3);
    fmt->rate = newVal;
  }
  else if (!strcmp(key, "channels"))
  {
    int newVal = luaL_checkint(L, 3);
    fmt->channels = newVal;
  }
  else if (!strcmp(key, "byte_format"))
  {
    const char *newVal = luaL_checkstring(L, 3);
    if (!strcmp(key, "little"))
      fmt->byte_format = AO_FMT_LITTLE;
    else if (!strcmp(key, "big"))
      fmt->byte_format = AO_FMT_BIG;
    else if (!strcmp(key, "native"))
      fmt->byte_format = AO_FMT_NATIVE;
    else
      luaL_error(L, "not a valid byte format");
  }
  //else if (!strcmp(key, "matrix"))
  //  lua_pushstring(L, fmt->matrix);
  else
    luaL_error(L, "not a valid member");
  return 0;
}
static int l___newindex(lua_State* L)
{
  luaobject *o = (luaobject *) lua_touserdata(L, 1);
  switch(o->type)
  {
  case SAMPLE_FORMAT:
    return l_sample_format__newindex(L);
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

static int l_new_sample_format(lua_State* L)
{
  size_t nbytes;
  luaobject *obj;
  ao_sample_format *fmt = (ao_sample_format *) malloc(sizeof(ao_sample_format));

  nbytes = sizeof(luaobject);
  obj = (luaobject *)lua_newuserdata(L, nbytes);
  luaL_getmetatable(L, "ao.object");
  lua_setmetatable(L, -2);

  memset(obj, 0, nbytes);

  obj->data.pointer = (void*) fmt;
  obj->type = SAMPLE_FORMAT;

  return 1;
}

static const luaL_Reg ao [] = {
  {"initialize", l_initialize},
  {"shutdown", l_shutdown},
  {"defaultDriverId", l_default_driver_id},
  {"device" , l_new_device},
  {"sampleFormat" , l_new_sample_format},
  {NULL, NULL}
};

int luaopen_ao(lua_State* L)
{
  luaL_newmetatable(L, "ao.object");
  lua_pushstring(L, "__gc");
  lua_pushcfunction(L, l___gc);
  lua_pushstring(L, "__index");
  lua_pushcfunction(L, l___index);
  lua_pushstring(L, "__newindex");
  lua_pushcfunction(L, l___newindex);
  lua_settable(L, -7);
  luaL_register(L, "ao", ao);
  return 1;
}

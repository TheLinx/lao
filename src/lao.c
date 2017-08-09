#include <stdio.h>
#include <string.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "ao/ao.h"

/* -- Library Setup/Teardown -- */
static int has_initialized = 0;
static int l_initialize(lua_State* L)
{
	(void)L;
	if (!has_initialized) {
		ao_initialize();
		has_initialized = 1;
	}
	return 0;
}
static int l_shutdown(lua_State* L)
{
	(void)L;
	if (has_initialized) {
		ao_shutdown();
		has_initialized = 0;
	}
	return 0;
}
static int l_setinitialized(lua_State* L)
{
	int new_initialized = (luaL_checktype(L, 1, LUA_TBOOLEAN), lua_toboolean(L, 1));
	has_initialized = new_initialized;
	return 0;
}

/* --  Device Setup/Playback/Teardown -- */
// backend stuff
static struct ao_sample_format table2sampleformat(lua_State* L, int index)
{
	if(lua_type(L, index) != LUA_TTABLE)
		luaL_error(L, "that's not a table!");

	ao_sample_format fmt;
	const char *byte_format;

	fmt.matrix = 0;

	lua_getfield(L, index, "bits");
	fmt.bits = luaL_checkinteger(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, index, "channels");
	fmt.channels = luaL_checkinteger(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, index, "rate");
	fmt.rate = luaL_checkinteger(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, index, "byteFormat");
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

static struct ao_option *table2option(lua_State* L, int index)
{
	struct ao_option *opt = NULL;
	if (!lua_isnoneornil(L, index))
	{
		lua_pushnil(L);
		while (lua_next(L, index) != 0)
		{
			const char *key = luaL_checkstring(L, -2);
			const char *val = luaL_checkstring(L, -1);
			if (!ao_append_option(&opt, key, val))
				luaL_error(L, "out of memory");
			lua_pop(L, 1);
		}
	}
	return opt;
}

//actual functions
static int l_open_live(lua_State* L)
{
	int driver_id = luaL_checkinteger(L, 1);
	struct ao_sample_format fmt = table2sampleformat(L, 2);
	struct ao_option *opt;
	lua_settop(L, 3);

	l_initialize(L);

	ao_device **dev = (ao_device **)lua_newuserdata(L, sizeof(ao_device*));
	luaL_getmetatable(L, "ao.device");
	lua_setmetatable(L, -2);

	opt = table2option(L, 3);

	*dev = ao_open_live(driver_id, &fmt, opt);
	if (opt)
		ao_free_options(opt);

	if (*dev == NULL)
	{
		switch (errno)
		{
			case AO_ENODRIVER:
				luaL_error(L, "no such driver");
				break;
			case AO_ENOTLIVE:
				luaL_error(L,  "not a live-type driver");
				break;
			case AO_EBADOPTION:
				luaL_error(L,  "a valid option-key has an invalid value");
				break;
			case AO_EOPENDEVICE:
				luaL_error(L,  "cannot open the device");
				break;
			case AO_EFAIL:
			default:
				luaL_error(L, "something went wrong");
				break;
		}
	}

	return 1;
}

static int l_open_file(lua_State* L)
{
	int driver_id = luaL_checkinteger(L, 1);
	const char *filename = luaL_checkstring(L, 2);
	if (!lua_isboolean(L, 3))
		luaL_error(L, "bad argument #3 to 'openFile' (boolean expected)");
	int overwrite = lua_toboolean(L, 3);
	struct ao_sample_format fmt = table2sampleformat(L, 4);
	struct ao_option *opt;
	lua_settop(L, 5);

	ao_device **dev = (ao_device **)lua_newuserdata(L, sizeof(ao_device*));
	luaL_getmetatable(L, "ao.device");
	lua_setmetatable(L, -2);

	opt = table2option(L, 5);

	l_initialize(L);

	*dev = ao_open_file(driver_id, filename, overwrite, &fmt, opt);
	if (opt)
		ao_free_options(opt);

	if (*dev == NULL)
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

	return 1;
}

static int l_play(lua_State *L)
{
	ao_device *dev = *((ao_device **) luaL_checkudata(L, 1, "ao.device"));
	size_t len;
	const char *samples = luaL_checklstring(L, 2, &len);
	uint_32 num_bytes = (uint_32) luaL_optinteger(L, 3, len);
	luaL_argcheck(L, num_bytes <= len, 3, "too many bytes");

	int result = ao_play(dev, (char*)samples, num_bytes);

	lua_pushboolean(L, result);
	return 1;
}

static int l_close_device(lua_State* L)
{
	ao_device **dev = luaL_checkudata(L, 1, "ao.device");
	if (*dev != NULL) {
		ao_close(*dev);
		*dev = NULL;
	}
	return 0;
}

/* -- Driver Information -- */
//backend stuff
static void info2luaTable(lua_State* L, ao_info* inf)
{
	int i;
	lua_createtable(L, 0, 7);

	switch (inf->type)
	{
		case AO_TYPE_LIVE:
			lua_pushliteral(L, "live");
			break;
		case AO_TYPE_FILE:
			lua_pushliteral(L, "file");
			break;
	}
	lua_setfield(L, -2, "type");

	lua_pushstring(L, inf->name);
	lua_setfield(L, -2, "name");

	lua_pushstring(L, inf->short_name);
	lua_setfield(L, -2, "shortName");

	if (inf->comment) {
		lua_pushstring(L, inf->comment);
		lua_setfield(L, -2, "comment");
	}

	switch (inf->preferred_byte_format)
	{
		case AO_FMT_LITTLE:
			lua_pushliteral(L, "little");
			break;
		case AO_FMT_BIG:
			lua_pushliteral(L, "big");
			break;
		case AO_FMT_NATIVE:
			lua_pushliteral(L, "native");
			break;
	}
	lua_setfield(L, -2, "preferredByteFormat");

	lua_pushinteger(L, inf->priority);
	lua_setfield(L, -2, "priority");

	lua_createtable(L, inf->option_count, 0);
	for (i = 0; i < inf->option_count; i++)
	{
		lua_pushstring(L, inf->options[i]);
		lua_rawseti(L, -2, i+1);
	}
	lua_setfield(L, -2, "options");
}

//actual functions
static int l_driver_id(lua_State* L)
{
	const char *driver = luaL_checkstring(L, 1);
	int driverId;

	l_initialize(L);

	if ((driverId = ao_driver_id((char *)driver)) == -1)
		lua_pushnil(L);
	else
		lua_pushinteger(L, driverId);
	return 1;

}
static int l_default_driver_id(lua_State* L)
{
	int default_driver;

	l_initialize(L);

	if ((default_driver = ao_default_driver_id()) == -1)
		lua_pushnil(L);
	else
		lua_pushinteger(L, default_driver);
	return 1;
}

static int l_driver_info(lua_State *L)
{
	int driver_id = luaL_checkinteger(L, 1);
	ao_info *inf;

	l_initialize(L);

	if (!(inf = ao_driver_info(driver_id)))
	{
		lua_pushnil(L);
		lua_pushliteral(L, "invalid device id");
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
	ao_info **infa;

	l_initialize(L);

	infa = ao_driver_info_list(&count);
	lua_createtable(L, count, 0);
	for (i = 0; i < count; i++)
	{
		driverid = ao_driver_id(infa[i]->short_name);
		info2luaTable(L, infa[i]);
		lua_rawseti(L, -2, driverid);
	}
	return 1;
}

static int l_file_extension(lua_State *L)
{
	int driverId = luaL_checkinteger(L, 1);

	l_initialize(L);

	lua_pushstring(L, ao_file_extension(driverId));
	return 1;
}

/* -- Miscellaneous -- */
static int l_is_big_endian(lua_State *L)
{
	lua_pushboolean(L, ao_is_big_endian());
	return 1;
}

static int l_append_global_option(lua_State *L)
{
	const char *key = luaL_checkstring(L, 1);
	const char *val = luaL_checkstring(L, 2);
	lua_pushboolean(L, ao_append_global_option(key, val));
	return 1;
}

/* -- Lua Stuff -- */
static const luaL_Reg ao [] = {
	{"initialize", l_initialize},
	{"setinitialized", l_setinitialized},
	{"shutdown", l_shutdown},
	{"openLive", l_open_live},
	{"openFile", l_open_file},
	{"driverId", l_driver_id},
	{"defaultDriverId", l_default_driver_id},
	{"driverInfo", l_driver_info},
	{"driverInfoList", l_driver_info_list},
	{"fileExtension", l_file_extension},
	{"isBigEndian", l_is_big_endian},
	{"appendGlobalOption", l_append_global_option},
	{NULL, NULL}
};

int luaopen_ao(lua_State* L)
{
	luaL_newmetatable(L, "ao.device");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, l_close_device);
	lua_setfield(L, -2, "__gc");
	lua_pushcfunction(L, l_close_device);
	lua_setfield(L, -2, "close");
	lua_pushcfunction(L, l_play);
	lua_setfield(L, -2, "play");
#if LUA_VERSION_NUM >= 502
	luaL_newlib(L, ao);    /* 5.2 */
#else
	luaL_register(L, "ao", ao);  /* 5.1 */
#endif
	return 1;
}

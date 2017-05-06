/*
 *  lua_interp.hpp
 *  author: Aleksei Kozadaev (2017)
 */

#include <stdlib.h>

#include "log.hpp"
#include "context.hpp"
#include "lua_interp.hpp"

using namespace imk;
using namespace std;

extern Context ctx;

static const char * const EVENTS_TABLE = "events";
static const char * const EVENT_OPEN = "open";
static const char * const EVENT_CLOSE = "close";
static const char * const EVENT_CREATE = "create";
static const char * const EVENT_READ = "read";
static const char * const EVENT_WRITE = "write";
static const char * const EVENT_DELETE = "delete";

//------------------------------------------------------------------------------
LuaInterp::LuaInterp() : m_deleter(),
    m_stateStore(luaL_newstate(), m_deleter), m_state(m_stateStore.get())
{
    /* Make standard libraries available in the Lua object */
    luaL_openlibs(m_state);
    luaopen_table(m_state);

    /* register api functions */
    lua_register(m_state, "imk_shell", LuaInterp::apiShell);
    lua_register(m_state, "imk_command", LuaInterp::apiCommand);
    lua_register(m_state, "imk_getfiles", LuaInterp::apiGetFiles);

    /* global events registry */
    lua_newtable(m_state);
    lua_setglobal(m_state, EVENTS_TABLE);
}

//------------------------------------------------------------------------------
LuaInterp::~LuaInterp()
{
    // nothing here
}

//------------------------------------------------------------------------------
LuaInterp::LuaInterp(LuaInterp &other) : m_deleter(),
    m_stateStore(other.m_state, m_deleter), m_state(other.m_state)
{
    other.m_stateStore.release();
    other.m_state = nullptr;
}

//------------------------------------------------------------------------------
LuaInterp&
LuaInterp::operator=(LuaInterp &other)
{
    m_stateStore.reset(other.m_state);
    m_state = other.m_state;
    other.m_stateStore.release();
    other.m_state = nullptr;

    return *this;
}

//------------------------------------------------------------------------------
void
LuaInterp::executeScript(const char* fname)
{
    int result;

    LOG_DEBUG("loading lua config %s", fname);

    // Load the program; this supports both source code and bytecode files.
    result = luaL_loadfile(m_state, fname);

    if (result != LUA_OK) {
        printError();
        return;
    }

    // Finally, execute the program by calling into it. Change the arguments
    // if you're not running vanilla Lua code.
    result = lua_pcall(m_state, 0, LUA_MULTRET, 0);
    if (result != LUA_OK) {
        printError();
        return;
    }
}

//------------------------------------------------------------------------------
int
LuaInterp::setupCb(const char *evTable, const char *key)
{
    // calling a function defined in .lua
    lua_getglobal(m_state, "events");
    if (!lua_istable(m_state, -1)) {
        LOG_ERR_VA("%s is not a table", EVENTS_TABLE);
        exit(1);
    }

    lua_pushstring(m_state, key);
    lua_gettable(m_state, -2);

    if (lua_isnil(m_state, -1)) {
        return LUA_ERRRUN;
    }

    return LUA_OK;
}

//------------------------------------------------------------------------------
int
LuaInterp::callCb(const char *key, size_t numArgs)
{
    if (!lua_isfunction(m_state, -(numArgs + 1))) {
        LOG_ERR_VA("%s.%s is not a function", EVENTS_TABLE, key);
        return LUA_ERRRUN;
    }
    return lua_pcall(m_state, numArgs, 0, 0);
}

//------------------------------------------------------------------------------
inline void
LuaInterp::printError()
{
    // The error message is on top of the stack.
    // Fetch it, print it and then pop it off the stack.
    const char* message = lua_tostring(m_state, -1);
    LOG_ERR_VA("%s", message);
    lua_pop(m_state, 1);
}

//------------------------------------------------------------------------------
int
LuaInterp::genericEvent(const char *evName, const char **args, size_t nArgs)
{
    int result = LUA_OK;
    if ((result = setupCb(EVENTS_TABLE, evName)) == LUA_OK) {
        // pass agruments
        for (size_t i = 0; i < nArgs; ++i) {
            lua_pushstring(m_state, args[i]);
        }

        result = callCb(evName, 1);
        if (result != LUA_OK) {
            lua_error(m_state);
        }
    }

    return result;
}

//------------------------------------------------------------------------------
//  Event triggers
//------------------------------------------------------------------------------
int
LuaInterp::eventOpen(const char *fname)
{
    LOG_DEBUG("open event - %s", fname);
    return genericEvent(EVENT_OPEN, &fname, 1);
}

//------------------------------------------------------------------------------
int
LuaInterp::eventClose(const char *fname)
{
    LOG_DEBUG("close event - %s", fname);
    return genericEvent(EVENT_CLOSE, &fname, 1);
}

//------------------------------------------------------------------------------
int
LuaInterp::eventCreate(const char *fname)
{
    LOG_DEBUG("create event - %s", fname);
    return genericEvent(EVENT_CREATE, &fname, 1);
}

//------------------------------------------------------------------------------
int
LuaInterp::eventRead(const char *fname)
{
    LOG_DEBUG("read event - %s", fname);
    return genericEvent(EVENT_READ, &fname, 1);
}

//------------------------------------------------------------------------------
int
LuaInterp::eventWrite(const char *fname)
{
    LOG_DEBUG("write event - %s", fname);
    return genericEvent(EVENT_WRITE, &fname, 1);
}

//------------------------------------------------------------------------------
int
LuaInterp::eventDelete(const char *fname)
{
    LOG_DEBUG("delete event - %s", fname);
    return genericEvent(EVENT_DELETE, &fname, 1);
}

//------------------------------------------------------------------------------
//  API calls
//------------------------------------------------------------------------------
int
LuaInterp::apiShell(lua_State *L)
{
    int nargs = lua_gettop(L);

    if (nargs != 1) {
        luaL_error(L, "shell: 1 argument expected");
    }

    if (lua_type(L, 1) != LUA_TSTRING) {
        luaL_error(L, "shell: the argument must be a string");
    }

    const char *cmd = lua_tostring(L, 1);

    return (system(cmd) == 0) ? LUA_OK : LUA_ERRRUN;
}

//------------------------------------------------------------------------------
int
LuaInterp::apiGetFiles(lua_State *L)
{
    int nargs = lua_gettop(L);

    if (nargs > 0) {
        luaL_error(L, "get_files: too many arguments");
    }

    lua_newtable(L);
    auto it = ctx.files().cbegin();
    for (int i = 0; it != ctx.files().cend(); ++it, ++i) {
        lua_pushnumber(L, i + 1);
        lua_pushstring(L, it->second.c_str());
        lua_settable(L, -3);
    }

    return 1;  // returning 1 table
}

//------------------------------------------------------------------------------
int
LuaInterp::apiCommand(lua_State *L)
{
    int nargs = lua_gettop(L);

    if (nargs > 0) {
        luaL_error(L, "command: too many arguments");
    }

    if (!ctx.command().empty()) {
        return (system(ctx.command().c_str()) == 0) ? LUA_OK : LUA_ERRRUN;
    }

    return LUA_OK;
}

//------------------------------------------------------------------------------

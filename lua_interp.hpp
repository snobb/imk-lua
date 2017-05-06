#ifndef __LUA_INTERP_HPP__
#define __LUA_INTERP_HPP__

#include <lua.hpp>
#include <memory>

namespace imk {

class LuaInterp
{
public:
    LuaInterp();
    ~LuaInterp();
    LuaInterp(LuaInterp&);
    LuaInterp &operator=(LuaInterp&);

    void executeScript(const char* fname);

    // event triggers
    int eventOpen(const char *fname);
    int eventClose(const char *fname);
    int eventCreate(const char *fname);
    int eventRead(const char *fname);
    int eventWrite(const char *fname);
    int eventDelete(const char *fname);

    // api calls
    static int apiShell(lua_State *L);
    static int apiCommand(lua_State *L);
    static int apiGetFiles(lua_State *L);

private:
    // disable const copy constructors;
    LuaInterp(const LuaInterp&);
    LuaInterp &operator=(const LuaInterp&);
    int setupCb(const char *evTable, const char *key);
    int callCb(const char *key, size_t numArgs);

    int genericEvent(const char *evName, const char **args, size_t numArgs);
    void printError();

    struct StateDeleter
    {
        void operator()(lua_State *p) { lua_close(p); }
    };

    StateDeleter m_deleter;
    std::unique_ptr<lua_State, StateDeleter&> m_stateStore;
    lua_State *m_state;
};

}  // namespace imk

#endif /* __LUA_INTERP_HPP__ */

#ifndef __CONTEXT_HPP__
#define __CONTEXT_HPP__

#include <stdlib.h>
#include <errno.h>

#include <map>
#include <algorithm>

#include "log.hpp"
#include "lua_interp.hpp"

namespace imk {

typedef std::map<int, std::string> FileRegistry;

class Context
{
public:
    Context();
    ~Context();

    void command(const std::string &command) { m_command = command; }
    std::string command() const { return m_command; }

    const FileRegistry &files() const { return m_files; }

    void registerFile(const char *path);
    int dispatch();

private:
    const char * const RC_FILE = ".imkrc.lua";

    int walkDir(const char *path);
    int findRcFile(char *rcFileOut);

    int m_qfd = -1;
    std::string m_command;
    FileRegistry m_files;
    LuaInterp m_luaInterp;

    // disable default and copy constructors and assignment operator
    Context(Context &) = delete;
    Context(const Context &) = delete;
    Context &operator=(Context &) = delete;
    Context &operator=(const Context &) = delete;
};

}  // namespace imk

#endif /* __CONTEXT_HPP__ */

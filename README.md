IMK
============
Simple file watcher similar to fswatch or inotify-wait.


Usage:
------
```bash
$ ./imk -h
usage: ./imk [-h] -c <command> <file ...>

   The options are as follows:
      -h          - display this text and exit
      -c <cmd>    - command to execute when event is triggered
      <file ...>  - list of files to monitor

   Please use quotes around the command if it is composed of multiple words

   Lua event handlers are searched in .luarc.lua file in the current directory and then in $HOME
```

To monitor all .cpp files and run make run the following:

```bash
$ ./imk -c 'make release' *.cpp
:: [20:00:00] start monitoring: cmd[make release] files[log.cpp main.cpp]
```

If any of the monitored files are modified, the command will be executed.

In Lua version the command is optional and will be executed on write event unless a Lua handler for write event is configured. Otherwise it can be executed using imk_command() call from any event handler. The handlers are loaded from .luarc.lua file (searched at ./ and then in ~/)

Known Issues:
-------------
 - when monitoring a directory, the command may be triggered more then once for every change of the contained files.
 - imk may not detect a change if the monitored file was edited with VIM with swapfiles enabled.

IMK
============
Simple file watcher similar to fswatch or inotify-wait.


Usage:
------
```bash
$ ./imk -h
usage: ./imk [-h] -c <command> <file ...>

   The options are as follows:
      -c <command> - command to execute when event triggered
      <file ...>   - list of files to monitor

   Please use quotes around command if it is composed of multiple words
```

To monitor all .cpp files and run make run the following:

```bash
$ ./imk -c 'make release' *.cpp
:: [20:00:00] start monitoring: cmd[make release] files[log.cpp main.cpp]
```

If any of the monitored files are modified, the command will be executed.

Known Issues:
-------------
 - when monitoring a directory, the command may be triggered more then once for every change of the contained files.
 - imk may not detect a change if the monitored file was edited with VIM with swapfiles enabled.

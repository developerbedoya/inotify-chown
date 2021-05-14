# inotify-chown

Watch file creation in a directory and change ownership of newly created files. I made this program to change ownership of bluetooth sent files, that are created with owner 'root'. This program immediately changes the ownership to the specified user.

This program was only tested in linux. I don't know if it's compatible with other POSIX platforms.

Right now the watched directory and the new owner user name are fixed variables inside the sopurce code, but I'll create parameters to do this instead. 

## Compiling
```
gcc inotify-chown.c -o inotify-chown
```

## TODO
* Create -d (--directory) flag for watched directory. Example: `-d /home/pi/bluetooth'
* Create -u (--user) flag for new owner username
* Create -h (--help) flag for help
* Create makefile (make and make install)
* Create user guide


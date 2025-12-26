#!/bin/bash
gcc -Wall -Wextra -Werror xbr.c -o xbr -lX11 -lXrandr
sudo mv xbr /usr/local/bin/xbr

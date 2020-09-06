#! /bin/bash

gcc close.c -o ./bin/close 
./bin/close

gcc file-shell.c -o ./bin/fshell
./bin/fshell . key bin

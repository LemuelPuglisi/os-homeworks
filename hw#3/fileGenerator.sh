#!/bin/bash

( for I in `seq 1000`; do echo $I | md5sum | cut -d' ' -f1 ; done ) > esempio.txt
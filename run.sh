#!/bin/bash

sudo setfacl --modify user:$USER:rw /dev/ttyUSB1

python3 turt.py

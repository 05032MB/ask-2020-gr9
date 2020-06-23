#!/bin/bash

sudo setfacl --modify user:$USER:rw /dev/ttyUSB*

python3 turt.py

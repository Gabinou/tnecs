#!/bin/sh

# style.sh
# Copyright (C) Gabriel Taillon, 2023
# Automatic formatting with astyle 3.2.1 for mace 

astyle --options=style.txt --verbose tnecs.c tnecs.h
#!/bin/sh
# Copyright (C) Gabriel Taillon, 2025

cppcheck --enable=warning --check-level=exhaustive --std=c99 tnecs.c tnecs.h
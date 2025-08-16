<div align="center">
    ![tnecs](logo.png "tnecs logo")
</div>

# tnecs (Tiny nECS) 

The _simplest possible_ C99 ECS library, only with the _minimum necessary features_. 

## Features
- Compatible: compiles with ```tcc```, ```gcc``` and ```clang```
- Cross-platform: Windows, Linux, Android (termux)
- Small: <2000 lines, 2 files.
- Fast: see benchmarks in test.c
- Simple: C99 API
- Independent: only depends on C99 standard libraries

## Detailed Introduction
Tiny C99 Entity-Component-System (ECS) library.
- [Tutorial](https://gitlab.com/Gabinou/tnecs/-/blob/master/TUTORIAL.md).
- [Design details](https://gitlab.com/Gabinou/tnecs/-/blob/master/DESIGN.md).

ECS is an architectural pattern that organizes data and functions by favoring composition over inheritance:
1. Components (i.e. `struct`) are purely data.
2. Entities (i.e. `ull`) can have any number of Cs.
3. Systems (i.e. functions) act on all entities that have a certain set of Cs.

Example:
- Enemy Entity: AIControlled component, Sprite Component, Physics Component
- Bullet Entity: Sprite Component, Physics Component, DamageonHit Component
- Main Character Entity: UserControlled Component, Sprite Component, Physics Component

## Running tests

```bash
rm -f test
tcc -O0 -fsanitize=undefined,address -fno-strict-aliasing -fwrapv -fno-delete-null-pointer-checks -Wall -Werror -g test.c -o test -lm
./test

rm -f test
gcc --std=iso9899:1999 -O0 -fsanitize=undefined,address -fno-strict-aliasing -fwrapv -fno-delete-null-pointer-checks -Wall -Werror -g test.c -o test -lm -fmax-errors=5
./test
```

## Credits
Copyright (c) 2025 Gabriel Taillon

Originally created for use in a game I am developing: [Codename Firesaga](https://gitlab.com/Gabinou/firesagamaker).

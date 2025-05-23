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
Tiny C99 Entity-Component-System (ECS) library. [Read the Tutorial](https://gitlab.com/Gabinou/tnecs/-/blob/master/TUTORIAL.md).

ECSs are an alternative way to organize data and functions to Object-Oriented programming (OOP).
* OOP: Objects/Classes contain data and methods. 
Methods act on objects. 
Children classes inherit methods and data structure from parents. 
* ECS: Components are purely data.
Any number of components can be attached to an entity.
Systems act on all entities that have a certain set of components.

Originally created for use in a game I am developping: [Codename Firesaga](https://gitlab.com/Gabinou/firesagamaker).

Example:
- Enemy Entity: AIControlled component, Sprite Component, Physics Component
- Bullet Entity: Sprite Component, Physics Component, DamageonHit Component
- Main Character Entity: UserControlled Component, Sprite Component, Physics Component

For more in-depth discussion about tnecs' design, see the [On Design](https://gitlab.com/Gabinou/tnecs/-/blob/master/DESIGN.md).

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
Copyright (c) 2025 Average Bear Games, Made by Gabriel Taillon

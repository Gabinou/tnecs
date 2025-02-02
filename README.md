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
- Independent: only dependent on C99 standard libraries

## Installation
Add ```tnecs.c``` and ```tnecs.h``` to your source code.

## Detailed Introduction
Tiny C99 Entity-Component-System (ECS) library. [Read the Tutorial](https://gitlab.com/Gabinou/tnecs/-/blob/master/TUTORIAL.md).

ECSs are an alternative way to organize data and functions to Object-Oriented programming (OOP).
* OOP: Objects/Classes contain data and methods. 
Methods act on objects. 
Children classes inherit methods and data structure from parents. 
* ECS: Components are purely data.
Any number of components can be attached to an entity.
Entities are acted upon by systems.

Originally created for use in a game I am developping: [Codename Firesaga](https://gitlab.com/Gabinou/firesagamaker).

Videogame Example:
- Enemy Entity: AIControlled component, Sprite Component, Physics Component
- Bullet Entity: Sprite Component, Physics Component, DamageonHit Component
- Main Character Entity: UserControlled Component, Sprite Component, Physics Component

In tnecs, an entity is an ```ull``` index. 
A component is user-defined ```struct```. 
A system is a user-defined ```function```.
All live inside the ```world```. 

The systems iterate over the entities that have a user-defined set of components, inclusively or exclusively, in phases.
Phases are user-defined ```ul```. 
System execution order is first-come first-served by default.
For more in-depth discussion about tnecs' design, see the [On Design](https://gitlab.com/Gabinou/tnecs/-/blob/master/DESIGN.md).

## Running tests

```bash
rm tnecs.o test.o test
gcc --std=iso9899:1999 -O1 -DNDEBUG -fno-strict-aliasing -fwrapv -fno-delete-null-pointer-checks -Wall -Werror -c -g tnecs.c -o tnecs.o
gcc --std=iso9899:1999 -O1 -DNDEBUG -fno-strict-aliasing -fwrapv -fno-delete-null-pointer-checks -Wall -Werror -c -g test.c -o test.o
gcc --std=iso9899:1999 -O1 -DNDEBUG -fno-strict-overflow -fno-strict-aliasing -fwrapv -fno-delete-null-pointer-checks -Wall -Werror -g -o test test.o tnecs.o -lm
./test

rm tnecs.o test.o test
gcc --std=iso9899:1999 -O0 -fsanitize=undefined -fsanitize=address -fno-strict-aliasing -fwrapv -fno-delete-null-pointer-checks -Wall -Werror -c -g tnecs.c -o tnecs.o
gcc --std=iso9899:1999 -O0 -fsanitize=undefined -fsanitize=address -fno-strict-aliasing -fwrapv -fno-delete-null-pointer-checks -Wall -Werror -c -g test.c -o test.o
gcc --std=iso9899:1999 -O0 -fsanitize=undefined -fsanitize=address -fno-strict-overflow -fno-strict-aliasing -fwrapv -fno-delete-null-pointer-checks -Wall -Werror -g -o test test.o tnecs.o -lm
./test
```

## Credits
Copyright (c) 2025 Average Bear Games, Made by Gabriel Taillon

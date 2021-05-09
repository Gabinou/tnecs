# tnecs (Tiny nECS)

Tiny C99 Entity-Component-System (ECS) library.
Originally created for use in a game I am developping using C99: [Codename Firesaga](https://gitlab.com/Gabinou/firesagamaker). Title pending. 

ECSs are an alternative way to organize data and functions to Object-Oriented programming (OOP).
* OOP: Objects/Classes contain data and methods. 
Methods act on objects. 
Children classes inherit methods and data structure from parents. 
* ECS: Components are purely data.
Any number of components can be attached to an entity.
Entities are acted upon by systems. 

In tnecs, an entity is an ```uint64_t``` index. 
A component is user-defined ```struct```. 
A system is a user-defined ```function```.
All live inside the ```world```. 

The systems iterate exclusively over the entities that have exactly the user-defined set of components, in phases.
The user can also modify the system execution order in each phase.

Videogame Example:
- Enemy Entity: AIControlled component, Sprite Component, Physics Component
- Bullet Entity: Sprite Component, Physics Component, DamageonHit Component
- Main Character Entity: UserControlled Component, Sprite Component, Physics Component

## Installation
Add tnecs.c and tnecs.h to your source code.

## Motivation
Make the _simplest possible_ ECS library, only with the _minimum necessary features_.
C99, compile with ```tcc```.

## Features
- Compatible: compiles with ```tcc```, ```gcc```, ```clang```
- Cross-platform: Windows, Linux, Android (termux)
- Small: <2000 lines for now.
- Fast: see simple benchmarks in test.c
- Simple: C99 API
- FOSS: Free and Open Source

## To do
- System input/iterator.
- World progress function
- Post V1.0 pruning

## Alternative ECS/Gamedev libraries for C/C++
- [flecs (C99/C++)](https://github.com/SanderMertens/flecs)
- [entt (C++)](https://github.com/skypjack/entt)
- [gamedev_libraries](https://github.com/raizam/gamedev_libraries)
- [stb list of single header libraries](https://github.com/nothings/single_file_libs)

## Special Thanks
Sanders Mertens for [his blog on ECS design](https://ajmmertens.medium.com/). 
He created [many other resources on ECSs](). 

## Credits
Copyright (c) 2021 Average Bear Games, Made by Gabriel Taillon
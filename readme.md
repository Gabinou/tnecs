# tnecs (Tiny nECS)

Tiny C99 Entity-Component-System (ECS) library.
Originally developed for use game I am developping: [Codename Firesaga](https://gitlab.com/Gabinou/firesagamaker). Title pending. 
ECSs are an alternative way to organize data and functions to Object-Oriented programming (OOP).

* OOP: Objects/Classes contain data and methods, children objects inherit from parents...

* ECS: Components are purely data.
Any number of components can be attached to an entity.
Entities are acted upon by systems. 

In tnecs, an entity is an uint64_t index determined. 
A component is user-defined struct. 
A system is a user-defined function.

The systems iterate only over entities that have a certain set of components.
They can either be exclusive or inclusive, as in including/excluding entities that have components other than the system's set.
Systems's execution order happens in phases, set by the user.
The user can also modify the system execution order in each phase.

Videogame Example:
- Enemy Entity: AIControlled component, Sprite Component, Physics Component
- Bullet Entity: Sprite Component, Physics Component, DamageonHit Component
- Main Character Entity: UserControlled Component, Sprite Component, Physics Component

## Installation
Add tnecs.c and tnecs.h to your source code.

## Features/Objectives
- Compileable using tcc, gcc, clang (msvc untested)
- Simple C99 API
- Small Codebase, <1000 lines (for now).
- Fast (as much as can be measured uing simple benchmarks in test.c)
- Exclusive/Inclusive systems
    * Exclusive systems iterate over the entities that only have the system's components. Inclusive system iterate over entities that may have components other than the system's.
- _Mostly_ [STB-style](https://github.com/nothings/stb/blob/master/docs/stb_howto.txt).
- Free and Open Source

## Alternative ECS/Gamedev libraries for C/C++
- [flecs (C99/C++)](https://github.com/SanderMertens/flecs)
- [entt (C++)](https://github.com/skypjack/entt)
- [gamedev_libraries](https://github.com/raizam/gamedev_libraries)
- [stb list of single header libraries](https://github.com/nothings/single_file_libs)

## Credits
Gabriel Taillon

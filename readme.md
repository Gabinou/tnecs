# tnECS (Tiny nECS)

Tiny C99 Entity-Component-System (ECS) library.
ECSs are an alternative way to organize data and functions to Object-Oriented programming (OOP).
OOP: Objects/Classes contain data and methods, children objects inherit from parents...

ECS: By contrast, components are purely data, user-defined structs.
Any number of components can be attached to an entity, which is simply an uint64_t index determined by tnECS.
Then, entities are acted upon by systems, which are user-defined functions.
The systems iterate only over entities that have a certain set of components.
In tnECS, the system can either be exclusive or inclusive, as in including/excluding entities that have components other than the system's set.

Videogame Example:
    - Enemy Entity: AIControlled component, Sprite Component, Physics Component
    - Bullet Entity: Sprite Component, Physics Component, DamageonHit Component
    - Main Character Entity: UserControlled Component, Sprite Component, Physics Component

## Features/Objective
- Compileable using tcc, gcc, clang (msvc untested)
- Simple API
- Small Codebase, <1000 lines (for now).
- Fast (as much as can be measures uing simple benchmarks in test.c)
- Exclusive/Inclusive systems
    * Exclusive systems iterate over the entities that only have the system's components. Inclusive system iterate over entities that may have components other than the system's.
- Be _mostly_ [STB-style](https://github.com/nothings/stb/blob/master/docs/stb_howto.txt).
- Free and Open Source

## Alternative ECS C/C++ libraries
- [flecs (C99/C++)](https://github.com/SanderMertens/flecs)
- [entt (C++)](https://github.com/skypjack/entt)
- [gamedev_libraries](https://github.com/raizam/gamedev_libraries)
- [stb list of single header libraries](https://github.com/nothings/single_file_libs)
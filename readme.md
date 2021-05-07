# tnecs (Tiny nECS)

Tiny C99 Entity-Component-System (ECS) library.
Originally created for use in a game I am developping: [Codename Firesaga](https://gitlab.com/Gabinou/firesagamaker). Title pending. 
ECSs are an alternative way to organize data and functions to Object-Oriented programming (OOP).

* OOP: Objects/Classes contain data and methods. 
Methods act on objects. 
Children classes inherit methods and data structure from parents. 

* ECS: Components are purely data.
Any number of components can be attached to an entity.
Entities are acted upon by systems. 

In tnecs, an entity is an uint64_t index. 
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

## Motivation
Make the _simplest possible_ ECS library, only with the _minimum necessary features_.
Be compileable with ```tcc```.

## Features
- Compileable using ```tcc```, ```gcc```, ```clang``` (```msvc``` untested)
- Runs on Windows, Linux (Manjaro), Android (termux)
- Simple C99 API
- Small Codebase, <2000 lines for now.
- Fast, see simple benchmarks in test.c
- Free and Open Source

## To do
- World updating function
- Exclusive/Inclusive systems
    * Exclusive systems iterate over the entities that only have the system's components. Inclusive system iterate over entities that may have components other than the system's.
- Post V1.0 pruning

## Alternative ECS/Gamedev libraries for C/C++
- [flecs (C99/C++)](https://github.com/SanderMertens/flecs)
- [entt (C++)](https://github.com/skypjack/entt)
- [gamedev_libraries](https://github.com/raizam/gamedev_libraries)
- [stb list of single header libraries](https://github.com/nothings/single_file_libs)

# tnecs Tutorial

## Initializing the world
```c
    struct tnECS_World * tnecs_world = tnecs_init();
```
The world contains everything tnecs needs.

## Entity creation/destruction
```c
    tnecs_entity_t Silou = tnecs_new_entity(test_world);
    tnecs_entity_t Pirou = TNECS_NEW_ENTITY(world);
```
```tnecs_entity_t``` is a ```uint64_t``` index. 

Entity 0 is always reserved for NULL.

Entities can be created with any number of components directly with this variadic macro: 
```c
    tnecs_entity_t Perignon = TNECS_NEW_ENTITY_WCOMPONENTS(test_world, Position, Unit);
```
## Register Component to the world
A component is a user-defined struct:
```c
    typedef struct Position {
        uint32_t x;
        uint32_t y;
    } Position;

    TNECS_REGISTER_COMPONENT(test_world, Position);
```
Then, the component's type and id can be obtained using:
```c
    tnecs_component_t Position_flag = TNECS_COMPONENT_TYPE(test_world, Position); 
    size_t Position_id = TNECS_COMPONENT_ID(test_world, Position);
```
```tnecs_component_t``` is a ```uint64_t``` integer, used as a bitflag: each component_flag has a one bit set, at component_id location. 

NOTE: type/flag are used interchangeably for a ```uint64_t``` only with one bit set i.e. for a component type/flag. Typeflag refers to a ```uint64_t``` bitflag with any number of set bits i.e. for system typeflags. 

This implies that a maximal number of 63 components can be registered. 
The relation between component ids and flags is:
```c
    Position_flag == (1 << (Position_id - 1));
    Position_id == ((tnecs_component_t)(log2(Position_id) + 1.1f));  // casting to int truncates to 0
```
which are accessible through the macros:
```c
    Position_id == TNECS_COMPONENT_ID2TYPE(Position_flag);
    Position_flag == TNECS_COMPONENT_TYPE2ID(Position_id);
```

When registered, the component names are stringified, then hashed with tnecs_hash_djb2 and saved in ```tnecs_world->component_hashes```.
Any component's id is also its index in ```world->component_hashes```.

You can get a component id with:
```c
    TNECS_COMPONENT_NAME2ID(tnecs_world, Position);
```
```TNECS_COMPONENT_NAME2ID``` wraps around ```tnecs_component_name2id``` by stringifying the "Position" token, so you can also write:
```c
    tnecs_component_name2id(tnecs_world, "Position");
```
Or, if you wish:
```c
    tnecs_component_hash2id(tnecs_world, tnecs_hash_djb2("Position"));
```


## Attach Components to Entities
```c 
    TNECS_ADD_COMPONENT(test_world, Silou, Position);
```
```c 
    struct Position * pos_Silou = TNECS_GET_COMPONENT(test_world, Silou, Position);
```
## Register System to the world
A system is a user-defined function, with ```struct tnecs_System_Input``` as input:
```c
    void SystemMove(struct tnecs_System_Input in_input) {
        Position *p = TNECS_COMPONENTS_LIST(entity_list, Position);
        Unit *v = TNECS_COMPONENTS_LIST(entity_list, Unit);

        for (int i = 0; i < in_input->entity_num; i++) {
            p[i].x += 2;
            p[i].y += 4;
        }
    }

    TNECS_REGISTER_SYSTEM(test_world, SystemMove, TNECS_PHASE_PREUPDATE, true, Position, Unit); 
```
System_id 0 is always reserved for NULL.
```c
```
## Iterating over Entities in a System
```c
```
## Updating the world
```c
```

## Credits
Gabriel Taillon

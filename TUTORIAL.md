

## Initializing the world
```c
    struct tnecs_World * world = tnecs_world_genesis();
```
The world contains everything tnecs needs.
Use ```tnecs_world_destroy``` to free all memory.

## Entity creation/destruction
```c
    tnecs_entity_t Silou = tnecs_entity_create(world);
    tnecs_entity_t Pirou = TNECS_ENTITY_CREATE(world);
```
```tnecs_entity_t``` is a ```uint64_t``` index. 

Entity index 0 is reserved for ```TNECS_NULL```.
Entities can be destroyed with ```tnecs_entity_destroy```, which frees all associated components.

Entities can be created with an index:
```c
    tnecs_entity_t Silou = tnecs_entity_create_windex(world, 100);
    tnecs_entity_t Pirou = TNECS_ENTITY_CREATE(world, 101);
```
```TNECS_ENTITY_CREATE``` is an overloaded macro.

Entities can be created in batches, with indices:
```c
    TNECS_ENTITIES_CREATE(world, 100);
    tnecs_entities_create(world, 100);
    tnecs_entity_t to_create[100] = ...;
    TNECS_ENTITIES_CREATE(world, 100, to_create);
    tnecs_entities_create_windices(world, 100, to_create);
```
```TNECS_NEW_ENTITIES``` is also an overloaded macro.

## Register Component to the world
A component is a user-defined struct:
```c
    typedef struct Position {
        uint32_t x;
        uint32_t y;
    } Position;

    TNECS_REGISTER_COMPONENT(world, Position);
```
When registered, the component names are stringified, then stored at ```world->component_names[component_id]```, then hashed with ```TNECS_HASH``` and stored at ```world->component_hashes[component_id]```.
```TNECS_HASH``` is an alias for ```tnecs_hash_djb2``` by default.

```tnecs_component_t``` is an ```uint64_t``` integer, used as a bitflag: each component type only has one bit set, at ```component_id``` location. 
Component index 0 is reserved for the NULL component.
For now, this implies that a maximal number of 63 components can be registered.

NOTE: type and flag are used interchangeably for an ```uint64_t``` only with one bit set i.e. component type or flag.
A typeflag refers to a ```uint64_t``` bitflag with any number of set bits i.e. system typeflags. 

The component's type can be obtained with:
```c
    tnecs_component_t Position_flag = TNECS_COMPONENT_TYPE(world, Position); 
```

The relation between component indices and flags is:
```c
    Position_flag == (1 << (Position_id - 1));
    Position_id == ((tnecs_component_t)(log2(Position_flag) + 1.1f));  // casting to int truncates to 0
```
which are accessible through the macros:
```c
    Position_id == TNECS_COMPONENT_ID2TYPE(Position_flag);
    Position_flag == TNECS_COMPONENT_TYPE2ID(Position_id);
```

You can get a component index with:
```c
    TNECS_COMPONENT_NAME2ID(world, Position);
```
```TNECS_COMPONENT_NAME2ID``` wraps around ```tnecs_component_name2id``` by stringifying the ```Position``` token, so you can also write:
```c
    tnecs_component_name2id(world, "Position");
```
Or, if you wish:
```c
    tnecs_component_hash2id(world, TNECS_HASH("Position"));
```

## Add Components to Entities
```c 
    TNECS_ADD_COMPONENT(world, Silou, Position);
```
By default, tnecs checks if the entity typeflag is new, when the new component is added.
```TNECS_ADD_COMPONENT``` is an overloaded macro, so you can specify if the typeflag is not new and skip a check for better performance:

```c
    bool isNew = false;
    TNECS_ADD_COMPONENT(world, Silou, Position, isNew);
```
Multiple components can also be added at once:
```c
    bool isNew = false;
    TNECS_ADD_COMPONENTS(world, Pirou, isNew, Position, Velocity);
```
```TNECS_ADD_COMPONENTS``` is NOT an overloaded macro.

You can get a pointer to the component:
```c 
    struct Position * pos_Silou = TNECS_GET_COMPONENT(world, Silou, Position);
    pos_Silou->x += 1;
    pos_Silou->y += 2;
```
By default, all component bits are set to zero with ```calloc```.

Entities can be created with any number of components directly with this variadic macro: 
```c
    tnecs_entity_t Perignon = TNECS_ENTITY_CREATE_wCOMPONENTS(world, Position, Unit);
```
```TNECS_NEW_ENTITY_WCOMPONENTS``` wraps around the variadic function ```tnecs_new_entity_wcomponents``` by counting the number of input components and hashing their names. So you can also write, if you wish:

```c
    tnecs_entity_t Perignon = tnecs_new_entity_wcomponents(world, 2, TNECS_HASH("Position"), TNECS_HASH("Unit"));
```

## Register System to the world
A system is a user-defined function, with a ```struct * tnecs_System_Input``` pointer as input and no output:
```c
    void SystemMove(tnecs_system_input_t * in_input) {
        Position *p = TNECS_COMPONENTS_LIST(in_input, Position);
        Velocity *v = TNECS_COMPONENTS_LIST(in_input, Velocity);

        for (int i = 0; i < in_input->entity_num; i++) {
            p[i].x += v[i].vx * in_input->deltat;
            p[i].y += v[i].vy * in_input->deltat;
        }
    }
    TNECS_REGISTER_SYSTEM(world, SystemMove, Position, Unit); 
```
System index 0 is reserved for NULL. 

Phases are greater than zero ```uint8_t``` integers that can be defined any way one wishes, though I suggest using an ```enum```:
Default phase is 0, the NULL phase, which always runs first.
Other phases run in order of their phase id. 

Inside each phase, each system is run first come first served.
This order can be changed with ```tnecs_system_order_switch```.

By default, systems are inclusive, meaning that entities that have additional components to the system's are also run by it. 
Inclusive systems are run once for every compatible supertype of the system typeflag, in the order saved in ```systems_torun``` in the ```world``` after each step.
If the system is set to exclusive, it runs only one time for the entities that only have exactly the system's components.

Systems can be registered directly with a phase and exclusivity:
```c
enum SYSTEM_PHASES {
    SYSTEM_PHASE_NULL = 0,
    SYSTEM_PHASE_PRE = 1,
    SYSTEM_PHASE_MID = 2,
    SYSTEM_PHASE_POST = 3,
};
    TNECS_REGISTER_SYSTEM_wPHASE(world, SystemMove, SYSTEM_PHASE_PRE, Position, Unit); 
    bool isExclusive = true;
    TNECS_REGISTER_SYSTEM_wEXCL(world, SystemMove, isExclusive, Position, Unit); 
    TNECS_REGISTER_SYSTEM_wPHASE_wEXCL(world, SystemMove, MYPHASE, isExclusive, Position, Unit); 

```

## Updating the world
```c
tnecs_time_ns_t frame_deltat;
tnecs_world_step(world, frame_deltat, NULL);
```
The frame time is the ```deltat``` member in ```tnecs_system_input_t```, accessible from inside registered systems.

## Error Handling
Upon an error, functions/macros that output values fail return 0, and ```NULL``` if the output is a pointer.
Otherwise, they return the value e.g. ```tnecs_create_entity``` returns the entity index or equivalents that evaluate to ```true```.
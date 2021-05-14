
## Initializing the world
```c
    struct tnecs_World * game_world = tnecs_init();
```
The world contains everything tnecs needs.

## Entity creation/destruction
```c
    tnecs_entity_t Silou = tnecs_new_entity(game_world);
    tnecs_entity_t Pirou = TNECS_NEW_ENTITY(world);
```
```tnecs_entity_t``` is a ```uint64_t``` index. 

Entity 0 is always reserved for NULL.

## Register Component to the world
A component is a user-defined struct:
```c
    typedef struct Position {
        uint32_t x;
        uint32_t y;
    } Position;

    TNECS_REGISTER_COMPONENT(game_world, Position);
```
When registered, the component names are stringified, then hashed with ```TNECS_HASH``` and stored at ```game_world->component_hashes[component_id]```.
```TNECS_HASH``` is an alias for ```tnecs_hash_djb2``` by default.

```tnecs_component_t``` is a ```uint64_t``` integer, used as a bitflag: each component_flag only has one bit set, at component_id location. For now, this implies that a maximal number of 63 components can be registered.

NOTE: type/flag are used interchangeably for a ```uint64_t``` only with one bit set i.e. for a component type/flag. Typeflag refers to a ```uint64_t``` bitflag with any number of set bits i.e. for system typeflags. 

The component's type can be obtained with:
```c
    tnecs_component_t Position_flag = TNECS_COMPONENT_TYPE(game_world, Position); 
```

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

You can get a component id with:
```c
    TNECS_COMPONENT_NAME2ID(game_world, Position);
```
```TNECS_COMPONENT_NAME2ID``` wraps around ```tnecs_component_name2id``` by stringifying the ```Position``` token, so you can also write:
```c
    tnecs_component_name2id(game_world, "Position");
```
Or, if you wish:
```c
    tnecs_component_hash2id(game_world, TNECS_HASH("Position"));
```

## Attach Components to Entities
```c 
    TNECS_ADD_COMPONENT(game_world, Silou, Position);
```
```c 
    struct Position * pos_Silou = TNECS_GET_COMPONENT(game_world, Silou, Position);
    pos_Silou->x += 1;
    pos_Silou->y += 2;
```
By default, all component bits are set to zero with ```calloc```.

Entities can be created with any number of components directly with this variadic macro: 
```c
    tnecs_entity_t Perignon = TNECS_NEW_ENTITY_WCOMPONENTS(game_world, Position, Unit);
```
```TNECS_NEW_ENTITY_WCOMPONENTS``` wraps around the variadic function ```tnecs_new_entity_wcomponents``` by counting the number of input compotents and hashing their names. So you can also write, if you wish:

```c
    tnecs_entity_t Perignon = tnecs_new_entity_wcomponents(game_world, 2, TNECS_HASH("Position"), TNECS_HASH("Unit"));
```

## Register System to the world
A system is a user-defined function, with ```struct tnecs_System_Input``` as input:
```c
    void SystemMove(tnecs_system_input_t in_input) {
        Position *p = TNECS_COMPONENTS_LIST(entity_list, Position);
        Velocity *v = TNECS_COMPONENTS_LIST(entity_list, Velocity);

        for (int i = 0; i < in_input->entity_num; i++) {
            p[i].x += v[i].vx;
            p[i].y += v[i].vy;
        }
    }

    TNECS_REGISTER_SYSTEM(game_world, SystemMove, Position, Unit); 
```
System_id 0 is always reserved for NULL. By default, the system phase is set to 0, which is also reserved for the NULL phase. ```tnecs_system_input_t``` is alias for struct tnecs_System_Input.

Phases are ```size_t``` integers can be defined any way one wishes, though I suggest using an ```enum```:
```c
enum SYSTEM_PHASES {
    SYSTEM_PHASE_NULL = 0,
    SYSTEM_PHASE_PRE = 1,
    SYSTEM_PHASE_MID = 2,
    SYSTEM_PHASE_POST = 3,
};

TNECS_REGISTER_SYSTEM_WPHASE(game_world, SystemMove, SYSTEM_PHASE_PRE, Position, Unit); 
```

## Updating the world
```c
tnecs_time_ns_t frame_deltat;
game_world_progress(game_world, frame_deltat);
```
```game_world_progress``` computes previous frame ```deltat``` if 0 is inputted. The frame time is the ```deltat``` member in ```tnecs_system_input_t```, accessible from inside registered systems.
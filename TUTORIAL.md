
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

## Register Component to the world
A component is a user-defined struct:
```c
    typedef struct Position {
        uint32_t x;
        uint32_t y;
    } Position;

    TNECS_REGISTER_COMPONENT(test_world, Position);
```
When registered, the component names are stringified, then hashed with ```TNECS_HASH``` and stored at ```tnecs_world->component_hashes[component_id]```.
```TNECS_HASH``` is an alias for ```tnecs_hash_djb2``` by default.

```tnecs_component_t``` is a ```uint64_t``` integer, used as a bitflag: each component_flag only has one bit set, at component_id location. For now, this implies that a maximal number of 63 components can be registered.

NOTE: type/flag are used interchangeably for a ```uint64_t``` only with one bit set i.e. for a component type/flag. Typeflag refers to a ```uint64_t``` bitflag with any number of set bits i.e. for system typeflags. 

The component's type can be obtained with:
```c
    tnecs_component_t Position_flag = TNECS_COMPONENT_TYPE(test_world, Position); 
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
    TNECS_COMPONENT_NAME2ID(tnecs_world, Position);
```
```TNECS_COMPONENT_NAME2ID``` wraps around ```tnecs_component_name2id``` by stringifying the ```Position``` token, so you can also write:
```c
    tnecs_component_name2id(tnecs_world, "Position");
```
Or, if you wish:
```c
    tnecs_component_hash2id(tnecs_world, TNECS_HASH("Position"));
```

## Attach Components to Entities
```c 
    TNECS_ADD_COMPONENT(test_world, Silou, Position);
```
```c 
    struct Position * pos_Silou = TNECS_GET_COMPONENT(test_world, Silou, Position);
    pos_Silou->x += 1;
    pos_Silou->y += 2;
```
By default, all component bits are set to zero with ```calloc```.

Entities can be created with any number of components directly with this variadic macro: 
```c
    tnecs_entity_t Perignon = TNECS_NEW_ENTITY_WCOMPONENTS(test_world, Position, Unit);
```
```TNECS_NEW_ENTITY_WCOMPONENTS``` wraps around the variadic function ```tnecs_new_entity_wcomponents``` by counting the number of input compotents and hashing their names. So you can also write, if you wish:

```c
    tnecs_entity_t Perignon = tnecs_new_entity_wcomponents(test_world, 2, TNECS_HASH("Position"), TNECS_HASH("Unit"));
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
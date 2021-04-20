# tnECS Tutorial

## Initializing the world
```c
    struct tnECS_World * tnecs_world = tnecs_init();
```

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
Then, the component's typeflag and id can be obtained using:
```c
    tnecs_component_t Position_flag = TNECS_COMPONENT_TYPEFLAG(test_world, Position); 
    size_t Position_id = TNECS_COMPONENT_ID(test_world, Position);
```
```tnecs_component_t``` is a ```uint64_t``` integer, used as a bitflag: each component_flag has a one bit set, at component_id location. So the following is always true:
```c
    Position_flag == (1 << (Position_id - 1));
    Position_flag == TNECS_COMPONENT_ID2TYPEFLAG(Position_id);
```
When registered, the component names are stringified, then hashed with hash_djb2 and saved in ```tnecs_world->component_hashes```.
Any component's id is also its index in ```world->component_hashes```.

You can get a component id with:
```c
TNECS_COMPONENT_NAME2ID(tnecs_world, Position);
```
TNECS_COMPONENT_NAME2ID wraps around tnecs_component_name2id by stringifying the "Position" token, so you can also write:
```c
tnecs_component_name2id(tnecs_world, "Position");
```
Or, if you wish:
```c
tnecs_component_hash2id(tnecs_world, hash_djb2("Position"));
```
A maximal number of 63 components can be registered.

## Attach Components to Entities
```c 
    TNECS_ADD_COMPONENT(test_world, Silou, Position);
```
Entities can be created with any number of components directly. 
```c
    tnecs_entity_t Perignon = TNECS_NEW_ENTITY_WCOMPONENTS(test_world, Position, Unit);
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
System id 0 is always reserved for NULL.
```c
```
## Iterating over Entities in a System
```c
```
## Updating the world
```c
```




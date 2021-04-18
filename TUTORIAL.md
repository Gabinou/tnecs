# tnECS Tutorial

## Initializing the world
```c
    struct tnECS_World * tnecs_world = tnecs_init();
```

## Entity creation/destruction
```c
```
tnecs_entity_t is a uint64_t index. 
Entity 0 is always reserved for NULL.

## Register Component to the world
```c
```
tnecs_component_t is a uint64_t flag: each component has a one bit set, at component_id location.
Component names are stringified, then hashed with hash_djb2 and saved in tnecs_world->component_hashes.
Any component's id is also its index in component_hashes.

A component is a user-defined struct:
```c
typedef struct Position {
    uint32_t x;
    uint32_t y;
} Position;
```

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
A maximal number of 64 components can be registered.

## Attach Components to Entities
```c
```
## Register System to the world
A system is a user-defined function:
```c
void SystemMove(struct tnecs_System_Input in_input) {
    Position *p = TNECS_COMPONENTS_LIST(entity_list, Position);
    Unit *v = TNECS_COMPONENTS_LIST(entity_list, Unit);

    for (int i = 0; i < in_input->entity_num; i++) {
        p[i].x += 2;
        p[i].y += 4;
    }
}
TNECS_REGISTER_SYSTEM()
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




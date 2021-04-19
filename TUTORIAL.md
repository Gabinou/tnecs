# tnECS Tutorial



## Initializing the world
```c
    struct tnECS_World * test_world = tnecs_init();
```

## Entity creation/destruction
```c
```
tnecs_entity_t is a uint64_t index. 

## Register Component to the world
```c
```
tnecs_component_t is a uint64_t flag: each component has a one bit set, at component_id location.
Component names stringified, then hashed with hash_djb2 and saved in component_hashes.
Any component's id is its index in component_hashes.

You can get a component id with:
```c
TNECS_COMPONENT_NAME2ID(test_world, Position);
```
TNECS_COMPONENT_NAME2ID wraps around tnecs_component_name2id by stringifying the "Position" token, so you can also write:
```c
tnecs_component_name2id(test_world, "Position");
```
Or, if you wish:
```c
tnecs_component_hash2id(test_world, hash_djb2("Position"));
```

## Attach components to entities
```c
```
## Register System to the world
```c
```
## Iteratung over entities in a system
```c
```
## Updating the world
```c
```




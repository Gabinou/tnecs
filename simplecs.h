#ifndef SIMPLECS_H
#define SIMPLECS_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
// #define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

typedef uint64_t simplecs_entity_t;
#define SIMPLECS_NULLENTITY 0
#define OPEN_IDS_BUFFER 128

struct Simplecs_World {
	simplecs_entity_t key; // id
    simplecs_entity_t * value; // component list
} * simplecs_world;

struct Simplecs_World * simplecs_init();

simplecs_entity_t opened_entity_ids[OPEN_IDS_BUFFER];
uint8_t num_opened_entity_ids;

simplecs_entity_t next_component_id; // ]0,  UINT16_MAX]
simplecs_entity_t next_entity_id; // ]UINT16_MAX,  UINT64_MAX]

void ** component_tables;
#define SIMPLECS_REGISTER_COMPONENT(name) struct Component_##name {\
    simplecs_entity_t key;\
    name * value;\
} component_##name;\
const Component_##name##_id = next_component_id++;\
arrput(component_tables, &component_##name);
// Error if component registered twice -> user responsibility

#define SIMPLECS_HAS_COMPONENT(name, entity_id) (hmget(Component_##name, entity_id) != NULL)
#define SIMPLECS_GET_COMPONENT(name, entity_id) hmget(Component_##name, entity_id)
#define SIMPLECS_GET_COMPONENT_TABLE(name) component_tables[Component_##name##id]

#define SIMPLECS_ADD_COMPONENT(world, name, entity_id) hmput(world, entity_id, NULL)
// hmget(world, entity_id)
// #define SIMPLECS_ADD_COMPONENT(world, name, entity_id) arrput(hmget(world, entity_id), Component_##name##_id);
// malloc(hmget(Component_##name, entity_id), sizeof(Component_##name))

simplecs_entity_t simplecs_new_entity(struct Simplecs_World * in_world);
simplecs_entity_t simplecs_entity_create(struct Simplecs_World * in_world);
simplecs_entity_t simplecs_entity_destroy(struct Simplecs_World * in_world, simplecs_entity_t in_entity);

#endif // SIMPLECS
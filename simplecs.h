
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

typedef uint64_t simplecs_entity_t;

struct Simplecs_World {
	simplecs_entity_t id;
    bool * used;
} simplecs_world;

#define OPEN_IDS_BUFFER 128
simplecs_entity_t open_entity_ids[OPEN_IDS_BUFFER];
uint8_t num_open_entity_ids = 0;

simplecs_entity_t next_component_id = 1; // 16 bits from 0 exclusive
simplecs_entity_t next_entity_id = UINT16_MAX + 1; // 48 bits from UINT16_MAX exclusive  

#define SIMPLECS_REGISTER_COMPONENT(name) struct Component_##name {\
    simplecs_entity_t id;\
    name * component;\
} component_##name;\
const COMPONENT_##name##_id = next_component_id++;

#define SIMPLECS_HAS_COMPONENT(name, entity_id) (hmget(component_##name, entity_id) != NULL)
#define SIMPLECS_GET_COMPONENT(name, entity_id) hmget(component_##name, entity_id);


simplecs_entity_t simplecs_new_entity(world);
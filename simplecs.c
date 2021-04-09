
#include "simplecs.h"

// Simplecs (pronounced simplex) is a very simple implementation of an entity-component-system (ECS)
// ECS is very useful in game programming.
// OOP: objects and methods, children inheriting from parents, etc.
// ECS: Entities can have any number of independent components, acted upon by systems
// Example: Videogame
//      -> main character: Physics component, PlayerControlled Component
//      -> enemies: Physics component, AIControlled Component
//      -> environment tiles: Destroyable Component
// Entities are indices (uint64_t)
// Component are structures
// Systems are functions
// The main loop iterates over systems
// There can be only one world.

uint8_t num_opened_entity_ids = 0;
simplecs_entity_t next_component_id = COMPONENT_ID_START; // ]0,  UINT16_MAX]
simplecs_entity_t next_entity_id = ENTITY_ID_START; // ]UINT16_MAX,  UINT64_MAX]

struct Simplecs_World * simplecs_init() {
    simplecs_entity_t temp = SIMPLECS_NULLENTITY;
    simplecs_entity_t * temp_value = NULL;
    simplecs_entity_t * temp_array = NULL;
    // struct Simplecs_World * simplecs_world = NULL;
    simplecs_world = NULL;
    // hmdefault(in_world, temp_value);
    // hmput(in_world, temp, temp_array);
    // hmput(in_world, temp+1, temp_array);
    // hmput(in_world, temp+2, temp_array);
    // hmput(in_world, temp+3, temp_array);
    // hmput(in_world, temp+4, temp_array);
    // hmput(in_world, temp+5, temp_array);
    arrput(component_tables, NULL);
    return (simplecs_world);
}

simplecs_entity_t simplecs_new_entity(struct Simplecs_World * in_world) {
    simplecs_entity_t out = 0;
    simplecs_entity_t * components_list;
    while ((out == 0) && (num_opened_entity_ids > 0)) {
        out = opened_entity_ids[--num_opened_entity_ids];
        opened_entity_ids[num_opened_entity_ids] = 0;
    }
    // printf("out %d\n", out);
    if (out == 0) {
        // next_entity_id++;
        out = next_entity_id++;
    }
    simplecs_entity_t temp = DEFAULT_COMPONENT_CAP;
    components_list = hmget(in_world, out);
    // printf("HERE1\n");
    if (components_list != NULL) {
        arrfree(components_list);
    }
    // printf("HERE2\n");
    components_list = NULL;
    // printf("HERE3\n");
    arrsetcap(components_list, temp);
    // printf("components_list %p\n", components_list);
    // printf("in_world %p\n", in_world);
    // printf("out %d\n", out);
    // printf("HERE4\n");
    hmput(in_world, out, components_list);
    // printf("HERE5\n");
    return (out);
}

simplecs_entity_t simplecs_entity_destroy(struct Simplecs_World * in_world, simplecs_entity_t in_entity) {
    simplecs_entity_t * components_list = hmget(in_world, in_entity);
    arrfree(components_list);
    hmput(in_world, in_entity, NULL);
    if (num_opened_entity_ids < OPEN_IDS_BUFFER) {
        opened_entity_ids[num_opened_entity_ids++] = in_entity;
    }
}


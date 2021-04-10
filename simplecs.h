#ifndef SIMPLECS_H
#define SIMPLECS_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
// #define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

typedef uint64_t simplecs_entity_t;
typedef uint16_t simplecs_system_t;
#define SIMPLECS_NULLENTITY 0
#define OPEN_IDS_BUFFER 128
#define DEFAULT_COMPONENT_CAP 10
#define COMPONENT_ID_START 1
#define ENTITY_ID_START UINT16_MAX + 1
#define DEFAULT_COMPONENT_NUM 4

struct Simplecs_System_Input{
    simplecs_entity_t * entities_list;
    size_t entities_num;
}

struct Simplecs_Systems_Table {
   void (**systems_list) (struct Simplecs_System_Input system_input);
   simplecs_entity_t ** components_lists;
   size_t * components_num;
}

// Should the world contain the next indices, the components lists and system lists? i think so. 
struct Simplecs_Entities_Table{
    simplecs_entity_t key; // id
    simplecs_entity_t * value; // components_list
};

struct Simplecs_World {
    struct Simplecs_Entities_Table entities_table;
    struct Simplecs_Systems_Table systems_table;

    simplecs_entity_t next_component_id; // ]0,  UINT16_MAX]
    simplecs_entity_t next_entity_id; // ]UINT16_MAX,  UINT64_MAX]
    simplecs_entity_t opened_entity_ids[OPEN_IDS_BUFFER];
    uint8_t num_opened_entity_ids;
     // Systems don't get destroyed
    simplecs_system_t next_system_id; //[0,...]
void ** component_tables;
} 

struct Simplecs_World * simplecs_init();

#define SIMPLECS_REGISTER_COMPONENT(name) struct Component_##name {\
    simplecs_entity_t key;\
    name * value;\
} * component_##name;\
component_##name = NULL;\
const simplecs_entity_t Component_##name##_id = next_component_id++;\
arrput(component_tables, &component_##name);
// Error if component registered twice -> user responsibility

#define SIMPLECS_HAS_COMPONENT(name, entity_id) (hmget(Component_##name, entity_id) != NULL)
#define SIMPLECS_GET_COMPONENT(name, entity_id) hmget(component_##name, entity_id)
#define SIMPLECS_GET_COMPONENT_TABLE(name) (name*)component_tables[Component_##name##_id]

// #define SIMPLECS_ADD_COMPONENT(world, name, entity_id) _SIMPLECS_ADD_COMPONENT(world, name, entity_id)
#define SIMPLECS_ADD_COMPONENT(world, name, entity_id) arrput(hmget(world, entity_id), Component_##name##_id);\
if (hmget(component_##name, entity_id) != NULL) {\
    free(hmget(component_##name, entity_id));\
}\
hmput(component_##name, entity_id, (name *)calloc(1, sizeof(name)));

simplecs_entity_t simplecs_new_entity(struct Simplecs_World * in_world);
simplecs_entity_t simplecs_entity_destroy(struct Simplecs_World * in_world, simplecs_entity_t in_entity);

// SIMPLECS_REGISTER_SYSTEM
//   1- Takes pointer of system function
//   2- Make associated system index
//   3- Takes list of component indices
#define SIMPLECS_REGISTER_SYSTEM(world, name_sys, when, ...) const simplecs_system_t System_##name_sys##_id = next_system_id++;\

#define SIMPLECS_COMPONENTS_LIST(entity_list, Position)


#endif // SIMPLECS
#ifndef SIMPLECS_H
#define SIMPLECS_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
// #define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

typedef uint64_t simplecs_entity_t;
typedef uint64_t simplecs_entities_t;
typedef uint64_t simplecs_component_t;  // 64 bit flags -> MAX 64 components
typedef uint64_t simplecs_components_t; // 64 bit flags -> MAX 64 components
// component type > 0 -> 1 nonzero bit -> unique for component
// component flag > 0 -> sum of component types -> determines Simplecs_System_Input
// component id > 0 -> unique for component (should be exponent of component type)
typedef uint16_t simplecs_system_t;
typedef uint16_t simplecs_systems_t;

#define SIMPLECS_NULL 0
#define COMPONENT_ID_START 1
#define ENTITY_ID_START 1
#define OPEN_IDS_BUFFER 128
#define STR_BUFFER 128
#define MAX_COMPONENT 63
#define DEFAULT_COMPONENT_NUM 4
#define DEFAULT_SYSTEM_CAP 16
#define DEFAULT_COMPONENT_CAP 64
#define DEFAULT_ENTITY_CAP 128
#define ENTITY_MAX_COMPONENT_NUM 10

// col->x, row->y, depth->z
// components_bytype_3d col->type, row->entity, depth->component
// entitiesbytype_2d  col->type, row->entity
#define index_2d(row, col, col_len) (row * col_len + col)
#define index_3d(row, col, depth, row_len, col_len) (row * col_len * row_len + col * row_len + depth)


#define CONCATENATE(arg1, arg2) CONCATENATE1(arg1, arg2)
#define CONCATENATE1(arg1, arg2) CONCATENATE2(arg1, arg2)
#define CONCATENATE2(arg1, arg2)  arg1##arg2

#define FOREACH_1(macro, x)\
    macro(x)

#define FOREACH_SUM_2(macro, x, ...)\
    macro(x)+\
    FOREACH_1(macro, __VA_ARGS__)

#define FOREACH_SUM_3(macro, x, ...)\
    macro(x)+\
    FOREACH_SUM_2(macro, __VA_ARGS__)

#define FOREACH_SUM_4(macro, x, ...)\
    macro(x)+\
    FOREACH_SUM_3(macro,  __VA_ARGS__)

#define FOREACH_SUM_5(macro, x, ...)\
    macro(x)+\
    FOREACH_SUM_4(macro,  __VA_ARGS__)

#define FOREACH_SUM_6(macro, x, ...)\
    macro(x)+\
    FOREACH_SUM_5(macro,  __VA_ARGS__)

#define FOREACH_SUM_7(macro, x, ...)\
    macro(x)+\
    FOREACH_SUM_6(macro,  __VA_ARGS__)

#define FOREACH_SUM_8(macro, x, ...)\
    macro(x)+\
    FOREACH_SUM_7(macro,  __VA_ARGS__)

#define FOREACH_COMMA_2(macro, x, ...)\
    macro(x),\
    FOREACH_1(macro, __VA_ARGS__)

#define FOREACH_COMMA_3(macro, x, ...)\
    macro(x),\
    FOREACH_COMMA_2(macro, __VA_ARGS__)

#define FOREACH_COMMA_4(macro, x, ...)\
    macro(x),\
    FOREACH_COMMA_3(macro,  __VA_ARGS__)

#define FOREACH_COMMA_5(macro, x, ...)\
    macro(x),\
    FOREACH_COMMA_4(macro,  __VA_ARGS__)

#define FOREACH_COMMA_6(macro, x, ...)\
    macro(x),\
    FOREACH_COMMA_5(macro,  __VA_ARGS__)

#define FOREACH_COMMA_7(macro, x, ...)\
    macro(x),\
    FOREACH_COMMA_6(macro,  __VA_ARGS__)

#define FOREACH_COMMA_8(macro, x, ...)\
    macro(x),\
    FOREACH_COMMA_7(macro,  __VA_ARGS__)

#define VARMACRO_EACH_ARGN(...) VARMACRO_EACH_ARGN_(__VA_ARGS__, VARMACRO_VARG_SEQ())
#define VARMACRO_EACH_ARGN_(...) VARMACRO_ARGN(__VA_ARGS__)
#define VARMACRO_ARGN(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N
#define VARMACRO_VARG_SEQ() 8, 7, 6, 5, 4, 3, 2, 1, 0

#define VARMACRO_FOREACH_SUM_(N, macro, ...) CONCATENATE(FOREACH_SUM_, N)(macro, __VA_ARGS__)
#define VARMACRO_FOREACH_SUM(macro, ...) VARMACRO_FOREACH_SUM_(VARMACRO_EACH_ARGN(__VA_ARGS__), macro, __VA_ARGS__)

#define VARMACRO_FOREACH_COMMA_(N, macro, ...) CONCATENATE(FOREACH_COMMA_, N)(macro, __VA_ARGS__)
#define VARMACRO_FOREACH_COMMA(macro, ...) VARMACRO_FOREACH_COMMA_(VARMACRO_EACH_ARGN(__VA_ARGS__), macro, __VA_ARGS__)

enum RUN_PHASES {
    SIMPLECS_PHASE_PREUPDATE = 0,
    SIMPLECS_PHASE_ONUPDATE = 1,
    SIMPLECS_PHASE_POSTUPDATE = 2,
};

struct Components_Array {
    simplecs_components_t type;   //single bit on
    void * components; // same order as entitiesbytype
};

struct Components_Hash {
    char * key;                  // name
    simplecs_components_t value; // type

};

struct Simplecs_System_Input {
    simplecs_entity_t * entities;
    simplecs_components_t typeflag;
    size_t num;
    size_t * components_order; // Always equal to the total length of components I guess.
    void ** components_lists;
};


struct Simplecs_World {
    simplecs_entity_t * entities;                 // Useless?
    simplecs_components_t * typeflags;            // [typeflag_id]
    simplecs_components_t * entity_typeflags;     // [entity]
    simplecs_components_t * system_typeflags;     // [system]
    bool * system_isExclusive;                    // [system]
    void (** systems)(struct Simplecs_System_Input);
    struct Components_Hash * component_typehash;
    struct Components_Hash * component_id;

    simplecs_entity_t ** entitiesbytype;          // [typeflag_id][num_entitiesbytype]
    simplecs_components_t ** component_idbytype;  // [typeflag_id][num_componentsbytype]
    simplecs_components_t ** component_flagbytype;// [typeflag_id][num_componentsbytype]
    size_t * num_componentsbytype;                // [typeflag_id]
    size_t * num_entitiesbytype;                  // [typeflag_id]
    size_t num_components;
    size_t num_systems;
    size_t num_typeflags;
    struct Components_Array *** components_bytype;  // [typeflag][entity_id][num_componentsbytype]
    simplecs_entity_t next_entity_id; // ]0,  UINT64_MAX]
    simplecs_system_t next_system_id; // [0, ...]

    simplecs_entity_t opened_entity_ids[OPEN_IDS_BUFFER];
    uint8_t num_opened_entity_ids;

    simplecs_component_t temp_typeflag;
    char temp_str[STR_BUFFER];
};

struct Simplecs_World * simplecs_init();

// Error if component registered twice -> user responsibility
#define SIMPLECS_REGISTER_COMPONENT(world, name) _SIMPLECS_REGISTER_COMPONENT(world, name)
#define _SIMPLECS_REGISTER_COMPONENT(world, name) world->temp_typeflag = (1 << world->num_components);\
strncpy(world->temp_str, #name, sizeof(#name));\
hmput(world->component_typehash, world->temp_str, world->temp_typeflag);\
world->num_components++;


// Redundant macro for API consistency
#define SIMPLECS_NEW_ENTITY(world) simplecs_new_entity(in_world)

// SIMPLECS_NEW_ENTITY_WCOMPONENTS's __VA_ARGS__ are user-defined component names/tokens
#define SIMPLECS_NEW_ENTITY_WCOMPONENTS(world,...) simplecs_new_entity_wcomponents(in_world, VARMACRO_FOREACH_SUM(SIMPLECS_COMPONENT_FLAG, __VA_ARGS__))



// UTILITY MACROS
#define SIMPLECS_COMPONENT_ID(name) Component_##name##_id
#define SIMPLECS_COMPONENT_FLAG(name) hmget(world->component_typehash, world->temp_str)
#define SIMPLECS_SYSTEMS_COMPONENTLIST(input, name) (* name)input->components_lists[input->components_order[Component_##name##_id]]


// SIMPLECS_ADD_COMPONENT is overloaded component adder macro
//      3 inputs required: (world, name, entity_id)
//      4th input speeds up if newtype is false
#define GET_ADD_COMPONENT(_1,_2,_3,_4,NAME,...) NAME
#define SIMPLECS_ADD_COMPONENT(...) GET_ADD_COMPONENT(__VA_ARGS__, SIMPLECS_ADD_COMPONENT4, SIMPLECS_ADD_COMPONENT3)(__VA_ARGS__)

#define SIMPLECS_ADD_COMPONENT3(world, name, entity_id) strncpy(world->temp_str, #name, sizeof(#name));\
world->temp_typeflag = (hmget(world->component_typehash, world->temp_str) + world->entity_typeflags[entity_id]);\
if (!simplecs_type_id(world->entity_typeflags, world->num_systems, world->temp_typeflag)) {\
    arrput(world->entity_typeflags, world->entity_typeflags[entity_id]);\
    world->num_typeflags++;\
}\
simplecs_entity_typeflag_change(world, entity_id, world->temp_typeflag)

#define SIMPLECS_ADD_COMPONENT4(world, name, entity_id, newtype) if (newtype) {\
strncpy(world->temp_str, #name, sizeof(#name));\
world->temp_typeflag = (hmget(world->component_typehash, world->temp_str) + world->entity_typeflags[entity_id]);\
if (!simplecs_type_id(world->entity_typeflags, world->num_systems, world->temp_typeflag)) {\
    arrput(world->entity_typeflags, world->entity_typeflags[entity_id]);\
    world->num_typeflags++;\
}\
simplecs_entity_typeflag_change(world, entity_id, world->temp_typeflag);\
}

#define SIMPLECS_REGISTER_SYSTEM(world, pfunc, phase, isexcl, ...) simplecs_register_system(world, pfunc, phase, isexcl, VARMACRO_EACH_ARGN(__VA_ARGS__), VARMACRO_FOREACH_SUM(SIMPLECS_COMPONENT_ID, __VA_ARGS__))
void simplecs_register_system(struct Simplecs_World * in_world, simplecs_entity_t * entities_list, uint8_t in_run_phase, bool isexclusive, size_t component_num, simplecs_components_t component_typeflag);


simplecs_entity_t simplecs_new_entity(struct Simplecs_World * in_world);
simplecs_entity_t simplecs_new_entity_wcomponents(struct Simplecs_World * in_world, simplecs_components_t components_typeflag);
simplecs_entity_t simplecs_entity_destroy(struct Simplecs_World * in_world, simplecs_entity_t in_entity);

void simplecs_new_component(struct Simplecs_World * in_world, simplecs_entity_t in_entity, simplecs_components_t typeflag, simplecs_components_t type_toadd);
void simplecs_new_typeflag(struct Simplecs_World * in_world, simplecs_components_t typeflag);
void simplecs_entity_typeflag_change(struct Simplecs_World * in_world, simplecs_entity_t in_entity, simplecs_components_t new_type);
bool simplecs_type_add(struct Simplecs_World * in_world, simplecs_components_t component_typeflag);
size_t simplecs_type_id(simplecs_components_t * in_typelist, size_t len, simplecs_components_t in_flag);
bool simplecs_componentsbytype_migrate(struct Simplecs_World * in_world, simplecs_entity_t in_entity, simplecs_components_t previous_flag, simplecs_components_t new_flag);
size_t simplecs_issubtype(simplecs_components_t * in_typelist, size_t len, simplecs_components_t in_flag);

#define SIMPLECS_COMPONENTS_LIST(entity_list, Position)

#endif // SIMPLECS
#ifndef TNECS_H
#define TNECS_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
// #define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t tnecs_entity_t;
typedef uint64_t tnecs_entities_t;
typedef uint64_t tnecs_component_t;  // 64 bit flags -> MAX 64 components
typedef uint64_t tnecs_components_t; // 64 bit flags -> MAX 64 components
// component type > 0 -> 1 nonzero bit -> unique for component
// component flag > 0 -> sum of component types -> determines Simplecs_System_Input
// component id > 0 -> unique for component (should be exponent of component type)
typedef uint16_t tnecs_system_t;
typedef uint16_t tnecs_system_t;

#define TNECS_NULL 0
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

#define STRINGIFY(x) #x
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

#define FOREACH_NEWLINE_2(macro, x, ...)\
    macro(x);\
    FOREACH_1(macro, __VA_ARGS__)

#define FOREACH_NEWLINE_3(macro, x, ...)\
    macro(x);\
    FOREACH_NEWLINE_2(macro, __VA_ARGS__)

#define FOREACH_NEWLINE_4(macro, x, ...)\
    macro(x);\
    FOREACH_NEWLINE_3(macro,  __VA_ARGS__)

#define FOREACH_NEWLINE_5(macro, x, ...)\
    macro(x);\
    FOREACH_NEWLINE_4(macro,  __VA_ARGS__)

#define FOREACH_NEWLINE_6(macro, x, ...)\
    macro(x);\
    FOREACH_NEWLINE_5(macro,  __VA_ARGS__)

#define FOREACH_NEWLINE_7(macro, x, ...)\
    macro(x);\
    FOREACH_NEWLINE_6(macro,  __VA_ARGS__)

#define FOREACH_NEWLINE_8(macro, x, ...)\
    macro(x);\
    FOREACH_NEWLINE_7(macro,  __VA_ARGS__)

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

#define FOREACH_S1(macro, x)\
    macro(#x)

#define FOREACH_SCOMMA_2(macro, x, ...)\
    macro(#x),\
    FOREACH_S1(macro, __VA_ARGS__)

#define FOREACH_SCOMMA_3(macro, x, ...)\
    macro(#x),\
    FOREACH_SCOMMA_2(macro, __VA_ARGS__)

#define FOREACH_SCOMMA_4(macro, x, ...)\
    macro(#x),\
    FOREACH_SCOMMA_3(macro,  __VA_ARGS__)

#define FOREACH_SCOMMA_5(macro, x, ...)\
    macro(#x),\
    FOREACH_SCOMMA_4(macro,  __VA_ARGS__)

#define FOREACH_SCOMMA_6(macro, x, ...)\
    macro(#x),\
    FOREACH_SCOMMA_5(macro,  __VA_ARGS__)

#define FOREACH_SCOMMA_7(macro, x, ...)\
    macro(#x),\
    FOREACH_SCOMMA_6(macro,  __VA_ARGS__)

#define FOREACH_SCOMMA_8(macro, x, ...)\
    macro(#x),\
    FOREACH_SCOMMA_7(macro,  __VA_ARGS__)

#define VARMACRO_EACH_ARGN(...) VARMACRO_EACH_ARGN_(__VA_ARGS__, VARMACRO_VARG_SEQ())
#define VARMACRO_EACH_ARGN_(...) VARMACRO_ARGN(__VA_ARGS__)
#define VARMACRO_ARGN(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N
#define VARMACRO_VARG_SEQ() 8, 7, 6, 5, 4, 3, 2, 1, 0

#define VARMACRO_FOREACH_SUM_(N, macro, ...) CONCATENATE(FOREACH_SUM_, N)(macro, __VA_ARGS__)
#define VARMACRO_FOREACH_SUM(macro, ...) VARMACRO_FOREACH_SUM_(VARMACRO_EACH_ARGN(__VA_ARGS__), macro, __VA_ARGS__)

#define VARMACRO_FOREACH_COMMA_(N, macro, ...) CONCATENATE(FOREACH_COMMA_, N)(macro, __VA_ARGS__)
#define VARMACRO_FOREACH_COMMA(macro, ...) VARMACRO_FOREACH_COMMA_(VARMACRO_EACH_ARGN(__VA_ARGS__), macro, __VA_ARGS__)

#define VARMACRO_FOREACH_SCOMMA_(N, macro, ...) CONCATENATE(FOREACH_SCOMMA_, N)(macro, __VA_ARGS__)
#define VARMACRO_FOREACH_SCOMMA(macro, ...) VARMACRO_FOREACH_SCOMMA_(VARMACRO_EACH_ARGN(__VA_ARGS__), macro, __VA_ARGS__)

#define VARMACRO_FOREACH_NEWLINE_(N, macro, ...) CONCATENATE(FOREACH_COMMA_, N)(macro, __VA_ARGS__)
#define VARMACRO_FOREACH_NEWLINE(macro, ...) VARMACRO_FOREACH_NEWLINE_(VARMACRO_EACH_ARGN(__VA_ARGS__), macro, __VA_ARGS__)

enum RUN_PHASES {
    TNECS_PHASE_PREUPDATE = 0,
    TNECS_PHASE_ONUPDATE = 1,
    TNECS_PHASE_POSTUPDATE = 2,
};

struct Components_Array {
    tnecs_components_t type; // single bit on
    void * components;       // same order as entitiesbytype
};

struct Simplecs_System_Input {
    tnecs_entity_t * entities;
    tnecs_components_t typeflag;
    size_t num;
    size_t * components_order; // Always equal to the total length of components I guess.
    void ** components_lists;
};

struct Simplecs_World {
    tnecs_entity_t * entities;                      // Useless?
    tnecs_components_t * typeflags;                 // [typeflag_id]
    tnecs_components_t * entity_typeflags;          // [entity]
    tnecs_components_t * system_typeflags;          // [system]
    bool * system_isExclusive;                      // [system]
    void (** systems)(struct Simplecs_System_Input);// [system]
    uint64_t * component_hashes;                    // [component]

    tnecs_entity_t ** entitiesbytype;               // [typeflag_id][num_entitiesbytype]
    tnecs_components_t ** component_idbytype;       // [typeflag_id][num_componentsbytype]
    tnecs_components_t ** component_flagbytype;     // [typeflag_id][num_componentsbytype]
    size_t * num_componentsbytype;                  // [typeflag_id]
    size_t * num_entitiesbytype;                    // [typeflag_id]
    size_t num_components;
    size_t num_systems;
    size_t num_typeflags;
    struct Components_Array ** components_bytype;  // [typeflag_id][num_componentsbytype]
    tnecs_entity_t next_entity_id; // ]0,  UINT64_MAX]
    tnecs_system_t next_system_id; // [0, ...]

    tnecs_entity_t opened_entity_ids[OPEN_IDS_BUFFER];
    uint8_t num_opened_entity_ids;

    tnecs_component_t temp_typeflag;
    char temp_str[STR_BUFFER];
};
typedef struct Simplecs_World tnecs_world_t;

struct Simplecs_World * tnecs_init();

// Error if component registered twice -> user responsibility
#define TNECS_REGISTER_COMPONENT(world, name) arrput(world->component_hashes, hash_djb2(#name));\
world->num_components++;


#define TNECS_NEW_ENTITY(world) tnecs_new_entity(in_world) // redundancy for API consistency
// TNECS_NEW_ENTITY_WCOMPONENTS's __VA_ARGS__ are user-defined component names/tokens
#define TNECS_NEW_ENTITY_WCOMPONENTS(world,...) tnecs_new_entity_wcomponents(world, tnecs_names2typeflag(world, VARMACRO_EACH_ARGN(__VA_ARGS__), VARMACRO_FOREACH_SCOMMA(hash_djb2, __VA_ARGS__)));

// UTILITY MACROS
#define TNECS_COMPONENT_HASH2ID(world, hash) tnecs_component_hash2id(world, hash)
#define TNECS_COMPONENT_ID(world, name) tnecs_component_hash2id(world, hash_djb2(#name))
#define TNECS_COMPONENT_NAME2TYPEFLAG(world, name) tnecs_names2typeflag(world, 1, #name)
#define TNECS_COMPONENTS_NAMES2TYPEFLAG(world, ...) tnecs_names2typeflag(world, VARMACRO_EACH_ARGN(__VA_ARGS__), VARMACRO_FOREACH_COMMA(STRINGIFY, __VA_ARGS__))
#define TNECS_COMPONENT_IDS2TYPEFLAG(...) tnecs_ids2typeflag(VARMACRO_EACH_ARGN(__VA_ARGS__), VARMACRO_FOREACH_COMMA(STRINGIFY, __VA_ARGS__))
#define TNECS_COMPONENT_NAME2ID(world, name) tnecs_name2id(world, #name)
#define TNECS_COMPONENT_ID2TYPEFLAG(id) (1 << (id - COMPONENT_ID_START))


#define TNECS_SYSTEMS_COMPONENTLIST(input, name) (* name)input->components

// TNECS_ADD_COMPONENT is overloaded component adder macro
//      3 inputs required: (world, name, entity_id)
//      4th input for speed if newtype is false
#define GET_ADD_COMPONENT(_1,_2,_3,_4,NAME,...) NAME
#define TNECS_ADD_COMPONENT(...) GET_ADD_COMPONENT(__VA_ARGS__, TNECS_ADD_COMPONENT4, TNECS_ADD_COMPONENT3)(__VA_ARGS__)

#define TNECS_ADD_COMPONENT3(world, name, entity_id) world->temp_typeflag = TNECS_NAMES2TYPEFLAG(world, name) + world->entity_typeflags[entity_id];\
if (!tnecs_type_id(world->entity_typeflags, world->num_systems, world->temp_typeflag)) {\
    arrput(world->entity_typeflags, world->temp_typeflag);\
    world->num_typeflags++;\
}\
tnecs_entity_typeflag_change(world, entity_id, world->temp_typeflag)

#define TNECS_ADD_COMPONENT4(world, name, entity_id, newtype) if (newtype) {\
strncpy(world->temp_str, #name, sizeof(#name));\
world->temp_typeflag = (hmget(world->component_typehash, world->temp_str) + world->entity_typeflags[entity_id]);\
if (!tnecs_type_id(world->entity_typeflags, world->num_systems, world->temp_typeflag)) {\
    arrput(world->entity_typeflags, world->entity_typeflags[entity_id]);\
    world->num_typeflags++;\
}\
tnecs_entity_typeflag_change(world, entity_id, world->temp_typeflag);\
}

#define TNECS_REGISTER_SYSTEM(world, pfunc, phase, isexcl, ...) tnecs_register_system(world, pfunc, phase, isexcl, VARMACRO_EACH_ARGN(__VA_ARGS__), VARMACRO_FOREACH_SUM(TNECS_COMPONENT_ID, __VA_ARGS__))
void tnecs_register_system(struct Simplecs_World * in_world, 
    uint8_t in_run_phase, bool isexclusive, size_t component_num, tnecs_components_t component_typeflag);


tnecs_entity_t tnecs_new_entity(struct Simplecs_World * in_world);
tnecs_entity_t tnecs_new_entity_wcomponents(struct Simplecs_World * in_world, tnecs_components_t components_typeflag);
tnecs_entity_t tnecs_entity_destroy(struct Simplecs_World * in_world, tnecs_entity_t in_entity);

// UTILITY FUNCTIONS
tnecs_component_t tnecs_name2id(struct Simplecs_World * in_world, const char * in_name);
tnecs_component_t tnecs_names2typeflag(struct Simplecs_World * in_world, uint8_t num, ...);
tnecs_component_t tnecs_ids2typeflag(uint8_t num, ...);


void tnecs_new_component(struct Simplecs_World * in_world, tnecs_entity_t in_entity, tnecs_components_t typeflag, tnecs_components_t type_toadd);
void tnecs_new_typeflag(struct Simplecs_World * in_world, tnecs_components_t typeflag);
void tnecs_entity_typeflag_change(struct Simplecs_World * in_world, tnecs_entity_t in_entity, tnecs_components_t new_type);
bool tnecs_type_add(struct Simplecs_World * in_world, tnecs_components_t component_typeflag);
size_t tnecs_type_id(tnecs_components_t * in_typelist, size_t len, tnecs_components_t in_flag);
bool tnecs_componentsbytype_migrate(struct Simplecs_World * in_world, tnecs_entity_t in_entity, tnecs_components_t previous_flag, tnecs_components_t new_flag);
size_t tnecs_issubtype(tnecs_components_t * in_typelist, size_t len, tnecs_components_t in_flag);

size_t tnecs_component_hash2id(struct Simplecs_World * in_world, uint64_t in_hash);

// STRING HASHING ALGORITHMS
// hash_djb2 slightly faster than hash_sdbm
uint64_t hash_djb2(const unsigned char * str);
uint64_t hash_sdbm(const unsigned char * str);

#ifdef __cplusplus
}
#endif

#endif // TNECS
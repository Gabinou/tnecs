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

// #define TNECS_DEBUG
#ifdef TNECS_DEBUG
#define TNECS_DEBUG_PRINTF(...) do {printf(__VA_ARGS__);}while(0)
#else
#define TNECS_DEBUG_PRINTF(...) (void)0
#endif

// ************************ TYPE DEFINITIONS ****************************
typedef uint64_t tnecs_entity_t;
typedef uint64_t tnecs_entities_t;
typedef uint64_t tnecs_component_t;  // 64 bit flags -> MAX 64 components
typedef uint64_t tnecs_components_t; // 64 bit flags -> MAX 64 components
// component type > 0 -> 1 nonzero bit -> unique for component
// component flag > 0 -> sum of component types -> determines tnecs_System_Input
// component id > 0 -> unique for component (should be exponent of component type)
typedef uint16_t tnecs_system_t;
typedef uint16_t tnecs_system_t;

// ********************** CONSTANT DEFINITIONS ************************
// entity, component, system: XXXX_id zero ALWAYS reserved for NULL
#define TNECS_NULL 0
#define TNECS_NOCOMPONENT_TYPEFLAG 0
#define TNECS_ID_START 1
#define OPEN_IDS_BUFFER 128
#define STR_BUFFER 128
#define MAX_COMPONENT 63
#define DEFAULT_COMPONENT_NUM 4
#define DEFAULT_SYSTEM_CAP 16
#define DEFAULT_COMPONENT_CAP 64
#define DEFAULT_ENTITY_CAP 128
#define ENTITY_MAX_COMPONENT_NUM 10

enum TNECS_RUN_PHASES {
    TNECS_PHASE_PREUPDATE = 0,
    TNECS_PHASE_ONUPDATE = 1,
    TNECS_PHASE_POSTUPDATE = 2,
};

// ****************** HACKY DISTRIBUTION FOR VARIADIC MACROS ****************** 
//   Distribution as in algebra: a(x+b) = ax + ab
//   TNECS_VARMACRO_FOREACH_XXXX(foo, __VA_ARGS__) applies foo to each __VA_ARGS__, PLUS
//      -> _SUM variant puts + after each (except last)
//      -> _SCOMMA variant stringifies and puts commas around each (except last)
//      -> _COMMA puts commas around each (except last)
//      -> _NEWLINE makes newline for each (except last)
//       up to 8 input args. Theoretically up to 63, if all TNECS_FOREACH_XXXX_N exist
//   TNECS_VARMACRO_EACH_ARGN(__VA_ARGS__) counts the number of args
//   -> up to 63, if TNECS_VARMACRO_ARGN and TNECS_VARMACRO_VARG_SEQ lists have up to 63
#define TNECS_STRINGIFY(x) #x
#define TNECS_CONCATENATE(arg1, arg2) TNECS_CONCATENATE1(arg1, arg2)
#define TNECS_CONCATENATE1(arg1, arg2) TNECS_CONCATENATE2(arg1, arg2)
#define TNECS_CONCATENATE2(arg1, arg2) arg1##arg2

#define TNECS_FOREACH_1(macro, x)\
    macro(x)

#define TNECS_FOREACH_SUM_2(macro, x, ...)\
    macro(x)+\
    TNECS_FOREACH_1(macro, __VA_ARGS__)

#define TNECS_FOREACH_SUM_3(macro, x, ...)\
    macro(x)+\
    TNECS_FOREACH_SUM_2(macro, __VA_ARGS__)

#define TNECS_FOREACH_SUM_4(macro, x, ...)\
    macro(x)+\
    TNECS_FOREACH_SUM_3(macro,  __VA_ARGS__)

#define TNECS_FOREACH_SUM_5(macro, x, ...)\
    macro(x)+\
    TNECS_FOREACH_SUM_4(macro,  __VA_ARGS__)

#define TNECS_FOREACH_SUM_6(macro, x, ...)\
    macro(x)+\
    TNECS_FOREACH_SUM_5(macro,  __VA_ARGS__)

#define TNECS_FOREACH_SUM_7(macro, x, ...)\
    macro(x)+\
    TNECS_FOREACH_SUM_6(macro,  __VA_ARGS__)

#define TNECS_FOREACH_SUM_8(macro, x, ...)\
    macro(x)+\
    TNECS_FOREACH_SUM_7(macro,  __VA_ARGS__)

#define TNECS_FOREACH_NEWLINE_2(macro, x, ...)\
    macro(x);\
    TNECS_FOREACH_1(macro, __VA_ARGS__)

#define TNECS_FOREACH_NEWLINE_3(macro, x, ...)\
    macro(x);\
    TNECS_FOREACH_NEWLINE_2(macro, __VA_ARGS__)

#define TNECS_FOREACH_NEWLINE_4(macro, x, ...)\
    macro(x);\
    TNECS_FOREACH_NEWLINE_3(macro,  __VA_ARGS__)

#define TNECS_FOREACH_NEWLINE_5(macro, x, ...)\
    macro(x);\
    TNECS_FOREACH_NEWLINE_4(macro,  __VA_ARGS__)

#define TNECS_FOREACH_NEWLINE_6(macro, x, ...)\
    macro(x);\
    TNECS_FOREACH_NEWLINE_5(macro,  __VA_ARGS__)

#define TNECS_FOREACH_NEWLINE_7(macro, x, ...)\
    macro(x);\
    TNECS_FOREACH_NEWLINE_6(macro,  __VA_ARGS__)

#define TNECS_FOREACH_NEWLINE_8(macro, x, ...)\
    macro(x);\
    TNECS_FOREACH_NEWLINE_7(macro,  __VA_ARGS__)

#define TNECS_FOREACH_COMMA_1(macro, x, ...)\
    macro(x)

#define TNECS_FOREACH_COMMA_2(macro, x, ...)\
    macro(x),\
    TNECS_FOREACH_1(macro, __VA_ARGS__)

#define TNECS_FOREACH_COMMA_3(macro, x, ...)\
    macro(x),\
    TNECS_FOREACH_COMMA_2(macro, __VA_ARGS__)

#define TNECS_FOREACH_COMMA_4(macro, x, ...)\
    macro(x),\
    TNECS_FOREACH_COMMA_3(macro,  __VA_ARGS__)

#define TNECS_FOREACH_COMMA_5(macro, x, ...)\
    macro(x),\
    TNECS_FOREACH_COMMA_4(macro,  __VA_ARGS__)

#define TNECS_FOREACH_COMMA_6(macro, x, ...)\
    macro(x),\
    TNECS_FOREACH_COMMA_5(macro,  __VA_ARGS__)

#define TNECS_FOREACH_COMMA_7(macro, x, ...)\
    macro(x),\
    TNECS_FOREACH_COMMA_6(macro,  __VA_ARGS__)

#define TNECS_FOREACH_COMMA_8(macro, x, ...)\
    macro(x),\
    TNECS_FOREACH_COMMA_7(macro,  __VA_ARGS__)

#define TNECS_FOREACH_S1(macro, x)\
    macro(#x)

#define TNECS_FOREACH_SCOMMA_1(macro, x)\
    macro(#x)

#define TNECS_FOREACH_SCOMMA_2(macro, x, ...)\
    macro(#x),\
    TNECS_FOREACH_S1(macro, __VA_ARGS__)

#define TNECS_FOREACH_SCOMMA_3(macro, x, ...)\
    macro(#x),\
    TNECS_FOREACH_SCOMMA_2(macro, __VA_ARGS__)

#define TNECS_FOREACH_SCOMMA_4(macro, x, ...)\
    macro(#x),\
    TNECS_FOREACH_SCOMMA_3(macro,  __VA_ARGS__)

#define TNECS_FOREACH_SCOMMA_5(macro, x, ...)\
    macro(#x),\
    TNECS_FOREACH_SCOMMA_4(macro,  __VA_ARGS__)

#define TNECS_FOREACH_SCOMMA_6(macro, x, ...)\
    macro(#x),\
    TNECS_FOREACH_SCOMMA_5(macro,  __VA_ARGS__)

#define TNECS_FOREACH_SCOMMA_7(macro, x, ...)\
    macro(#x),\
    TNECS_FOREACH_SCOMMA_6(macro,  __VA_ARGS__)

#define TNECS_FOREACH_SCOMMA_8(macro, x, ...)\
    macro(#x),\
    TNECS_FOREACH_SCOMMA_7(macro,  __VA_ARGS__)

#define TNECS_VARMACRO_EACH_ARGN(...) TNECS_VARMACRO_EACH_ARGN_(__VA_ARGS__, TNECS_VARMACRO_VARG_SEQ())
#define TNECS_VARMACRO_EACH_ARGN_(...) TNECS_VARMACRO_ARGN(__VA_ARGS__)
#define TNECS_VARMACRO_ARGN(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N
#define TNECS_VARMACRO_VARG_SEQ() 8, 7, 6, 5, 4, 3, 2, 1, 0

#define TNECS_VARMACRO_FOREACH_SUM_(N, macro, ...) TNECS_CONCATENATE(TNECS_FOREACH_SUM_, N)(macro, __VA_ARGS__)
#define TNECS_VARMACRO_FOREACH_SUM(macro, ...) TNECS_VARMACRO_FOREACH_SUM_(TNECS_VARMACRO_EACH_ARGN(__VA_ARGS__), macro, __VA_ARGS__)

#define TNECS_VARMACRO_FOREACH_COMMA_(N, macro, ...) TNECS_CONCATENATE(TNECS_FOREACH_COMMA_, N)(macro, __VA_ARGS__)
#define TNECS_VARMACRO_FOREACH_COMMA(macro, ...) TNECS_VARMACRO_FOREACH_COMMA_(TNECS_VARMACRO_EACH_ARGN(__VA_ARGS__), macro, __VA_ARGS__)

#define TNECS_VARMACRO_FOREACH_SCOMMA_(N, macro, ...) TNECS_CONCATENATE(TNECS_FOREACH_SCOMMA_, N)(macro, __VA_ARGS__)
#define TNECS_VARMACRO_FOREACH_SCOMMA(macro, ...) TNECS_VARMACRO_FOREACH_SCOMMA_(TNECS_VARMACRO_EACH_ARGN(__VA_ARGS__), macro, __VA_ARGS__)

#define TNECS_VARMACRO_FOREACH_NEWLINE_(N, macro, ...) TNECS_CONCATENATE(TNECS_FOREACH_COMMA_, N)(macro, __VA_ARGS__)
#define TNECS_VARMACRO_FOREACH_NEWLINE(macro, ...) TNECS_VARMACRO_FOREACH_NEWLINE_(TNECS_VARMACRO_EACH_ARGN(__VA_ARGS__), macro, __VA_ARGS__)

// ************************ TNECS STRUCTS DEFINITIONS *****************************
struct tnecs_Components_Array {
    tnecs_components_t type; // single bit on
    void * components;       // same order as entities_bytype
};

struct tnecs_System_Input {
    tnecs_entity_t * entities;
    tnecs_components_t typeflag;
    size_t num;
    size_t * components_order; // Always equal to the total length of components I guess.
    void ** components_lists;
};

struct tnecs_World {
    tnecs_entity_t * entities;                      // Deleted Entities are 0
    tnecs_components_t * typeflags;                 // [typeflag_id]
    tnecs_components_t * entity_typeflags;          // [entity]
    tnecs_components_t * system_typeflags;          // [system]
    void (** systems)(struct tnecs_System_Input);// [system]
    bool * system_isExclusive;                      // [system_id]
    uint8_t * system_phase;                         // [system_id]
    uint64_t * component_hashes;                    // [component_id]
    uint64_t * system_hashes;                       // [system_id]

    // the by_type array are exclusive.
    struct tnecs_Components_Array ** components_bytype;   // [typeflag_id][num_componentsbytype]
    tnecs_entity_t ** entities_bytype;              // [typeflag_id][num_entitiesbytype]
    tnecs_components_t ** component_idbytype;       // [typeflag_id][num_componentsbytype]
    tnecs_components_t ** component_flagbytype;     // [typeflag_id][num_componentsbytype]
    size_t * num_componentsbytype;                  // [typeflag_id]
    size_t * num_entitiesbytype;                    // [typeflag_id]
    size_t num_components;                          // includes NULL component
    size_t num_systems;
    size_t num_entities;
    size_t num_typeflags;

    tnecs_entity_t next_entity_id; // ]0,  UINT64_MAX]
    tnecs_entity_t opened_entity_ids[OPEN_IDS_BUFFER];
    uint8_t num_opened_entity_ids;

    size_t temp_size;
    tnecs_component_t temp_typeflag;
    size_t temp_id;
    uint64_t temp_hash;
    char temp_str[STR_BUFFER];
};
typedef struct tnecs_World tnecs_world_t;


// ********************* FUNCTIONALITY MACROS AND FUNCTIONS ************************
struct tnecs_World * tnecs_init();

// Error if component registered twice -> user responsibility
#define TNECS_REGISTER_COMPONENT(world, name) tnecs_register_component(world, hash_djb2(#name))

#define TNECS_NEW_ENTITY(world) tnecs_new_entity(in_world) // redundancy for API consistency
// TNECS_NEW_ENTITY_WCOMPONENTS's __VA_ARGS__ are user-defined component names/tokens
#define TNECS_NEW_ENTITY_WCOMPONENTS(world,...) tnecs_new_entity_wcomponents(world, TNECS_VARMACRO_EACH_ARGN(__VA_ARGS__), TNECS_VARMACRO_FOREACH_SCOMMA(hash_djb2, __VA_ARGS__));

// UTILITY MACROS
#define TNECS_HASH(name) hash_djb2(#name)
#define TNECS_NAME2HASH(name) hash_djb2(#name)

#define TNECS_COMPONENT_HASH(name) TNECS_HASH(name)
#define TNECS_COMPONENT_NAME2HASH(name) TNECS_NAME2HASH(name)
#define TNECS_COMPONENT_HASH2ID(world, hash) tnecs_component_hash2id(world, hash)
#define TNECS_COMPONENT_ID(world, name) tnecs_component_hash2id(world, hash_djb2(#name))
#define TNECS_COMPONENT_TYPEFLAG(world, name) tnecs_component_names2typeflag(world, 1, #name)
#define TNECS_COMPONENT_NAME2TYPEFLAG(world, name) TNECS_COMPONENT_TYPEFLAG(world, name)
#define TNECS_COMPONENT_NAMES2TYPEFLAG(world, ...) tnecs_component_names2typeflag(world, TNECS_VARMACRO_EACH_ARGN(__VA_ARGS__), TNECS_VARMACRO_FOREACH_COMMA(TNECS_STRINGIFY, __VA_ARGS__))
#define TNECS_COMPONENT_IDS2TYPEFLAG(...) tnecs_component_ids2typeflag(TNECS_VARMACRO_EACH_ARGN(__VA_ARGS__), TNECS_VARMACRO_FOREACH_COMMA(TNECS_STRINGIFY, __VA_ARGS__))
#define TNECS_COMPONENT_NAME2ID(world, name) tnecs_component_name2id(world, #name)
#define TNECS_COMPONENT_ID2TYPEFLAG(id) (1 << (id - TNECS_ID_START))

#define TNECS_SYSTEM_HASH(name) TNECS_NAME2HASH(name)
#define TNECS_SYSTEM_NAME2HASH(name) TNECS_NAME2HASH(name)
#define TNECS_SYSTEM_ID(world, name) tnecs_system_name2id(world, #name)
#define TNECS_SYSTEM_ID2TYPEFLAG(world, id) world->system_typeflags[id]
#define TNECS_SYSTEM_TYPEFLAG(world, name) tnecs_system_name2typeflag(world, #name)
#define TNECS_SYSTEM_NAME2TYPEFLAG(world, name) TNECS_SYSTEM_TYPEFLAG(world, name)
#define TNECS_SYSTEMS_COMPONENTLIST(input, name) (* name)input->components

#define TNECS_COMPONENTARRAY_ADD(world, name, entity, typeflag) world->temp_typeflag = tnecs_component_names2typeflag(world, 1, #name)
// world->temp_id = tnecs_component_hash2id(world, hash_djb2(#name));\
// for (size_t i = 0; i < world->num_componentsbytype[]; i++) {
//     if ()

//     }
// arraddn(world->components_bytype[world->temp_typeflag][entity], num)


// TNECS_ADD_COMPONENT is overloaded component adder macro
//      3 inputs required: (world, name, entity_id)
//      4th input for speed if newtype is false
#define TNECS_CHOOSE_ADD_COMPONENT(_1,_2,_3,_4,NAME,...) NAME
#define TNECS_ADD_COMPONENT(...) TNECS_CHOOSE_ADD_COMPONENT(__VA_ARGS__, TNECS_ADD_COMPONENT4, TNECS_ADD_COMPONENT3)(__VA_ARGS__)

#define TNECS_ADD_COMPONENT3(world, entity_id, component_name) world->temp_typeflag = TNECS_COMPONENT_NAMES2TYPEFLAG(world, component_name) + world->entity_typeflags[entity_id];\
if (!tnecs_type_id(world->entity_typeflags, world->num_systems, world->temp_typeflag)) {\
    arrput(world->entity_typeflags, world->temp_typeflag);\
    world->num_typeflags++;\
}\
tnecs_entity_typeflag_change(world, entity_id, world->temp_typeflag)

#define TNECS_ADD_COMPONENT4(world, entity, component, isnewtype) if (newtype) {\
strncpy(world->temp_str, #name, sizeof(#name));\
world->temp_typeflag = (hmget(world->component_typehash, world->temp_str) + world->entity_typeflags[entity_id]);\
tnecs_new_typeflag(world, temp_typeflag);\
tnecs_entity_typeflag_change(world, entity_id, world->temp_typeflag);

// if (!tnecs_type_id(world->entity_typeflags, world->num_systems, world->temp_typeflag)) {\
//     arrput(world->entity_typeflags, world->entity_typeflags[entity_id]);\
//     world->num_typeflags++;\
// }\
}


// ************************ COMPONENT AND SYSTEM REGISTERING ******************************
#define TNECS_REGISTER_SYSTEM(world, pfunc, phase, isexcl, ...) tnecs_register_system(world, hash_djb2(#pfunc), &pfunc, phase, isexcl,  TNECS_VARMACRO_EACH_ARGN(__VA_ARGS__), tnecs_component_names2typeflag(world, TNECS_VARMACRO_EACH_ARGN(__VA_ARGS__), TNECS_VARMACRO_FOREACH_COMMA(TNECS_STRINGIFY, __VA_ARGS__)))


void tnecs_register_component(struct tnecs_World * in_world, uint64_t in_hash);
void tnecs_register_system(struct tnecs_World * in_world, uint64_t in_hash, void (* in_system)(struct tnecs_System_Input), uint8_t in_run_phase, bool isexclusive, size_t component_num, tnecs_components_t component_typeflag);


// ****************** ENTITY MANIPULATION ************************
tnecs_entity_t tnecs_new_entity(struct tnecs_World * in_world);
tnecs_entity_t tnecs_new_entity_wcomponents(struct tnecs_World * in_world, size_t argnum, ...);
void tnecs_entity_destroy(struct tnecs_World * in_world, tnecs_entity_t in_entity);
void tnecs_new_component(struct tnecs_World * in_world, tnecs_entity_t in_entity, tnecs_components_t typeflag, tnecs_components_t type_toadd);
bool tnecs_componentsbytype_migrate(struct tnecs_World * in_world, tnecs_entity_t in_entity, tnecs_components_t previous_flag, tnecs_components_t new_flag);
void tnecs_entity_typeflag_change(struct tnecs_World * in_world, tnecs_entity_t in_entity, tnecs_components_t new_type);
size_t tnecs_new_typeflag(struct tnecs_World * in_world, size_t num_components, tnecs_components_t typeflag);
tnecs_component_t tnecs_names2typeflag(struct tnecs_World * in_world, size_t argnum, ...);

// ****************** UTILITY FUNCTIONS ************************
size_t tnecs_component_name2id(struct tnecs_World * in_world, const unsigned char * in_name);
tnecs_component_t tnecs_component_names2typeflag(struct tnecs_World * in_world, size_t argnum, ...);
tnecs_component_t tnecs_component_ids2typeflag(size_t argnum, ...);
size_t tnecs_component_typeflag2id(struct tnecs_World * in_world, tnecs_component_t in_typeflag);
size_t tnecs_component_hash2id(struct tnecs_World * in_world, uint64_t in_hash);
size_t tnecs_type_id(tnecs_components_t * in_typelist, size_t len, tnecs_components_t in_flag);
size_t tnecs_issubtype(tnecs_components_t * in_typelist, size_t len, tnecs_components_t in_flag);

size_t tnecs_system_hash2id(struct tnecs_World * in_world, uint64_t in_hash);
size_t tnecs_system_name2id(struct tnecs_World * in_world, const unsigned char * in_name);
tnecs_component_t tnecs_system_name2typeflag(struct tnecs_World * in_world, const unsigned char * in_name);


// ****************** STRING HASHING ************************
// hash_djb2 slightly faster than hash_sdbm
uint64_t hash_djb2(const unsigned char * str);
uint64_t hash_sdbm(const unsigned char * str);

// ***************** SET BIT COUNTING ***********************
int8_t setBits_KnR_uint64_t(uint64_t in_flag);

#ifdef __cplusplus
}
#endif

#endif // TNECS
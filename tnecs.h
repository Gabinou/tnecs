#ifndef __TNECS_H__
#define __TNECS_H__

/* Tiny C99 Entity-Component-System (ECS) library.
* Originally developed for use in a game I am developping: [Codename Firesaga](https://gitlab.com/Gabinou/firesagamaker). Title pending.
* ECSs are an alternative way to organize data and functions to Object-Oriented programming (OOP).
* OOP: Objects/Classes contain data and methods, children objects inherit from parents...
* ECS: Components are purely data.
* Any number of components can be attached to an entity.
* Entities are acted upon by systems.
* In tnecs, an entity is an uint64_t index.
* A component is user-defined struct.
* A system is a user-defined function.
* The systems iterate only over entities that have a certain set of components.
* They can either be exclusive or inclusive, as in including/excluding entities that have components other than the system's set.
* Systems's execution order happens in phases, set by the user.
* The user can also modify the system execution order in each phase.
* Videogame Example:
* - Enemy Entity: AIControlled component, Sprite Component, Physics Component
* - Bullet Entity: Sprite Component, Physics Component, DamageonHit Component
* - Main Character Entity: UserControlled Component, Sprite Component, Physics Component */

/* Un-viral MIT License
* Copyright (c) 2021 Gabriel Taillon
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE. */

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdarg.h>
#include <math.h>
#ifndef log2 // because tcc SUCKS, does NOT DEFINE log2
#define log2(x) (log(x)/log(2.0f))
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define TNECS_DEBUG_A // asserts are ignored if undefined
#ifdef TNECS_DEBUG_A
#define TNECS_DEBUG_ASSERT(in) do {assert(in);}while(0)
#else
#define TNECS_DEBUG_ASSERT(...) (void)0
#endif

// #define TNECS_DEBUG_P // printf are ignored if undefined
#ifdef TNECS_DEBUG_P
#define TNECS_DEBUG_PRINTF(...) do {printf(__VA_ARGS__);}while(0)
#else
#define TNECS_DEBUG_PRINTF(...) (void)0
#endif

// ************************ TYPE DEFINITIONS ****************************
typedef uint64_t tnecs_entity_t;     // simple 64 bit integer
typedef uint64_t tnecs_component_t;  // 64 bit flags -> MAX 63 components
// typeflag > 0 -> sum of component types -> determines tnecs_System_Input
typedef uint16_t tnecs_system_t;
typedef uint64_t tnecs_time_ns_t;
typedef unsigned char tnecs_byte_t;

// ********************** CONSTANT DEFINITIONS ************************
// entity, component, system, id, typeflag: 0 ALWAYS reserved for NULL
#define TNECS_NULL 0
#define TNECS_NOCOMPONENT_TYPEFLAG 0
#define TNECS_ID_START 1
#define TNECS_MAX_COMPONENT 63
#define TNECS_COMPONENT_CAP 64
#define TNECS_STR_BUFFER 128
#define TNECS_OPEN_IDS_BUFFER 128
#define TNECS_INITIAL_ENTITY_CAP 128
#define TNECS_INITIAL_COMPONENT_CAP 8
#define TNECS_INITIAL_SYSTEM_CAP 16
#define TNECS_ARRAY_INCREMENT 128
#define TNECS_ARRAY_GROWTH_FACTOR 2 // in general 2 or 1.5
#define TNECS_COMPONENT_ALLOCBLOCK 16
#define ENTITY_MAX_COMPONENT_NUM 10

enum TNECS_RUN_PHASES {
    TNECS_PHASE_PREUPDATE = 0,
    TNECS_PHASE_ONUPDATE = 1,
    TNECS_PHASE_POSTUPDATE = 2,
};

// ****************** HACKY DISTRIBUTION FOR VARIADIC MACROS ******************
// Distribution as in algebra: a(x+b) = ax + ab
// TNECS_VARMACRO_FOREACH_XXXX(foo, __VA_ARGS__) applies foo to each __VA_ARGS__, PLUS
//      -> _SUM variant puts + after each (except last)
//      -> _SCOMMA variant stringifies and puts commas around each (except last)
//      -> _COMMA puts commas around each (except last)
//      -> _NEWLINE makes newline for each (except last)
//      up to 8 input args. Theoretically up to 63, if all TNECS_FOREACH_XXXX_N exist
// TNECS_VARMACRO_EACH_ARGN(__VA_ARGS__) counts the number of args
//      -> up to 63, if elements in TNECS_VARMACRO_ARGN and TNECS_VARMACRO_VARG_SEQ exist

#define TNECS_STRINGIFY(x) #x
#define TNECS_CONCATENATE(arg1, arg2) TNECS_CONCATENATE1(arg1, arg2)
#define TNECS_CONCATENATE1(arg1, arg2) TNECS_CONCATENATE2(arg1, arg2)
#define TNECS_CONCATENATE2(arg1, arg2) arg1##arg2

#define TNECS_FOREACH_1(macro, x)\
    macro(x)

#define TNECS_FOREACH_SUM_1(macro, x)\
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

#define TNECS_FOREACH_SSUM_1(macro, x)\
    macro(#x)

#define TNECS_FOREACH_SSUM_2(macro, x, ...)\
    macro(#x)+\
    TNECS_FOREACH_S1(macro, __VA_ARGS__)

#define TNECS_FOREACH_SSUM_3(macro, x, ...)\
    macro(#x)+\
    TNECS_FOREACH_SSUM_2(macro, __VA_ARGS__)

#define TNECS_FOREACH_SSUM_4(macro, x, ...)\
    macro(#x)+\
    TNECS_FOREACH_SSUM_3(macro,  __VA_ARGS__)

#define TNECS_FOREACH_SSUM_5(macro, x, ...)\
    macro(#x)+\
    TNECS_FOREACH_SSUM_4(macro,  __VA_ARGS__)

#define TNECS_FOREACH_SSUM_6(macro, x, ...)\
    macro(#x)+\
    TNECS_FOREACH_SSUM_5(macro,  __VA_ARGS__)

#define TNECS_FOREACH_SSUM_7(macro, x, ...)\
    macro(#x)+\
    TNECS_FOREACH_SSUM_6(macro,  __VA_ARGS__)

#define TNECS_FOREACH_SSUM_8(macro, x, ...)\
    macro(#x)+\
    TNECS_FOREACH_SSUM_7(macro,  __VA_ARGS__)

#define TNECS_VARMACRO_EACH_ARGN(...) TNECS_VARMACRO_EACH_ARGN_(__VA_ARGS__, TNECS_VARMACRO_VARG_SEQ())
#define TNECS_VARMACRO_EACH_ARGN_(...) TNECS_VARMACRO_ARGN(__VA_ARGS__)
#define TNECS_VARMACRO_ARGN(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N
#define TNECS_VARMACRO_VARG_SEQ() 8, 7, 6, 5, 4, 3, 2, 1, 0

#define TNECS_VARMACRO_FOREACH_SUM_(N, macro, ...) TNECS_CONCATENATE(TNECS_FOREACH_SUM_, N)(macro, __VA_ARGS__)
#define TNECS_VARMACRO_FOREACH_SUM(macro, ...) TNECS_VARMACRO_FOREACH_SUM_(TNECS_VARMACRO_EACH_ARGN(__VA_ARGS__), macro, __VA_ARGS__)

#define TNECS_VARMACRO_FOREACH_COMMA_(N, macro, ...) TNECS_CONCATENATE(TNECS_FOREACH_COMMA_, N)(macro, __VA_ARGS__)
#define TNECS_VARMACRO_FOREACH_COMMA(macro, ...) TNECS_VARMACRO_FOREACH_COMMA_(TNECS_VARMACRO_EACH_ARGN(__VA_ARGS__), macro, __VA_ARGS__)

#define TNECS_VARMACRO_FOREACH_SSUM_(N, macro, ...) TNECS_CONCATENATE(TNECS_FOREACH_SSUM_, N)(macro, __VA_ARGS__)
#define TNECS_VARMACRO_FOREACH_SSUM(macro, ...) TNECS_VARMACRO_FOREACH_SSUM_(TNECS_VARMACRO_EACH_ARGN(__VA_ARGS__), macro, __VA_ARGS__)

#define TNECS_VARMACRO_FOREACH_SCOMMA_(N, macro, ...) TNECS_CONCATENATE(TNECS_FOREACH_SCOMMA_, N)(macro, __VA_ARGS__)
#define TNECS_VARMACRO_FOREACH_SCOMMA(macro, ...) TNECS_VARMACRO_FOREACH_SCOMMA_(TNECS_VARMACRO_EACH_ARGN(__VA_ARGS__), macro, __VA_ARGS__)

#define TNECS_VARMACRO_FOREACH_NEWLINE_(N, macro, ...) TNECS_CONCATENATE(TNECS_FOREACH_COMMA_, N)(macro, __VA_ARGS__)
#define TNECS_VARMACRO_FOREACH_NEWLINE(macro, ...) TNECS_VARMACRO_FOREACH_NEWLINE_(TNECS_VARMACRO_EACH_ARGN(__VA_ARGS__), macro, __VA_ARGS__)

// ************************ TNECS STRUCTS DEFINITIONS *****************************
struct tnecs_Components_Array {
    tnecs_component_t type; // single bit on
    size_t num_components;
    size_t len_components;
    void * components;  // [entity_order_bytype]
    // how to access components[n]? need to cast always I guess?
};

struct tnecs_System_Input {
    tnecs_entity_t * entities;
    tnecs_component_t typeflag;
    size_t num_entities;
    size_t num_components;
    size_t * components_order;
    void ** components_lists;
};

struct tnecs_World {
    tnecs_entity_t * entities; // (entities[entity_id] == entity_id) unless deleted
    tnecs_component_t * typeflags;                  // [typeflag_id]
    tnecs_component_t * entity_typeflags;           // [entity_id]
    tnecs_component_t * system_typeflags;           // [system_id]
    void (** systems)(struct tnecs_System_Input);   // [system_id]
    void (** systems_byphase)(struct tnecs_System_Input);// [system_id]
    bool * system_exclusive;                      // [system_id]
    uint8_t * system_phase;                         // [system_id]
    uint64_t component_hashes[TNECS_COMPONENT_CAP]; // [component_id]
    size_t component_bytesizes[TNECS_COMPONENT_CAP];// [component_id]
    uint64_t * system_hashes;                       // [system_id]

    // the by_type array are exclusive
    //   -> entities are unique in components_bytype
    //   -> easier to build inclusive entity lists.
    struct tnecs_Components_Array ** components_bytype; // [typeflag_id][component_order_bytype]
    size_t * entity_orders;                            // [entity_id]
    tnecs_entity_t ** entities_bytype;               // [typeflag_id][entity_order_bytype]
    tnecs_component_t ** components_idbytype;        // [typeflag_id][component_order_bytype]
    tnecs_component_t ** components_flagbytype;      // [typeflag_id][component_order_bytype]

    // len is allocated size
    // num is active elements in array
    size_t len_entities;
    size_t num_entities;
    size_t len_typeflags;
    size_t num_typeflags;
    size_t len_systems;
    size_t num_systems;
    size_t len_phases;
    size_t num_phases;
    size_t num_components;

    size_t * len_components_bytype;                 // [typeflag_id]
    size_t * num_components_bytype;                 // [typeflag_id]
    size_t * len_entities_bytype;                   // [typeflag_id]
    size_t * num_entities_bytype;                   // [typeflag_id]
    size_t * len_components_idbytype;               // [typeflag_id]
    size_t * num_components_idbytype;               // [typeflag_id]
    size_t * len_components_flagbytype;             // [typeflag_id]
    size_t * num_components_flagbytype;             // [typeflag_id]
    size_t * len_system_byphase;                    // [phase_id]
    size_t * num_system_byphase;                    // [phase_id]

    tnecs_entity_t next_entity_id;
    tnecs_entity_t opened_entity_ids[TNECS_OPEN_IDS_BUFFER];
    uint8_t num_opened_entity_ids;
};
typedef struct tnecs_World tnecs_world_t;


// ********************* FUNCTIONALITY MACROS AND FUNCTIONS ************************
struct tnecs_World * tnecs_init();

#define TNECS_NEW_ENTITY(world) tnecs_new_entity(world) // redundancy for API consistency
#define TNECS_NEW_ENTITY_WCOMPONENTS(world, ...) tnecs_new_entity_wcomponents(world, TNECS_VARMACRO_EACH_ARGN(__VA_ARGS__), TNECS_VARMACRO_FOREACH_SCOMMA(tnecs_hash_djb2, __VA_ARGS__))


#define TNECS_HASH(name) tnecs_hash_djb2(#name)
#define TNECS_NAME2HASH(name) TNECS_HASH(name)
#define TNECS_GET_COMPONENT(world, entity_id, name) TNECS_ENTITY_GET_COMPONENT(world, entity_id, name)
#define TNECS_ENTITY_GET_COMPONENT(world, entity_id, name) (name *)tnecs_entity_get_component(world, entity_id, tnecs_component_name2id(world, #name))
#define TNECS_ENTITY_TYPEFLAG(world, entity) world->entity_typeflags[entity]
#define TNECS_TYPEFLAGID(world, typeflag) tnecs_typeflagid(in_world, typeflag)

#define TNECS_COMPONENT_HASH(name) TNECS_HASH(name)
#define TNECS_COMPONENT_NAME2HASH(name) TNECS_NAME2HASH(name)
#define TNECS_COMPONENT_HASH2ID(world, hash) tnecs_component_hash2id(world, hash)
#define TNECS_COMPONENT_HASH2TYPE(world, hash) tnecs_component_hash2type(world, hash)
#define TNECS_COMPONENT_TYPEFLAG(world, name) tnecs_component_names2typeflag(world, 1, #name)
#define TNECS_COMPONENT_NAME2TYPEFLAG(world, name) TNECS_COMPONENT_TYPEFLAG(world, name)
#define TNECS_COMPONENT_NAMES2TYPEFLAG(world, ...) tnecs_component_names2typeflag(world, TNECS_VARMACRO_EACH_ARGN(__VA_ARGS__), TNECS_VARMACRO_FOREACH_COMMA(TNECS_STRINGIFY, __VA_ARGS__))
#define TNECS_COMPONENT_IDS2TYPEFLAG(...) tnecs_component_ids2typeflag(TNECS_VARMACRO_EACH_ARGN(__VA_ARGS__), TNECS_VARMACRO_FOREACH_COMMA(TNECS_STRINGIFY, __VA_ARGS__))
#define TNECS_COMPONENT_ID(world, name) TNECS_COMPONENT_NAME2ID(world, name)
#define TNECS_COMPONENT_NAME2ID(world, name) tnecs_component_name2id(world, #name)
#define TNECS_COMPONENT_ID2TYPEFLAG(id) (1 << (id - TNECS_ID_START))
#define TNECS_COMPONENT_ID2TYPE(id) (1 << (id - TNECS_ID_START))
// I guess casting float to integer is floor()
#define TNECS_COMPONENT_TYPE2ID(type) ((tnecs_component_t)(log2(type) + 1.1f))

#define TNECS_SYSTEM_HASH(name) TNECS_NAME2HASH(name)
#define TNECS_SYSTEM_NAME2HASH(name) TNECS_NAME2HASH(name)
#define TNECS_SYSTEM_ID(world, name) tnecs_system_name2id(world, #name)
#define TNECS_SYSTEM_ID2TYPEFLAG(world, id) world->system_typeflags[id]
#define TNECS_SYSTEM_TYPEFLAG(world, name) tnecs_system_name2typeflag(world, #name)
#define TNECS_SYSTEM_NAME2TYPEFLAG(world, name) TNECS_SYSTEM_TYPEFLAG(world, name)
#define TNECS_SYSTEMS_COMPONENTLIST(input, name) (* name)input->components
#define TNECS_ENTITY_ORDER(world, ent) world->entity_orders[ent];

// TNECS_ADD_COMPONENT is overloaded
//      3 inputs required: (world, name, entity_id)
//      4th input if newtype is false, to skip checks for execution speed
#define TNECS_CHOOSE_ADD_COMPONENT(_1,_2,_3,_4,NAME,...) NAME
#define TNECS_ADD_COMPONENT(...) TNECS_CHOOSE_ADD_COMPONENT(__VA_ARGS__, TNECS_ADD_COMPONENT4, TNECS_ADD_COMPONENT3)(__VA_ARGS__)
#define TNECS_ADD_COMPONENT3(world, entity_id, component) tnecs_entity_add_components(world, entity_id, 1, tnecs_component_names2typeflag(world, 1, #component), true)
#define TNECS_ADD_COMPONENT4(world, entity_id, isnewtype, component) tnecs_entity_add_components(world, entity_id, 1, tnecs_component_names2typeflag(world, 1, #component), isnewtype)

#define TNECS_ADD_COMPONENTS(world, entity_id, isnewtype, ...) tnecs_entity_add_components(world, entity_id, TNECS_VARMACRO_EACH_ARGN(__VA_ARGS__), tnecs_component_names2typeflag(world, TNECS_VARMACRO_EACH_ARGN(__VA_ARGS__), TNECS_VARMACRO_FOREACH_COMMA(TNECS_STRINGIFY, __VA_ARGS__)), isnewtype)

// ************************ COMPONENT AND SYSTEM REGISTERING ******************************
#define TNECS_REGISTER_SYSTEM(world, pfunc, phase, isexcl, ...) tnecs_register_system(world, tnecs_hash_djb2(#pfunc), &pfunc, phase, isexcl,  TNECS_VARMACRO_EACH_ARGN(__VA_ARGS__), tnecs_component_names2typeflag(world, TNECS_VARMACRO_EACH_ARGN(__VA_ARGS__), TNECS_VARMACRO_FOREACH_COMMA(TNECS_STRINGIFY, __VA_ARGS__)));\

#define TNECS_REGISTER_COMPONENT(world, name) tnecs_register_component(world, tnecs_hash_djb2(#name), sizeof(name))
// Error if component registered twice -> user responsibility

void tnecs_register_component(struct tnecs_World * in_world, uint64_t in_hash, size_t in_bytesize);
void tnecs_register_system(struct tnecs_World * in_world, uint64_t in_hash, void (* in_system)(struct tnecs_System_Input), uint8_t in_run_phase, bool isexclusive, size_t component_num, tnecs_component_t component_typeflag);

// ****************** ENTITY MANIPULATION ************************
tnecs_entity_t tnecs_new_entity(struct tnecs_World * in_world);
tnecs_entity_t tnecs_new_entity_wcomponents(struct tnecs_World * in_world, size_t argnum, ...);
void * tnecs_entity_allocate_component(struct tnecs_World * in_world, tnecs_entity_t in_entity_id, uint64_t component_hash, void * calloced_component);

void tnecs_entity_destroy(struct tnecs_World * in_world, tnecs_entity_t in_entity);
void * tnecs_entity_get_component(struct tnecs_World * in_world, tnecs_entity_t in_entity, tnecs_component_t in_component_id);
void tnecs_entity_add_components(struct tnecs_World * in_world, tnecs_entity_t in_entity, size_t num_components, tnecs_component_t typeflag, bool isNew);
size_t tnecs_new_typeflag(struct tnecs_World * in_wormakld, size_t num_components, tnecs_component_t typeflag);
void tnecs_component_array_new(struct tnecs_World * in_world, size_t num_components, tnecs_component_t typeflag);

bool tnecs_component_migrate(struct tnecs_World * in_world, tnecs_entity_t in_entity, size_t entity_order_new, tnecs_component_t new_flag);
void tnecs_component_del(struct tnecs_World * in_world, tnecs_entity_t in_entity, tnecs_component_t old_flag);
void tnecs_component_copy(struct tnecs_World * in_world, tnecs_entity_t in_entity, tnecs_component_t old_flag, tnecs_component_t new_flag);
void tnecs_component_add(struct tnecs_World * in_world, tnecs_component_t in_flag);
void tnecs_component_array_init(struct tnecs_World * in_world, struct tnecs_Components_Array * in_array, size_t in_component_id);
void tnecs_component_array_realloc(struct tnecs_World * in_world, tnecs_component_t entity_typeflag, tnecs_component_t id_toinit);

void tnecs_entity_typeflag_add(struct tnecs_World * in_world, tnecs_entity_t in_entity, tnecs_component_t in_typeflag);

size_t tnecs_entitiesbytype_migrate(struct tnecs_World * in_world, tnecs_entity_t in_entity, tnecs_component_t old_type, tnecs_component_t new_type);
size_t tnecs_entitiesbytype_add(struct tnecs_World * in_world, tnecs_entity_t in_entity, tnecs_component_t new_type);
void tnecs_entitiesbytype_del(struct tnecs_World * in_world, tnecs_entity_t in_entity, tnecs_component_t old_type);

tnecs_component_t tnecs_names2typeflag(struct tnecs_World * in_world, size_t argnum, ...);

// ****************** UTILITY FUNCTIONS ************************
tnecs_component_t tnecs_component_hash2typeflag(struct tnecs_World * in_world, uint64_t in_hash);
size_t tnecs_componentflag_order_bytype(struct tnecs_World * in_world, tnecs_component_t in_component_flag, tnecs_component_t in_typeflag);
size_t tnecs_componentid_order_bytype(struct tnecs_World * in_world, size_t in_component_id, tnecs_component_t in_typeflag);
size_t tnecs_entity_order_bytype(struct tnecs_World * in_world, tnecs_entity_t in_entity, tnecs_component_t in_typeflag);
size_t tnecs_entity_order_bytypeid(struct tnecs_World * in_world, tnecs_entity_t in_entity, tnecs_component_t in_typeflag_id);
size_t tnecs_system_order_byphase(struct tnecs_World * in_world, size_t system_id, uint8_t in_phase);

size_t tnecs_typeflagid(struct tnecs_World * in_world, tnecs_component_t typeflag);

size_t tnecs_component_name2id(struct tnecs_World * in_world, const unsigned char * in_name);
tnecs_component_t tnecs_component_names2typeflag(struct tnecs_World * in_world, size_t argnum, ...);
tnecs_component_t tnecs_component_ids2typeflag(size_t argnum, ...);
size_t tnecs_typeflagid(struct tnecs_World * in_world, tnecs_component_t in_typeflag);
size_t tnecs_component_hash2id(struct tnecs_World * in_world, uint64_t in_hash);
tnecs_component_t tnecs_component_hash2type(struct tnecs_World * in_world, uint64_t in_hash);
size_t tnecs_type_id(tnecs_component_t * in_typelist, size_t len, tnecs_component_t in_flag);
size_t tnecs_issubtype(tnecs_component_t * in_typelist, size_t len, tnecs_component_t in_flag);
size_t tnecs_system_hash2id(struct tnecs_World * in_world, uint64_t in_hash);
size_t tnecs_system_name2id(struct tnecs_World * in_world, const unsigned char * in_name);
tnecs_component_t tnecs_system_name2typeflag(struct tnecs_World * in_world, const unsigned char * in_name);

// ****************** "DYNAMIC" ARRAYS  *****************************
void * tnecs_realloc(void * ptr, size_t old_len, size_t new_len, size_t elem_bytesize);
void * tnecs_arrdel(void * arr, size_t elem, size_t len, size_t bytesize);
void * tnecs_arrdel_scramble(void * arr, size_t elem, size_t len, size_t bytesize);
void tnecs_growArray_phase(struct tnecs_World * in_world);
void tnecs_growArray_entity(struct tnecs_World * in_world);
void tnecs_growArray_system(struct tnecs_World * in_world);
void tnecs_growArray_typeflag(struct tnecs_World * in_world);

#define TNECS_REALLOC(ptr, old_len, new_len, bytesize) tnecs_realloc(ptr, old_len, new_len, bytesize)
#define TNECS_DEL(arr, elem, len, bytesize) tnecs_arrdel(arr, elem, len, bytesize)
#define TNECS_DELMACRO(arr, elem, len, bytesize) memcpy(arr + (elem * bytesize), arr + ((elem + 1) * bytesize), bytesize * (len - elem - 1))

#define TNECS_ARRAY_GROWS(world, arrname) if ((world->num_##arrname + 1) >= world->len_##arrname) { \
            size_t old_len = in_world->len_##arrname; \
            in_world->len_##arrname *= TNECS_ARRAY_GROWTH_FACTOR; \
            in_world->arrname = tnecs_realloc(in_world->arrname, old_len, in_world->len_##arrname, sizeof(*in_world->arrname)); \
        }

// ****************** STRING HASHING ************************
// tnecs_hash_djb2 slightly faster than tnecs_hash_sdbm
uint64_t tnecs_hash_djb2(const unsigned char * str);
uint64_t tnecs_hash_sdbm(const unsigned char * str);


#ifdef __cplusplus
}
#endif

#endif // __TNECS_H__
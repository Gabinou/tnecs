#ifndef __TNECS_H__
#define __TNECS_H__
/* Tiny C99 Entity-Component-System (ECS) library.
* tnecs.h
*
* Copyright (C) Gabriel Taillon, 2023
*
* ECSs are an alternative way to organize data and functions to
* Object-Oriented programming (OOP).
* OOP:
* - Objects/Classes contain data and methods.
* - Methods act on objects.
* - Children classes inherit methods and data structure from parents.
* ECS:
* - Components are purely data.
* - Any number of components can be attached to an entity.
* - Entities are acted upon by systems.
*
* In tnecs, an entity is an ```uint64_t``` index.
* A component is user-defined ```struct```.
* A system is a user-defined ```function```.
* All live inside the ```world```.
*
* The systems iterate over the entities that have a user-defined set of components,
* inclusively or exclusively, in phases.
* Phases are user-defined ```uint8_t```.
* System execution order is first-come first-served by default.
* Systems are inclusive by default, meaning that they run over entities with
* additional components to the system's.
*/

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>
#ifndef log2 // tcc SUCKS and DOES NOT define log2
    #define log2(x)  (x > 0 ? (log(x)/log(2.0f)) : -INFINITY)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/********************* DEBUGGING *********************/
#define TNECS_DEBUG_A // TNECS_DEBUG_ASSERT are ignored if undefined
#ifdef TNECS_DEBUG_A
#define TNECS_DEBUG_ASSERT(in) do {assert(in);}while(0)
#else
#define TNECS_DEBUG_ASSERT(...) (void)0
#endif

/******************* TYPE DEFINITIONS *******************/
typedef unsigned long long int  tnecs_entity;       // simple 64 bit integer
typedef tnecs_entity            tnecs_component;    // 64 bit flags -> MAX 63 components
typedef long long int           i64;                // 64 bit flags -> MAX 63 components
typedef uint64_t                tnecs_hash;
typedef uint32_t                tnecs_phase;
typedef uint64_t                tnecs_ns;
typedef int32_t                 b32;
typedef unsigned char           tnecs_byte;




/*** Forward declarations ***/
typedef struct tnecs_system_input     tnecs_system_input;
typedef struct tnecs_component_array  tnecs_component_array;

/*** Function pointer ***/
typedef void (*tnecs_system_ptr)(struct tnecs_system_input *);

/******************* CONSTANT DEFINITIONS *******************/
enum TNECS {
    TNECS_NULL                  =         0,
    TNECS_NULLSHIFT             =         1,
    TNECS_INIT_ENTITY_LEN       =       128,
    TNECS_INIT_PHASE_LEN        =         8,
    TNECS_INIT_COMPONENT_LEN    =         8,
    TNECS_INIT_ARCHETYPE_LEN    =        16,
    TNECS_ROUNDING_MULTIPLE     =        16,
    TNECS_COMPONENT_CAP         =        64,
    TNECS_ENTITIES_CAP          = 100000000,
    TNECS_PHASES_CAP            = TNECS_INIT_PHASE_LEN * 8 + 1,
    TNECS_OPEN_IDS_BUFFER       =       128,
    TNECS_CHUNK_BYTESIZE        =     16384,
    TNECS_INIT_WORLD_BYTESIZE   = TNECS_CHUNK_BYTESIZE * TNECS_INIT_ARCHETYPE_LEN * 2,
    TNECS_ARRAY_GROWTH_FACTOR   =         2 // in general 2 or 1.5
};

/********************* UTILITY MACROS***********************/
#define TNECS_STRINGIFY(x) #x
#define TNECS_DONOTHING(x) x
#define TNECS_CONCATENATE( arg1, arg2) TNECS_CONCATENATE1(arg1, arg2)
#define TNECS_CONCATENATE1(arg1, arg2) TNECS_CONCATENATE2(arg1, arg2)
#define TNECS_CONCATENATE2(arg1, arg2) arg1##arg2
#define TNECS_ARCHETYPE_HAS_TYPE(archetype, type) ((archetype & type) > 0)
#define TNECS_ARCHETYPE_IS_ARCHETYPE(archetype1, archetype2) ((archetype1 & archetype2) == archetype1) // checks if archetype2 is a archetype of archetype1

/*********** HACKY DISTRIBUTION FOR VARIADIC MACROS *********/
// Distribution as in algebra: a(x+b) -> ax + ab

// TNECS_VAR_EACH_ARGN(__VA_ARGS__) counts the number of args
//      -> up to 63, if elements in TNECS_VAR_ARGN and TNECS_VAR_VARG_SEQ exist
#define TNECS_VAR_EACH_ARGN(...) TNECS_VAR_EACH_ARGN_(__VA_ARGS__, TNECS_VAR_VARG_SEQ())
#define TNECS_VAR_EACH_ARGN_(...) TNECS_VAR_ARGN(__VA_ARGS__)
#define TNECS_VAR_ARGN(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N
#define TNECS_VAR_VARG_SEQ() 8, 7, 6, 5, 4, 3, 2, 1, 0

// TNECS_VARMACRO_FOREACH_XXXX(foo, __VA_ARGS__) applies foo to each __VA_ARGS__, PLUS
//      -> _SCOMMA variant stringifies and puts commas around each (except last)
//      -> _COMMA puts commas around each (except last)
//      up to 63 args if all TNECS_FOREACH_XXXX_N exist
#define TNECS_FOREACH_COMMA_1(macro,  x)      macro(x)
#define TNECS_FOREACH_COMMA_2(macro,  x, ...) macro(x),  TNECS_FOREACH_COMMA_1(macro, __VA_ARGS__)
#define TNECS_FOREACH_COMMA_3(macro,  x, ...) macro(x),  TNECS_FOREACH_COMMA_2(macro, __VA_ARGS__)
#define TNECS_FOREACH_COMMA_4(macro,  x, ...) macro(x),  TNECS_FOREACH_COMMA_3(macro,  __VA_ARGS__)
#define TNECS_FOREACH_COMMA_5(macro,  x, ...) macro(x),  TNECS_FOREACH_COMMA_4(macro,  __VA_ARGS__)
#define TNECS_FOREACH_COMMA_6(macro,  x, ...) macro(x),  TNECS_FOREACH_COMMA_5(macro,  __VA_ARGS__)
#define TNECS_FOREACH_COMMA_7(macro,  x, ...) macro(x),  TNECS_FOREACH_COMMA_6(macro,  __VA_ARGS__)
#define TNECS_FOREACH_COMMA_8(macro,  x, ...) macro(x),  TNECS_FOREACH_COMMA_7(macro,  __VA_ARGS__)
#define TNECS_FOREACH_SCOMMA_1(macro, x)      macro(#x)
#define TNECS_FOREACH_SCOMMA_2(macro, x, ...) macro(#x), TNECS_FOREACH_SCOMMA_1(macro, __VA_ARGS__)
#define TNECS_FOREACH_SCOMMA_3(macro, x, ...) macro(#x), TNECS_FOREACH_SCOMMA_2(macro, __VA_ARGS__)
#define TNECS_FOREACH_SCOMMA_4(macro, x, ...) macro(#x), TNECS_FOREACH_SCOMMA_3(macro,  __VA_ARGS__)
#define TNECS_FOREACH_SCOMMA_5(macro, x, ...) macro(#x), TNECS_FOREACH_SCOMMA_4(macro,  __VA_ARGS__)
#define TNECS_FOREACH_SCOMMA_6(macro, x, ...) macro(#x), TNECS_FOREACH_SCOMMA_5(macro,  __VA_ARGS__)
#define TNECS_FOREACH_SCOMMA_7(macro, x, ...) macro(#x), TNECS_FOREACH_SCOMMA_6(macro,  __VA_ARGS__)
#define TNECS_FOREACH_SCOMMA_8(macro, x, ...) macro(#x), TNECS_FOREACH_SCOMMA_7(macro,  __VA_ARGS__)

#define TNECS_VARMACRO_FOREACH_COMMA_(N, macro, ...) TNECS_CONCATENATE(TNECS_FOREACH_COMMA_, N)(macro, __VA_ARGS__)
#define TNECS_VARMACRO_FOREACH_COMMA(macro, ...) TNECS_VARMACRO_FOREACH_COMMA_(TNECS_VAR_EACH_ARGN(__VA_ARGS__), macro, __VA_ARGS__)

#define TNECS_VARMACRO_FOREACH_SCOMMA_(N, macro, ...) TNECS_CONCATENATE(TNECS_FOREACH_SCOMMA_, N)(macro, __VA_ARGS__)
#define TNECS_VARMACRO_FOREACH_SCOMMA(macro, ...) TNECS_VARMACRO_FOREACH_SCOMMA_(TNECS_VAR_EACH_ARGN(__VA_ARGS__), macro, __VA_ARGS__)

/************ STRUCTS DEFINITIONS ***************/
typedef struct tnecs_system_input {
    // Note: Systems run over entity_order_bytype for entity_order_bytype
    tnecs_world     *world;
    tnecs_ns         deltat;
    tnecs_component  system_archetype;
    size_t           num_entities;
    size_t           entity_archetype_id;
    void            *data;
} tnecs_system_input;

// 1D array of 1 component.
typedef struct tnecs_component_array {
    tnecs_component  type;
    size_t           num_components;
    size_t           len_components;
    // problems: 
    //      - components array is not in structure
    //      - need 2 allocs: for struct, and then array
    void            *components;      /* [entity_order_bytype] */
} tnecs_component_array;

typedef struct tnecs_arena_array {
    i64 handle;
    size_t num;
    size_t len;
} tnecs_arena_array;

typedef struct tnecs_phase_arena {
    i64 arena;
    size_t num;
    size_t len;
    // All handles
    i64 system_byphase;
    i64 system_idbyphase;
} tnecs_phase_arena;

typedef struct tnecs_entities_arena {
    i64 arena;
    size_t num;
    size_t len;
    tnecs_entity next;

    // All handles
    i64 id;         // *tnecs_entity
    i64 order;      // *size_t
    i64 archetype;  // *tnecs_component
} tnecs_entities_arena;

typedef struct tnecs_system_arena {
    i64 arena;
    size_t num;
    size_t len;
    // All handles
    i64 archetype;  // *tnecs_component
    i64 order;      // *size_t
    i64 name;       // *i64
    i64 hash;       // *tnecs_hash
    i64 phase;      // *tnecs_phase
    i64 exclusive;  // *b32
} tnecs_system_arena;
    
typedef struct tnecs_archetype_arena {
    i64 arena;
    size_t num;
    size_t len;
    
    i64 id;                         // *tnecs_component
    // len/num of bytype arrays is archetype_id.num/len
    i64 id_bytype;                  // *i64 -> *tnecs_component
    i64 components_idbytype;        // *i64 -> *tnecs_component
    i64 components_orderbytype;     // *i64 -> *size_t
    i64 entities_bytype;            // *i64 -> *tnecs_entity
    i64 chunks_bytype;              // *i64 -> *tnecs_chunk
} tnecs_archetype_arena;

typedef struct tnecs_components_arena {
    size_t      component_bytesizes[TNECS_COMPONENT_CAP];  // [component_id]
    tnecs_hash  component_hashes[TNECS_COMPONENT_CAP];     // [component_id]
    i64         component_names[TNECS_COMPONENT_CAP];      // [component_id]
} tnecs_components_arena;
  

/*** tnecs_worlds ***/
typedef struct tnecs_world {
    tnecs_phase_arena        phases;
    tnecs_system_arena       systems;
    tnecs_entities_arena     entities;
    tnecs_archetype_arena    archetypes;
    tnecs_components_arena   components;
    
    tnecs_arena_array entities_open;
    tnecs_arena_array system_torun;
    
    b32 reuse_entities;

    tnecs_byte mem[];
} tnecs_world;
    
// tnecs_Chunk: memory reserved for all components of archetype
// - Each component has an array inside the chunk.
// - Each chunk is 16kB total.
// - Entity order determines if chunk is full
#define TNECS_CHUNK_COMPONENTS_BYTESIZE (TNECS_CHUNK_BYTESIZE - 2 * sizeof(size_t) - sizeof(tnecs_component))
typedef struct tnecs_chunk {
    tnecs_component  archetype;
    size_t           components_num;
    size_t           entities_len; 
    // Raw memory chunk:
    //  - Header: cumulative bytesizes: components_num * size_t.
    //  - Body:   components arrays, each: entities_len * component_bytesize.
    tnecs_byte       mem[TNECS_CHUNK_COMPONENTS_BYTESIZE];
} tnecs_chunk;

// Arena with memory is in struct (flexible array member) */
// No copy assignment.
// No local variable, only ptr.
typedef struct tnecs_arena {
    i64         size;   /* [bytes]  total malloced bytesize     */
    i64         fill;   /* [bytes]  part of _start used         */
    tnecs_byte  mem[];  /* Note: byte 0 is NULL                 */
} tnecs_arena;


tnecs_chunk tnecs_chunk_Init(const tnecs_world *world, const tnecs_component archetype);
size_t  *tnecs_chunk_BytesizeArr( const tnecs_chunk *chunk);
size_t   tnecs_chunk_TotalBytesize(const tnecs_chunk *chunk);
void    *tnecs_chunk_ComponentArr(tnecs_chunk *chunk, const size_t corder);
b32      tnecs_chunk_Full(const tnecs_chunk *chunk);

size_t tnecs_EntityOrder_to_ChunkOrder(     const tnecs_chunk *chunk, const size_t entity_order);
size_t tnecs_EntityOrder_to_ArchetypeChunk(const tnecs_chunk *chunk, const size_t entity_order);

/******************** WORLD FUNCTIONS **********************/
b32 tnecs_world_genesis(tnecs_world **w);
b32 tnecs_world_destroy(tnecs_world **w);

tnecs_arena *tnecs_world_arena(tnecs_world *w);

b32 tnecs_world_step(      tnecs_world *w, tnecs_ns     deltat, void *data);
b32 tnecs_world_step_phase(tnecs_world *w, tnecs_phase  phase, tnecs_ns deltat, void *data);

/******************* SYSTEM FUNCTIONS ********************/
b32 tnecs_system_run(tnecs_world *w, size_t id, tnecs_ns deltat, void *data);
b32 tnecs_custom_system_run(tnecs_world *w, tnecs_system_ptr c,
                             tnecs_component ar, tnecs_ns deltat, void *data);

/************* REGISTRATION *********************/
tnecs_component tnecs_register_component(tnecs_world *w,
                                         const char *name, size_t b);

size_t tnecs_register_system(tnecs_world *w, const char *name,
                             tnecs_system_ptr system, tnecs_phase run_phase,
                             b32 isExclusive, size_t component_num, tnecs_component component_archetype);
size_t tnecs_register_phase(tnecs_world *w, tnecs_phase phase);


#define TNECS_REGISTER_SYSTEM(world, pfunc, ...) tnecs_register_system(world, #pfunc, &pfunc, 0, 0, TNECS_VAR_EACH_ARGN(__VA_ARGS__), tnecs_component_names2archetype(world, TNECS_VAR_EACH_ARGN(__VA_ARGS__), TNECS_VARMACRO_FOREACH_COMMA(TNECS_STRINGIFY, __VA_ARGS__)))
#define TNECS_REGISTER_SYSTEM_wPHASE(world, pfunc, phase, ...) tnecs_register_system(world, #pfunc, &pfunc, phase, 0,TNECS_VAR_EACH_ARGN(__VA_ARGS__), tnecs_component_names2archetype(world, TNECS_VAR_EACH_ARGN(__VA_ARGS__), TNECS_VARMACRO_FOREACH_COMMA(TNECS_STRINGIFY, __VA_ARGS__)))
#define TNECS_REGISTER_SYSTEM_wEXCL(world, pfunc, excl, ...) tnecs_register_system(world, #pfunc, &pfunc, 0, excl, TNECS_VAR_EACH_ARGN(__VA_ARGS__), tnecs_component_names2archetype(world, TNECS_VAR_EACH_ARGN(__VA_ARGS__), TNECS_VARMACRO_FOREACH_COMMA(TNECS_STRINGIFY, __VA_ARGS__)))
#define TNECS_REGISTER_SYSTEM_wPHASE_wEXCL(world, pfunc, phase, excl, ...) tnecs_register_system(world, #pfunc, &pfunc, phase, excl,TNECS_VAR_EACH_ARGN(__VA_ARGS__), tnecs_component_names2archetype(world, TNECS_VAR_EACH_ARGN(__VA_ARGS__), TNECS_VARMACRO_FOREACH_COMMA(TNECS_STRINGIFY, __VA_ARGS__)))
#define TNECS_REGISTER_SYSTEM_wEXCL_wPHASE(world, pfunc, excl, phase, ...) tnecs_register_system(world, #pfunc, &pfunc, phase, excl,TNECS_VAR_EACH_ARGN(__VA_ARGS__), tnecs_component_names2archetype(world, TNECS_VAR_EACH_ARGN(__VA_ARGS__), TNECS_VARMACRO_FOREACH_COMMA(TNECS_STRINGIFY, __VA_ARGS__)))

#define TNECS_REGISTER_COMPONENT(world, name) tnecs_register_component(world, #name, sizeof(name))
#define TNECS_CHECK_ALLOC(name) do {\
        if (name == TNECS_NULL) { \
            printf("tnecs: failed allocation " #name "\n"); \
            return(0); \
        } \
    } while (0)

#define TNECS_CHECK_CALL(call) do {\
        if (!call) { \
            printf("tnecs: failed function call " #call "\n"); \
            return(0); \
        } \
    } while (0)

/************ ENTITY MANIPULATION *************/
/* -- Public -- */
tnecs_entity tnecs_entity_create(tnecs_world *w);
tnecs_entity tnecs_entity_create_wID(tnecs_world *w, tnecs_entity entity);
tnecs_entity tnecs_entities_create(tnecs_world *w, size_t num);
tnecs_entity tnecs_entities_create_wID(tnecs_world *w, size_t num,
                                       tnecs_entity *ents);
tnecs_entity tnecs_entity_create_wcomponents(tnecs_world *w, size_t argnum, ...);

b32 tnecs_entity_destroy(tnecs_world *w, tnecs_entity entity);

// Overloaded macros: -> TNECS_ENTITY_CREATE, TNECS_ENTITIES_CREATE, TNECS_ADD_COMPONENT
#define TNECS_ENTITY_CREATE(...) TNECS_CHOOSE_ENTITY_CREATE(__VA_ARGS__, TNECS_ENTITY_CREATE2, TNECS_ENTITY_CREATE1)(__VA_ARGS__)
#define TNECS_CHOOSE_ENTITY_CREATE(_1,_2,NAME,...) NAME
#define TNECS_ENTITY_CREATE1(world) tnecs_entity_create(world)
#define TNECS_ENTITY_CREATE2(world, index) tnecs_entity_create_wID(world, index)
#define TNECS_ENTITY_EXISTS(world, index) (world->entities[index] > TNECS_NULL)

#define TNECS_ENTITIES_CREATE(...) TNECS_CHOOSE_ENTITIES_CREATE(__VA_ARGS__, TNECS_ENTITIES_CREATE3, TNECS_ENTITIES_CREATE2)(__VA_ARGS__)
#define TNECS_CHOOSE_ENTITIES_CREATE(_1,_2,_3,NAME,...) NAME
#define TNECS_ENTITIES_CREATE2(world, num) tnecs_entities_create(world, num)
#define TNECS_ENTITIES_CREATE3(world, num, indices) tnecs_entities_create_wID(world, num, indices)

#define TNECS_ENTITY_CREATE_wCOMPONENTS(world, ...) tnecs_entity_create_wcomponents(world, TNECS_VAR_EACH_ARGN(__VA_ARGS__), TNECS_VARMACRO_FOREACH_SCOMMA(TNECS_HASH, __VA_ARGS__))
#define TNECS_ENTITY_ARCHETYPE(world, entity) world->entity_archetypes[entity]
#define TNECS_ENTITY_HASCOMPONENT(world, entity, name) ((world->entity_archetypes[entity] &tnecs_component_names2archetype(world, 1, #name)) > 0)
#define TNECS_ENTITY_HASCOMPONENT(world, entity, name) ((world->entity_archetypes[entity] &tnecs_component_names2archetype(world, 1, #name)) > 0)

#define TNECS_ADD_COMPONENT(...) TNECS_CHOOSE_ADD_COMPONENT(__VA_ARGS__, TNECS_ADD_COMPONENT4, TNECS_ADD_COMPONENT3)(__VA_ARGS__)
#define TNECS_CHOOSE_ADD_COMPONENT(_1,_2,_3,_4,NAME,...) NAME
#define TNECS_ADD_COMPONENT3(world, entity_id, component) tnecs_entity_add_components(world, entity_id, 1, tnecs_component_names2archetype(world, 1, #component), true)
#define TNECS_ADD_COMPONENT4(world, entity_id, component, isnewtype) tnecs_entity_add_components(world, entity_id, 1, tnecs_component_names2archetype(world, 1, #component), isnewtype)

#define TNECS_ADD_COMPONENTS(world, entity_id, isnewtype, ...) tnecs_entity_add_components(world, entity_id, TNECS_VAR_EACH_ARGN(__VA_ARGS__), tnecs_component_names2archetype(world, TNECS_VAR_EACH_ARGN(__VA_ARGS__), TNECS_VARMACRO_FOREACH_COMMA(TNECS_STRINGIFY, __VA_ARGS__)), isnewtype)

#define TNECS_REMOVE_COMPONENTS(world, entity_id, ...) tnecs_entity_remove_components(world, entity_id, TNECS_VAR_EACH_ARGN(__VA_ARGS__), tnecs_component_names2archetype(world, TNECS_VAR_EACH_ARGN(__VA_ARGS__), TNECS_VARMACRO_FOREACH_COMMA(TNECS_STRINGIFY, __VA_ARGS__)))

#define TNECS_GET_COMPONENT(world, entity_id, name) tnecs_entity_get_component(world, entity_id, tnecs_component_name2id(world, #name))

/********************************************************/
/****************** TNECS INTERNALS *********************/
/********************************************************/
tnecs_entity tnecs_entity_add_components(tnecs_world *w, tnecs_entity entity,
                                         size_t num_components, tnecs_component archetype, b32 isNew);
b32  tnecs_entity_remove_components(tnecs_world *w,
                                    tnecs_entity entity, size_t num_components, tnecs_component archetype);
void *tnecs_entity_get_component(tnecs_world *w, tnecs_entity entity,
                                 tnecs_component component_id);

b32 tnecs_entitiesbytype_add(tnecs_world *w, tnecs_entity entity,
                                tnecs_component new_type);
b32 tnecs_entitiesbytype_del(tnecs_world *w, tnecs_entity entity,
                                tnecs_component old_type);
b32 tnecs_entitiesbytype_migrate(tnecs_world *w, tnecs_entity entity,
                                    tnecs_component old_type, tnecs_component new_type);

b32 tnecs_component_add(tnecs_world *w, tnecs_component flag);
b32 tnecs_component_copy(tnecs_world *w, tnecs_entity entity,
                          tnecs_component old_flag, tnecs_component new_flag);
b32 tnecs_component_del(tnecs_world *w, tnecs_entity entity,
                         tnecs_component old_flag);
b32 tnecs_component_migrate(tnecs_world *w, tnecs_entity entity,
                             tnecs_component old_flag, tnecs_component new_flag);

b32 tnecs_component_array_new(tnecs_world *w, size_t num_components,
                               tnecs_component archetype);
b32 tnecs_component_array_init(tnecs_world *w,
                                tnecs_component_array *array, size_t component_id);

b32 tnecs_system_order_switch(tnecs_world *w, tnecs_phase phase,
                               size_t order1, size_t order2);

/************* UTILITY FUNCTIONS/MACROS *************/
size_t tnecs_round_up(size_t to_round);
size_t tnecs_component_name2id(tnecs_world *w, const char *name);
size_t tnecs_component_hash2id(tnecs_world *w, tnecs_hash hash);
size_t tnecs_component_order_bytype(tnecs_world *w, size_t component_id,
                                    tnecs_component archetype);
size_t tnecs_component_order_bytypeid(tnecs_world *w, size_t component_id,
                                      size_t archetype_id);
tnecs_component tnecs_component_names2archetype(tnecs_world *w, size_t argnum, ...);
tnecs_component tnecs_component_ids2archetype(size_t argnum, ...);
tnecs_component tnecs_component_hash2type(tnecs_world *w, tnecs_hash hash);

void tnecs_component_names_print(tnecs_world *w, tnecs_entity ent);

size_t tnecs_system_name2id(tnecs_world *w, const char *name);
size_t tnecs_system_hash2id(tnecs_world *w, tnecs_hash hash);
tnecs_component tnecs_system_name2archetype(tnecs_world *w, const char *name);

size_t tnecs_archetypeid(tnecs_world *w, tnecs_component archetype);

#define TNECS_COMPONENT_HASH2ID(world, hash) tnecs_component_hash2id(world, hash)
#define TNECS_COMPONENT_HASH2TYPE(world, hash) tnecs_component_hash2type(world, hash)
#define TNECS_COMPONENT_NAME2TYPE(world, name) tnecs_component_names2archetype(world, 1, #name)
#define TNECS_COMPONENT_NAMES2ARCHETYPE(world, ...) tnecs_component_names2archetype(world, TNECS_VAR_EACH_ARGN(__VA_ARGS__), TNECS_VARMACRO_FOREACH_COMMA(TNECS_STRINGIFY, __VA_ARGS__))
#define TNECS_COMPONENT_NAMES2ARCHETYPEID(world, ...) tnecs_archetypeid(world, tnecs_component_names2archetype(world, TNECS_VAR_EACH_ARGN(__VA_ARGS__), TNECS_VARMACRO_FOREACH_COMMA(TNECS_STRINGIFY, __VA_ARGS__)))
#define TNECS_COMPONENT_ID2TYPE(id) (1 << (id - TNECS_NULLSHIFT))
#define TNECS_COMPONENT_IDS2ARCHETYPE(...) tnecs_component_ids2archetype(TNECS_VAR_EACH_ARGN(__VA_ARGS__), TNECS_VARMACRO_FOREACH_COMMA(TNECS_DONOTHING, __VA_ARGS__))
#define TNECS_COMPONENT_NAME2ID(world, name) tnecs_component_name2id(world, #name)
#define TNECS_COMPONENT_TYPE2ID(type) (type >=1 ? (tnecs_component)(log2(type) + 1.1f): 0) // casting to int floors
#define TNECS_COMPONENTS_LIST(input, component_name) (input->world->components_bytype[input->entity_archetype_id][input->world->components_orderbytype[input->entity_archetype_id][tnecs_component_name2id(input->world, #component_name)]].components)

#define TNECS_SYSTEM_ID2ARCHETYPE(world, id) world->system_archetypes[id]
#define TNECS_SYSTEM_NAME2ID(world, name) tnecs_system_name2id(world, #name)
#define TNECS_SYSTEM_NAME2ARCHETYPE(world, name) tnecs_system_name2archetype(world, #name)
#define TNECS_SYSTEM_NAME2ARCHETYPEID(world, name) tnecs_archetypeid(world, tnecs_system_name2archetype(world, #name))

#define TNECS_ARCHETYPEID(world, archetype) tnecs_archetypeid(world, archetype)

/******************** "DYNAMIC" ARRAYS *********************/
void *tnecs_realloc(void *ptr, size_t old_len, size_t new_len, size_t elem_bytesize);
void *tnecs_arrdel(void *arr, size_t elem, size_t len, size_t bytesize);
void *tnecs_arrdel_scramble(void *arr, size_t elem, size_t len, size_t bytesize);

b32 tnecs_growArray_bytype(tnecs_world *w, size_t archetype_id);
b32 tnecs_growArray_entity(tnecs_world *w);
b32 tnecs_growArray_system(tnecs_world *w);
b32 tnecs_growArray_archetype(tnecs_world *w);
b32 tnecs_growArray_phase(tnecs_world *w);
b32 tnecs_growArray_torun(tnecs_world *w);

/****************** STRING HASHING ****************/
uint64_t tnecs_hash_djb2(const char *str);  // slightly faster
uint64_t tnecs_hash_sdbm(const char *str);
uint64_t tnecs_hash_combine(uint64_t h1, uint64_t h2);
#define TNECS_HASH(name) tnecs_hash_djb2(name)

/****************** SET BIT COUNTING *****************/
size_t setBits_KnR_uint64_t(uint64_t flags);

/******************** tnecs_arena *******************/
i64 tnecs_arena_push(tnecs_arena *arena, i64 size);

#ifdef __cplusplus
}
#endif
#endif // __TNECS_H__
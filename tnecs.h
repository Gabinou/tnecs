#ifndef __TNECS_H__
#define __TNECS_H__
/*
**  Copyright (C) Gabriel Taillon 2023-2025
**  Licensed under GPLv3
**
**      Éloigne de moi l'esprit d'oisiveté, de
**          découragement, de domination et de
**          vaines paroles.
**      Accorde-moi l'esprit d'intégrité,
**          d'humilité, de patience et de charité.
**      Donne-moi de voir mes fautes.
**
***************************************************
**
** tnecs: Tiny C99 Entity-Component-System (ECS) library.
**      
**  The simplest possible C99 ECS library, 
**  only with the minimum necessary features. 
**
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <math.h>
#ifndef log2 /* for tcc: log2(x) = log(x) / log(2) */
    #define log2(x) (log(x) * 1.44269504088896340736)
#endif

/******************** DEBUG *********************/
#define TNECS_CHECK_ALLOC(name) do {\
        if (name == NULL) { \
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

/* --- TYPEDEFS --- */
typedef unsigned long long int  tnecs_ns;
typedef unsigned long long int  tnecs_Ph;
typedef unsigned long long int  tnecs_E;
typedef unsigned long long int  tnecs_Pi;
typedef unsigned long long int  tnecs_C;

/* -- Forward declarations -- */
struct tnecs_In;
struct tnecs_W;
typedef struct tnecs_In tnecs_In;
typedef struct tnecs_W  tnecs_W;

/* -- Functions -- */
typedef void (*tnecs_S)(tnecs_In *);
typedef void (*tnecs_free_f)(void *);
typedef void (*tnecs_init_f)(void *);

/* --- CONSTANTS --- */
enum TNECS_PUBLIC {
    TNECS_NULL                  =         0,
    TNECS_NULLSHIFT             =         1,
    TNECS_E_CAP          = 100000000,
    TNECS_Pi_CAP         =        64,
    TNECS_Ph_CAP            =        64,
    TNECS_C_CAP         =        64,
    TNECS_ARRAY_GROWTH   =         2
};

/* --- UTILITY MACROS --- */
#define TNECS_CONCAT( arg1, arg2) TNECS_CONCAT1(arg1, arg2)
#define TNECS_CONCAT1(arg1, arg2) TNECS_CONCAT2(arg1, arg2)
#define TNECS_CONCAT2(arg1, arg2) arg1##arg2
#define TNECS_ARCHETYPE_HAS_TYPE(archetype, type) \
    ((archetype & type) > 0)
#define TNECS_ARCHETYPE_IS_SUBTYPE(archetype1, archetype2) \
    ((archetype1 & archetype2) == archetype1)

/* --- HACKY DISTRIBUTION FOR VARIADIC MACROS --- */
/* Distribution as in algebra: a(x + b) -> ax + ab */

// TNECS_VAR_EACH_ARGN(__VA_ARGS__) counts the number of args
//  - up to 63, if TNECS_VAR_ARGN and TNECS_VAR_VARG_SEQ exist
#define TNECS_VAR_EACH_ARGN(...) \
    TNECS_VAR_EACH_ARGN_(__VA_ARGS__, TNECS_VAR_VARG_SEQ())
#define TNECS_VAR_EACH_ARGN_(...) \
    TNECS_VAR_ARGN(__VA_ARGS__)
#define TNECS_VAR_ARGN(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) \
    N
#define TNECS_VAR_VARG_SEQ() \
    8, 7, 6, 5, 4, 3, 2, 1, 0

// TNECS_VARMACRO_COMMA(__VA_ARGS__) puts commas after each arg,
// except the last.
//  - up to 63 args if all TNECS_COMMA_N exist
#define TNECS_VARMACRO_COMMA(...) \
    TNECS_VARMACRO_COMMA_(\
        TNECS_VAR_EACH_ARGN(__VA_ARGS__), \
        __VA_ARGS__\
    )
#define TNECS_VARMACRO_COMMA_(N, ...) \
    TNECS_CONCAT(TNECS_COMMA_, N)(__VA_ARGS__)

#define TNECS_COMMA_1(x)      x
#define TNECS_COMMA_2(x, ...) x, TNECS_COMMA_1(__VA_ARGS__)
#define TNECS_COMMA_3(x, ...) x, TNECS_COMMA_2(__VA_ARGS__)
#define TNECS_COMMA_4(x, ...) x, TNECS_COMMA_3(__VA_ARGS__)
#define TNECS_COMMA_5(x, ...) x, TNECS_COMMA_4(__VA_ARGS__)
#define TNECS_COMMA_6(x, ...) x, TNECS_COMMA_5(__VA_ARGS__)
#define TNECS_COMMA_7(x, ...) x, TNECS_COMMA_6(__VA_ARGS__)
#define TNECS_COMMA_8(x, ...) x, TNECS_COMMA_7(__VA_ARGS__)

/* ---  STRUCTS --- */

/* --- WORLD --- */
int tnecs_world_genesis(tnecs_W **w);
int tnecs_world_destroy(tnecs_W **w);

void tnecs_world_toggle_reuse(tnecs_W *w, int toggle);

/* Run all systems in all pipelines, by phases */
int tnecs_world_step(tnecs_W *world, tnecs_ns deltat, void *data);

/* --- PIPELINES --- */
/* Run all systems in pipeline, by phases */
int tnecs_pipeline_step(tnecs_W *w, tnecs_ns deltat,
                        void *data, tnecs_Pi pipeline);
/* Run all systems in input pipeline and input phase */
int tnecs_pipeline_step_phase(
    tnecs_W *w, tnecs_ns deltat,
    void *data, tnecs_Pi pipeline,
    tnecs_Ph phase);

#define TNECS_PIPELINE_GET(world, pipeline) \
    &world->pipelines.byphase[(pipeline)]

/* --- SYSTEM --- */
int tnecs_system_run(
    tnecs_W *w, size_t id,
    tnecs_ns deltat, void *data);

int tnecs_custom_system_run(
    tnecs_W     *w,     tnecs_S    c,    
    tnecs_C  ar,    tnecs_ns            dt,    
    void            *data);

/* --- REGISTRATION --- */
/* Phases start at 1, increment every call. */
size_t tnecs_register_phase(    tnecs_W    *w, 
                                tnecs_Pi  p);
/* Pipelines start at 1, increment every call. */
size_t tnecs_register_pipeline( tnecs_W    *w);

size_t tnecs_register_system(
    tnecs_W         *w,
    tnecs_S     system,
    tnecs_Pi       pipe,
    tnecs_Ph          p,    
    int                  isExclusive,
    size_t               num, 
    tnecs_C      archetype);
#define TNECS_REGISTER_SYSTEM(world, pfunc, pipeline, phase, excl, ...) \
    tnecs_register_system(\
        world, &pfunc, pipeline, phase, excl, \
        TNECS_VAR_EACH_ARGN(__VA_ARGS__), \
        tnecs_component_ids2archetype(\
            TNECS_VAR_EACH_ARGN(__VA_ARGS__), \
            TNECS_VARMACRO_COMMA(__VA_ARGS__)\
        )\
    )

tnecs_C tnecs_register_component(
    tnecs_W    *w,      size_t          b,
    tnecs_free_f  ffree,  tnecs_init_f  finit);

#define TNECS_REGISTER_COMPONENT(world, name, ffinit, ffree) \
    tnecs_register_component(world, sizeof(name), ffinit, ffree)

/* --- ENTITY --- */
tnecs_E tnecs_entity_isOpen( 
    tnecs_W *w, tnecs_E ent);
tnecs_E tnecs_entity_create( 
    tnecs_W *w);
tnecs_E tnecs_entity_destroy(
    tnecs_W *w, tnecs_E ent);
tnecs_E tnecs_entity_create_wcomponents(
    tnecs_W *w, size_t argnum, ...);

tnecs_E tnecs_entity_add_components(
    tnecs_W     *w,         tnecs_E    eID,
    tnecs_C  archetype, int             isNew);
tnecs_E tnecs_entity_remove_components(
    tnecs_W *w, tnecs_E eID,
    tnecs_C archetype);

int tnecs_entities_open_reuse(tnecs_W *w);
int tnecs_entities_open_flush(tnecs_W *w);

#define TNECS_ENTITY_CREATE_wCOMPONENTS(world, ...) \
    tnecs_entity_create_wcomponents(\
        world, \
        TNECS_VAR_EACH_ARGN(__VA_ARGS__), \
        TNECS_VARMACRO_COMMA(__VA_ARGS__)\
    )
#define TNECS_ENTITY_EXISTS(world, index) \
    ((index != TNECS_NULL) && (world->entities.id[index] == index))
#define TNECS_ENTITY_ARCHETYPE(world, entity) \
    world->entities.archetypes[entity]

/* --- COMPONENT --- */
void *tnecs_get_component(
    tnecs_W *w, tnecs_E eID,
    tnecs_C cID);

#define TNECS_ENTITY_HASCOMPONENT(world, entity, cID) \
    (\
        (\
            world->entities.archetypes[entity] & \
            tnecs_component_ids2archetype(1, cID)\
        ) > 0\
    )
#define TNECS_ADD_COMPONENT(...) \
    TNECS_CHOOSE_ADD_COMPONENT(\
        __VA_ARGS__, \
        TNECS_ADD_COMPONENT4, \
        TNECS_ADD_COMPONENT3\
    )(__VA_ARGS__)
#define TNECS_CHOOSE_ADD_COMPONENT(_1,_2,_3,_4,NAME,...) \
    NAME
#define TNECS_ADD_COMPONENT3(world, entity_id, cID) \
    tnecs_entity_add_components(\
        world, \
        entity_id, \
        tnecs_component_ids2archetype(1, cID), \
        true\
    )
#define TNECS_ADD_COMPONENT4(world, entity_id, cID, isnewtype) \
    tnecs_entity_add_components(\
        world, \
        entity_id, \
        tnecs_component_ids2archetype(1, cID), \
        isnewtype\
    )
#define TNECS_ADD_COMPONENTS(world, entity_id, isnewtype, ...) \
    tnecs_entity_add_components(\
        world, \
        entity_id, \
        tnecs_component_ids2archetype(\
            TNECS_VAR_EACH_ARGN(__VA_ARGS__), \
            TNECS_VARMACRO_COMMA(__VA_ARGS__)\
        ), \
        isnewtype\
    )
#define TNECS_REMOVE_COMPONENTS(world, entity_id, ...) \
    tnecs_entity_remove_components(\
        world, \
        entity_id, \
        tnecs_component_ids2archetype(\
            TNECS_VAR_EACH_ARGN(__VA_ARGS__), \
            TNECS_VARMACRO_COMMA(__VA_ARGS__)\
        )\
    )

/* --- COMPONENT ARRAY --- */
void *tnecs_component_array(
        tnecs_W *w, const size_t cID,
        const size_t tID);

#define TNECS_COMPONENT_ARRAY(input, cID) \
    tnecs_component_array(\
        input->world, \
        cID, \
        input->entity_archetype_id\
    )

/* --- ARCHETYPES --- */
tnecs_C tnecs_component_ids2archetype(
    size_t argnum, ...);
tnecs_C tnecs_archetypeid(
    const tnecs_W *const w, tnecs_C arch);

#define TNECS_COMPONENT_ID2TYPE(id) \
    (((id >= TNECS_NULLSHIFT) && (id < TNECS_C_CAP)) ? (1ULL << (id - TNECS_NULLSHIFT)) : 0ULL)
#define TNECS_COMPONENT_TYPE2ID(type) \
    (type >= 1 ? (tnecs_C)(log2(type) + 1.1f) : 0) 
#define TNECS_COMPONENT_IDS2ARCHETYPE(...) \
    tnecs_component_ids2archetype(\
        TNECS_VAR_EACH_ARGN(__VA_ARGS__), \
        TNECS_VARMACRO_COMMA(__VA_ARGS__)\
    )
#define TNECS_COMPONENT_IDS2ARCHETYPEID(world, ...) \
    tnecs_archetypeid(\
        world, \
        TNECS_COMPONENT_IDS2ARCHETYPE(__VA_ARGS__)\
    )

/* --- SYSTEM --- */
#define TNECS_SYSTEM_ID2ARCHETYPE(world, id) \
    world->systems.archetypes[id]

/* --- PHASE --- */
#define TNECS_PHASE_VALID(world, pipeline, phase) \
    (phase < world->pipelines.byphase[pipeline].num)

/* --- PIPELINE --- */
#define TNECS_PIPELINE_VALID(world, pipeline) \
    (pipeline < world->pipelines.num)

#endif /* __TNECS_H__ */

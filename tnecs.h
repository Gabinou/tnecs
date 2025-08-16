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
**  Glossary:
**      - E:    Entity
**      - C:    Component
**      - S:    System
**      - A:    Archetype  i.e. ull w/ many bits set
**      - T:    Type       i.e. ull w/ single bit set 
**      - Pi:   Pipeline
**      - Ph:   Phase
**      - W:    World
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

/* --- TYPEDEFS --- */
typedef unsigned long long int  tnecs_ns;
typedef unsigned long long int  tnecs_Ph;
typedef unsigned long long int  tnecs_E;
typedef unsigned long long int  tnecs_Pi;
typedef unsigned long long int  tnecs_C;

/* -- Forward declarations -- */
struct tnecs_W;
struct tnecs_In;
typedef struct tnecs_W  tnecs_W;
typedef struct tnecs_In tnecs_In;

/* -- Functions -- */
typedef void (*tnecs_S)(tnecs_In *);
typedef void (*tnecs_free_f)(void *);
typedef void (*tnecs_init_f)(void *);

/* --- CONSTANTS --- */
enum TNECS_PUBLIC {
    TNECS_NULL           =         0,
    TNECS_NULLSHIFT      =         1,
    TNECS_E_CAP          = 100000000,
    TNECS_Pi_CAP         =        64,
    TNECS_Ph_CAP         =        64,
    TNECS_C_CAP          =        64
};

/* --- UTILITY MACROS --- */
#define TNECS_CONCAT( arg1, arg2) TNECS_CONCAT1(arg1, arg2)
#define TNECS_CONCAT1(arg1, arg2) TNECS_CONCAT2(arg1, arg2)
#define TNECS_CONCAT2(arg1, arg2) arg1##arg2

#define TNECS_A_HAS_T(A, T)         ((A & T) > 0)
#define TNECS_A_IS_subT(A1, A2)     ((A1 & A2) == A1)

/* --- HACKY DISTRIBUTION FOR VARIADIC MACROS --- */
/* Distribution as in algebra: a(x + b) -> ax + ab */

/* TNECS_ARGN(__VA_ARGS__) counts the number of args,
**  _TNECS_SEQ pushes the args so correct N is output.
**  Example: 
**      1. TNECS_ARGN(x,y,z)
**      2. _TNECS_ARGN_(x, y, z, _TNECS_SEQ())
**      3. _TNECS_VARGN(x, y, z, 8, 7, 6, 5, 4, 3, 2, 1, 0)
**         i.e.         _1,_2,_3,_4,_5,_6,_7,_8,N, ...  
**      4. 3 is output
**  - up to 63 args, if _TNECS_VARGN and TNECS_VARG_SEQ exist
*/
#define TNECS_ARGN(...) _TNECS_ARGN_(__VA_ARGS__, _TNECS_SEQ())
#define _TNECS_ARGN_(...) _TNECS_VARGN(__VA_ARGS__)
#define _TNECS_VARGN(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N
#define _TNECS_SEQ() 8, 7, 6, 5, 4, 3, 2, 1, 0

/* TNECS_EACH_COMMA puts commas after each arg except last. 
**  - up to 63 args, if all TNECS_COMMA_N exist */
#define TNECS_EACH_COMMA(...) \
    TNECS_EACH_COMMA_(\
        TNECS_ARGN(__VA_ARGS__), \
        __VA_ARGS__\
    )
#define TNECS_EACH_COMMA_(N, ...) \
    TNECS_CONCAT(TNECS_COMMA_, N)(__VA_ARGS__)

#define TNECS_COMMA_1(x)      x
#define TNECS_COMMA_2(x, ...) x, TNECS_COMMA_1(__VA_ARGS__)
#define TNECS_COMMA_3(x, ...) x, TNECS_COMMA_2(__VA_ARGS__)
#define TNECS_COMMA_4(x, ...) x, TNECS_COMMA_3(__VA_ARGS__)
#define TNECS_COMMA_5(x, ...) x, TNECS_COMMA_4(__VA_ARGS__)
#define TNECS_COMMA_6(x, ...) x, TNECS_COMMA_5(__VA_ARGS__)
#define TNECS_COMMA_7(x, ...) x, TNECS_COMMA_6(__VA_ARGS__)
#define TNECS_COMMA_8(x, ...) x, TNECS_COMMA_7(__VA_ARGS__)

/* --- WORLD --- */
int tnecs_W_genesis(tnecs_W **w);
int tnecs_W_destroy(tnecs_W **w);

void tnecs_W_reuse(tnecs_W *w, int toggle);

/* Run all systems in all pipelines, by phases */
int tnecs_W_step(   tnecs_W *w, tnecs_ns dt, 
                    void    *data);

/* --- PIPELINES --- */
/* Run all systems in pipeline, by phases */
int tnecs_Pi_step(  tnecs_W *w,     tnecs_ns dt,
                    void    *data,  tnecs_Pi pi);
/* Run all systems in input pipeline and input phase */
int tnecs_Pi_step_Ph(   tnecs_W     *w, tnecs_ns dt,
                        void        *d, tnecs_Pi pi,
                        tnecs_Ph     ph);

#define TNECS_Pi_GET(world, pipeline) \
    &world->pipelines.byphase[(pipeline)]

/* --- SYSTEM --- */
int tnecs_S_run(tnecs_W *w,     size_t   id,
                tnecs_ns dt,    void    *data);

int tnecs_custom_S_run( tnecs_W *w,     tnecs_S     s,    
                        tnecs_C  a,     tnecs_ns    dt,    
                        void    *data);

/* --- REGISTRATION --- */
/* Phases start at 1, increment every call. */
size_t tnecs_register_Ph(    tnecs_W    *w, 
                                tnecs_Pi  p);
/* Pipelines start at 1, increment every call. */
size_t tnecs_register_Pi( tnecs_W    *w);

size_t tnecs_register_S(
    tnecs_W         *w,
    tnecs_S     system,
    tnecs_Pi       pipe,
    tnecs_Ph          p,    
    int         isExclusive,
    size_t               num, 
    tnecs_C      archetype);

#define TNECS_REGISTER_S(world, pfunc, pipeline, phase, excl, ...) \
    tnecs_register_S(\
        world, pfunc, pipeline, phase, excl, \
        TNECS_ARGN(__VA_ARGS__), \
        tnecs_C_ids2A(\
            TNECS_ARGN(__VA_ARGS__), \
            TNECS_EACH_COMMA(__VA_ARGS__)\
        )\
    )

tnecs_C tnecs_register_C(   tnecs_W         *w,
                            size_t           b,
                            tnecs_free_f     ffree,  
                            tnecs_init_f     finit);

#define TNECS_REGISTER_C(world, name, ffinit, ffree) \
    tnecs_register_C(world, sizeof(name), ffinit, ffree)

/* --- ENTITY --- */
tnecs_E tnecs_E_isOpen(     tnecs_W *w, tnecs_E ent);
tnecs_E tnecs_E_create(     tnecs_W *w);
tnecs_E tnecs_E_destroy(    tnecs_W *w, tnecs_E ent);
tnecs_E tnecs_E_create_wC(  tnecs_W *w, size_t argnum, ...);

tnecs_E tnecs_E_add_C(  tnecs_W *w, tnecs_E eID,
                        tnecs_C  A, int     isNew);
tnecs_E tnecs_E_rm_C(   tnecs_W *w, tnecs_E eID, 
                        tnecs_C A);

int tnecs_E_reuse(tnecs_W *w);
int tnecs_E_flush(tnecs_W *w);

#define TNECS_E_CREATE_wC(world, ...) \
    tnecs_E_create_wC(\
        world, \
        TNECS_ARGN(__VA_ARGS__), \
        TNECS_EACH_COMMA(__VA_ARGS__)\
    )
#define TNECS_E_EXISTS(world, index) \
    (\
        (index != TNECS_NULL) && \
        (world->entities.id[index] == index) \
    )
#define TNECS_E_A(world, entity) \
    world->entities.archetypes[entity]

/* --- COMPONENT --- */
void *tnecs_get_C(tnecs_W *w, tnecs_E eID, tnecs_C cID);

#define TNECS_E_HAS_C(world, entity, cID) \
    (\
        (\
            world->entities.archetypes[entity] & \
            tnecs_C_ids2A(1, cID)\
        ) > 0\
    )
#define TNECS_ADD_C(...) \
    TNECS_CHOOSE_ADD_C(\
        __VA_ARGS__, \
        TNECS_ADD_C4, \
        TNECS_ADD_C3\
    )(__VA_ARGS__)
#define TNECS_CHOOSE_ADD_C(_1,_2,_3,_4,NAME,...) \
    NAME
#define TNECS_ADD_C3(world, entity_id, cID) \
    tnecs_E_add_C(\
        world, \
        entity_id, \
        tnecs_C_ids2A(1, cID), \
        true\
    )
#define TNECS_ADD_C4(world, entity_id, cID, isnewtype) \
    tnecs_E_add_C(\
        world, \
        entity_id, \
        tnecs_C_ids2A(1, cID), \
        isnewtype\
    )
#define TNECS_ADD_Cs(world, entity_id, isnewtype, ...) \
    tnecs_E_add_C(\
        world, \
        entity_id, \
        tnecs_C_ids2A(\
            TNECS_ARGN(__VA_ARGS__), \
            TNECS_EACH_COMMA(__VA_ARGS__)\
        ), \
        isnewtype\
    )
#define TNECS_REMOVE_C(world, entity_id, ...) \
    tnecs_E_rm_C(\
        world, \
        entity_id, \
        tnecs_C_ids2A(\
            TNECS_ARGN(__VA_ARGS__), \
            TNECS_EACH_COMMA(__VA_ARGS__)\
        )\
    )

/* --- COMPONENT ARRAY --- */
void *tnecs_C_array(tnecs_W         *w, 
                    const size_t     cID,
                    const size_t     tID);

#define TNECS_C_ARRAY(input, cID) \
    tnecs_C_array(\
        input->world, \
        cID, \
        input->entity_archetype_id\
    )

/* --- ARCHETYPES --- */
tnecs_C tnecs_C_ids2A(size_t argnum, ...);
tnecs_C tnecs_A_id(const tnecs_W *const w, tnecs_C arch);

#define TNECS_C_ID2T(id) \
    ( \
        ((id >= TNECS_NULLSHIFT) && (id < TNECS_C_CAP)) ? \
        (1ULL << (id - TNECS_NULLSHIFT)) : \
        0ULL \
    )
#define TNECS_C_T2ID(type) \
    (type >= 1 ? (tnecs_C)(log2(type) + 1.1f) : 0) 
#define TNECS_C_IDS2A(...) \
    tnecs_C_ids2A(\
        TNECS_ARGN(__VA_ARGS__), \
        TNECS_EACH_COMMA(__VA_ARGS__)\
    )
#define TNECS_C_IDS2AID(world, ...) \
    tnecs_A_id(world, TNECS_C_IDS2A(__VA_ARGS__))

/* --- SYSTEM --- */
#define TNECS_S_ID2A(world, id) \
    world->systems.archetypes[id]

/* --- PHASE --- */
#define TNECS_Ph_VALID(world, pipeline, phase) \
    (phase < world->pipelines.byphase[pipeline].num)

/* --- PIPELINE --- */
#define TNECS_Pi_VALID(world, pipeline) \
    (pipeline < world->pipelines.num)

#endif /* __TNECS_H__ */

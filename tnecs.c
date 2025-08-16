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
*/

#include "tnecs.h"

/* --- Early return on fail --- */
#define TNECS_CHECK(check) do {\
        if (!check) { \
            printf("tnecs: failed " #check "\n"); \
            return(0); \
        } \
    } while (0)

/* --- PRIVATE DECLARATIONS --- */
typedef unsigned char tnecs_byte;

enum TNECS_PRIVATE {
    TNECS_E_0LEN        = 128,
    TNECS_Ph_0LEN       =   4,
    TNECS_Pi_0LEN       =   4,
    TNECS_C_0LEN        =   8,
    TNECS_S_0LEN        =  16,
    TNECS_A_0LEN        =  16,
    TNECS_ARR_GROW  =   2
};

/* -- struct -- */
typedef struct tnecs_carr { /* 1D array of components */
    tnecs_C  type;
    size_t   num;
    size_t   len;
    void    *components; /* [E_order_byT] */
} tnecs_carr;

typedef struct tnecs_array {
    void    *arr;
    size_t   num;
    size_t   len;
} tnecs_array;

typedef struct tnecs_phases {
    size_t num;
    size_t len;

    size_t   *len_systems; /* [ph_id] */
    size_t   *num_systems; /* [ph_id] */
    size_t  **systems_id;  /* [ph_id][S_order] */
    tnecs_S **systems;     /* [ph_id][system_id]    */
} tnecs_phases;

typedef struct tnecs_pipelines {
    size_t num;
    size_t len;

    tnecs_phases *byphase;   /* [pipeline_id] */
} tnecs_pipelines;

typedef struct tnecs_entities {
    // - .num doesn't change even if entities get deleted
    // - if reuse_entities: add deleted entities to entities_open
    //      - Call tnecs_E_reuse to add entities with
    //        id[ent] == false entities_open.
    size_t num;
    size_t len;

    tnecs_E     *id;            /* [entity_id] -> eID */
    size_t      *orders;        /* [eID] */
    tnecs_C     *archetypes;    /* [eID] */
    tnecs_array  open;
} tnecs_entities;

typedef struct tnecs_system {
    size_t num;
    size_t len;

    tnecs_Ph    *phases;        /* [S_id] -> sID */
    size_t      *orders;        /* [sID] */
    int         *exclusive;     /* [sID] */
    tnecs_C     *archetypes;    /* [sID] */
    tnecs_Pi    *pipeline;      /* [sID] */
#ifndef NDEBUG
    /* Systems that might be run in current pipeline */
    tnecs_array to_run;
    /* Systems ran in pipeline, if num_entities > 0 */
    tnecs_array ran;
#endif /* NDEBUG */
} tnecs_system;

typedef struct tnecs_archetype {
    size_t num;
    size_t len;

    tnecs_C      *id;                   /* [A_id] -> aID */
    size_t       *num_Cs;               /* [aID] */
    size_t       *len_entities;         /* [aID] */
    size_t       *num_entities;         /* [aID] */
    size_t       *num_archetype_ids;    /* [aID] */

    size_t      **archetype_id;     /* [aID][A_id_order]    */
    tnecs_E     **entities;         /* [aID][E_order_byT]   */
    size_t      **components_order; /* [aID][cID]           */
    tnecs_C     **components_id;    /* [aID][C_order_byT]   */
    tnecs_carr  **components;       /* [aID][C_order_byT]   */
} tnecs_archetype;

typedef struct tnecs_Cs {
    size_t      num;
    size_t      bytesizes[TNECS_C_CAP]; /* [cID] */
    tnecs_init  finit[TNECS_C_CAP];     /* [cID] */
    tnecs_free  ffree[TNECS_C_CAP];     /* [cID] */
} tnecs_Cs;

struct tnecs_W {
    tnecs_system    systems;
    tnecs_entities  entities;
    tnecs_archetype byT;
    tnecs_pipelines pipelines;
    tnecs_Cs        components;

    int reuse_entities;
};

struct tnecs_In {
    tnecs_W  *world;
    tnecs_ns  deltat;
    tnecs_C   system_archetype;
    size_t     num_entities;
    size_t     entity_archetype_id;
    void      *data;
};


/* --- WORLD FUNCTIONS --- */
static int _tnecs_breath_phases(        tnecs_phases      *byphase);
static int _tnecs_breath_systems(       tnecs_system      *systems);
static int _tnecs_breath_entities(      tnecs_entities    *entities);
static int _tnecs_breath_pipelines(     tnecs_pipelines   *pipelines);
static int _tnecs_breath_C(             tnecs_Cs  *components);
static int _tnecs_breath_archetypes(    tnecs_archetype   *byT);

static int _tnecs_destroy_phases(       tnecs_phases     *byphase);
static int _tnecs_destroy_systems(      tnecs_system     *systems);
static int _tnecs_destroy_entities(     tnecs_entities   *entities);
static int _tnecs_destroy_pipelines(    tnecs_pipelines  *pipelines);
static int _tnecs_destroy_archetypes(   tnecs_archetype  *byT);

/* --- REGISTRATION  --- */
static size_t _tnecs_register_archetype( 
    tnecs_W *w, size_t num_c, tnecs_C a);

/* --- SET BIT COUNTING --- */
static size_t setBits_KnR(tnecs_C flags);

/* --- "DYNAMIC" ARRAYS --- */
static void *tnecs_arrdel(  void *arr,  size_t elem,
                            size_t len, size_t bytesize);
static void *tnecs_realloc( void *ptr,  size_t olen,
                            size_t len, size_t bytesize);

#ifndef NDEBUG
static int tnecs_grow_ran(          tnecs_W *w);
#endif /* NDEBUG */

static int tnecs_grow_phase(        tnecs_W *w, tnecs_Pi    pi);
static int tnecs_grow_byT(          tnecs_W *w, size_t      aID);
static int tnecs_grow_E(            tnecs_W *w);
static int tnecs_grow_system(       tnecs_W *w);
static int tnecs_grow_pipeline(     tnecs_W *w);
static int tnecs_grow_archetype(    tnecs_W *w);
static int tnecs_grow_entities_open(tnecs_W *w);

static int tnecs_grow_system_byphase(tnecs_phases   *byphase,
                                     tnecs_Ph        phase);
static int tnecs_grow_C_array(  tnecs_W     *w, 
                                tnecs_carr  *comp_arr, 
                                size_t tID, size_t corder);

/* --- UTILITIES --- */
static size_t tnecs_C_order_byT( const tnecs_W *const w,
                                    size_t cID, tnecs_C arch);
static size_t tnecs_C_order_byTid(   const tnecs_W *const w, 
                                        size_t cID, size_t aID);

/* --- COMPONENT ARRAY --- */
static int tnecs_carr_new(  tnecs_W *w, size_t num,
                            tnecs_C a);
static int tnecs_carr_init( tnecs_W *w, tnecs_carr *arr,
                            size_t cID);

/* --- byT --- */
static int tnecs_entitiesbyT_add(tnecs_W *w, tnecs_E e,
                                    tnecs_C nt);
static int tnecs_entitiesbyT_del(tnecs_W *w, tnecs_E e,
                                    tnecs_C ot);
static int tnecs_entitiesbyT_migrate(tnecs_W *w, tnecs_E e,
                                        tnecs_C ot, tnecs_C nt);

/* --- COMPONENT --- */
static int tnecs_C_add( tnecs_W *w,     tnecs_C flag);
static int tnecs_C_del( tnecs_W *w,     tnecs_E ent,
                        tnecs_C of);
static int tnecs_C_init(tnecs_W *w,     tnecs_E ent,
                        tnecs_C of);
static int tnecs_C_free(tnecs_W *w,     tnecs_E ent,
                        tnecs_C of);
static int tnecs_C_run( tnecs_W *w,     tnecs_E    ent,
                        tnecs_C  of,    tnecs_free *f);

static int tnecs_C_copy(tnecs_W *w,     tnecs_E ent, 
                        tnecs_C  of,    tnecs_C nf);
static int tnecs_C_migrate( tnecs_W *w,     tnecs_E ent,
                            tnecs_C  of,    tnecs_C nf);

/************* WORLD FUNCTIONS ***************/
int tnecs_W_genesis(tnecs_W **world) {
    if (*world != NULL)
        TNECS_CHECK(tnecs_W_destroy(world));

    *world = calloc(1, sizeof(tnecs_W));
    TNECS_CHECK(*world);

    /* Allocate world members */
    TNECS_CHECK(_tnecs_breath_systems(   &((*world)->systems)));
    TNECS_CHECK(_tnecs_breath_entities(  &((*world)->entities)));
    TNECS_CHECK(_tnecs_breath_pipelines( &((*world)->pipelines)));
    TNECS_CHECK(_tnecs_breath_archetypes(&((*world)->byT)));
    TNECS_CHECK(_tnecs_breath_C(&((*world)->components)));
    return (1);
}

int tnecs_W_destroy(tnecs_W **world) {
    TNECS_CHECK(_tnecs_destroy_pipelines(   &((*world)->pipelines)));
    TNECS_CHECK(_tnecs_destroy_systems(     &((*world)->systems)));
    TNECS_CHECK(_tnecs_destroy_entities(    &((*world)->entities)));
    TNECS_CHECK(_tnecs_destroy_archetypes(  &((*world)->byT)));
    free(*world);

    *world = NULL;
    return (1);
}

int _tnecs_breath_C(tnecs_Cs *components) {
    components->num                   = TNECS_NULLSHIFT;
    components->bytesizes[TNECS_NULL] = TNECS_NULL;
    return (1);
}

int _tnecs_breath_entities(tnecs_entities *entities) {
    /* Variables */
    entities->num         = TNECS_NULLSHIFT;
    entities->len         = TNECS_E_0LEN;
    entities->open.len    = TNECS_E_0LEN;
    entities->open.num    = 0;

    /* Allocs */
    entities->id          = calloc(entities->len,
                                   sizeof(*entities->id));
    entities->orders      = calloc(entities->len,
                                   sizeof(*entities->orders));
    entities->open.arr    = calloc(entities->len,
                                   sizeof(tnecs_E));
    entities->archetypes  = calloc(entities->len,
                                   sizeof(*entities->archetypes));
    TNECS_CHECK(entities->id);
    TNECS_CHECK(entities->orders);
    TNECS_CHECK(entities->open.arr);
    TNECS_CHECK(entities->archetypes);
    return (1);
}

int _tnecs_breath_pipelines(tnecs_pipelines *pipelines) {
    pipelines->len  = TNECS_Pi_0LEN;
    pipelines->num  = TNECS_NULLSHIFT;

    pipelines->byphase = calloc(pipelines->len,
                                sizeof(*pipelines->byphase));
    _tnecs_breath_phases(&pipelines->byphase[TNECS_NULL]);
    return (1);
}

int _tnecs_breath_phases(tnecs_phases *byphase) {
    byphase->len    = TNECS_Ph_0LEN;
    byphase->num    = TNECS_NULLSHIFT;

    byphase->systems        = calloc(byphase->len,
                                     sizeof(*byphase->systems));
    byphase->systems_id     = calloc(byphase->len,
                                     sizeof(*byphase->systems_id));
    byphase->num_systems    = calloc(byphase->len,
                                     sizeof(*byphase->num_systems));
    byphase->len_systems    = calloc(byphase->len,
                                     sizeof(*byphase->len_systems));

    TNECS_CHECK(byphase->systems);
    TNECS_CHECK(byphase->systems_id);
    TNECS_CHECK(byphase->num_systems);
    TNECS_CHECK(byphase->len_systems);

    /* Alloc & check for entities_byphase elements */
    for (size_t i = 0; i < byphase->len; i++) {
        byphase->systems[i]     = calloc(byphase->len,
                                         sizeof(**byphase->systems));
        TNECS_CHECK(byphase->systems[i]);
        byphase->systems_id[i]  = calloc(byphase->len,
                                         sizeof(**byphase->systems_id));
        TNECS_CHECK(byphase->systems_id[i]);

        byphase->num_systems[i] = 0;
        byphase->len_systems[i] = byphase->len;
    }
    return (1);
}

int _tnecs_breath_systems(tnecs_system *systems) {
    /* Variables */
    systems->len        = TNECS_S_0LEN;
    systems->num        = TNECS_NULLSHIFT;
#ifndef NDEBUG
    systems->to_run.len = TNECS_S_0LEN;
    systems->ran.len    = TNECS_S_0LEN;
#endif /* NDEBUG */

    /* Allocs */
    systems->phases     = calloc(systems->len, 
                                 sizeof(*systems->phases));
    systems->orders     = calloc(systems->len,
                                 sizeof(*systems->orders));
    systems->pipeline   = calloc(systems->len,
                                 sizeof(*systems->pipeline));
    systems->exclusive  = calloc(systems->len,
                                 sizeof(*systems->exclusive));
    systems->archetypes = calloc(systems->len,
                                 sizeof(*systems->archetypes));
#ifndef NDEBUG
    systems->ran.arr    = calloc(systems->ran.len,
                                 sizeof(tnecs_S));
    systems->to_run.arr = calloc(systems->to_run.len,
                                 sizeof(tnecs_S));
#endif /* NDEBUG */

    TNECS_CHECK(systems->phases);
    TNECS_CHECK(systems->orders);
    TNECS_CHECK(systems->pipeline);
    TNECS_CHECK(systems->exclusive);
    TNECS_CHECK(systems->archetypes);
#ifndef NDEBUG
    TNECS_CHECK(systems->ran.arr);
    TNECS_CHECK(systems->to_run.arr);
#endif /* NDEBUG */

    return (1);
}

int _tnecs_breath_archetypes(tnecs_archetype *byT) {
    /* Variables */
    byT->num = TNECS_NULLSHIFT;
    byT->len = TNECS_A_0LEN;

    /* Allocs */
    byT->id                  = calloc(byT->len,
                                         sizeof(*byT->id));
    byT->entities            = calloc(byT->len,
                                         sizeof(*byT->entities));
    byT->components          = calloc(byT->len,
                                         sizeof(*byT->components));
    byT->len_entities        = calloc(byT->len,
                                         sizeof(*byT->len_entities));
    byT->num_entities        = calloc(byT->len,
                                         sizeof(*byT->num_entities));
    byT->archetype_id        = calloc(byT->len,
                                         sizeof(*byT->archetype_id));
    byT->components_id       = calloc(byT->len,
                                         sizeof(*byT->components_id));
    byT->num_Cs      = calloc(byT->len,
                                         sizeof(*byT->num_Cs));
    byT->components_order    = calloc(byT->len,
                                         sizeof(*byT->components_order));
    byT->num_archetype_ids   = calloc(byT->len,
                                         sizeof(*byT->num_archetype_ids));

    TNECS_CHECK(byT->id);
    TNECS_CHECK(byT->entities);
    TNECS_CHECK(byT->components);
    TNECS_CHECK(byT->archetype_id);
    TNECS_CHECK(byT->len_entities);
    TNECS_CHECK(byT->num_entities);
    TNECS_CHECK(byT->components_id);
    TNECS_CHECK(byT->num_Cs);
    TNECS_CHECK(byT->components_order);
    TNECS_CHECK(byT->num_archetype_ids);

    /* Alloc & check for id_byT elements */
    for (size_t i = 0; i < byT->len; i++) {
        byT->archetype_id[i] = calloc(TNECS_C_CAP, sizeof(**byT->archetype_id));
        TNECS_CHECK(byT->archetype_id[i]);
        byT->entities[i]     = calloc(TNECS_E_0LEN, sizeof(**byT->entities));
        TNECS_CHECK(byT->entities[i]);

        byT->num_entities[i] = 0;
        byT->len_entities[i] = TNECS_E_0LEN;
    }
    return (1);
}

static int _tnecs_destroy_phases(tnecs_phases *byphase) {
    for (size_t i = 0; i < byphase->len; i++) {
        if (byphase->systems != NULL)
            free(byphase->systems[i]);
        if (byphase->systems_id != NULL)
            free(byphase->systems_id[i]);
    }
    free(byphase->systems);
    free(byphase->systems_id);
    free(byphase->len_systems);
    free(byphase->num_systems);

    return(1);
}

static int _tnecs_destroy_systems(tnecs_system *systems) {
    free(systems->orders);
    free(systems->phases);
    free(systems->pipeline);
    free(systems->exclusive);
    free(systems->archetypes);
    #ifndef NDEBUG
    free(systems->to_run.arr);
    free(systems->ran.arr);
    #endif /* NDEBUG */

    return(1);
}

static int _tnecs_destroy_entities(tnecs_entities *entities) {
    free(entities->id);
    free(entities->orders);
    free(entities->open.arr);
    free(entities->archetypes);
    
    return(1);
}

static int _tnecs_destroy_pipelines(tnecs_pipelines *pipelines) {
    for (size_t i = 0; i < pipelines->len; i++) {
        _tnecs_destroy_phases(&pipelines->byphase[i]);
    }
    free(pipelines->byphase);
    
    return(1);
}

static int _tnecs_destroy_archetypes(tnecs_archetype *byT) {
    for (size_t i = 0; i < byT->len; i++) {
        if (byT->entities != NULL)
            free(byT->entities[i]);
        if (byT->components_id != NULL)
            free(byT->components_id[i]);
        if (byT->components_order != NULL)
            free(byT->components_order[i]);
        if (byT->archetype_id != NULL)
            free(byT->archetype_id[i]);
        if (byT->components != NULL) {
            for (size_t j = 0; j < byT->num_Cs[i]; j++) {
                free(byT->components[i][j].components);
            }
            free(byT->components[i]);
        }
    }

    free(byT->id);
    free(byT->entities);
    free(byT->components);
    free(byT->len_entities);
    free(byT->num_entities);
    free(byT->archetype_id);
    free(byT->components_id);
    free(byT->num_Cs);
    free(byT->components_order);
    free(byT->num_archetype_ids);

    return(1);
}

/********************* STEPPING ********************/
int tnecs_Pi_step(  tnecs_W     *world, tnecs_ns     deltat,
                    void        *data,  tnecs_Pi     pipeline) {
    #ifndef NDEBUG
    world->systems.to_run.num   = 0;
    world->systems.ran.num      = 0;
    #endif /* NDEBUG */

    tnecs_phases *byphase = TNECS_Pi_GET(world, pipeline);
    for (size_t phase = 0; phase < byphase->num; phase++) {
        TNECS_CHECK(tnecs_Pi_step_Ph(world, deltat, data, pipeline, phase));
    }

    return(1);
}

int tnecs_Pi_step_Ph(   tnecs_W  *world,    tnecs_ns  deltat,
                        void     *data,     tnecs_Pi  pipeline,
                        tnecs_Ph  phase) {
    tnecs_phases *byphase = TNECS_Pi_GET(world, pipeline);
    size_t num = byphase->num_systems[phase];
    for (size_t sorder = 0; sorder < num; sorder++) {
        size_t system_id = byphase->systems_id[phase][sorder];
        TNECS_CHECK(tnecs_S_run(world, system_id,
                     deltat, data));
    }

    return(1);
}

int tnecs_W_step(   tnecs_W    *world, tnecs_ns        deltat,
                    void           *data) {
    for (size_t p = 0; p < world->pipelines.num; p++) {
        TNECS_CHECK(tnecs_Pi_step(world, deltat, data, p));
    }
    return (1);
}

/************* SYSTEM FUNCTIONS ***************/
int tnecs_custom_S_run( tnecs_W     *world,
                        tnecs_S      custom_system,
                        tnecs_C      archetype,
                        tnecs_ns     deltat, 
                        void        *data) {
    /* Building the systems input */
    tnecs_In input = {.world = world, .deltat = deltat, .data = data};
    size_t tID = tnecs_A_id(world, archetype);
    if (tID == TNECS_NULL) {
        printf("tnecs: Input archetype is unknown.\n");
        return (0);
    }

    /* Running the exclusive custom system */
    input.entity_archetype_id   = tID;
    input.num_entities          = world->byT.num_entities[input.entity_archetype_id];
    custom_system(&input);

    /* Running the non-exclusive/inclusive custom system */
    for (size_t tsub = 0; tsub < world->byT.num_archetype_ids[tID]; tsub++) {
        input.entity_archetype_id   = world->byT.archetype_id[tID][tsub];
        input.num_entities          = world->byT.num_entities[input.entity_archetype_id];
        custom_system(&input);
    }
    return (1);
}

int tnecs_S_run(tnecs_W *world, size_t system_id,
                tnecs_ns deltat, void *data) {
    /* Building the systems input */
    tnecs_In input = {.world = world, .deltat = deltat, .data = data};
    size_t sorder     = world->systems.orders[system_id];
    tnecs_Ph phase    = world->systems.phases[system_id];
    tnecs_Pi pipeline = world->systems.pipeline[system_id];
    size_t system_archetype_id  = tnecs_A_id(world, world->systems.archetypes[system_id]);

    input.entity_archetype_id   = system_archetype_id;
    input.num_entities          = world->byT.num_entities[input.entity_archetype_id];
    tnecs_phases *byphase       = TNECS_Pi_GET(world, pipeline);

    while (world->systems.to_run.num >= (world->systems.to_run.len - 1)) {
        TNECS_CHECK(tnecs_grow_ran(world));
    }

    tnecs_S system = byphase->systems[phase][sorder];
#ifndef NDEBUG
    tnecs_S *system_ptr;
    size_t system_num;
    system_num              = world->systems.to_run.num++;
    system_ptr              = world->systems.to_run.arr;
    system_ptr[system_num]  = system;
#endif /* NDEBUG */

    /* - Running the exclusive systems in current phase - */
    if (input.num_entities > 0) {
        /* Skip running system if no entities! */
    #ifndef NDEBUG
        system_num              = world->systems.ran.num++;
        system_ptr              = world->systems.ran.arr;
        system_ptr[system_num]  = system;
    #endif /* NDEBUG */
        system(&input);
    }

    if (world->systems.exclusive[system_id])
        return (1);

    /* - Running the inclusive systems in current phase - */
    for (size_t tsub = 0; tsub < world->byT.num_archetype_ids[system_archetype_id]; tsub++) {

        input.entity_archetype_id   = world->byT.archetype_id[system_archetype_id][tsub];
        input.num_entities          = world->byT.num_entities[input.entity_archetype_id];
        tnecs_S system = byphase->systems[phase][sorder];

    #ifndef NDEBUG
        system_num              = world->systems.to_run.num++;
        system_ptr              = world->systems.to_run.arr;
        system_ptr[system_num]  = system;
    #endif /* NDEBUG */

        while (world->systems.to_run.num >= (world->systems.to_run.len - 1)) {
            TNECS_CHECK(tnecs_grow_ran(world));
        }

        if (input.num_entities <= 0) {
            continue;
        }

        #ifndef NDEBUG
            system_num              = world->systems.ran.num++;
            system_ptr              = world->systems.ran.arr;
            system_ptr[system_num]  = system;
        #endif /* NDEBUG */

        system(&input);
    }
    return (1);
}

/*************** REGISTRATION ***************/
size_t tnecs_register_S(tnecs_W *world,     tnecs_S     system,
                        tnecs_Pi pipeline,  tnecs_Ph    phase,
                        int isExclusive,    size_t      num_Cs,
                        tnecs_C archetype) {
    /* Check if phase exist */
    if (!TNECS_Pi_VALID(world, pipeline)) {
        printf("tnecs: System pipeline '%lld' is invalid.\n", pipeline);
        return (TNECS_NULL);
    }
    if (!TNECS_Ph_VALID(world, pipeline, phase)) {
        printf("tnecs: System phase '%lld' is invalid (pipeline '%lld').\n", phase, pipeline);
        return (TNECS_NULL);
    }

    /* Compute new id */
    size_t system_id = world->systems.num++;

    /* Realloc systems if too many */
    if (world->systems.num >= world->systems.len) {
        TNECS_CHECK(tnecs_grow_system(world));
    }

    /* Realloc systems_byphase if too many */
    tnecs_phases *byphase = TNECS_Pi_GET(world, pipeline);
    if (byphase->num_systems[phase] >= byphase->len_systems[phase]) {
        TNECS_CHECK(tnecs_grow_system_byphase(byphase, phase));
    }

    /* -- Actual registration -- */
    /* Note: phase is exclusive to pipeline */
    world->systems.phases[system_id]        = phase;
    world->systems.pipeline[system_id]      = pipeline;
    world->systems.exclusive[system_id]     = isExclusive;
    world->systems.archetypes[system_id]    = archetype;

    /* System order */
    size_t S_order                         = byphase->num_systems[phase]++;
    world->systems.orders[system_id]            = S_order;
    byphase->systems[phase][S_order]       = system;
    byphase->systems_id[phase][S_order]    = system_id;
    TNECS_CHECK(_tnecs_register_archetype(world, num_Cs, archetype));
    return (system_id);
}

tnecs_C tnecs_register_C(   tnecs_W     *world,
                            size_t       bytesize,
                            tnecs_free finit,
                            tnecs_free ffree) {
    /* Checks */
    if (bytesize <= 0) {
        printf("tnecs: Component should have >0 bytesize.\n");
        return (TNECS_NULL);
    }
    if (world->components.num >= TNECS_C_CAP) {
        printf("tnecs: Component capacity reached.\n");
        return (TNECS_NULL);
    }

    /* Registering */
    tnecs_C new_C_id    = world->components.num++;
    tnecs_C new_C_flag  = TNECS_C_ID2T(new_C_id);
    world->components.bytesizes[new_C_id]   = bytesize;
    world->components.ffree[new_C_id]       = ffree;
    world->components.finit[new_C_id]       = finit;
    TNECS_CHECK(_tnecs_register_archetype(world, 1, new_C_flag));
    return (new_C_id);
}

size_t _tnecs_register_archetype(tnecs_W    *world,
                                 size_t      num_Cs,
                                 tnecs_C     archetype_new) {
    // 0- Check if archetype exists, return
    for (size_t i = 0 ; i < world->byT.num; i++) {
        if (archetype_new == world->byT.id[i]) {
            return (i);
        }
    }

    // 1- Add new byT.components at [tID]
    if ((world->byT.num + 1) >= world->byT.len)
        TNECS_CHECK(tnecs_grow_archetype(world));
    world->byT.id[world->byT.num++] = archetype_new;
    size_t tID = tnecs_A_id(world, archetype_new);
    assert(tID == (world->byT.num - 1));
    world->byT.num_Cs[tID] = num_Cs;

    // 2- Add arrays to byT.components[tID] for each component
    TNECS_CHECK(tnecs_carr_new(world, num_Cs, archetype_new));

    // 3- Add all components to byT.components_id
    tnecs_C archetype_reduced = archetype_new, archetype_added = 0;
    size_t bytesize1 = sizeof(**world->byT.components_id);
    size_t bytesize2 = sizeof(**world->byT.components_order);
    world->byT.components_id[tID]     = calloc(num_Cs,      bytesize1);
    TNECS_CHECK(world->byT.components_id[tID]);
    world->byT.components_order[tID]  = calloc(TNECS_C_CAP, bytesize2);
    TNECS_CHECK(world->byT.components_order[tID]);

    size_t k = 0;
    while (archetype_reduced) {
        archetype_reduced &= (archetype_reduced - 1);

        tnecs_C component_type_toadd = (archetype_reduced + archetype_added) ^ archetype_new;
        archetype_added      += component_type_toadd;
        assert(component_type_toadd > 0);
        tnecs_C component_id_toadd   = TNECS_C_T2ID(component_type_toadd);

        world->byT.components_id[tID][k]      = component_id_toadd;
        world->byT.components_order[tID][component_id_toadd] = k++;
    }

    // 4- Check archetypes.
    for (size_t i = 1 ; i < world->byT.num; i++) {
        world->byT.num_archetype_ids[i] = 0;
        for (size_t j = 1 ; j < (world->byT.num); j++) {
            if (i == j)
                continue;

            if (!TNECS_A_IS_subT(world->byT.id[i], world->byT.id[j]))
                continue;

            // j is an archetype of i
            world->byT.archetype_id[i][world->byT.num_archetype_ids[i]++] = j;
        }
    }

    return (tID);
}

size_t tnecs_register_Pi(tnecs_W *world) {
    tnecs_Pi pipeline = world->pipelines.num++;
    while (pipeline >= world->pipelines.len) {
        TNECS_CHECK(tnecs_grow_pipeline(world));
    }
    tnecs_phases *byphase = TNECS_Pi_GET(world, pipeline);
    _tnecs_breath_phases(byphase);

    return (pipeline);
}

size_t tnecs_register_Ph(tnecs_W   *world, tnecs_Pi pipeline) {
    if (!TNECS_Pi_VALID(world, pipeline)) {
        printf("tnecs: Pipeline '%lld' is invalid for new phase.\n", pipeline);
        return (TNECS_NULL);
    }

    tnecs_phases *byphase   = TNECS_Pi_GET(world, pipeline);
    tnecs_Ph phase       = byphase->num++;
    while (phase >= byphase->len) {
        TNECS_CHECK(tnecs_grow_phase(world, pipeline));
    }
    return (phase);
}

/**************** ENTITY MANIPULATION **************/
tnecs_E tnecs_E_create(tnecs_W *world) {
    tnecs_E out = TNECS_NULL;

    /* Check if an open entity exists */
    tnecs_E *arr = world->entities.open.arr;
    while ((out == TNECS_NULL) &&
           (world->entities.open.num > 0) && 
           (world->entities.open.num < TNECS_E_CAP)
          ) {
        out = arr[--world->entities.open.num];
        arr[world->entities.open.num] = TNECS_NULL;
    }

    /* If no open entity existed, create one */
    if (out == TNECS_NULL) {
        do {
            if (world->entities.num >= world->entities.len) {
                if (!tnecs_grow_E(world)) {
                    printf("tnecs: Could not allocate more memory for entities.\n");
                    return (TNECS_NULL);
                }
            }
            out = world->entities.num++;
        } while (TNECS_E_EXISTS(world, out));
    }
    assert(out != TNECS_NULL);

    /* Set entity and checks  */
    world->entities.id[out] = out;
    TNECS_CHECK(tnecs_entitiesbyT_add(world, out, TNECS_NULL));
    assert(world->entities.id[out]                                          == out);
    assert(world->byT.entities[TNECS_NULL][world->entities.orders[out]]  == out);
    return (out);
}

tnecs_E tnecs_E_create_wC(tnecs_W *world, size_t argnum, ...) {
    /* Get archetype of all vararg components ids */
    va_list ap;
    va_start(ap, argnum);
    tnecs_C archetype = 0;
    for (size_t i = 0; i < argnum; i++) {
        tnecs_C component_id = va_arg(ap, tnecs_C);
        archetype += TNECS_C_ID2T(component_id);
    }
    va_end(ap);

    /* Create entity with all components */
    tnecs_E new_E = tnecs_E_create(world);
    if (new_E == TNECS_NULL) {
        printf("tnecs: could not create new entity\n");
        return (TNECS_NULL);
    }
    TNECS_CHECK(tnecs_E_add_C(world, new_E, archetype, 1));

#ifndef NDEBUG
    size_t tID      = tnecs_A_id(world, archetype);
    size_t order    = world->entities.orders[new_E];
    assert(world->byT.entities[tID][order]   == new_E);
    assert(world->entities.id[new_E]       == new_E);
#endif /* NDEBUG */

    return (new_E);
}

int tnecs_E_reuse(tnecs_W *world) {
    // Adds all null entities to open list
    for (tnecs_E i = TNECS_NULLSHIFT; i < world->entities.num; i++) {
        if (TNECS_E_EXISTS(world, i))
            continue; /* Skip if entity exists */

        if (tnecs_E_isOpen(world, i))
            continue; /* Skip if already in open list */

        TNECS_CHECK(tnecs_grow_entities_open(world));
        tnecs_E *arr = world->entities.open.arr;
        arr[world->entities.open.num++] = i;
    }
    return (1);
};

int tnecs_E_flush(tnecs_W *world) {
    /* Get rid of all entities in entities_open */
    world->entities.open.num = 0;
    return (1);
}

tnecs_E tnecs_E_isOpen(tnecs_W *world, tnecs_E entity) {
    if (entity <= TNECS_NULL) {
        return (0);
    }

    const tnecs_E * const open_arr = world->entities.open.arr;

    for (tnecs_E i = TNECS_NULLSHIFT; i < world->entities.open.num; i++) {
        if (open_arr[i] == entity) {
            return (1);
        }
    }
    return (0);
}

tnecs_E tnecs_E_destroy(tnecs_W *world, tnecs_E entity) {
    if (entity <= TNECS_NULL) {
        return (1);
    }

    if (!TNECS_E_EXISTS(world, entity)) {
        world->entities.id[entity]         = TNECS_NULL;
        world->entities.orders[entity]     = TNECS_NULL;
        world->entities.archetypes[entity] = TNECS_NULL;
        return (1);
    }

    /* Preliminaries */
    tnecs_C archetype   = world->entities.archetypes[entity];

    /* Delete components */
    TNECS_CHECK(tnecs_C_free(world, entity, archetype));
    TNECS_CHECK(tnecs_C_del(world, entity, archetype));

#ifndef NDEBUG
    size_t entity_order         = world->entities.orders[entity];
    size_t tID                  = tnecs_A_id(world, archetype);
    assert(world->byT.num_entities[tID] > TNECS_NULL);
    assert(world->byT.len_entities[tID] >= entity_order);
    assert(world->byT.num_entities[tID] > TNECS_NULL);
#endif /* NDEBUG */

    /* Delete entitiesbyT */
    TNECS_CHECK(tnecs_entitiesbyT_del(world, entity, archetype));

    /* Delete entity */
    world->entities.id[entity]         = TNECS_NULL;

    // Note: reuse_entities used to add to entities_open, so that
    // user can call tnecs_E_reuse to reuse entities manually.
    if (world->reuse_entities) {
        /* Add deleted entity to open entities */
        TNECS_CHECK(tnecs_grow_entities_open(world));
        tnecs_E *arr = world->entities.open.arr;
        arr[world->entities.open.num++] = entity;
    }
    assert(!TNECS_E_EXISTS(world, entity));
    assert(world->entities.orders[entity]       == TNECS_NULL);
    assert(world->entities.archetypes[entity]   == TNECS_NULL);
    assert(world->entities.orders[entity_order] != entity);
    return (1);
}

void tnecs_W_reuse(tnecs_W *world, int toggle) {
    world->reuse_entities = toggle;
}

/*****************************************************/
/***************** TNECS INTERNALS *******************/
/*****************************************************/
tnecs_E tnecs_E_add_C(  tnecs_W *world,
                        tnecs_E  entity,
                        tnecs_C  archetype_toadd,
                        int      isNew) {
    if (archetype_toadd <= 0) {
        return (TNECS_NULL);
    }

    if (!TNECS_E_EXISTS(world, entity)) {
        return (TNECS_NULL);
    }

    tnecs_C archetype_old = world->entities.archetypes[entity];

    if (TNECS_A_HAS_T(archetype_toadd, archetype_old)) {
        return (entity);
    }

    tnecs_C archetype_new = archetype_toadd + archetype_old;
    assert(archetype_new != archetype_old);
    if (isNew)
        TNECS_CHECK(_tnecs_register_archetype(world, 
                                                   setBits_KnR(archetype_new),
                                                   archetype_new));

    TNECS_CHECK(tnecs_C_migrate(world,      entity, archetype_old, archetype_new));
    TNECS_CHECK(tnecs_entitiesbyT_migrate(world, entity, archetype_old, archetype_new));
    TNECS_CHECK(tnecs_C_init(world,         entity, archetype_toadd));

#ifndef NDEBUG
    size_t tID_new = tnecs_A_id(world, archetype_new);
    size_t new_order = world->byT.num_entities[tID_new] - 1;
    assert(world->entities.archetypes[entity]           == archetype_new);
    assert(world->byT.entities[tID_new][new_order]   == entity);
    assert(world->entities.orders[entity]               == new_order);
#endif /* NDEBUG */
    return (world->entities.id[entity]);
}

tnecs_E tnecs_E_rm_C(   tnecs_W *world,
                        tnecs_E  entity,
                        tnecs_C  archetype) {
    /* Get new archetype. Since it is a archetype, just need to substract. */
    tnecs_C archetype_old = world->entities.archetypes[entity];
    tnecs_C archetype_new = archetype_old - archetype;

    /* Free removed components. */
    TNECS_CHECK(tnecs_C_free(world, entity, archetype));
    if (archetype_new != TNECS_NULL) {
        /* Migrate remaining components to new archetype array. */
        TNECS_CHECK(_tnecs_register_archetype(world,
                                                   setBits_KnR(archetype_new),
                                                   archetype_new));
        TNECS_CHECK(tnecs_C_migrate(world, entity, archetype_old, archetype_new));
    } else {
        /* No remaining component, delete everything. */
        TNECS_CHECK(tnecs_C_del(world, entity, archetype_old));
    }
    /* Migrate entity to new byT array. */
    TNECS_CHECK(tnecs_entitiesbyT_migrate(world, entity, archetype_old, archetype_new));
    assert(archetype_new == world->entities.archetypes[entity]);
    return (1);
}

void *tnecs_get_C(  tnecs_W *world, tnecs_E  eID,
                    tnecs_C  cID) {
    if (!TNECS_E_EXISTS(world, eID))
        return (NULL);

    tnecs_C component_flag      = TNECS_C_ID2T(cID);
    tnecs_C entity_archetype    = TNECS_E_A(world, eID);
    // If entity has component, get output it. If not output NULL.
    if (!TNECS_A_HAS_T(component_flag, entity_archetype))
        return (NULL);

    size_t tID = tnecs_A_id(world, entity_archetype);
    assert(tID > 0);
    size_t component_order = tnecs_C_order_byT(world, cID, entity_archetype);
    assert(component_order <= world->byT.num_Cs[tID]);
    size_t entity_order = world->entities.orders[eID];
    size_t bytesize     = world->components.bytesizes[cID];

    tnecs_carr *comp_array = &world->byT.components[tID][component_order];
    assert(comp_array != NULL);
    tnecs_byte *temp_C_bytesptr = (tnecs_byte *)(comp_array->components);
    void *out = temp_C_bytesptr + (bytesize * entity_order);

    return (out);
}

int tnecs_entitiesbyT_add(tnecs_W *world, tnecs_E  entity,
                             tnecs_C  archetype_new) {
    size_t tID_new = tnecs_A_id(world, archetype_new);
    if ((world->byT.num_entities[tID_new] + 1) >= world->byT.len_entities[tID_new]) {
        TNECS_CHECK(tnecs_grow_byT(world, tID_new));
    }
    size_t new_order                            = world->byT.num_entities[tID_new]++;
    world->entities.orders[entity]              = new_order;
    world->entities.archetypes[entity]          = archetype_new;
    world->byT.entities[tID_new][new_order]  = entity;
    return (1);
}

int tnecs_entitiesbyT_del(tnecs_W *world, tnecs_E  entity,
                             tnecs_C archetype_old) {

    if (!TNECS_E_EXISTS(world, entity))
        return (1);

    if (entity >= world->entities.len)
        return (1);

    size_t archetype_old_id = tnecs_A_id(world, archetype_old);
    size_t old_num          = world->byT.num_entities[archetype_old_id];
    if (old_num <= 0)
        return (1);

    size_t entity_order_old = world->entities.orders[entity];
    assert(archetype_old == world->entities.archetypes[entity]);

    assert(entity_order_old < world->byT.len_entities[archetype_old_id]);
    assert(world->byT.entities[archetype_old_id][entity_order_old] == entity);

    tnecs_E top_E = world->byT.entities[archetype_old_id][old_num - 1];

    /* components scrambles -> entitiesbyT too */
    TNECS_CHECK(tnecs_arrdel(world->byT.entities[archetype_old_id], entity_order_old, old_num,
                 sizeof(**world->byT.entities)));

    if (top_E != entity) {
        world->entities.orders[top_E] = entity_order_old;
        assert(world->byT.entities[archetype_old_id][entity_order_old] == top_E);
    }

    world->entities.orders[entity]      = TNECS_NULL;
    world->entities.archetypes[entity]  = TNECS_NULL;

    --world->byT.num_entities[archetype_old_id];
    return (1);
}

int tnecs_entitiesbyT_migrate(tnecs_W    *world, 
                                 tnecs_E    entity,
                                 tnecs_C archetype_old,
                                 tnecs_C archetype_new) {
    /* Migrate entities into correct byT array */
    TNECS_CHECK(tnecs_entitiesbyT_del(world, entity, archetype_old));
    assert(world->entities.archetypes[entity]   == TNECS_NULL);
    assert(world->entities.orders[entity]       == TNECS_NULL);
    TNECS_CHECK(tnecs_entitiesbyT_add(world, entity, archetype_new));

#ifndef NDEBUG
    size_t tID_new      = tnecs_A_id(world, archetype_new);
    size_t order_new    = world->entities.orders[entity];
    assert(world->entities.archetypes[entity]         == archetype_new);
    assert(world->byT.num_entities[tID_new] - 1    == order_new);
    assert(world->byT.entities[tID_new][order_new] == entity);
#endif /* NDEBUG */
    return (1);
}

int tnecs_C_add(tnecs_W *world, tnecs_C archetype) {
    /* Check if need to grow component array after adding new component */
    size_t tID          = tnecs_A_id(world, archetype);
    size_t new_comp_num = world->byT.num_Cs[tID];
#ifndef NDEBUG
    size_t entity_order = world->byT.num_entities[tID];
#endif /* NDEBUG */

    for (size_t corder = 0; corder < new_comp_num; corder++) {
        // Take component array of current archetype_id
        tnecs_carr *comp_arr = &world->byT.components[tID][corder];
        // check if it need to grow after adding new component
        assert(entity_order == comp_arr->num);

        if (++comp_arr->num >= comp_arr->len)
            TNECS_CHECK(tnecs_grow_C_array(world, comp_arr, tID, corder));
    }

    return (1);
}

int tnecs_C_copy(tnecs_W *world, tnecs_E entity,
                tnecs_C old_A, tnecs_C new_A) {
    /* Copy components from old order unto top of new type component array */
    if (old_A == new_A) {
        return (1);
    }

    size_t old_tID          = tnecs_A_id(world, old_A);
    size_t new_tID          = tnecs_A_id(world, new_A);
    size_t old_E_order = world->entities.orders[entity];
    size_t new_E_order = world->byT.num_entities[new_tID];
    size_t num_comp_new     = world->byT.num_Cs[new_tID];
    size_t num_comp_old     = world->byT.num_Cs[old_tID];

#ifndef NDEBUG
    // Sanity check: entity order is the same in new components array
    for (int i = 0; i < num_comp_new; ++i) {
        size_t num = world->byT.components[new_tID][i].num;
        assert((num - 1) == new_E_order);
    }
#endif /* NDEBUG */

    size_t old_C_id, new_C_id, component_bytesize;
    tnecs_carr *old_array,              *new_array;
    tnecs_byte *old_C_ptr,      *new_C_ptr;
    tnecs_byte *old_C_bytesptr, *new_C_bytesptr;

    for (size_t old_corder = 0; old_corder < num_comp_old; old_corder++) {
        old_C_id = world->byT.components_id[old_tID][old_corder];
        for (size_t new_corder = 0; new_corder < num_comp_new; new_corder++) {
            new_C_id = world->byT.components_id[new_tID][new_corder];
            if (old_C_id != new_C_id)
                continue;

            new_array = &world->byT.components[new_tID][new_corder];
            old_array = &world->byT.components[old_tID][old_corder];
            assert(old_array->type == new_array->type);
            assert(old_array != new_array);

            component_bytesize = world->components.bytesizes[old_C_id];
            assert(component_bytesize > 0);

            old_C_bytesptr = (tnecs_byte *)(old_array->components);
            assert(old_C_bytesptr != NULL);

            old_C_ptr = (old_C_bytesptr + (component_bytesize * old_E_order));
            assert(old_C_ptr != NULL);

            new_C_bytesptr = (tnecs_byte *)(new_array->components);
            assert(new_C_bytesptr != NULL);

            new_C_ptr = (new_C_bytesptr + (component_bytesize * new_E_order));
            assert(new_C_ptr != NULL);
            assert(new_C_ptr != old_C_ptr);

#ifndef NDEBUG
            const void *const out = memmove(new_C_ptr, old_C_ptr, component_bytesize);
            assert(out == new_C_ptr);
#else
            memmove(new_C_ptr, old_C_ptr, component_bytesize);
#endif /* NDEBUG */
            break;
        }
    }
    return (1);
}

int tnecs_C_run(tnecs_W     *world,
                        tnecs_E     entity,
                        tnecs_C  archetype,
                        tnecs_init  *funcs) {
    size_t tID      = tnecs_A_id(world, archetype);
    size_t comp_num = world->byT.num_Cs[tID];
    for (size_t corder = 0; corder < comp_num; corder++) {
        size_t cID = world->byT.components_id[tID][corder];
        tnecs_init func = funcs[cID]; 
        if (func == NULL) {
            continue;
        }
        void *comp = tnecs_get_C(world, entity, cID);
        assert(comp != NULL);
        func(comp);
    }
    return(1);
}

int tnecs_C_init(tnecs_W     *world,
                         tnecs_E     entity,
                         tnecs_C  archetype) {
    /* Init ALL entity's components in archetype */
    return(tnecs_C_run(world,
                               entity,
                               archetype,  
                               world->components.finit));
}

int tnecs_C_free(tnecs_W     *world,
                         tnecs_E     entity,
                         tnecs_C  archetype) {
    /* Free ALL entity's components in archetype */
    return(tnecs_C_run(world,
                               entity,
                               archetype,
                               world->components.ffree));
}

int tnecs_C_del(tnecs_W     *world,
                        tnecs_E     entity,
                        tnecs_C  old_A) {
    /* Delete ALL components from componentsbyT at old entity order */
    size_t old_tID      = tnecs_A_id(world, old_A);
    size_t order_old    = world->entities.orders[entity];
    size_t old_comp_num = world->byT.num_Cs[old_tID];
    for (size_t corder = 0; corder < old_comp_num; corder++) {
        size_t current_C_id = world->byT.components_id[old_tID][corder];
        tnecs_carr   *old_array  = &world->byT.components[old_tID][corder];
        tnecs_byte   *comp_ptr   = old_array->components;
        assert(comp_ptr != NULL);

        /* Scramble components too */
        size_t comp_by       = world->components.bytesizes[current_C_id];
        size_t new_comp_num  = world->byT.num_entities[old_tID];
        const tnecs_byte *const scramble = tnecs_arrdel(comp_ptr, order_old, new_comp_num, comp_by);
        TNECS_CHECK(scramble);

        old_array->num--;
    }
    return (1);
}

int tnecs_C_migrate(tnecs_W *world, tnecs_E entity,
                    tnecs_C  old_A, tnecs_C new_A) {
    if (old_A != world->entities.archetypes[entity]) {
        return (0);
    }
    TNECS_CHECK(tnecs_C_add(world,  new_A));
    if (old_A > TNECS_NULL) {
        TNECS_CHECK(tnecs_C_copy(world, entity, old_A, new_A));
        TNECS_CHECK(tnecs_C_del( world, entity, old_A));
    }
    return (1);
}

int tnecs_carr_new(tnecs_W *world, size_t   num_Cs,
                   tnecs_C archetype) {
    tnecs_carr *comp_arr = calloc(num_Cs, sizeof(tnecs_carr));
    TNECS_CHECK(comp_arr);

    tnecs_C archetype_reduced = archetype, archetype_added = 0;
    tnecs_C tID = tnecs_A_id(world, archetype);
    size_t id_toadd = 0, num_flags = 0;

    while (archetype_reduced) {
        archetype_reduced &= (archetype_reduced - 1);
        tnecs_C type_toadd = (archetype_reduced + archetype_added) ^ archetype;
        assert(type_toadd > 0);
        id_toadd = TNECS_C_T2ID(type_toadd);
        assert(id_toadd > 0);
        assert(id_toadd < world->components.num);
        TNECS_CHECK(tnecs_carr_init(world, &comp_arr[num_flags], id_toadd));
        num_flags++;
        archetype_added += type_toadd;
    }
    world->byT.components[tID] = comp_arr;
    assert(id_toadd < world->components.num);
    return ((archetype_added == archetype) && (num_flags == num_Cs));
}

int tnecs_carr_init(tnecs_W *world, tnecs_carr  *comp_arr,
                    size_t   cID) {
    assert(cID > 0);
    assert(cID < world->components.num);
    tnecs_C in_type = TNECS_C_ID2T(cID);
    assert(in_type <= TNECS_C_ID2T(world->components.num));

    size_t bytesize = world->components.bytesizes[cID];
    assert(bytesize > 0);

    comp_arr->type          = in_type;
    comp_arr->num           = 0;
    comp_arr->len           = TNECS_C_0LEN;
    comp_arr->components    = calloc(TNECS_C_0LEN, bytesize);
    TNECS_CHECK(comp_arr->components);
    return (1);
}

/*********** UTILITY FUNCTIONS/MACROS **************/
size_t tnecs_C_order_byT(   const tnecs_W *const world,
                            size_t cID, tnecs_C flag) {
    tnecs_C tID = tnecs_A_id(world, flag);
    return (tnecs_C_order_byTid(world, cID, tID));
}

size_t tnecs_C_order_byTid( const tnecs_W *const world,
                            size_t cID, size_t tID) {
    size_t order = TNECS_C_CAP;
    for (size_t i = 0; i < world->byT.num_Cs[tID]; i++) {
        if (world->byT.components_id[tID][i] == cID) {
            order = i;
            break;
        }
    }
    return (order);
}

tnecs_C tnecs_C_ids2A(size_t argnum, ...) {
    tnecs_C out = 0;
    va_list ap;
    va_start(ap, argnum);
    for (size_t i = 0; i < argnum; i++) {
        size_t id = va_arg(ap, size_t);
        out += TNECS_C_ID2T(id);
    }
    va_end(ap);
    return (out);
}

tnecs_C tnecs_A_id(const tnecs_W *const world,
                    tnecs_C archetype) {
    size_t id = 0;
    for (size_t i = 0; i < world->byT.num; i++) {
        if (archetype == world->byT.id[i]) {
            id = i;
            break;
        }
    }
    return (id);
}

/***************** "DYNAMIC" ARRAYS ******************/
void *tnecs_realloc(void   *ptr,     size_t old_len,
                    size_t  new_len, size_t elem_bytesize) {
    if (!ptr)
        return (NULL);
    void *realloced = calloc(new_len, elem_bytesize);
    if (realloced == NULL) {
        printf("tnecs: failed allocation realloced\n");
        return(NULL);
    }

    memcpy(realloced, ptr, (new_len > old_len ? old_len : new_len) * elem_bytesize);
    free(ptr);
    return (realloced);
}

void *tnecs_arrdel(void *arr,  size_t elem, 
                   size_t len, size_t bytesize) {
    /* Scrambles by copying top element at [len - 1] to [elem] */
    tnecs_byte *bytes = arr;
    if (elem != (len - 1))
        memmove(bytes + (elem * bytesize), bytes + ((len - 1) * bytesize), bytesize);

    memset(bytes + ((len - 1) * bytesize), TNECS_NULL, bytesize);
    return (arr);
}

#ifndef NDEBUG
int tnecs_grow_ran(tnecs_W *world) {
    /* Realloc systems ran if too many */
    size_t old_len              = world->systems.ran.len;
    size_t new_len              = old_len * TNECS_ARR_GROW;
    world->systems.ran.len      = new_len;
    world->systems.to_run.len   = new_len;
    size_t bytesize             = sizeof(tnecs_S);

    world->systems.ran.arr  = tnecs_realloc(world->systems.ran.arr, old_len, new_len, bytesize);
    world->systems.to_run.arr  = tnecs_realloc(world->systems.to_run.arr, old_len, new_len, bytesize);
    TNECS_CHECK(world->systems.ran.arr);
    TNECS_CHECK(world->systems.to_run.arr);
    return (1);
}
#endif /* NDEBUG */

int tnecs_grow_entities_open(tnecs_W *world) {
    /* Realloc entities_open if too many */
    if ((world->entities.open.num + 1) >= world->entities.open.len) {
        size_t old_len              = world->entities.open.len;
        size_t new_len              = old_len * TNECS_ARR_GROW;
        size_t bytesize             = sizeof(tnecs_E);
        world->entities.open.len    = new_len;

        world->entities.open.arr = tnecs_realloc(world->entities.open.arr, old_len, new_len, bytesize);
        TNECS_CHECK(world->entities.open.arr);
    }
    return (1);
}

int tnecs_grow_C_array( tnecs_W     *world,
                        tnecs_carr  *comp_arr,
                        size_t tID, size_t corder) {
    size_t old_len      = comp_arr->len;
    size_t new_len      = old_len * TNECS_ARR_GROW;
    comp_arr->len       = new_len;

    size_t cID = world->byT.components_id[tID][corder];

    size_t bytesize         = world->components.bytesizes[cID];
    comp_arr->components    = tnecs_realloc(comp_arr->components, old_len, new_len, bytesize);
    TNECS_CHECK(comp_arr->components);
    return (1);
}

int tnecs_grow_E(tnecs_W *world) {
    size_t olen = world->entities.len;
    size_t nlen = world->entities.len * TNECS_ARR_GROW;
    world->entities.len = nlen;
    if (nlen >= TNECS_E_CAP) {
        printf("tnecs: entities cap reached\n");
        return (TNECS_NULL);
    }

    world->entities.id          = tnecs_realloc(world->entities.id,
                                                olen, nlen,
                                                sizeof(*world->entities.id));
    TNECS_CHECK(world->entities.id);
    world->entities.orders      = tnecs_realloc(world->entities.orders,
                                                olen, nlen,
                                                sizeof(*world->entities.orders));
    TNECS_CHECK(world->entities.orders);
    world->entities.archetypes  = tnecs_realloc(world->entities.archetypes,
                                                olen, nlen,
                                                sizeof(*world->entities.archetypes));
    TNECS_CHECK(world->entities.archetypes);

    return (1);
}

int tnecs_grow_system(tnecs_W *world) {
    size_t olen = world->systems.len;
    size_t nlen = olen * TNECS_ARR_GROW;
    assert(olen > 0);
    world->systems.len          = nlen;

    world->systems.phases       = tnecs_realloc(world->systems.phases,
                                                olen, nlen,
                                                sizeof(*world->systems.phases));
    TNECS_CHECK(world->systems.phases);
    world->systems.orders       = tnecs_realloc(world->systems.orders,
                                                olen, nlen,
                                                sizeof(*world->systems.orders));
    TNECS_CHECK(world->systems.orders);
    world->systems.exclusive    = tnecs_realloc(world->systems.exclusive,
                                                olen, nlen,
                                                sizeof(*world->systems.exclusive));
    TNECS_CHECK(world->systems.pipeline);
    world->systems.pipeline    = tnecs_realloc(world->systems.pipeline,
                                               olen, nlen,
                                               sizeof(*world->systems.pipeline));
    TNECS_CHECK(world->systems.exclusive);
    world->systems.archetypes   = tnecs_realloc(world->systems.archetypes,
                                                olen, nlen,
                                                sizeof(*world->systems.archetypes));
    TNECS_CHECK(world->systems.archetypes);

    return (1);
}

int tnecs_grow_archetype(tnecs_W *world) {
    size_t olen = world->byT.len;
    size_t nlen = olen * TNECS_ARR_GROW;
    world->byT.len = nlen;

    world->byT.id               = tnecs_realloc(world->byT.id,
                                                olen, nlen,
                                                sizeof(*world->byT.id));
    world->byT.entities         = tnecs_realloc(world->byT.entities,
                                                olen, nlen,
                                                sizeof(*world->byT.entities));
    world->byT.num_entities     = tnecs_realloc(world->byT.num_entities,
                                                olen, nlen,
                                                sizeof(*world->byT.num_entities));
    world->byT.len_entities     = tnecs_realloc(world->byT.len_entities,
                                                olen, nlen,
                                                sizeof(*world->byT.len_entities));
    world->byT.archetype_id     = tnecs_realloc(world->byT.archetype_id,
                                                olen, nlen,
                                                sizeof(*world->byT.archetype_id));
    world->byT.components_id    = tnecs_realloc(world->byT.components_id,
                                                olen, nlen,
                                                sizeof(*world->byT.components_id));
    world->byT.num_Cs           = tnecs_realloc(world->byT.num_Cs,
                                                olen, nlen,
                                                sizeof(*world->byT.num_Cs));
    world->byT.components_order = tnecs_realloc(world->byT.components_order,
                                                olen, nlen,
                                                sizeof(*world->byT.components_order));
    world->byT.num_archetype_ids    = tnecs_realloc(world->byT.num_archetype_ids,
                                                olen, nlen,
                                                sizeof(*world->byT.num_archetype_ids));

    TNECS_CHECK(world->byT.id);
    TNECS_CHECK(world->byT.entities);
    TNECS_CHECK(world->byT.num_entities);
    TNECS_CHECK(world->byT.archetype_id);
    TNECS_CHECK(world->byT.len_entities);
    TNECS_CHECK(world->byT.components_id);
    TNECS_CHECK(world->byT.num_Cs);
    TNECS_CHECK(world->byT.components_order);
    TNECS_CHECK(world->byT.num_archetype_ids);

    world->byT.components        = tnecs_realloc(world->byT.components,           olen, nlen,
                                                    sizeof(*world->byT.components));
    TNECS_CHECK(world->byT.components);

    for (size_t i = olen; i < world->byT.len; i++) {
        world->byT.entities[i]       = calloc(  TNECS_E_0LEN,
                                                sizeof(**world->byT.entities));
        TNECS_CHECK(world->byT.entities[i]);
        world->byT.archetype_id[i]   = calloc(  TNECS_C_CAP,
                                                sizeof(**world->byT.archetype_id));
        TNECS_CHECK(world->byT.archetype_id[i]);

        world->byT.len_entities[i] = TNECS_E_0LEN;
        world->byT.num_entities[i] = 0;
    }
    return (1);
}

int tnecs_grow_pipeline(tnecs_W *world) {
    size_t olen = world->pipelines.len;
    size_t nlen = olen * TNECS_ARR_GROW;
    world->pipelines.len = nlen;
    if (nlen >= TNECS_Pi_CAP) {
        printf("tnecs: pipelines cap reached\n");
        return (TNECS_NULL);
    }

    world->pipelines.byphase = tnecs_realloc(world->pipelines.byphase,
                                             olen, nlen,
                                             sizeof(*world->pipelines.byphase));
    TNECS_CHECK(world->pipelines.byphase);

    return (1);
}

int tnecs_grow_phase(tnecs_W    *world,
                     tnecs_Pi  pipeline) {
    tnecs_phases *byphase = TNECS_Pi_GET(world, pipeline);
    size_t olen = byphase->len;
    size_t nlen = olen * TNECS_ARR_GROW;
    byphase->len = nlen;
    if (nlen >= TNECS_Ph_CAP) {
        printf("tnecs: phases cap reached\n");
        return (TNECS_NULL);
    }

    byphase->systems      = tnecs_realloc(byphase->systems,
                                          olen, nlen,
                                          sizeof(*byphase->systems));
    TNECS_CHECK(byphase->systems);
    byphase->systems_id   = tnecs_realloc(byphase->systems_id,
                                          olen, nlen,
                                          sizeof(*byphase->systems_id));
    TNECS_CHECK(byphase->systems_id);
    byphase->len_systems  = tnecs_realloc(byphase->len_systems,
                                          olen, nlen,
                                          sizeof(*byphase->len_systems));
    TNECS_CHECK(byphase->len_systems);
    byphase->num_systems  = tnecs_realloc(byphase->num_systems,
                                          olen, nlen,
                                          sizeof(*byphase->num_systems));
    TNECS_CHECK(byphase->num_systems);

    for (size_t i = olen; i < byphase->len; i++) {
        size_t bytesize1 = sizeof(**byphase->systems);
        size_t bytesize2 = sizeof(**byphase->systems_id);

        byphase->systems[i]       = calloc(TNECS_Ph_0LEN, bytesize1);
        TNECS_CHECK(byphase->systems[i]);
        byphase->systems_id[i]    = calloc(TNECS_Ph_0LEN, bytesize2);
        TNECS_CHECK(byphase->systems_id[i]);

        byphase->len_systems[i] = TNECS_Ph_0LEN;
        byphase->num_systems[i] = 0;
    }
    return (1);
}

int tnecs_grow_system_byphase(tnecs_phases  *byphase,
                              tnecs_Ph       phase) {
    size_t olen                 = byphase->len_systems[phase];
    size_t nlen                 = olen * TNECS_ARR_GROW;
    byphase->len_systems[phase] = nlen;
    size_t bs                   = sizeof(**byphase->systems);
    size_t bsid                 = sizeof(**byphase->systems_id);

    tnecs_S *systems   = byphase->systems[phase];
    size_t *system_id           = byphase->systems_id[phase];
    byphase->systems[phase]     = tnecs_realloc(systems, olen, nlen, bs);
    TNECS_CHECK(byphase->systems[phase]);
    byphase->systems_id[phase]  = tnecs_realloc(system_id, olen, nlen, bsid);
    TNECS_CHECK(byphase->systems_id[phase]);
    return (1);
}

int tnecs_grow_byT(tnecs_W *world, size_t tID) {
    size_t olen = world->byT.len_entities[tID];
    size_t nlen = olen * TNECS_ARR_GROW;

    assert(olen > 0);
    world->byT.len_entities[tID] = nlen;

    size_t bytesize             = sizeof(*world->byT.entities[tID]);
    tnecs_E *ptr           = world->byT.entities[tID];
    world->byT.entities[tID] = tnecs_realloc(ptr, olen, nlen, bytesize);
    TNECS_CHECK(world->byT.entities[tID]);

    return (1);
}

/*************** SET BIT COUNTING *******************/
size_t setBits_KnR(tnecs_C in_flags) {
    /* Credits to Kernighan&Ritchie in 'C Programming Language' */
    size_t count = 0;
    while (in_flags) {
        in_flags &= (in_flags - 1);
        count++;
    }
    return (count);
}

void *tnecs_C_array(tnecs_W *world, 
                    const size_t cID,
                    const size_t tID) {
    if ((cID == TNECS_NULL) || (tID == TNECS_NULL))
        return (NULL);

    if (cID >= world->components.num)
        return (NULL);

    tnecs_carr *carr    = world->byT.components[tID];
    size_t      corder  = world->byT.components_order[tID][cID];

    return (carr[corder].components);
}

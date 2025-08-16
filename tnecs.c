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
typedef struct tnecs_carr { /* 1D array of Cs */
    tnecs_C  type;
    size_t   num;
    size_t   len;
    void    *Cs; /* [E_order_byT] */
} tnecs_carr;

typedef struct tnecs_arr {
    void    *arr;
    size_t   num;
    size_t   len;
} tnecs_arr;

typedef struct tnecs_Phs {
    size_t num;
    size_t len;

    size_t       *len_Ss; /* [ph_id] */
    size_t       *num_Ss; /* [ph_id] */
    size_t      **Ss_id;  /* [ph_id][S_order] */
    tnecs_S_f   **Ss;     /* [ph_id][system_id]    */
} tnecs_Phs;

typedef struct tnecs_Pis {
    size_t num;
    size_t len;

    tnecs_Phs *byPh;   /* [pipeline_id] */
} tnecs_Pis;

typedef struct tnecs_Es {
    // - .num doesn't change even if Es get deleted
    // - if reuse_Es: add deleted Es to Es_open
    //      - Call tnecs_E_reuse to add Es with
    //        id[ent] == false Es_open.
    size_t num;
    size_t len;

    tnecs_E     *id;            /* [entity_id] -> eID */
    size_t      *Os;        /* [eID] */
    tnecs_C     *As;    /* [eID] */
    tnecs_arr  open;
} tnecs_Es;

typedef struct tnecs_Ss {
    size_t num;
    size_t len;

    tnecs_Ph    *Phs;           /* [S_id] -> sID */
    size_t      *Os;            /* [sID] */
    int         *exclusive;     /* [sID] */
    tnecs_C     *As;            /* [sID] */
    tnecs_Pi    *pipeline;      /* [sID] */
#ifndef NDEBUG
    /* Systems that might be run in current pipeline */
    tnecs_arr to_run;
    /* Systems ran in pipeline, if num_Es > 0 */
    tnecs_arr ran;
#endif /* NDEBUG */
} tnecs_Ss;

typedef struct tnecs_As {
    size_t num;
    size_t len;

    tnecs_C      *id;           /* [A_id] -> aID */
    size_t       *num_Cs;       /* [aID] */
    size_t       *len_Es;       /* [aID] */
    size_t       *num_Es;       /* [aID] */
    size_t       *num_A_ids;    /* [aID] */

    size_t      **A_id;     /* [aID][A_id_order]    */
    tnecs_E     **Es;       /* [aID][E_order_byT]   */
    size_t      **Cs_O;     /* [aID][cID]           */
    tnecs_C     **Cs_id;    /* [aID][C_O_byT]   */
    tnecs_carr  **Cs;       /* [aID][C_O_byT]   */
} tnecs_As;

typedef struct tnecs_Cs {
    size_t          num;
    size_t          bytesizes[TNECS_C_CAP]; /* [cID] */
    tnecs_init_f    finit[TNECS_C_CAP];     /* [cID] */
    tnecs_free_f    ffree[TNECS_C_CAP];     /* [cID] */
} tnecs_Cs;

struct tnecs_W {
    tnecs_Ss    Ss;
    tnecs_Es    Es;
    tnecs_As    byT;
    tnecs_Pis   Pis;
    tnecs_Cs    Cs;

    int reuse_Es;
};

struct tnecs_In {
    tnecs_W *world;
    tnecs_ns dt;
    tnecs_C  S_A;
    size_t   num_Es;
    size_t   E_A_id;
    void    *data;
};


/* --- WORLD FUNCTIONS --- */
static int _tnecs_breath_C(   tnecs_Cs  *Cs);
static int _tnecs_breath_As(  tnecs_As  *byT);
static int _tnecs_breath_Es(  tnecs_Es  *Es);
static int _tnecs_breath_Ss(  tnecs_Ss  *Ss);
static int _tnecs_breath_Phs( tnecs_Phs *byPh);
static int _tnecs_breath_Pis( tnecs_Pis *Pis);

static int _tnecs_destroy_Ss(   tnecs_Ss    *Ss);
static int _tnecs_destroy_Es(   tnecs_Es    *Es);
static int _tnecs_destroy_As(   tnecs_As    *byT);
static int _tnecs_destroy_Phs(  tnecs_Phs   *byPh);
static int _tnecs_destroy_Pis(  tnecs_Pis   *Pis);

/* --- REGISTRATION  --- */
static size_t _tnecs_register_A( tnecs_W *w, size_t num_c,
                                 tnecs_C a);

/* --- SET BIT COUNTING --- */
static size_t setBits_KnR(tnecs_C flags);

/* --- "DYNAMIC" ARRAYS --- */
static void *tnecs_arrdel(  void *arr,  size_t elem,
                            size_t len, size_t bytesize);
static void *tnecs_realloc( void *ptr,  size_t olen,
                            size_t len, size_t bytesize);

#ifndef NDEBUG
static int tnecs_grow_ran(      tnecs_W *w);
#endif /* NDEBUG */

static int tnecs_grow_Ph(       tnecs_W *w, tnecs_Pi    pi);
static int tnecs_grow_byT(      tnecs_W *w, size_t      aID);
static int tnecs_grow_E(        tnecs_W *w);
static int tnecs_grow_S(        tnecs_W *w);
static int tnecs_grow_Pi(       tnecs_W *w);
static int tnecs_grow_A(        tnecs_W *w);
static int tnecs_grow_Es_open(  tnecs_W *w);

static int tnecs_grow_S_byPh(   tnecs_Phs   *byPh,
                                tnecs_Ph     phase);
static int tnecs_grow_C_array(  tnecs_W     *w, 
                                tnecs_carr  *comp_arr, 
                                size_t tID, size_t corder);

/* --- UTILITIES --- */
static size_t tnecs_C_O_byT(    const tnecs_W *const w,
                                size_t cID, tnecs_C arch);
static size_t tnecs_C_O_byTid(  const tnecs_W *const w, 
                                size_t cID, size_t aID);

/* --- COMPONENT ARRAY --- */
static int tnecs_carr_new(  tnecs_W *w, size_t num,
                            tnecs_C a);
static int tnecs_carr_init( tnecs_W *w, tnecs_carr *arr,
                            size_t cID);

/* --- byT --- */
static int tnecs_EsbyT_add( tnecs_W *w, tnecs_E e,
                            tnecs_C nt);
static int tnecs_EsbyT_del( tnecs_W *w, tnecs_E e,
                            tnecs_C ot);
static int tnecs_EsbyT_migrate( tnecs_W *w, tnecs_E e,
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
                        tnecs_C  of,    tnecs_free_f *f);
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
    TNECS_CHECK(_tnecs_breath_Ss(   &((*world)->Ss)));
    TNECS_CHECK(_tnecs_breath_Es(   &((*world)->Es)));
    TNECS_CHECK(_tnecs_breath_Pis(  &((*world)->Pis)));
    TNECS_CHECK(_tnecs_breath_As(   &((*world)->byT)));
    TNECS_CHECK(_tnecs_breath_C(    &((*world)->Cs)));
    return (1);
}

int tnecs_W_destroy(tnecs_W **world) {
    TNECS_CHECK(_tnecs_destroy_Pis( &((*world)->Pis)));
    TNECS_CHECK(_tnecs_destroy_Ss(  &((*world)->Ss)));
    TNECS_CHECK(_tnecs_destroy_Es(  &((*world)->Es)));
    TNECS_CHECK(_tnecs_destroy_As(  &((*world)->byT)));
    free(*world);

    *world = NULL;
    return (1);
}

int _tnecs_breath_C(tnecs_Cs *Cs) {
    Cs->num                   = TNECS_NULLSHIFT;
    Cs->bytesizes[TNECS_NULL] = TNECS_NULL;
    return (1);
}

int _tnecs_breath_Es(tnecs_Es *Es) {
    /* Variables */
    Es->num         = TNECS_NULLSHIFT;
    Es->len         = TNECS_E_0LEN;
    Es->open.len    = TNECS_E_0LEN;
    Es->open.num    = 0;

    /* Allocs */
    Es->id          = calloc(Es->len, sizeof(*Es->id));
    Es->Os          = calloc(Es->len, sizeof(*Es->Os));
    Es->open.arr    = calloc(Es->len, sizeof(tnecs_E));
    Es->As          = calloc(Es->len, sizeof(*Es->As));
    TNECS_CHECK(Es->id);
    TNECS_CHECK(Es->Os);
    TNECS_CHECK(Es->open.arr);
    TNECS_CHECK(Es->As);
    return (1);
}

int _tnecs_breath_Pis(tnecs_Pis *Pis) {
    Pis->len  = TNECS_Pi_0LEN;
    Pis->num  = TNECS_NULLSHIFT;

    Pis->byPh = calloc(Pis->len, sizeof(*Pis->byPh));
    _tnecs_breath_Phs(&Pis->byPh[TNECS_NULL]);
    return (1);
}

int _tnecs_breath_Phs(tnecs_Phs *byPh) {
    byPh->len    = TNECS_Ph_0LEN;
    byPh->num    = TNECS_NULLSHIFT;

    byPh->Ss        = calloc(byPh->len, sizeof(*byPh->Ss));
    byPh->Ss_id     = calloc(byPh->len, sizeof(*byPh->Ss_id));
    byPh->num_Ss    = calloc(byPh->len, sizeof(*byPh->num_Ss));
    byPh->len_Ss    = calloc(byPh->len, sizeof(*byPh->len_Ss));

    TNECS_CHECK(byPh->Ss);
    TNECS_CHECK(byPh->Ss_id);
    TNECS_CHECK(byPh->num_Ss);
    TNECS_CHECK(byPh->len_Ss);

    /* Alloc & check for Es_byPh elements */
    for (size_t i = 0; i < byPh->len; i++) {
        byPh->Ss[i]     = calloc(byPh->len, sizeof(**byPh->Ss));
        TNECS_CHECK(byPh->Ss[i]);
        byPh->Ss_id[i]  = calloc(byPh->len, sizeof(**byPh->Ss_id));
        TNECS_CHECK(byPh->Ss_id[i]);

        byPh->num_Ss[i] = 0;
        byPh->len_Ss[i] = byPh->len;
    }
    return (1);
}

int _tnecs_breath_Ss(tnecs_Ss *Ss) {
    /* Variables */
    Ss->len        = TNECS_S_0LEN;
    Ss->num        = TNECS_NULLSHIFT;
#ifndef NDEBUG
    Ss->to_run.len = TNECS_S_0LEN;
    Ss->ran.len    = TNECS_S_0LEN;
#endif /* NDEBUG */

    /* Allocs */
    Ss->Phs         = calloc(Ss->len, sizeof(*Ss->Phs));
    Ss->Os          = calloc(Ss->len, sizeof(*Ss->Os));
    Ss->pipeline    = calloc(Ss->len, sizeof(*Ss->pipeline));
    Ss->exclusive   = calloc(Ss->len, sizeof(*Ss->exclusive));
    Ss->As          = calloc(Ss->len, sizeof(*Ss->As));
#ifndef NDEBUG
    Ss->ran.arr    = calloc(Ss->ran.len,    sizeof(tnecs_S_f));
    Ss->to_run.arr = calloc(Ss->to_run.len, sizeof(tnecs_S_f));
#endif /* NDEBUG */

    TNECS_CHECK(Ss->Phs);
    TNECS_CHECK(Ss->Os);
    TNECS_CHECK(Ss->pipeline);
    TNECS_CHECK(Ss->exclusive);
    TNECS_CHECK(Ss->As);
#ifndef NDEBUG
    TNECS_CHECK(Ss->ran.arr);
    TNECS_CHECK(Ss->to_run.arr);
#endif /* NDEBUG */

    return (1);
}

int _tnecs_breath_As(tnecs_As *byT) {
    /* Variables */
    byT->num = TNECS_NULLSHIFT;
    byT->len = TNECS_A_0LEN;

    /* Allocs */
    byT->id         = calloc(byT->len, sizeof(*byT->id));
    byT->Es         = calloc(byT->len, sizeof(*byT->Es));
    byT->Cs         = calloc(byT->len, sizeof(*byT->Cs));
    byT->len_Es     = calloc(byT->len, sizeof(*byT->len_Es));
    byT->num_Es     = calloc(byT->len, sizeof(*byT->num_Es));
    byT->A_id       = calloc(byT->len, sizeof(*byT->A_id));
    byT->Cs_id      = calloc(byT->len, sizeof(*byT->Cs_id));
    byT->num_Cs     = calloc(byT->len, sizeof(*byT->num_Cs));
    byT->Cs_O       = calloc(byT->len, sizeof(*byT->Cs_O));
    byT->num_A_ids  = calloc(byT->len, sizeof(*byT->num_A_ids));

    TNECS_CHECK(byT->id);
    TNECS_CHECK(byT->Es);
    TNECS_CHECK(byT->Cs);
    TNECS_CHECK(byT->A_id);
    TNECS_CHECK(byT->len_Es);
    TNECS_CHECK(byT->num_Es);
    TNECS_CHECK(byT->Cs_id);
    TNECS_CHECK(byT->num_Cs);
    TNECS_CHECK(byT->Cs_O);
    TNECS_CHECK(byT->num_A_ids);

    /* Alloc & check for id_byT elements */
    for (size_t i = 0; i < byT->len; i++) {
        byT->A_id[i] = calloc(TNECS_C_CAP, sizeof(**byT->A_id));
        TNECS_CHECK(byT->A_id[i]);
        byT->Es[i]   = calloc(TNECS_E_0LEN, sizeof(**byT->Es));
        TNECS_CHECK(byT->Es[i]);

        byT->num_Es[i] = 0;
        byT->len_Es[i] = TNECS_E_0LEN;
    }
    return (1);
}

static int _tnecs_destroy_Phs(tnecs_Phs *byPh) {
    for (size_t i = 0; i < byPh->len; i++) {
        if (byPh->Ss != NULL)
            free(byPh->Ss[i]);
        if (byPh->Ss_id != NULL)
            free(byPh->Ss_id[i]);
    }
    free(byPh->Ss);
    free(byPh->Ss_id);
    free(byPh->len_Ss);
    free(byPh->num_Ss);

    return(1);
}

static int _tnecs_destroy_Ss(tnecs_Ss *Ss) {
    free(Ss->Os);
    free(Ss->Phs);
    free(Ss->pipeline);
    free(Ss->exclusive);
    free(Ss->As);
    #ifndef NDEBUG
    free(Ss->to_run.arr);
    free(Ss->ran.arr);
    #endif /* NDEBUG */

    return(1);
}

static int _tnecs_destroy_Es(tnecs_Es *Es) {
    free(Es->id);
    free(Es->Os);
    free(Es->open.arr);
    free(Es->As);
    
    return(1);
}

static int _tnecs_destroy_Pis(tnecs_Pis *Pis) {
    for (size_t i = 0; i < Pis->len; i++) {
        _tnecs_destroy_Phs(&Pis->byPh[i]);
    }
    free(Pis->byPh);
    
    return(1);
}

static int _tnecs_destroy_As(tnecs_As *byT) {
    for (size_t i = 0; i < byT->len; i++) {
        if (byT->Es != NULL)
            free(byT->Es[i]);
        if (byT->Cs_id != NULL)
            free(byT->Cs_id[i]);
        if (byT->Cs_O != NULL)
            free(byT->Cs_O[i]);
        if (byT->A_id != NULL)
            free(byT->A_id[i]);
        if (byT->Cs != NULL) {
            for (size_t j = 0; j < byT->num_Cs[i]; j++) {
                free(byT->Cs[i][j].Cs);
            }
            free(byT->Cs[i]);
        }
    }

    free(byT->id);
    free(byT->Es);
    free(byT->Cs);
    free(byT->len_Es);
    free(byT->num_Es);
    free(byT->A_id);
    free(byT->Cs_id);
    free(byT->num_Cs);
    free(byT->Cs_O);
    free(byT->num_A_ids);

    return(1);
}

/********************* STEPPING ********************/
int tnecs_W_step(   tnecs_W *W, tnecs_ns dt,
                    void    *data) {
    for (size_t p = 0; p < W->Pis.num; p++) {
        TNECS_CHECK(tnecs_Pi_step(W, dt, data, p));
    }
    return (1);
}

int tnecs_Pi_step(  tnecs_W     *w,     tnecs_ns     dt,
                    void        *data,  tnecs_Pi     pi) {
    #ifndef NDEBUG
    w->Ss.to_run.num   = 0;
    w->Ss.ran.num      = 0;
    #endif /* NDEBUG */

    tnecs_Phs *byPh = TNECS_Pi_GET(w, pi);
    for (size_t ph = 0; ph < byPh->num; ph++) {
        TNECS_CHECK(tnecs_Pi_step_Ph(w, dt, data, pi, ph));
    }
    return(1);
}

int tnecs_Pi_step_Ph(   tnecs_W  *w,    tnecs_ns  dt,
                        void     *data, tnecs_Pi  pi,
                        tnecs_Ph  ph) {
    tnecs_Phs *byPh = TNECS_Pi_GET(w, pi);
    size_t num = byPh->num_Ss[ph];
    for (size_t S_O = 0; S_O < num; S_O++) {
        size_t S_id = byPh->Ss_id[ph][S_O];
        TNECS_CHECK(tnecs_S_run(w, S_id, dt, data));
    }

    return(1);
}

/************* SYSTEM FUNCTIONS ***************/
int tnecs_custom_S_run( tnecs_W     *W, tnecs_S_f    S,
                        tnecs_C      A, tnecs_ns     dt, 
                        void        *data) {
    /* Building the Ss input */
    tnecs_In input = {.world = W, .dt = dt, .data = data};
    size_t A_id = tnecs_A_id(W, A);
    if (A_id == TNECS_NULL) {
        printf("tnecs: Input archetype is unknown.\n");
        return (0);
    }

    /* Running the exclusive custom system */
    input.E_A_id   = A_id;
    input.num_Es        = W->byT.num_Es[input.E_A_id];
    S(&input);

    /* Running the non-exclusive/inclusive custom system */
    for (size_t tsub = 0; tsub < W->byT.num_A_ids[A_id]; tsub++) {
        input.E_A_id   = W->byT.A_id[A_id][tsub];
        input.num_Es        = W->byT.num_Es[input.E_A_id];
        S(&input);
    }
    return (1);
}

int tnecs_S_run(tnecs_W *W, size_t system_id,
                tnecs_ns dt, void *data) {
    /* Building the Ss input */
    tnecs_In input = {.world = W, .dt = dt, .data = data};
    size_t S_O     = W->Ss.Os[system_id];
    tnecs_Ph phase    = W->Ss.Phs[system_id];
    tnecs_Pi pipeline = W->Ss.pipeline[system_id];
    size_t system_A_id  = tnecs_A_id(W, W->Ss.As[system_id]);

    input.E_A_id   = system_A_id;
    input.num_Es          = W->byT.num_Es[input.E_A_id];
    tnecs_Phs *byPh       = TNECS_Pi_GET(W, pipeline);

    while (W->Ss.to_run.num >= (W->Ss.to_run.len - 1)) {
        TNECS_CHECK(tnecs_grow_ran(W));
    }

    tnecs_S_f system = byPh->Ss[phase][S_O];
#ifndef NDEBUG
    tnecs_S_f *system_ptr;
    size_t system_num;
    system_num              = W->Ss.to_run.num++;
    system_ptr              = W->Ss.to_run.arr;
    system_ptr[system_num]  = system;
#endif /* NDEBUG */

    /* - Running the exclusive Ss in current phase - */
    if (input.num_Es > 0) {
        /* Skip running system if no Es! */
    #ifndef NDEBUG
        system_num              = W->Ss.ran.num++;
        system_ptr              = W->Ss.ran.arr;
        system_ptr[system_num]  = system;
    #endif /* NDEBUG */
        system(&input);
    }

    if (W->Ss.exclusive[system_id])
        return (1);

    /* - Running the inclusive Ss in current phase - */
    for (size_t tsub = 0; tsub < W->byT.num_A_ids[system_A_id]; tsub++) {

        input.E_A_id   = W->byT.A_id[system_A_id][tsub];
        input.num_Es          = W->byT.num_Es[input.E_A_id];
        tnecs_S_f system = byPh->Ss[phase][S_O];

    #ifndef NDEBUG
        system_num              = W->Ss.to_run.num++;
        system_ptr              = W->Ss.to_run.arr;
        system_ptr[system_num]  = system;
    #endif /* NDEBUG */

        while (W->Ss.to_run.num >= (W->Ss.to_run.len - 1)) {
            TNECS_CHECK(tnecs_grow_ran(W));
        }

        if (input.num_Es <= 0) {
            continue;
        }

        #ifndef NDEBUG
            system_num              = W->Ss.ran.num++;
            system_ptr              = W->Ss.ran.arr;
            system_ptr[system_num]  = system;
        #endif /* NDEBUG */

        system(&input);
    }
    return (1);
}

/*************** REGISTRATION ***************/
size_t tnecs_register_S(tnecs_W *W,     tnecs_S_f     system,
                        tnecs_Pi pipeline,  tnecs_Ph    phase,
                        int isExclusive,    size_t      num_Cs,
                        tnecs_C archetype) {
    /* Check if phase exist */
    if (!TNECS_Pi_VALID(W, pipeline)) {
        printf("tnecs: System pipeline '%lld' is invalid.\n", pipeline);
        return (TNECS_NULL);
    }
    if (!TNECS_Ph_VALID(W, pipeline, phase)) {
        printf("tnecs: System phase '%lld' is invalid (pipeline '%lld').\n", phase, pipeline);
        return (TNECS_NULL);
    }

    /* Compute new id */
    size_t system_id = W->Ss.num++;

    /* Realloc Ss if too many */
    if (W->Ss.num >= W->Ss.len) {
        TNECS_CHECK(tnecs_grow_S(W));
    }

    /* Realloc Ss_byPh if too many */
    tnecs_Phs *byPh = TNECS_Pi_GET(W, pipeline);
    if (byPh->num_Ss[phase] >= byPh->len_Ss[phase]) {
        TNECS_CHECK(tnecs_grow_S_byPh(byPh, phase));
    }

    /* -- Actual registration -- */
    /* Note: phase is exclusive to pipeline */
    W->Ss.Phs[system_id]        = phase;
    W->Ss.pipeline[system_id]      = pipeline;
    W->Ss.exclusive[system_id]     = isExclusive;
    W->Ss.As[system_id]    = archetype;

    /* System order */
    size_t S_order                         = byPh->num_Ss[phase]++;
    W->Ss.Os[system_id]            = S_order;
    byPh->Ss[phase][S_order]       = system;
    byPh->Ss_id[phase][S_order]    = system_id;
    TNECS_CHECK(_tnecs_register_A(W, num_Cs, archetype));
    return (system_id);
}

tnecs_C tnecs_register_C(   tnecs_W     *W,
                            size_t       bytesize,
                            tnecs_free_f finit,
                            tnecs_free_f ffree) {
    /* Checks */
    if (bytesize <= 0) {
        printf("tnecs: Component should have >0 bytesize.\n");
        return (TNECS_NULL);
    }
    if (W->Cs.num >= TNECS_C_CAP) {
        printf("tnecs: Component capacity reached.\n");
        return (TNECS_NULL);
    }

    /* Registering */
    tnecs_C new_C_id    = W->Cs.num++;
    tnecs_C new_C_flag  = TNECS_C_ID2T(new_C_id);
    W->Cs.bytesizes[new_C_id]   = bytesize;
    W->Cs.ffree[new_C_id]       = ffree;
    W->Cs.finit[new_C_id]       = finit;
    TNECS_CHECK(_tnecs_register_A(W, 1, new_C_flag));
    return (new_C_id);
}

size_t _tnecs_register_A(   tnecs_W    *world,
                            size_t      num_Cs,
                            tnecs_C     archetype_new) {
    // 0- Check if archetype exists, return
    for (size_t i = 0 ; i < world->byT.num; i++) {
        if (archetype_new == world->byT.id[i]) {
            return (i);
        }
    }

    // 1- Add new byT.Cs at [tID]
    if ((world->byT.num + 1) >= world->byT.len)
        TNECS_CHECK(tnecs_grow_A(world));
    world->byT.id[world->byT.num++] = archetype_new;
    size_t tID = tnecs_A_id(world, archetype_new);
    assert(tID == (world->byT.num - 1));
    world->byT.num_Cs[tID] = num_Cs;

    // 2- Add arrays to byT.Cs[tID] for each component
    TNECS_CHECK(tnecs_carr_new(world, num_Cs, archetype_new));

    // 3- Add all Cs to byT.Cs_id
    tnecs_C archetype_reduced = archetype_new, archetype_added = 0;
    size_t bytesize1 = sizeof(**world->byT.Cs_id);
    size_t bytesize2 = sizeof(**world->byT.Cs_O);
    world->byT.Cs_id[tID]     = calloc(num_Cs,      bytesize1);
    TNECS_CHECK(world->byT.Cs_id[tID]);
    world->byT.Cs_O[tID]  = calloc(TNECS_C_CAP, bytesize2);
    TNECS_CHECK(world->byT.Cs_O[tID]);

    size_t k = 0;
    while (archetype_reduced) {
        archetype_reduced &= (archetype_reduced - 1);

        tnecs_C component_type_toadd = (archetype_reduced + archetype_added) ^ archetype_new;
        archetype_added      += component_type_toadd;
        assert(component_type_toadd > 0);
        tnecs_C component_id_toadd   = TNECS_C_T2ID(component_type_toadd);

        world->byT.Cs_id[tID][k]      = component_id_toadd;
        world->byT.Cs_O[tID][component_id_toadd] = k++;
    }

    // 4- Check As.
    for (size_t i = 1 ; i < world->byT.num; i++) {
        world->byT.num_A_ids[i] = 0;
        for (size_t j = 1 ; j < (world->byT.num); j++) {
            if (i == j)
                continue;

            if (!TNECS_A_IS_subT(world->byT.id[i], world->byT.id[j]))
                continue;

            // j is an archetype of i
            world->byT.A_id[i][world->byT.num_A_ids[i]++] = j;
        }
    }

    return (tID);
}

size_t tnecs_register_Pi(tnecs_W *world) {
    tnecs_Pi pipeline = world->Pis.num++;
    while (pipeline >= world->Pis.len) {
        TNECS_CHECK(tnecs_grow_Pi(world));
    }
    tnecs_Phs *byPh = TNECS_Pi_GET(world, pipeline);
    _tnecs_breath_Phs(byPh);

    return (pipeline);
}

size_t tnecs_register_Ph(tnecs_W   *world, tnecs_Pi pipeline) {
    if (!TNECS_Pi_VALID(world, pipeline)) {
        printf("tnecs: Pipeline '%lld' is invalid for new phase.\n", pipeline);
        return (TNECS_NULL);
    }

    tnecs_Phs *byPh   = TNECS_Pi_GET(world, pipeline);
    tnecs_Ph phase       = byPh->num++;
    while (phase >= byPh->len) {
        TNECS_CHECK(tnecs_grow_Ph(world, pipeline));
    }
    return (phase);
}

/**************** ENTITY MANIPULATION **************/
tnecs_E tnecs_E_create(tnecs_W *world) {
    tnecs_E out = TNECS_NULL;

    /* Check if an open entity exists */
    tnecs_E *arr = world->Es.open.arr;
    while ((out == TNECS_NULL) &&
           (world->Es.open.num > 0) && 
           (world->Es.open.num < TNECS_E_CAP)
          ) {
        out = arr[--world->Es.open.num];
        arr[world->Es.open.num] = TNECS_NULL;
    }

    /* If no open entity existed, create one */
    if (out == TNECS_NULL) {
        do {
            if (world->Es.num >= world->Es.len) {
                if (!tnecs_grow_E(world)) {
                    printf("tnecs: Could not allocate more memory for Es.\n");
                    return (TNECS_NULL);
                }
            }
            out = world->Es.num++;
        } while (TNECS_E_EXISTS(world, out));
    }
    assert(out != TNECS_NULL);

    /* Set entity and checks  */
    world->Es.id[out] = out;
    TNECS_CHECK(tnecs_EsbyT_add(world, out, TNECS_NULL));
    assert(world->Es.id[out]                                          == out);
    assert(world->byT.Es[TNECS_NULL][world->Es.Os[out]]  == out);
    return (out);
}

tnecs_E tnecs_E_create_wC(tnecs_W *world, size_t argnum, ...) {
    /* Get archetype of all vararg Cs ids */
    va_list ap;
    va_start(ap, argnum);
    tnecs_C archetype = 0;
    for (size_t i = 0; i < argnum; i++) {
        tnecs_C component_id = va_arg(ap, tnecs_C);
        archetype += TNECS_C_ID2T(component_id);
    }
    va_end(ap);

    /* Create entity with all Cs */
    tnecs_E new_E = tnecs_E_create(world);
    if (new_E == TNECS_NULL) {
        printf("tnecs: could not create new entity\n");
        return (TNECS_NULL);
    }
    TNECS_CHECK(tnecs_E_add_C(world, new_E, archetype, 1));

#ifndef NDEBUG
    size_t tID      = tnecs_A_id(world, archetype);
    size_t order    = world->Es.Os[new_E];
    assert(world->byT.Es[tID][order]   == new_E);
    assert(world->Es.id[new_E]       == new_E);
#endif /* NDEBUG */

    return (new_E);
}

int tnecs_E_reuse(tnecs_W *world) {
    // Adds all null Es to open list
    for (tnecs_E i = TNECS_NULLSHIFT; i < world->Es.num; i++) {
        if (TNECS_E_EXISTS(world, i))
            continue; /* Skip if entity exists */

        if (tnecs_E_isOpen(world, i))
            continue; /* Skip if already in open list */

        TNECS_CHECK(tnecs_grow_Es_open(world));
        tnecs_E *arr = world->Es.open.arr;
        arr[world->Es.open.num++] = i;
    }
    return (1);
};

int tnecs_E_flush(tnecs_W *world) {
    /* Get rid of all Es in Es_open */
    world->Es.open.num = 0;
    return (1);
}

tnecs_E tnecs_E_isOpen(tnecs_W *world, tnecs_E entity) {
    if (entity <= TNECS_NULL) {
        return (0);
    }

    const tnecs_E * const open_arr = world->Es.open.arr;

    for (tnecs_E i = TNECS_NULLSHIFT; i < world->Es.open.num; i++) {
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
        world->Es.id[entity]         = TNECS_NULL;
        world->Es.Os[entity]     = TNECS_NULL;
        world->Es.As[entity] = TNECS_NULL;
        return (1);
    }

    /* Preliminaries */
    tnecs_C archetype   = world->Es.As[entity];

    /* Delete Cs */
    TNECS_CHECK(tnecs_C_free(world, entity, archetype));
    TNECS_CHECK(tnecs_C_del(world, entity, archetype));

#ifndef NDEBUG
    size_t entity_order         = world->Es.Os[entity];
    size_t tID                  = tnecs_A_id(world, archetype);
    assert(world->byT.num_Es[tID] > TNECS_NULL);
    assert(world->byT.len_Es[tID] >= entity_order);
    assert(world->byT.num_Es[tID] > TNECS_NULL);
#endif /* NDEBUG */

    /* Delete EsbyT */
    TNECS_CHECK(tnecs_EsbyT_del(world, entity, archetype));

    /* Delete entity */
    world->Es.id[entity]         = TNECS_NULL;

    // Note: reuse_Es used to add to Es_open, so that
    // user can call tnecs_E_reuse to reuse Es manually.
    if (world->reuse_Es) {
        /* Add deleted entity to open Es */
        TNECS_CHECK(tnecs_grow_Es_open(world));
        tnecs_E *arr = world->Es.open.arr;
        arr[world->Es.open.num++] = entity;
    }
    assert(!TNECS_E_EXISTS(world, entity));
    assert(world->Es.Os[entity]       == TNECS_NULL);
    assert(world->Es.As[entity]   == TNECS_NULL);
    assert(world->Es.Os[entity_order] != entity);
    return (1);
}

void tnecs_W_reuse(tnecs_W *world, int toggle) {
    world->reuse_Es = toggle;
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

    tnecs_C archetype_old = world->Es.As[entity];

    if (TNECS_A_HAS_T(archetype_toadd, archetype_old)) {
        return (entity);
    }

    tnecs_C archetype_new = archetype_toadd + archetype_old;
    assert(archetype_new != archetype_old);
    if (isNew)
        TNECS_CHECK(_tnecs_register_A(world, 
                                                   setBits_KnR(archetype_new),
                                                   archetype_new));

    TNECS_CHECK(tnecs_C_migrate(world,      entity, archetype_old, archetype_new));
    TNECS_CHECK(tnecs_EsbyT_migrate(world, entity, archetype_old, archetype_new));
    TNECS_CHECK(tnecs_C_init(world,         entity, archetype_toadd));

#ifndef NDEBUG
    size_t tID_new = tnecs_A_id(world, archetype_new);
    size_t new_order = world->byT.num_Es[tID_new] - 1;
    assert(world->Es.As[entity]           == archetype_new);
    assert(world->byT.Es[tID_new][new_order]   == entity);
    assert(world->Es.Os[entity]               == new_order);
#endif /* NDEBUG */
    return (world->Es.id[entity]);
}

tnecs_E tnecs_E_rm_C(   tnecs_W *world,
                        tnecs_E  entity,
                        tnecs_C  archetype) {
    /* Get new archetype. Since it is a archetype, just need to substract. */
    tnecs_C archetype_old = world->Es.As[entity];
    tnecs_C archetype_new = archetype_old - archetype;

    /* Free removed Cs. */
    TNECS_CHECK(tnecs_C_free(world, entity, archetype));
    if (archetype_new != TNECS_NULL) {
        /* Migrate remaining Cs to new archetype array. */
        TNECS_CHECK(_tnecs_register_A(world,
                                                   setBits_KnR(archetype_new),
                                                   archetype_new));
        TNECS_CHECK(tnecs_C_migrate(world, entity, archetype_old, archetype_new));
    } else {
        /* No remaining component, delete everything. */
        TNECS_CHECK(tnecs_C_del(world, entity, archetype_old));
    }
    /* Migrate entity to new byT array. */
    TNECS_CHECK(tnecs_EsbyT_migrate(world, entity, archetype_old, archetype_new));
    assert(archetype_new == world->Es.As[entity]);
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
    size_t component_order = tnecs_C_O_byT(world, cID, entity_archetype);
    assert(component_order <= world->byT.num_Cs[tID]);
    size_t entity_order = world->Es.Os[eID];
    size_t bytesize     = world->Cs.bytesizes[cID];

    tnecs_carr *comp_array = &world->byT.Cs[tID][component_order];
    assert(comp_array != NULL);
    tnecs_byte *temp_C_bytesptr = (tnecs_byte *)(comp_array->Cs);
    void *out = temp_C_bytesptr + (bytesize * entity_order);

    return (out);
}

int tnecs_EsbyT_add(tnecs_W *world, tnecs_E  entity,
                             tnecs_C  archetype_new) {
    size_t tID_new = tnecs_A_id(world, archetype_new);
    if ((world->byT.num_Es[tID_new] + 1) >= world->byT.len_Es[tID_new]) {
        TNECS_CHECK(tnecs_grow_byT(world, tID_new));
    }
    size_t new_order                            = world->byT.num_Es[tID_new]++;
    world->Es.Os[entity]              = new_order;
    world->Es.As[entity]          = archetype_new;
    world->byT.Es[tID_new][new_order]  = entity;
    return (1);
}

int tnecs_EsbyT_del(tnecs_W *world, tnecs_E  entity,
                             tnecs_C archetype_old) {

    if (!TNECS_E_EXISTS(world, entity))
        return (1);

    if (entity >= world->Es.len)
        return (1);

    size_t archetype_old_id = tnecs_A_id(world, archetype_old);
    size_t old_num          = world->byT.num_Es[archetype_old_id];
    if (old_num <= 0)
        return (1);

    size_t entity_order_old = world->Es.Os[entity];
    assert(archetype_old == world->Es.As[entity]);

    assert(entity_order_old < world->byT.len_Es[archetype_old_id]);
    assert(world->byT.Es[archetype_old_id][entity_order_old] == entity);

    tnecs_E top_E = world->byT.Es[archetype_old_id][old_num - 1];

    /* Cs scrambles -> EsbyT too */
    TNECS_CHECK(tnecs_arrdel(world->byT.Es[archetype_old_id], entity_order_old, old_num,
                 sizeof(**world->byT.Es)));

    if (top_E != entity) {
        world->Es.Os[top_E] = entity_order_old;
        assert(world->byT.Es[archetype_old_id][entity_order_old] == top_E);
    }

    world->Es.Os[entity]      = TNECS_NULL;
    world->Es.As[entity]  = TNECS_NULL;

    --world->byT.num_Es[archetype_old_id];
    return (1);
}

int tnecs_EsbyT_migrate(tnecs_W    *world, 
                                 tnecs_E    entity,
                                 tnecs_C archetype_old,
                                 tnecs_C archetype_new) {
    /* Migrate Es into correct byT array */
    TNECS_CHECK(tnecs_EsbyT_del(world, entity, archetype_old));
    assert(world->Es.As[entity]   == TNECS_NULL);
    assert(world->Es.Os[entity]       == TNECS_NULL);
    TNECS_CHECK(tnecs_EsbyT_add(world, entity, archetype_new));

#ifndef NDEBUG
    size_t tID_new      = tnecs_A_id(world, archetype_new);
    size_t order_new    = world->Es.Os[entity];
    assert(world->Es.As[entity]         == archetype_new);
    assert(world->byT.num_Es[tID_new] - 1    == order_new);
    assert(world->byT.Es[tID_new][order_new] == entity);
#endif /* NDEBUG */
    return (1);
}

int tnecs_C_add(tnecs_W *world, tnecs_C archetype) {
    /* Check if need to grow component array after adding new component */
    size_t tID          = tnecs_A_id(world, archetype);
    size_t new_comp_num = world->byT.num_Cs[tID];
#ifndef NDEBUG
    size_t entity_order = world->byT.num_Es[tID];
#endif /* NDEBUG */

    for (size_t corder = 0; corder < new_comp_num; corder++) {
        // Take component array of current A_id
        tnecs_carr *comp_arr = &world->byT.Cs[tID][corder];
        // check if it need to grow after adding new component
        assert(entity_order == comp_arr->num);

        if (++comp_arr->num >= comp_arr->len)
            TNECS_CHECK(tnecs_grow_C_array(world, comp_arr, tID, corder));
    }

    return (1);
}

int tnecs_C_copy(tnecs_W *world, tnecs_E entity,
                tnecs_C old_A, tnecs_C new_A) {
    /* Copy Cs from old order unto top of new type component array */
    if (old_A == new_A) {
        return (1);
    }

    size_t old_tID          = tnecs_A_id(world, old_A);
    size_t new_tID          = tnecs_A_id(world, new_A);
    size_t old_E_order = world->Es.Os[entity];
    size_t new_E_order = world->byT.num_Es[new_tID];
    size_t num_comp_new     = world->byT.num_Cs[new_tID];
    size_t num_comp_old     = world->byT.num_Cs[old_tID];

#ifndef NDEBUG
    // Sanity check: entity order is the same in new Cs array
    for (int i = 0; i < num_comp_new; ++i) {
        size_t num = world->byT.Cs[new_tID][i].num;
        assert((num - 1) == new_E_order);
    }
#endif /* NDEBUG */

    size_t old_C_id, new_C_id, component_bytesize;
    tnecs_carr *old_array,              *new_array;
    tnecs_byte *old_C_ptr,      *new_C_ptr;
    tnecs_byte *old_C_bytesptr, *new_C_bytesptr;

    for (size_t old_corder = 0; old_corder < num_comp_old; old_corder++) {
        old_C_id = world->byT.Cs_id[old_tID][old_corder];
        for (size_t new_corder = 0; new_corder < num_comp_new; new_corder++) {
            new_C_id = world->byT.Cs_id[new_tID][new_corder];
            if (old_C_id != new_C_id)
                continue;

            new_array = &world->byT.Cs[new_tID][new_corder];
            old_array = &world->byT.Cs[old_tID][old_corder];
            assert(old_array->type == new_array->type);
            assert(old_array != new_array);

            component_bytesize = world->Cs.bytesizes[old_C_id];
            assert(component_bytesize > 0);

            old_C_bytesptr = (tnecs_byte *)(old_array->Cs);
            assert(old_C_bytesptr != NULL);

            old_C_ptr = (old_C_bytesptr + (component_bytesize * old_E_order));
            assert(old_C_ptr != NULL);

            new_C_bytesptr = (tnecs_byte *)(new_array->Cs);
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

int tnecs_C_run(tnecs_W *world,     tnecs_E entity,
                tnecs_C archetype,  tnecs_init_f *funcs) {
    size_t tID      = tnecs_A_id(world, archetype);
    size_t comp_num = world->byT.num_Cs[tID];
    for (size_t corder = 0; corder < comp_num; corder++) {
        size_t cID = world->byT.Cs_id[tID][corder];
        tnecs_init_f func = funcs[cID]; 
        if (func == NULL) {
            continue;
        }
        void *comp = tnecs_get_C(world, entity, cID);
        assert(comp != NULL);
        func(comp);
    }
    return(1);
}

int tnecs_C_init(   tnecs_W *W, tnecs_E E, 
                    tnecs_C  A) {
    /* Init ALL entity's Cs in archetype */
    return(tnecs_C_run(W, E, A, W->Cs.finit));
}

int tnecs_C_free(   tnecs_W *W, tnecs_E E,
                    tnecs_C  A) {
    /* Free ALL entity's Cs in archetype */
    return(tnecs_C_run(W, E, A, W->Cs.ffree));
}

int tnecs_C_del(tnecs_W *world, tnecs_E entity,
                tnecs_C  old_A) {
    /* Delete ALL Cs from CsbyT at old entity order */
    size_t old_tID      = tnecs_A_id(world, old_A);
    size_t order_old    = world->Es.Os[entity];
    size_t old_comp_num = world->byT.num_Cs[old_tID];
    for (size_t corder = 0; corder < old_comp_num; corder++) {
        size_t current_C_id = world->byT.Cs_id[old_tID][corder];
        tnecs_carr   *old_array  = &world->byT.Cs[old_tID][corder];
        tnecs_byte   *comp_ptr   = old_array->Cs;
        assert(comp_ptr != NULL);

        /* Scramble Cs too */
        size_t comp_by       = world->Cs.bytesizes[current_C_id];
        size_t new_comp_num  = world->byT.num_Es[old_tID];
        const tnecs_byte *const scramble = tnecs_arrdel(comp_ptr, order_old, new_comp_num, comp_by);
        TNECS_CHECK(scramble);

        old_array->num--;
    }
    return (1);
}

int tnecs_C_migrate(tnecs_W *world, tnecs_E entity,
                    tnecs_C  old_A, tnecs_C new_A) {
    if (old_A != world->Es.As[entity]) {
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
        assert(id_toadd < world->Cs.num);
        TNECS_CHECK(tnecs_carr_init(world, &comp_arr[num_flags], id_toadd));
        num_flags++;
        archetype_added += type_toadd;
    }
    world->byT.Cs[tID] = comp_arr;
    assert(id_toadd < world->Cs.num);
    return ((archetype_added == archetype) && (num_flags == num_Cs));
}

int tnecs_carr_init(tnecs_W *world, tnecs_carr  *comp_arr,
                    size_t   cID) {
    assert(cID > 0);
    assert(cID < world->Cs.num);
    tnecs_C in_type = TNECS_C_ID2T(cID);
    assert(in_type <= TNECS_C_ID2T(world->Cs.num));

    size_t bytesize = world->Cs.bytesizes[cID];
    assert(bytesize > 0);

    comp_arr->type          = in_type;
    comp_arr->num           = 0;
    comp_arr->len           = TNECS_C_0LEN;
    comp_arr->Cs    = calloc(TNECS_C_0LEN, bytesize);
    TNECS_CHECK(comp_arr->Cs);
    return (1);
}

/*********** UTILITY FUNCTIONS/MACROS **************/
size_t tnecs_C_O_byT(   const tnecs_W *const world,
                            size_t cID, tnecs_C flag) {
    tnecs_C tID = tnecs_A_id(world, flag);
    return (tnecs_C_O_byTid(world, cID, tID));
}

size_t tnecs_C_O_byTid( const tnecs_W *const world,
                            size_t cID, size_t tID) {
    size_t order = TNECS_C_CAP;
    for (size_t i = 0; i < world->byT.num_Cs[tID]; i++) {
        if (world->byT.Cs_id[tID][i] == cID) {
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
    /* Realloc Ss ran if too many */
    size_t old_len              = world->Ss.ran.len;
    size_t new_len              = old_len * TNECS_ARR_GROW;
    world->Ss.ran.len      = new_len;
    world->Ss.to_run.len   = new_len;
    size_t bytesize             = sizeof(tnecs_S_f);

    world->Ss.ran.arr  = tnecs_realloc(world->Ss.ran.arr, old_len, new_len, bytesize);
    world->Ss.to_run.arr  = tnecs_realloc(world->Ss.to_run.arr, old_len, new_len, bytesize);
    TNECS_CHECK(world->Ss.ran.arr);
    TNECS_CHECK(world->Ss.to_run.arr);
    return (1);
}
#endif /* NDEBUG */

int tnecs_grow_Es_open(tnecs_W *world) {
    /* Realloc Es_open if too many */
    if ((world->Es.open.num + 1) >= world->Es.open.len) {
        size_t old_len              = world->Es.open.len;
        size_t new_len              = old_len * TNECS_ARR_GROW;
        size_t bytesize             = sizeof(tnecs_E);
        world->Es.open.len    = new_len;

        world->Es.open.arr = tnecs_realloc(world->Es.open.arr, old_len, new_len, bytesize);
        TNECS_CHECK(world->Es.open.arr);
    }
    return (1);
}

int tnecs_grow_C_array( tnecs_W     *world,
                        tnecs_carr  *comp_arr,
                        size_t tID, size_t corder) {
    size_t old_len      = comp_arr->len;
    size_t new_len      = old_len * TNECS_ARR_GROW;
    comp_arr->len       = new_len;

    size_t cID = world->byT.Cs_id[tID][corder];

    size_t bytesize         = world->Cs.bytesizes[cID];
    comp_arr->Cs    = tnecs_realloc(comp_arr->Cs, old_len, new_len, bytesize);
    TNECS_CHECK(comp_arr->Cs);
    return (1);
}

int tnecs_grow_E(tnecs_W *world) {
    size_t olen = world->Es.len;
    size_t nlen = world->Es.len * TNECS_ARR_GROW;
    world->Es.len = nlen;
    if (nlen >= TNECS_E_CAP) {
        printf("tnecs: Es cap reached\n");
        return (TNECS_NULL);
    }

    world->Es.id          = tnecs_realloc(world->Es.id,
                                                olen, nlen,
                                                sizeof(*world->Es.id));
    TNECS_CHECK(world->Es.id);
    world->Es.Os      = tnecs_realloc(world->Es.Os,
                                                olen, nlen,
                                                sizeof(*world->Es.Os));
    TNECS_CHECK(world->Es.Os);
    world->Es.As  = tnecs_realloc(world->Es.As,
                                                olen, nlen,
                                                sizeof(*world->Es.As));
    TNECS_CHECK(world->Es.As);

    return (1);
}

int tnecs_grow_S(tnecs_W *world) {
    size_t olen = world->Ss.len;
    size_t nlen = olen * TNECS_ARR_GROW;
    assert(olen > 0);
    world->Ss.len          = nlen;

    world->Ss.Phs       = tnecs_realloc(world->Ss.Phs,
                                                olen, nlen,
                                                sizeof(*world->Ss.Phs));
    TNECS_CHECK(world->Ss.Phs);
    world->Ss.Os       = tnecs_realloc(world->Ss.Os,
                                                olen, nlen,
                                                sizeof(*world->Ss.Os));
    TNECS_CHECK(world->Ss.Os);
    world->Ss.exclusive    = tnecs_realloc(world->Ss.exclusive,
                                                olen, nlen,
                                                sizeof(*world->Ss.exclusive));
    TNECS_CHECK(world->Ss.pipeline);
    world->Ss.pipeline    = tnecs_realloc(world->Ss.pipeline,
                                               olen, nlen,
                                               sizeof(*world->Ss.pipeline));
    TNECS_CHECK(world->Ss.exclusive);
    world->Ss.As   = tnecs_realloc(world->Ss.As,
                                                olen, nlen,
                                                sizeof(*world->Ss.As));
    TNECS_CHECK(world->Ss.As);

    return (1);
}

int tnecs_grow_A(tnecs_W *world) {
    size_t olen = world->byT.len;
    size_t nlen = olen * TNECS_ARR_GROW;
    world->byT.len = nlen;

    world->byT.id               = tnecs_realloc(world->byT.id,
                                                olen, nlen,
                                                sizeof(*world->byT.id));
    world->byT.Es         = tnecs_realloc(world->byT.Es,
                                                olen, nlen,
                                                sizeof(*world->byT.Es));
    world->byT.num_Es     = tnecs_realloc(world->byT.num_Es,
                                                olen, nlen,
                                                sizeof(*world->byT.num_Es));
    world->byT.len_Es     = tnecs_realloc(world->byT.len_Es,
                                                olen, nlen,
                                                sizeof(*world->byT.len_Es));
    world->byT.A_id     = tnecs_realloc(world->byT.A_id,
                                                olen, nlen,
                                                sizeof(*world->byT.A_id));
    world->byT.Cs_id    = tnecs_realloc(world->byT.Cs_id,
                                                olen, nlen,
                                                sizeof(*world->byT.Cs_id));
    world->byT.num_Cs           = tnecs_realloc(world->byT.num_Cs,
                                                olen, nlen,
                                                sizeof(*world->byT.num_Cs));
    world->byT.Cs_O = tnecs_realloc(world->byT.Cs_O,
                                                olen, nlen,
                                                sizeof(*world->byT.Cs_O));
    world->byT.num_A_ids    = tnecs_realloc(world->byT.num_A_ids,
                                                olen, nlen,
                                                sizeof(*world->byT.num_A_ids));

    TNECS_CHECK(world->byT.id);
    TNECS_CHECK(world->byT.Es);
    TNECS_CHECK(world->byT.num_Es);
    TNECS_CHECK(world->byT.A_id);
    TNECS_CHECK(world->byT.len_Es);
    TNECS_CHECK(world->byT.Cs_id);
    TNECS_CHECK(world->byT.num_Cs);
    TNECS_CHECK(world->byT.Cs_O);
    TNECS_CHECK(world->byT.num_A_ids);

    world->byT.Cs        = tnecs_realloc(world->byT.Cs,           olen, nlen,
                                                    sizeof(*world->byT.Cs));
    TNECS_CHECK(world->byT.Cs);

    for (size_t i = olen; i < world->byT.len; i++) {
        world->byT.Es[i]       = calloc(  TNECS_E_0LEN,
                                                sizeof(**world->byT.Es));
        TNECS_CHECK(world->byT.Es[i]);
        world->byT.A_id[i]   = calloc(  TNECS_C_CAP,
                                                sizeof(**world->byT.A_id));
        TNECS_CHECK(world->byT.A_id[i]);

        world->byT.len_Es[i] = TNECS_E_0LEN;
        world->byT.num_Es[i] = 0;
    }
    return (1);
}

int tnecs_grow_Pi(tnecs_W *world) {
    size_t olen = world->Pis.len;
    size_t nlen = olen * TNECS_ARR_GROW;
    world->Pis.len = nlen;
    if (nlen >= TNECS_Pi_CAP) {
        printf("tnecs: Pis cap reached\n");
        return (TNECS_NULL);
    }

    world->Pis.byPh = tnecs_realloc(world->Pis.byPh,
                                             olen, nlen,
                                             sizeof(*world->Pis.byPh));
    TNECS_CHECK(world->Pis.byPh);

    return (1);
}

int tnecs_grow_Ph(tnecs_W    *world,
                     tnecs_Pi  pipeline) {
    tnecs_Phs *byPh = TNECS_Pi_GET(world, pipeline);
    size_t olen = byPh->len;
    size_t nlen = olen * TNECS_ARR_GROW;
    byPh->len = nlen;
    if (nlen >= TNECS_Ph_CAP) {
        printf("tnecs: Phs cap reached\n");
        return (TNECS_NULL);
    }

    byPh->Ss      = tnecs_realloc(byPh->Ss,
                                          olen, nlen,
                                          sizeof(*byPh->Ss));
    TNECS_CHECK(byPh->Ss);
    byPh->Ss_id   = tnecs_realloc(byPh->Ss_id,
                                          olen, nlen,
                                          sizeof(*byPh->Ss_id));
    TNECS_CHECK(byPh->Ss_id);
    byPh->len_Ss  = tnecs_realloc(byPh->len_Ss,
                                          olen, nlen,
                                          sizeof(*byPh->len_Ss));
    TNECS_CHECK(byPh->len_Ss);
    byPh->num_Ss  = tnecs_realloc(byPh->num_Ss,
                                          olen, nlen,
                                          sizeof(*byPh->num_Ss));
    TNECS_CHECK(byPh->num_Ss);

    for (size_t i = olen; i < byPh->len; i++) {
        size_t bytesize1 = sizeof(**byPh->Ss);
        size_t bytesize2 = sizeof(**byPh->Ss_id);

        byPh->Ss[i]       = calloc(TNECS_Ph_0LEN, bytesize1);
        TNECS_CHECK(byPh->Ss[i]);
        byPh->Ss_id[i]    = calloc(TNECS_Ph_0LEN, bytesize2);
        TNECS_CHECK(byPh->Ss_id[i]);

        byPh->len_Ss[i] = TNECS_Ph_0LEN;
        byPh->num_Ss[i] = 0;
    }
    return (1);
}

int tnecs_grow_S_byPh(tnecs_Phs  *byPh,
                              tnecs_Ph       phase) {
    size_t olen                 = byPh->len_Ss[phase];
    size_t nlen                 = olen * TNECS_ARR_GROW;
    byPh->len_Ss[phase] = nlen;
    size_t bs                   = sizeof(**byPh->Ss);
    size_t bsid                 = sizeof(**byPh->Ss_id);

    tnecs_S_f *Ss   = byPh->Ss[phase];
    size_t *system_id           = byPh->Ss_id[phase];
    byPh->Ss[phase]     = tnecs_realloc(Ss, olen, nlen, bs);
    TNECS_CHECK(byPh->Ss[phase]);
    byPh->Ss_id[phase]  = tnecs_realloc(system_id, olen, nlen, bsid);
    TNECS_CHECK(byPh->Ss_id[phase]);
    return (1);
}

int tnecs_grow_byT(tnecs_W *world, size_t tID) {
    size_t olen = world->byT.len_Es[tID];
    size_t nlen = olen * TNECS_ARR_GROW;

    assert(olen > 0);
    world->byT.len_Es[tID] = nlen;

    size_t bytesize             = sizeof(*world->byT.Es[tID]);
    tnecs_E *ptr           = world->byT.Es[tID];
    world->byT.Es[tID] = tnecs_realloc(ptr, olen, nlen, bytesize);
    TNECS_CHECK(world->byT.Es[tID]);

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

    if (cID >= world->Cs.num)
        return (NULL);

    tnecs_carr *carr    = world->byT.Cs[tID];
    size_t      corder  = world->byT.Cs_O[tID][cID];

    return (carr[corder].Cs);
}

/* Clock is POSIX, not C99 */
#ifndef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE 199309L
#endif /* _POSIX_C_SOURCE */

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <string.h>

#include "tnecs.h"


/********************** 0.1 MICROSECOND RESOLUTION CLOCK **********************/
//  Modified from: https://gist.github.com/ForeverZer0/0a4f80fc02b96e19380ebb7a3debbee5
#if defined(__linux)
#  define MICROSECOND_CLOCK
#  define HAVE_POSIX_TIMER
#  include <time.h>
#  ifdef CLOCK_MONOTONIC
#     define CLOCKID CLOCK_MONOTONIC
#  else
#     define CLOCKID CLOCK_REALTIME
#  endif
#elif defined(__APPLE__)
#  define MICROSECOND_CLOCK
#  define HAVE_MACH_TIMER
#  include <mach/mach_time.h>
#elif defined(_WIN32)
#  define MICROSECOND_CLOCK
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

/********************** 0.1 MICROSECOND RESOLUTION CLOCK **********************/
uint64_t tnecs_get_ns() {
    static uint64_t is_init = 0;
    #if defined(__APPLE__)
    static mach_timebase_info_data_t info;
    if (0 == is_init) {
        mach_timebase_info(&info);
        is_init = 1;
    }
    uint64_t now;
    now = mach_absolute_time();
    now *= info.numer;
    now /= info.denom;
    return now;
    #elif defined(__linux)
    static struct timespec linux_rate;
    if (0 == is_init) {
        clock_getres(CLOCKID, &linux_rate);
        is_init = 1;
    }
    uint64_t now;
    struct timespec spec;
    clock_gettime(CLOCKID, &spec);
    now = spec.tv_sec * 1.0e9 + spec.tv_nsec;
    return now;
    #elif defined(_WIN32)
    static LARGE_INTEGER win_frequency;
    if (0 == is_init) {
        QueryPerformanceFrequency(&win_frequency);
        is_init = 1;
    }
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (uint64_t)((1e9 * now.QuadPart) / win_frequency.QuadPart);
    #endif
}
#ifdef MICROSECOND_CLOCK
double tnecs_get_us() {
    return (tnecs_get_ns() / 1e3);
}
#else
#  define FAILSAFE_CLOCK
#  define tnecs_get_us() (((double)clock())/CLOCKS_PER_SEC*1e6) // [us]
#  define tnecs_get_ns() (((double)clock())/CLOCKS_PER_SEC*1e9) // [ns]
#endif


// TO DO:
//   -> tests for component_del
//   -> tests for entity destroy

/* MINCTEST - Minimal C Test Library - 0.2.0
*  ---------> MODIFIED FOR TNECS <----------
* Copyright (c) 2014-2017 Lewis Van Winkle
*
* http://CodePlea.com
*
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software
*    in a product, an acknowledgement in the product documentation would be
*    appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
*    misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#ifndef __MINCTEST_H__
#define __MINCTEST_H__
/*****************************TEST GLOBALS*****************************/
FILE *globalf;
/* NB all tests should be in one file. */
static int ltests = 0;
static int lfails = 0;

/* Display the test results. */
#define lresults() do {\
        if (lfails == 0) {\
            dupprintf(globalf,"ALL TESTS PASSED (%d/%d)\n", ltests, ltests);\
        } else {\
            dupprintf(globalf,"SOME TESTS FAILED (%d/%d)\n", ltests-lfails, ltests);\
        }\
    } while (0)

/* Run a test. Name can be any string to print out, test is the function name to call. */
#define lrun(name, test) do {\
        const int ts = ltests;\
        const int fs = lfails;\
        const clock_t start = clock();\
        dupprintf(globalf,"\t%-14s", name);\
        test();\
        dupprintf(globalf,"pass:%2d   fail:%2d   %4dms\n",\
                  (ltests-ts)-(lfails-fs), lfails-fs,\
                  (int)((clock() - start) * 1000 / CLOCKS_PER_SEC));\
    } while (0)

/* Assert a true statement. */
#define lok(test) do {\
        ++ltests;\
        if (!(test)) {\
            ++lfails;\
            dupprintf(globalf,"%s:%d error \n", __FILE__, __LINE__);\
        }} while (0)

#endif /*__MINCTEST_H__*/

void dupprintf(FILE *f, char const *fmt, ...) {   // duplicate printf
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    va_start(ap, fmt);
    vfprintf(f, fmt, ap);
    va_end(ap);
}

/*******************************TEST COMPONENTS***************************/
typedef struct Position {
    uint32_t x;
    uint32_t y;
} Position;

typedef struct Unit {
    uint16_t hp;
    uint16_t str;
} Unit;

typedef struct Velocity {
    uint64_t vx;
    uint64_t vy;
    uint64_t vz;
    uint64_t vw;
} Velocity;


typedef struct Sprite {
    uint32_t texture;
    bool isAnimated;
} Sprite;

struct Unit Unit_default = {.hp = 0, .str = 0 };

typedef struct Position2 {
    uint32_t x;
    uint32_t y;
} Position2;

typedef struct Unit2 {
    uint32_t hp;
    uint32_t str;
} Unit2;

void SystemMove2(struct tnecs_system_input *in_input) {
    // printf("SystemMove2\n");
    struct Position2 *p = TNECS_COMPONENTS_LIST(in_input, Position2);
    struct Unit2 *v = TNECS_COMPONENTS_LIST(in_input, Unit2);
    for (int i = 0; i < in_input->num_entities; i++) {
        // printf("i %d \n", i);
        p[i].x += v[i].hp;
        p[i].y += v[i].str;
    }
}
void SystemMovePhase1(struct tnecs_system_input *in_input) {
    // printf("SystemMovePhase1\n");
    // struct Position2 * p = TNECS_COMPONENTS_LIST(in_input, Position2);
    // struct Unit2 * v = TNECS_COMPONENTS_LIST(in_input, Unit2);
    for (int ent = 0; ent < in_input->num_entities; ent++) {
        // printf("in_input->world->bytype.entities[in_input->entity_archetype_id][ent] %d\n", in_input->world->bytype.entities[in_input->entity_archetype_id][ent]);
        // printf("in_input->world->entities.id[in_input->world->bytype.entities[in_input->entity_archetype_id][ent]] %d\n", in_input->world->entities.id[in_input->world->bytype.entities[in_input->entity_archetype_id][ent]]);
        tnecs_entity current_ent = in_input->world->bytype.entities[in_input->entity_archetype_id][ent];
        lok(current_ent);
        lok(in_input->world->entities.id[in_input->world->bytype.entities[in_input->entity_archetype_id][ent]]
            == in_input->world->bytype.entities[in_input->entity_archetype_id][ent]);
        lok(in_input->entity_archetype_id == tnecs_archetypeid(in_input->world,
                                                             in_input->world->entities.archetypes[current_ent]));
    }

    //     // printf("i %d \n", i);
    //     p[i].x += v[i].hp;
    //     p[i].y += v[i].str;
}

void SystemMovePhase4(struct tnecs_system_input *in_input) {
    // printf("SystemMovePhase4\n");
    for (int ent = 0; ent < in_input->num_entities; ent++) {
        tnecs_entity current_ent = in_input->world->bytype.entities[in_input->entity_archetype_id][ent];
        lok(current_ent);
        lok(in_input->world->entities.id[in_input->world->bytype.entities[in_input->entity_archetype_id][ent]]
            == in_input->world->bytype.entities[in_input->entity_archetype_id][ent]);
        lok(in_input->entity_archetype_id == tnecs_archetypeid(in_input->world,
                                                             in_input->world->entities.archetypes[current_ent]));
        // printf("in_input->world->bytype.entities[in_input->entity_archetype_id][ent] %d\n", in_input->world->bytype.entities[in_input->entity_archetype_id][ent]);
        // printf("in_input->world->entities.id[in_input->world->bytype.entities[in_input->entity_archetype_id][ent]] %d\n", in_input->world->entities.id[in_input->world->bytype.entities[in_input->entity_archetype_id][ent]]);
    }

}

void SystemMovePhase2(struct tnecs_system_input *in_input) {
    // printf("SystemMovePhase2\n");
    // struct Position2 * p = TNECS_COMPONENTS_LIST(in_input, Position2);
    // struct Unit2 * v = TNECS_COMPONENTS_LIST(in_input, Unit2);
    for (int ent = 0; ent < in_input->num_entities; ent++) {
        tnecs_entity current_ent = in_input->world->bytype.entities[in_input->entity_archetype_id][ent];
        lok(current_ent);
        lok(in_input->world->entities.id[in_input->world->bytype.entities[in_input->entity_archetype_id][ent]]
            == in_input->world->bytype.entities[in_input->entity_archetype_id][ent]);
        lok(in_input->entity_archetype_id == tnecs_archetypeid(in_input->world,
                                                             in_input->world->entities.archetypes[current_ent]));        // printf("in_input->world->bytype.entities[in_input->entity_archetype_id][ent] %d\n", in_input->world->bytype.entities[in_input->entity_archetype_id][ent]);
        // printf("in_inputf->world->entities.id[in_input->world->bytype.entities[in_input->entity_archetype_id][ent]] %d\n", in_input->world->entities.id[in_input->world->bytype.entities[in_input->entity_archetype_id][ent]]);
        // in_input->world->bytype.entities[in_input->entity_archetype_id][ent]
    }
}

/*****************************TEST CONSTANTS***************************/
#define ITERATIONS 10000
#define ARRAY_LEN 100
size_t fps_iterations = 10;

/*******************************TEST SYSTEMS***************************/
tnecs_entity test_entities[ITERATIONS];
struct Unit unit_array[ARRAY_LEN];
tnecs_entity        *components_list;
struct Position     *temp_position;
struct Unit         *temp_unit;
struct Sprite       *temp_sprite;
struct tnecs_world  *test_world;

void SystemMove(struct tnecs_system_input *in_input) {
    // printf("SystemMove\n");
    struct Position *p = TNECS_COMPONENTS_LIST(in_input, Position);
    struct Velocity *v = TNECS_COMPONENTS_LIST(in_input, Velocity);
    for (int ent = 0; ent < in_input->num_entities; ent++) {
        tnecs_entity current_ent = in_input->world->bytype.entities[in_input->entity_archetype_id][ent];
        lok(current_ent);
        lok(in_input->world->entities.id[in_input->world->bytype.entities[in_input->entity_archetype_id][ent]]
            == in_input->world->bytype.entities[in_input->entity_archetype_id][ent]);
        lok(in_input->entity_archetype_id == tnecs_archetypeid(in_input->world,
                                                             in_input->world->entities.archetypes[current_ent]));        // printf("in_input->world->bytype.entities[in_input->entity_archetype_id][ent] %d\n", in_input->world->bytype.entities[in_input->entity_archetype_id][ent]);
        // printf("in_input->world->entities.id[in_input->world->bytype.entities[in_input->entity_archetype_id][ent]] %d\n", in_input->world->entities.id[in_input->world->bytype.entities[in_input->entity_archetype_id][ent]]);

        // in_input->world->bytype.entities[in_input->entity_archetype_id][ent]
        p[ent].x = p[ent].x + v[ent].vx;
        p[ent].y = p[ent].y + v[ent].vy;
    }

    // for (int i = 0; i < in_input->num_entities; i++) {

    // }
}

/*******************************ACTUAL TESTS***************************/
void tnecs_test_utilities() {
    lok(TNECS_COMPONENT_TYPE2ID(1 << 0) == 1);
    lok(TNECS_COMPONENT_TYPE2ID(1 << 1) == 2);
    lok(TNECS_COMPONENT_TYPE2ID(1 << 2) == 3);
    lok(TNECS_COMPONENT_TYPE2ID(1 << 3) == 4);
    lok(TNECS_COMPONENT_TYPE2ID(1 << 4) == 5);
    lok(TNECS_COMPONENT_TYPE2ID(1 << 5) == 6);
    lok(TNECS_COMPONENT_TYPE2ID(1 << 6) == 7);
    lok(TNECS_COMPONENT_TYPE2ID(1 << 7) == 8);
    lok(TNECS_COMPONENT_TYPE2ID(1 << 8) == 9);
    lok(TNECS_COMPONENT_TYPE2ID(1 << 9) == 10);
    lok(TNECS_COMPONENT_TYPE2ID(1 << 10) == 11);
    lok(TNECS_COMPONENT_TYPE2ID(1 << 11) == 12);
    lok(TNECS_COMPONENT_TYPE2ID(1 << 12) == 13);
    lok(TNECS_COMPONENT_TYPE2ID(1 << 13) == 14);
    lok(TNECS_COMPONENT_TYPE2ID(1 << 14) == 15);
    lok(TNECS_COMPONENT_TYPE2ID(1 << 15) == 16);
    lok(TNECS_COMPONENT_TYPE2ID(1 << 16) == 17);
    lok(TNECS_COMPONENT_TYPE2ID(1 << 17) == 18);
    lok(TNECS_COMPONENT_TYPE2ID(1 << 18) == 19);
    lok(TNECS_COMPONENT_TYPE2ID(1 << 19) == 20);
    lok(TNECS_COMPONENT_TYPE2ID(1 << 20) == 21);
    lok(TNECS_COMPONENT_TYPE2ID(1 << 21) == 22);
    lok(TNECS_COMPONENT_TYPE2ID(1 << 22) == 23);
    lok(TNECS_COMPONENT_TYPE2ID(1 << 23) == 24);
    lok(TNECS_COMPONENT_TYPE2ID(1 << 24) == 25);
    lok(TNECS_COMPONENT_TYPE2ID(1 << 25) == 26);

    lok(TNECS_COMPONENT_ID2TYPE(1) == 1);
    lok(TNECS_COMPONENT_ID2TYPE(2) == 2);
    lok(TNECS_COMPONENT_ID2TYPE(3) == 4);
    lok(TNECS_COMPONENT_ID2TYPE(4) == 8);
    lok(TNECS_COMPONENT_ID2TYPE(5) == 16);
    lok(TNECS_COMPONENT_ID2TYPE(6) == 32);

    lok(TNECS_ARCHETYPE_IS_SUBTYPE(4, (4 + 8 + 16)));
    lok(!TNECS_ARCHETYPE_IS_SUBTYPE(2, (4 + 8 + 16)));

    lok(setBits_KnR_uint64_t(1) == 1);
    lok(setBits_KnR_uint64_t(2) == 1);
    lok(setBits_KnR_uint64_t(3) == 2);
    lok(setBits_KnR_uint64_t(4) == 1);
    lok(setBits_KnR_uint64_t(5) == 2);
    lok(setBits_KnR_uint64_t(6) == 2);
    lok(setBits_KnR_uint64_t(7) == 3);

    size_t arrtest1[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    size_t arrtest2[10] = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
    tnecs_arrdel(arrtest2, 1, 10, sizeof(*arrtest2));
    tnecs_arrdel(arrtest1, 2, 10, sizeof(*arrtest2));
    lok(arrtest2[0] == 9);
    lok(arrtest2[1] == 7);
    lok(arrtest2[2] == 6);
    lok(arrtest2[3] == 5);
    lok(arrtest2[4] == 4);
    lok(arrtest2[5] == 3);
    lok(arrtest2[6] == 2);
    lok(arrtest2[7] == 1);
    lok(arrtest2[8] == 0);
    lok(arrtest2[9] == 0);
    lok(arrtest1[0] == 0);
    lok(arrtest1[1] == 1);
    lok(arrtest1[2] == 3);
    lok(arrtest1[3] == 4);
    lok(arrtest1[4] == 5);
    lok(arrtest1[5] == 6);
    lok(arrtest1[6] == 7);
    lok(arrtest1[7] == 8);
    lok(arrtest1[8] == 9);
    lok(arrtest1[9] == 9);

    size_t *temp_sizes = calloc(2, sizeof(size_t));
    temp_sizes[0] = 1;
    temp_sizes[1] = 2;
    tnecs_arrdel(temp_sizes, 1, 2, sizeof(size_t));
    lok(temp_sizes[1] == 0);
    lok(temp_sizes[0] == 1);
    free(temp_sizes);
    temp_sizes = calloc(4, sizeof(size_t));
    temp_sizes[0] = 1;
    temp_sizes[1] = 2;
    temp_sizes[2] = 3;
    temp_sizes[3] = 4;
    tnecs_arrdel_scramble(temp_sizes, 2, 4, sizeof(size_t));
    lok(temp_sizes[0] == 1);
    lok(temp_sizes[1] == 2);
    lok(temp_sizes[2] == 4);
    lok(temp_sizes[3] == 0);
    free(temp_sizes);
}

void tnecs_test_component_registration() {
    tnecs_world_genesis(&test_world);
    lok(test_world != NULL);
    TNECS_REGISTER_COMPONENT(test_world, Position);
    size_t temp_comp_flag       = 1;
    size_t temp_comp_id         = 1;
    size_t temp_comp_order      = 0;
    size_t temp_archetype_id    = 1;
    size_t temp_archetype       = 1;
    lok(tnecs_component_hash2type(test_world, TNECS_HASH("Position")) == temp_archetype);
    lok(TNECS_COMPONENT_NAME2ID(test_world, Position) == temp_comp_id);
    lok(TNECS_COMPONENT_ID2TYPE(temp_comp_id) == temp_archetype);
    lok(test_world->bytype.components_id[temp_comp_id][temp_comp_order] == temp_comp_id);
    lok(test_world->components.hashes[TNECS_COMPONENT_NAME2ID(test_world,
                                                             Position)] == TNECS_HASH("Position"));
    lok(test_world->bytype.id[0] == 0);
    lok(test_world->bytype.id[1] == (TNECS_NULLSHIFT << 0));
    lok(test_world->bytype.id[1] == temp_comp_flag);
    lok(test_world->components.num == 2);
    lok(TNECS_COMPONENT_NAME2TYPE(test_world, Position) == (TNECS_NULLSHIFT << 0));
    lok(TNECS_COMPONENT_NAME2TYPE(test_world, Position) == test_world->bytype.id[temp_archetype_id]);

    TNECS_REGISTER_COMPONENT(test_world, Unit);
    temp_comp_flag = 2;
    temp_comp_id = 2;
    temp_comp_order = 0;
    temp_archetype_id = 2;
    temp_archetype = 2;
    lok(tnecs_component_hash2type(test_world, TNECS_HASH("Unit")) == temp_archetype);
    lok(TNECS_COMPONENT_NAME2ID(test_world, Unit) == temp_comp_id);
    lok(TNECS_COMPONENT_ID2TYPE(temp_comp_id) == temp_archetype);
    lok(test_world->bytype.components_id[temp_comp_id][temp_comp_order] == temp_comp_id);
    lok(test_world->components.hashes[TNECS_COMPONENT_NAME2ID(test_world, Unit)] == TNECS_HASH("Unit"));
    lok(test_world->bytype.id[0] == 0);
    lok(test_world->bytype.id[1] == (TNECS_NULLSHIFT << 0));
    lok(test_world->bytype.id[2] == (TNECS_NULLSHIFT << 1));
    lok(test_world->bytype.id[2] == temp_comp_flag);
    lok(test_world->components.num == 3);
    lok(TNECS_COMPONENT_NAME2TYPE(test_world, Unit) == (TNECS_NULLSHIFT << 1));
    lok(TNECS_COMPONENT_NAME2TYPE(test_world, Unit) == test_world->bytype.id[temp_archetype_id]);
    lok(test_world->components.hashes[TNECS_COMPONENT_NAME2ID(test_world, Unit)] == TNECS_HASH("Unit"));

    TNECS_REGISTER_COMPONENT(test_world, Sprite);
    temp_comp_flag = 4;
    temp_comp_id = 3;
    temp_comp_order = 0;
    temp_archetype_id = 3;
    temp_archetype = 4;
    lok(tnecs_component_hash2type(test_world, TNECS_HASH("Sprite")) == temp_archetype);
    lok(TNECS_COMPONENT_NAME2ID(test_world, Sprite) == temp_comp_id);
    lok(TNECS_COMPONENT_ID2TYPE(temp_comp_id) == temp_archetype);
    lok(test_world->bytype.components_id[temp_comp_id][temp_comp_order] == temp_comp_id);
    lok(test_world->components.hashes[TNECS_COMPONENT_NAME2ID(test_world,
                                                             Sprite)] == TNECS_HASH("Sprite"));
    lok(test_world->bytype.id[0] == 0);
    lok(test_world->bytype.id[1] == (TNECS_NULLSHIFT << 0));
    lok(test_world->bytype.id[2] == (TNECS_NULLSHIFT << 1));
    lok(test_world->bytype.id[3] == (TNECS_NULLSHIFT << 2));
    lok(test_world->bytype.id[3] == temp_comp_flag);
    lok(test_world->components.num == 4);
    lok(TNECS_COMPONENT_NAME2TYPE(test_world, Sprite) == (TNECS_NULLSHIFT << 2));
    lok(TNECS_COMPONENT_NAME2TYPE(test_world, Sprite) == test_world->bytype.id[temp_archetype_id]);
    lok(test_world->components.hashes[TNECS_COMPONENT_NAME2ID(test_world,
                                                             Sprite)] == TNECS_HASH("Sprite"));

    TNECS_REGISTER_COMPONENT(test_world, Velocity);
    temp_comp_flag = 8;
    temp_comp_id = 4;
    temp_comp_order = 0;
    temp_archetype_id = 4;
    temp_archetype = 8;
    lok(TNECS_COMPONENT_NAME2ID(test_world, Velocity) == temp_comp_id);
    lok(TNECS_COMPONENT_HASH2ID(test_world, TNECS_HASH("Velocity")) == temp_comp_id);
    lok(TNECS_COMPONENT_ID2TYPE(temp_comp_id) == temp_archetype);
    lok(test_world->bytype.components_id[temp_comp_id][temp_comp_order] == temp_comp_id);
    lok(test_world->components.hashes[TNECS_COMPONENT_NAME2ID(test_world,
                                                             Velocity)] == TNECS_HASH("Velocity"));
    lok(test_world->bytype.id[0] == 0);
    lok(test_world->bytype.id[1] == (TNECS_NULLSHIFT << 0));
    lok(test_world->bytype.id[2] == (TNECS_NULLSHIFT << 1));
    lok(test_world->bytype.id[3] == (TNECS_NULLSHIFT << 2));
    lok(test_world->bytype.id[4] == (TNECS_NULLSHIFT << 3));
    lok(test_world->bytype.id[4] == temp_comp_flag);
    lok(test_world->components.num == 5);
    lok(TNECS_COMPONENT_NAME2TYPE(test_world, Velocity) == (TNECS_NULLSHIFT << 3));
    lok(TNECS_COMPONENT_NAME2TYPE(test_world, Velocity) == test_world->bytype.id[temp_archetype_id]);
    lok(test_world->components.hashes[TNECS_COMPONENT_NAME2ID(test_world,
                                                             Velocity)] == TNECS_HASH("Velocity"));

    lok(TNECS_COMPONENT_IDS2ARCHETYPE(1, 2, 3) == (1 + 2 + 4));
    lok(TNECS_COMPONENT_NAMES2ARCHETYPE(test_world, Position, Unit, Velocity) == (1 + 2 + 8));

}

void tnecs_test_system_registration() {
    TNECS_REGISTER_SYSTEM_wEXCL(test_world, SystemMove, 1, Position, Velocity);
    size_t temp_comp_id = 1;
    size_t temp_archetype_id = 5;
    size_t temp_archetype = 1 + 8;

    lok(TNECS_SYSTEM_NAME2ID(test_world, SystemMove) == temp_comp_id);
    lok(TNECS_SYSTEM_NAME2ARCHETYPE(test_world, SystemMove) == temp_archetype);
    lok(TNECS_SYSTEM_NAME2ID(test_world, SystemMove) == 1);
    lok(TNECS_HASH("SystemMove") == tnecs_hash_djb2("SystemMove"));
    lok(test_world->systems.hashes[TNECS_SYSTEM_NAME2ID(test_world,
                                                       SystemMove)] == TNECS_HASH("SystemMove"));
    lok(TNECS_SYSTEM_ID2ARCHETYPE(test_world, TNECS_SYSTEM_NAME2ID(test_world,
                                 SystemMove)) == temp_archetype);
    lok(TNECS_SYSTEM_NAME2ARCHETYPE(test_world, SystemMove) == temp_archetype);
    lok(TNECS_SYSTEM_NAME2ARCHETYPE(test_world, SystemMove) == temp_archetype);


    lok(test_world->bytype.components_id[temp_archetype_id][0] == TNECS_COMPONENT_NAME2ID(test_world,
            Position));
    lok(test_world->bytype.components_id[temp_archetype_id][1] == TNECS_COMPONENT_NAME2ID(test_world,
            Velocity));
}

void tnecs_test_entity_creation() {
    // dupprintf(globalf, "tnecs_test_entity_creation \n");

    lok(test_world->entities.num == TNECS_NULLSHIFT);
    TNECS_REGISTER_COMPONENT(test_world, Sprite);
    tnecs_entity Silou = tnecs_entity_create(test_world);
    lok(Silou == TNECS_NULLSHIFT);
    lok(test_world->entities.num == (TNECS_NULLSHIFT + 1));
    tnecs_entity Pirou = TNECS_ENTITY_CREATE(test_world);
    lok(Pirou == (TNECS_NULLSHIFT + 1));
    lok(test_world->entities.num == (TNECS_NULLSHIFT + 2));
    lok(Silou != Pirou);
    tnecs_entity Perignon = TNECS_ENTITY_CREATE_wCOMPONENTS(test_world, Position, Unit);
    temp_position = TNECS_GET_COMPONENT(test_world, Perignon, Position);
    lok(temp_position != NULL);
    if (temp_position != NULL) {
        lok(temp_position->x == 0);
        lok(temp_position->y == 0);
        temp_position->x = 3;
        temp_position->y = 6;
    }

    lok(TNECS_COMPONENT_NAME2ID(test_world, Position) == 1);
    lok(TNECS_COMPONENT_NAME2ID(test_world, Unit) == 2);
    lok((TNECS_COMPONENT_NAME2ID(test_world, Unit) + TNECS_COMPONENT_NAME2ID(test_world,
            Position)) == 3);
    lok(test_world->entities.archetypes[Perignon] == (TNECS_COMPONENT_NAME2ID(test_world,
                                                   Position) + TNECS_COMPONENT_NAME2ID(test_world, Unit)));

    temp_position = TNECS_GET_COMPONENT(test_world, Perignon, Sprite);
    lok(temp_position == NULL);
    temp_unit = TNECS_GET_COMPONENT(test_world, Perignon, Unit);
    lok(temp_unit != NULL);
    if (temp_unit != NULL) {
        lok(temp_unit->hp  == 0);
        lok(temp_unit->str == 0);
    }

    tnecs_entity_destroy(test_world, Silou);
    tnecs_entity *open_arr = test_world->entities_open.arr;
    lok(test_world->entities_open.num == 0);
    lok(!test_world->entities.id[Silou]);
    tnecs_entities_open_reuse(test_world);
    lok(test_world->entities_open.num == 1);
    lok(open_arr[0] != TNECS_NULL);
    lok(open_arr[0] == Silou);
    tnecs_entity_create(test_world);
    lok(test_world->entities.id[Silou]);

    TNECS_ENTITIES_CREATE(test_world, 100);
    lok(test_world->entities.num == 104);
    lok(tnecs_entity_create(test_world));
    lok(test_world->entities.num == 105);

    tnecs_world *test_world3 = NULL;
    tnecs_world_genesis(&test_world3);
    tnecs_world_destroy(&test_world3);

    // MORE TESTS FOR COVERAGE
    tnecs_world *test_world2 = NULL;
    tnecs_world_genesis(&test_world2);

    test_world2->bytype.num = TNECS_INIT_SYSTEM_LEN;
    TNECS_REGISTER_COMPONENT(test_world2, Position2);
    lok(test_world2->components.num == 2);

    // Coverage for if in tnecs_register_system
    tnecs_world_destroy(&test_world2);
    lok(test_world2 == NULL);
    tnecs_world_genesis(&test_world2);

    TNECS_REGISTER_COMPONENT(test_world2, Position2);
    test_world2->byphase.num_systems[0] = TNECS_INIT_PHASE_LEN;
    TNECS_REGISTER_SYSTEM(test_world2, SystemMovePhase1, Position2);
    tnecs_world_destroy(&test_world2);

    // Coverage for "for" in tnecs_component_del
    tnecs_world_genesis(&test_world2);
    TNECS_REGISTER_COMPONENT(test_world2, Unit2);
    TNECS_REGISTER_COMPONENT(test_world2, Position2);
    tnecs_entity Erwin = TNECS_ENTITY_CREATE_wCOMPONENTS(test_world2, Position2, Unit2);
    tnecs_component_del(test_world2, Erwin, (1 + 2));
    tnecs_world_destroy(&test_world2);
}

void tnecs_test_component_add() {
    tnecs_entity Silou = tnecs_entity_create(test_world);
    TNECS_ADD_COMPONENT(test_world, Silou, Position);
    lok((test_world->entities.archetypes[Silou] & TNECS_COMPONENT_NAME2TYPE(test_world, Unit)) == 0);
    TNECS_ADD_COMPONENT(test_world, Silou, Unit);
    lok((test_world->entities.archetypes[Silou] & TNECS_COMPONENT_NAME2TYPE(test_world, Position)) > 0);
    lok((test_world->entities.archetypes[Silou] & TNECS_COMPONENT_NAME2TYPE(test_world, Unit)) > 0);
    lok((test_world->entities.archetypes[Silou] & TNECS_COMPONENT_NAME2TYPE(test_world, Sprite)) == 0);

    tnecs_entity Pirou = tnecs_entity_create(test_world);
    TNECS_ADD_COMPONENT(test_world, Pirou, Position);
    lok((test_world->entities.archetypes[Pirou] & TNECS_COMPONENT_NAME2TYPE(test_world, Position)) > 0);
    lok((test_world->entities.archetypes[Pirou] & TNECS_COMPONENT_NAME2TYPE(test_world, Unit)) == 0);
    TNECS_ADD_COMPONENT(test_world, Pirou, Unit);
    lok((test_world->entities.archetypes[Pirou] & TNECS_COMPONENT_NAME2TYPE(test_world, Unit)) > 0);
    lok((test_world->entities.archetypes[Pirou] & TNECS_COMPONENT_NAME2TYPE(test_world, Position)) > 0);
    lok((test_world->entities.archetypes[Pirou] & TNECS_COMPONENT_NAME2TYPE(test_world, Sprite)) == 0);

    tnecs_entity Chasse = tnecs_entity_create(test_world);
    TNECS_ADD_COMPONENTS(test_world, Chasse, 1, Sprite, Position);
    lok((test_world->entities.archetypes[Chasse] & TNECS_COMPONENT_NAME2TYPE(test_world, Unit)) == 0);
    lok((test_world->entities.archetypes[Chasse] & TNECS_COMPONENT_NAME2TYPE(test_world, Sprite)) > 0);
    lok((test_world->entities.archetypes[Chasse] & TNECS_COMPONENT_NAME2TYPE(test_world, Position)) > 0);

    temp_position = TNECS_GET_COMPONENT(test_world, Silou, Position);
    lok(temp_position != NULL);
    lok(temp_position->x == 0);
    lok(temp_position->y == 0);
    temp_position->x = 1;
    temp_position->y = 2;
    lok(temp_position->x == 1);
    lok(temp_position->y == 2);
    temp_position = TNECS_GET_COMPONENT(test_world, Silou, Position);
    lok(temp_position->x == 1);
    lok(temp_position->y == 2);

    temp_unit = TNECS_GET_COMPONENT(test_world, Silou, Unit);
    lok(temp_unit != NULL);
    lok(temp_unit->hp == 0);
    lok(temp_unit->str == 0);
    temp_unit->hp = 3;
    temp_unit->str = 4;
    lok(temp_unit->hp == 3);
    lok(temp_unit->str == 4);
    temp_unit = TNECS_GET_COMPONENT(test_world, Silou, Unit);
    lok(temp_unit->hp == 3);
    lok(temp_unit->str == 4);
    temp_position = TNECS_GET_COMPONENT(test_world, Silou, Position);
    lok(temp_position->x == 1);
    lok(temp_position->y == 2);

    temp_position = TNECS_GET_COMPONENT(test_world, Pirou, Position);
    lok(temp_position->x == 0);
    lok(temp_position->y == 0);
    temp_position->x = 5;
    temp_position->y = 6;
    lok(temp_position->x == 5);
    lok(temp_position->y == 6);
    temp_position = TNECS_GET_COMPONENT(test_world, Pirou, Position);
    lok(temp_position->x == 5);
    lok(temp_position->y == 6);

    temp_unit = TNECS_GET_COMPONENT(test_world, Pirou, Unit);
    lok(temp_unit->hp == 0);
    lok(temp_unit->str == 0);
    temp_unit->hp = 7;
    temp_unit->str = 8;
    lok(temp_unit->hp == 7);
    lok(temp_unit->str == 8);
    temp_position = TNECS_GET_COMPONENT(test_world, Pirou, Position);
    lok(temp_position->x == 5);
    lok(temp_position->y == 6);
    temp_unit = TNECS_GET_COMPONENT(test_world, Pirou, Unit);
    lok(temp_unit->hp == 7);
    lok(temp_unit->str == 8);
}

void tnecs_test_component_remove() {
    tnecs_entity Silou = tnecs_entity_create(test_world);
    TNECS_ADD_COMPONENT(test_world, Silou, Position);
    lok(TNECS_ENTITY_HASCOMPONENT(test_world, Silou, Position));
    TNECS_REMOVE_COMPONENTS(test_world, Silou, Position);
    lok(!TNECS_ENTITY_HASCOMPONENT(test_world, Silou, Position));

    tnecs_entity Perignon = TNECS_ENTITY_CREATE_wCOMPONENTS(test_world, Position, Velocity);
    lok(TNECS_ENTITY_HASCOMPONENT(test_world, Perignon, Position));
    lok(TNECS_ENTITY_HASCOMPONENT(test_world, Perignon, Velocity));
    lok(!TNECS_ENTITY_HASCOMPONENT(test_world, Perignon, Unit));

    TNECS_REMOVE_COMPONENTS(test_world, Perignon, Velocity);
    lok(!TNECS_ENTITY_HASCOMPONENT(test_world, Perignon, Velocity));
    lok(TNECS_ENTITY_HASCOMPONENT(test_world, Perignon, Position));
    lok(!TNECS_ENTITY_HASCOMPONENT(test_world, Perignon, Unit));

    tnecs_entity Pirou = TNECS_ENTITY_CREATE_wCOMPONENTS(test_world, Position, Velocity, Unit);
    lok(TNECS_ENTITY_HASCOMPONENT(test_world, Pirou, Position));
    lok(TNECS_ENTITY_HASCOMPONENT(test_world, Pirou, Velocity));
    lok(TNECS_ENTITY_HASCOMPONENT(test_world, Pirou, Unit));

    TNECS_REMOVE_COMPONENTS(test_world, Pirou, Position, Velocity);
    lok(TNECS_ENTITY_HASCOMPONENT(test_world, Pirou, Unit));
    lok(!TNECS_ENTITY_HASCOMPONENT(test_world, Pirou, Position));
    lok(!TNECS_ENTITY_HASCOMPONENT(test_world, Pirou, Velocity));

}

void tnecs_test_component_array() {
    tnecs_world *arr_world = NULL;
    tnecs_world_genesis(&arr_world);
    
    lok(TNECS_REGISTER_COMPONENT(arr_world, Velocity));
    lok(TNECS_REGISTER_COMPONENT(arr_world, Position));
    lok(TNECS_REGISTER_COMPONENT(arr_world, Sprite));
    lok(TNECS_REGISTER_COMPONENT(arr_world, Unit));
    TNECS_REGISTER_SYSTEM_wEXCL(arr_world, SystemMove, 0, Unit); // 4X
    TNECS_REGISTER_SYSTEM_wEXCL(arr_world, SystemMovePhase1, 0, Unit, Velocity);  // 2X
    TNECS_REGISTER_SYSTEM_wEXCL(arr_world, SystemMovePhase2, 0, Unit, Position); // 2X
    TNECS_REGISTER_SYSTEM_wEXCL(arr_world, SystemMovePhase4, 0, Unit, Position, Velocity); // 1X

    tnecs_entity temp_ent = TNECS_ENTITY_CREATE_wCOMPONENTS(arr_world, Unit);
    TNECS_ENTITY_CREATE_wCOMPONENTS(arr_world, Unit, Velocity);
    TNECS_ENTITY_CREATE_wCOMPONENTS(arr_world, Unit, Velocity);
    TNECS_ENTITY_CREATE_wCOMPONENTS(arr_world, Unit, Velocity);
    TNECS_ENTITY_CREATE_wCOMPONENTS(arr_world, Unit, Position);
    TNECS_ENTITY_CREATE_wCOMPONENTS(arr_world, Unit, Position);
    TNECS_ENTITY_CREATE_wCOMPONENTS(arr_world, Unit, Position);
    TNECS_ENTITY_CREATE_wCOMPONENTS(arr_world, Unit, Position);
    TNECS_ENTITY_CREATE_wCOMPONENTS(arr_world, Unit, Position, Velocity);
    TNECS_ENTITY_CREATE_wCOMPONENTS(arr_world, Unit, Position, Velocity);

    size_t temp_archetypeid = TNECS_COMPONENT_NAMES2ARCHETYPEID(arr_world, Unit, Position);
    size_t temp_component_order = tnecs_component_order_bytypeid(arr_world,
                                  TNECS_COMPONENT_NAME2ID(arr_world, Position), temp_archetypeid);
    lok(arr_world->bytype.components[temp_archetypeid][temp_component_order].num_components == 4);
    temp_component_order = tnecs_component_order_bytypeid(arr_world, TNECS_COMPONENT_NAME2ID(arr_world,
                                                          Unit), temp_archetypeid);
    lok(arr_world->bytype.components[temp_archetypeid][temp_component_order].num_components == 4);

    temp_archetypeid = TNECS_COMPONENT_NAMES2ARCHETYPEID(arr_world, Unit, Velocity);
    temp_component_order = tnecs_component_order_bytypeid(arr_world, TNECS_COMPONENT_NAME2ID(arr_world,
                                                          Unit), temp_archetypeid);
    lok(arr_world->bytype.components[temp_archetypeid][temp_component_order].num_components == 3);
    temp_component_order = tnecs_component_order_bytypeid(arr_world, TNECS_COMPONENT_NAME2ID(arr_world,
                                                          Velocity), temp_archetypeid);
    lok(arr_world->bytype.components[temp_archetypeid][temp_component_order].num_components == 3);

    temp_archetypeid = TNECS_COMPONENT_NAMES2ARCHETYPEID(arr_world, Unit, Velocity, Position);
    temp_component_order = tnecs_component_order_bytypeid(arr_world, TNECS_COMPONENT_NAME2ID(arr_world,
                                                          Unit), temp_archetypeid);
    lok(arr_world->bytype.components[temp_archetypeid][temp_component_order].num_components == 2);
    temp_component_order = tnecs_component_order_bytypeid(arr_world, TNECS_COMPONENT_NAME2ID(arr_world,
                                                          Position), temp_archetypeid);
    lok(arr_world->bytype.components[temp_archetypeid][temp_component_order].num_components == 2);
    temp_component_order = tnecs_component_order_bytypeid(arr_world, TNECS_COMPONENT_NAME2ID(arr_world,
                                                          Velocity), temp_archetypeid);
    lok(arr_world->bytype.components[temp_archetypeid][temp_component_order].num_components == 2);

    size_t old_entity_order = arr_world->entities.orders[temp_ent];
    lok(old_entity_order == 0);
    lok(arr_world->entities.archetypes[temp_ent] == TNECS_COMPONENT_NAMES2ARCHETYPE(arr_world, Unit));
    size_t old_archetypeid = TNECS_COMPONENT_NAMES2ARCHETYPEID(arr_world, Unit);
    lok(arr_world->bytype.num_entities[old_archetypeid] == 1);
    lok(old_archetypeid == 4);
    lok(TNECS_COMPONENT_NAME2ID(arr_world, Unit) == 4);
    size_t old_component_order = tnecs_component_order_bytypeid(arr_world,
                                                                TNECS_COMPONENT_NAME2ID(arr_world, Unit), old_archetypeid);

    lok(old_component_order < TNECS_COMPONENT_CAP);
    lok(old_component_order == 0);

    lok(arr_world->bytype.components[old_archetypeid][old_component_order].num_components == 1);

    struct Unit *temp_unit = TNECS_GET_COMPONENT(arr_world, temp_ent, Unit);
    struct Position *temp_pos = TNECS_GET_COMPONENT(arr_world, temp_ent, Position);
    lok(temp_pos == NULL);
    lok(temp_unit->hp == 0);
    lok(temp_unit->str == 0);
    temp_unit->hp = 10;
    temp_unit->str = 12;
    temp_unit = TNECS_GET_COMPONENT(arr_world, temp_ent + 1, Unit);
    TNECS_ENTITIES_CREATE(arr_world, 10);
    temp_unit = TNECS_GET_COMPONENT(arr_world, temp_ent, Unit);
    lok(temp_unit->hp == 10);
    lok(temp_unit->str == 12);

    size_t new_archetypeid = TNECS_COMPONENT_NAMES2ARCHETYPEID(arr_world, Unit, Position);
    lok(arr_world->bytype.num_entities[new_archetypeid] == 4);
    TNECS_ADD_COMPONENT(arr_world, temp_ent, Position);
    lok(arr_world->bytype.num_entities[old_archetypeid] == 0);
    lok(arr_world->entities.archetypes[temp_ent] == TNECS_COMPONENT_NAMES2ARCHETYPE(arr_world, Unit,
            Position));
    lok(arr_world->bytype.num_entities[new_archetypeid] == 5);
    temp_unit = TNECS_GET_COMPONENT(arr_world, temp_ent, Unit);
    size_t new_entity_order = arr_world->entities.orders[temp_ent];
    lok(new_entity_order == 4);
    lok(new_entity_order != old_entity_order);

    temp_pos = TNECS_GET_COMPONENT(arr_world, temp_ent, Position);
    lok(temp_unit->hp == 10);
    lok(temp_unit->str == 12);
    temp_unit->hp++;
    temp_unit->str++;
    lok(temp_unit->hp == 11);
    lok(temp_unit->str == 13);
    lok(temp_pos->x == 0);
    lok(temp_pos->y == 0);

    tnecs_world_destroy(&arr_world);
}

void tnecs_test_world_progress() {
    struct Velocity *temp_velocity;
    tnecs_entity Perignon = TNECS_ENTITY_CREATE_wCOMPONENTS(test_world, Position, Velocity);
    lok(TNECS_ENTITY_HASCOMPONENT(test_world, Perignon, Position));
    lok(TNECS_ENTITY_HASCOMPONENT(test_world, Perignon, Velocity));
    lok(!TNECS_ENTITY_HASCOMPONENT(test_world, Perignon, Unit));

    lok(test_world->entities.archetypes[Perignon] == (1 + 8));
    lok(test_world->bytype.num_entities[tnecs_archetypeid(test_world, 1 + 8)] == 1);

    temp_position = TNECS_GET_COMPONENT(test_world, Perignon, Position);
    temp_velocity = TNECS_GET_COMPONENT(test_world, Perignon, Velocity);
    temp_position->x = 100;
    temp_position->y = 200;

    TNECS_REGISTER_SYSTEM_wPHASE_wEXCL(test_world, SystemMovePhase4, 4, 1, Velocity);
    TNECS_REGISTER_SYSTEM_wPHASE_wEXCL(test_world, SystemMovePhase2, 2, 1, Velocity);
    TNECS_REGISTER_SYSTEM_wPHASE_wEXCL(test_world, SystemMovePhase1, 1, 1, Position);
    TNECS_REGISTER_SYSTEM_wPHASE_wEXCL(test_world, SystemMovePhase2, 1, 1, Unit);
    lok(test_world->byphase.num== 5);
    lok(test_world->byphase.systems[1][0] == &SystemMovePhase1);
    lok(test_world->byphase.systems[1][1] == &SystemMovePhase2);
    lok(test_world->byphase.systems[2][0] == &SystemMovePhase2);
    lok(test_world->byphase.systems[4][0] == &SystemMovePhase4);
    tnecs_system_order_switch(test_world, 1, 0, 1);
    lok(test_world->byphase.systems[1][0] == &SystemMovePhase2);
    lok(test_world->byphase.systems[1][1] == &SystemMovePhase1);
    tnecs_system_order_switch(test_world, 1, 0, 1);
    lok(test_world->byphase.systems[1][0] == &SystemMovePhase1);
    lok(test_world->byphase.systems[1][1] == &SystemMovePhase2);

    temp_velocity->vx = 1;
    temp_velocity->vy = 2;
    tnecs_world_step(test_world, 1, NULL);
    temp_position = TNECS_GET_COMPONENT(test_world, Perignon, Position);
    temp_velocity = TNECS_GET_COMPONENT(test_world, Perignon, Velocity);

    lok(test_world->systems_torun.num == 5);
    tnecs_system_ptr *torun_arr = test_world->systems_torun.arr;
    lok(torun_arr[0] == &SystemMove);
    lok(torun_arr[1] == &SystemMovePhase1);
    lok(torun_arr[2] == &SystemMovePhase2);
    lok(torun_arr[3] == &SystemMovePhase2);
    lok(torun_arr[4] == &SystemMovePhase4);
    lok(torun_arr[0] != NULL);
    lok(torun_arr[1] != NULL);
    lok(torun_arr[2] != NULL);
    lok(torun_arr[3] != NULL);
    lok(torun_arr[4] != NULL);
    lok(temp_position->x == 101);
    lok(temp_position->y == 202);
    lok(temp_velocity->vx == 1);
    lok(temp_velocity->vy == 2);
    tnecs_world_step(test_world, 1, NULL);
    temp_position = TNECS_GET_COMPONENT(test_world, Perignon, Position);
    temp_velocity = TNECS_GET_COMPONENT(test_world, Perignon, Velocity);
    lok(test_world->systems_torun.num == 5);
    torun_arr = test_world->systems_torun.arr;

    lok(torun_arr[0] == &SystemMove);
    lok(torun_arr[1] == &SystemMovePhase1);
    lok(torun_arr[2] == &SystemMovePhase2);
    lok(torun_arr[3] == &SystemMovePhase2);
    lok(torun_arr[4] == &SystemMovePhase4);
    lok(torun_arr[0] != NULL);
    lok(torun_arr[1] != NULL);
    lok(torun_arr[2] != NULL);
    lok(torun_arr[3] != NULL);
    lok(torun_arr[4] != NULL);
    lok(temp_position->x == 102);
    lok(temp_position->y == 204);
    lok(temp_velocity->vx == 1);
    lok(temp_velocity->vy == 2);
    tnecs_world_step(test_world, 0, NULL);

    lok(test_world->entities.archetypes[Perignon] == (1 + 8));
    lok(test_world->bytype.num_entities[TNECS_ARCHETYPEID(test_world, 1 + 8)] == 1);
    tnecs_entity_destroy(test_world, Perignon);

    tnecs_grow_phase(test_world);
    tnecs_grow_system(test_world);
    tnecs_grow_archetype(test_world);

    struct tnecs_world *inclusive_world = NULL;
    tnecs_world_genesis(&inclusive_world);
    lok(inclusive_world != NULL);

    TNECS_REGISTER_COMPONENT(inclusive_world, Velocity);
    TNECS_REGISTER_COMPONENT(inclusive_world, Position);
    TNECS_REGISTER_COMPONENT(inclusive_world, Sprite);
    TNECS_REGISTER_COMPONENT(inclusive_world, Unit);
    TNECS_REGISTER_SYSTEM_wEXCL(inclusive_world, SystemMove, 0, Unit); // 4X
    TNECS_REGISTER_SYSTEM_wEXCL(inclusive_world, SystemMovePhase1, 0, Unit, Velocity);  // 2X
    TNECS_REGISTER_SYSTEM_wEXCL(inclusive_world, SystemMovePhase2, 0, Unit, Position); // 2X
    TNECS_REGISTER_SYSTEM_wEXCL(inclusive_world, SystemMovePhase4, 0, Unit, Position, Velocity); // 1X
    lok(TNECS_SYSTEM_NAME2ARCHETYPE(inclusive_world, SystemMove) == 8);
    lok(TNECS_SYSTEM_NAME2ARCHETYPE(inclusive_world, SystemMovePhase1) == 8 + 1);
    lok(TNECS_SYSTEM_NAME2ARCHETYPE(inclusive_world, SystemMovePhase2) == 8 + 2);
    lok(TNECS_SYSTEM_NAME2ARCHETYPE(inclusive_world, SystemMovePhase4) == 8 + 2 + 1);

    lok(inclusive_world->bytype.num_archetype_ids[TNECS_ARCHETYPEID(inclusive_world,
                                                            TNECS_SYSTEM_NAME2ARCHETYPE(inclusive_world, SystemMove))] == 3);
    lok(inclusive_world->bytype.archetype_id[TNECS_ARCHETYPEID(inclusive_world,
                                                              TNECS_SYSTEM_NAME2ARCHETYPE(inclusive_world, SystemMove))][0] == TNECS_ARCHETYPEID(inclusive_world,
                                                                      TNECS_SYSTEM_NAME2ARCHETYPE(inclusive_world, SystemMovePhase1)));
    lok(inclusive_world->bytype.archetype_id[TNECS_ARCHETYPEID(inclusive_world,
                                                              TNECS_SYSTEM_NAME2ARCHETYPE(inclusive_world, SystemMove))][1] == TNECS_ARCHETYPEID(inclusive_world,
                                                                      TNECS_SYSTEM_NAME2ARCHETYPE(inclusive_world, SystemMovePhase2)));
    lok(inclusive_world->bytype.archetype_id[TNECS_ARCHETYPEID(inclusive_world,
                                                              TNECS_SYSTEM_NAME2ARCHETYPE(inclusive_world, SystemMove))][2] == TNECS_ARCHETYPEID(inclusive_world,
                                                                      TNECS_SYSTEM_NAME2ARCHETYPE(inclusive_world, SystemMovePhase4)));
    lok(inclusive_world->bytype.num_archetype_ids[TNECS_ARCHETYPEID(inclusive_world,
                                                            TNECS_SYSTEM_NAME2ARCHETYPE(inclusive_world, SystemMovePhase1))] == 1);
    lok(inclusive_world->bytype.archetype_id[TNECS_ARCHETYPEID(inclusive_world,
                                                              TNECS_SYSTEM_NAME2ARCHETYPE(inclusive_world,
                                                                      SystemMovePhase1))][0] == TNECS_ARCHETYPEID(inclusive_world,
                                                                              TNECS_SYSTEM_NAME2ARCHETYPE(inclusive_world, SystemMovePhase4)));
    lok(inclusive_world->bytype.num_archetype_ids[TNECS_ARCHETYPEID(inclusive_world,
                                                            TNECS_SYSTEM_NAME2ARCHETYPE(inclusive_world, SystemMovePhase2))] == 1);
    lok(inclusive_world->bytype.archetype_id[TNECS_ARCHETYPEID(inclusive_world,
                                                              TNECS_SYSTEM_NAME2ARCHETYPE(inclusive_world,
                                                                      SystemMovePhase2))][0] == TNECS_ARCHETYPEID(inclusive_world,
                                                                              TNECS_SYSTEM_NAME2ARCHETYPE(inclusive_world, SystemMovePhase4)));
    lok(inclusive_world->bytype.num_archetype_ids[TNECS_ARCHETYPEID(inclusive_world,
                                                            TNECS_SYSTEM_NAME2ARCHETYPE(inclusive_world, SystemMovePhase4))] == 0);

    lok(inclusive_world->bytype.num == 8);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit, Velocity);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit, Position);
    tnecs_entity temp_todestroy = TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit, Position);
    tnecs_entity_destroy(inclusive_world, temp_todestroy);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit, Position);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit, Position);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit, Position);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit, Velocity);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit, Position);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit, Position, Velocity);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit, Position, Velocity);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit, Position);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit, Position, Velocity);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit, Position, Velocity);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit, Position, Velocity);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit, Position, Velocity);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit, Position, Velocity);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit, Position, Velocity);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit, Position, Velocity);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit, Position, Velocity);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit, Position, Velocity);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit, Position, Velocity);
    lok(inclusive_world->bytype.num_entities[TNECS_ARCHETYPEID(inclusive_world,
                                                              TNECS_SYSTEM_ID2ARCHETYPE(inclusive_world, TNECS_SYSTEM_NAME2ID(inclusive_world,
                                                                      SystemMove)))] == 9);
    lok(inclusive_world->bytype.num_entities[TNECS_ARCHETYPEID(inclusive_world,
                                                              TNECS_SYSTEM_ID2ARCHETYPE(inclusive_world, TNECS_SYSTEM_NAME2ID(inclusive_world,
                                                                      SystemMovePhase1)))] == 2);
    lok(inclusive_world->bytype.num_entities[TNECS_ARCHETYPEID(inclusive_world,
                                                              TNECS_SYSTEM_ID2ARCHETYPE(inclusive_world, TNECS_SYSTEM_NAME2ID(inclusive_world,
                                                                      SystemMovePhase2)))] == 6);
    lok(inclusive_world->bytype.num_entities[TNECS_ARCHETYPEID(inclusive_world,
                                                              TNECS_SYSTEM_ID2ARCHETYPE(inclusive_world, TNECS_SYSTEM_NAME2ID(inclusive_world,
                                                                      SystemMovePhase4)))] == 12);
    lok(inclusive_world->bytype.num == 8);
    tnecs_world_step(inclusive_world, 1, NULL);

    lok(inclusive_world->systems_torun.num == 9);
    torun_arr = inclusive_world->systems_torun.arr;
    lok(torun_arr[0] == &SystemMove);
    lok(torun_arr[1] == &SystemMove);
    lok(torun_arr[2] == &SystemMove);
    lok(torun_arr[3] == &SystemMove);
    lok(torun_arr[4] == &SystemMovePhase1);
    lok(torun_arr[5] == &SystemMovePhase1);
    lok(torun_arr[6] == &SystemMovePhase2);
    lok(torun_arr[7] == &SystemMovePhase2);
    lok(torun_arr[8] == &SystemMovePhase4);
    lok(torun_arr[9] == NULL);
    lok(torun_arr[10] == NULL);
    lok(torun_arr[11] == NULL);
    lok(torun_arr[12] == NULL);
    lok(torun_arr[13] == NULL);
    lok(torun_arr[14] == NULL);
    lok(torun_arr[15] == NULL);

    struct tnecs_world *inclusive_world2 = NULL;
    tnecs_world_genesis(&inclusive_world2);
    lok(inclusive_world2 != NULL);

    TNECS_REGISTER_COMPONENT(inclusive_world2, Velocity);
    TNECS_REGISTER_COMPONENT(inclusive_world2, Position);
    TNECS_REGISTER_COMPONENT(inclusive_world2, Sprite);
    TNECS_REGISTER_COMPONENT(inclusive_world2, Unit);
    lok(strcmp(inclusive_world2->components.names[0], "NULL") == 0);
    lok(strcmp(inclusive_world2->components.names[1], "Velocity") == 0);
    lok(strcmp(inclusive_world2->components.names[2], "Position") == 0);
    lok(strcmp(inclusive_world2->components.names[3], "Sprite") == 0);
    lok(strcmp(inclusive_world2->components.names[4], "Unit") == 0);

    TNECS_REGISTER_SYSTEM_wEXCL_wPHASE(inclusive_world2, SystemMove, 0, 2, Unit); // 4X
    TNECS_REGISTER_SYSTEM_wEXCL_wPHASE(inclusive_world2, SystemMovePhase1, 0, 1, Unit, Velocity);  // 2X
    TNECS_REGISTER_SYSTEM_wEXCL_wPHASE(inclusive_world2, SystemMovePhase2, 0, 4, Unit, Position); // 2X
    TNECS_REGISTER_SYSTEM_wEXCL_wPHASE(inclusive_world2, SystemMovePhase4, 0, 3, Unit, Position,
                                       Velocity); // 1X
    lok(strcmp(inclusive_world2->systems.names[0], "NULL") == 0);
    lok(strcmp(inclusive_world2->systems.names[1], "SystemMove") == 0);
    lok(strcmp(inclusive_world2->systems.names[2], "SystemMovePhase1") == 0);
    lok(strcmp(inclusive_world2->systems.names[3], "SystemMovePhase2") == 0);
    lok(strcmp(inclusive_world2->systems.names[4], "SystemMovePhase4") == 0);

    lok(TNECS_SYSTEM_NAME2ARCHETYPE(inclusive_world2, SystemMove) == 8);
    lok(TNECS_SYSTEM_NAME2ARCHETYPE(inclusive_world2, SystemMovePhase1) == 8 + 1);
    lok(TNECS_SYSTEM_NAME2ARCHETYPE(inclusive_world2, SystemMovePhase2) == 8 + 2);
    lok(TNECS_SYSTEM_NAME2ARCHETYPE(inclusive_world2, SystemMovePhase4) == 8 + 2 + 1);

    lok(inclusive_world->bytype.num_archetype_ids[TNECS_ARCHETYPEID(inclusive_world2,
                                                            TNECS_SYSTEM_NAME2ARCHETYPE(inclusive_world2, SystemMove))] == 3);
    lok(inclusive_world->bytype.num_archetype_ids[TNECS_ARCHETYPEID(inclusive_world2,
                                                            TNECS_SYSTEM_NAME2ARCHETYPE(inclusive_world2, SystemMovePhase1))] == 1);
    lok(inclusive_world->bytype.num_archetype_ids[TNECS_ARCHETYPEID(inclusive_world2,
                                                            TNECS_SYSTEM_NAME2ARCHETYPE(inclusive_world2, SystemMovePhase2))] == 1);
    lok(inclusive_world->bytype.num_archetype_ids[TNECS_ARCHETYPEID(inclusive_world2,
                                                            TNECS_SYSTEM_NAME2ARCHETYPE(inclusive_world2, SystemMovePhase4))] == 0);

    lok(inclusive_world2->bytype.num == 8);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world2, Unit);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world2, Unit, Velocity);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world2, Unit, Position);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world2, Unit, Position, Velocity);
    lok(inclusive_world2->bytype.num == 8);
    tnecs_world_step(inclusive_world2, 1, NULL);

    lok(inclusive_world2->systems_torun.num == 9);
    torun_arr = inclusive_world2->systems_torun.arr; 
    lok(torun_arr[0] == &SystemMovePhase1);
    lok(torun_arr[1] == &SystemMovePhase1);
    lok(torun_arr[2] == &SystemMove);
    lok(torun_arr[3] == &SystemMove);
    lok(torun_arr[4] == &SystemMove);
    lok(torun_arr[5] == &SystemMove);
    lok(torun_arr[6] == &SystemMovePhase4);
    lok(torun_arr[7] == &SystemMovePhase2);
    lok(torun_arr[8] == &SystemMovePhase2);
    lok(torun_arr[9] == NULL);
    lok(torun_arr[10] == NULL);
    lok(torun_arr[11] == NULL);
    lok(torun_arr[12] == NULL);
    lok(torun_arr[13] == NULL);
    lok(torun_arr[14] == NULL);
    lok(torun_arr[15] == NULL);
    tnecs_world_destroy(&inclusive_world2);
    tnecs_world_destroy(&inclusive_world);
}

uint64_t tnecs_hash_sdbm(const char *str) {
    /* sdbm hashing algorithm by Dan Bernstein.
    * Description: This algorithm was created for sdbm (a public-domain
    * reimplementation of ndbm) database library. It was found to do
    * well in scrambling bits, causing better distribution of the
    * keys and fewer splits. It also happens to be a good general hashing
    * function with good distribution. The actual function is
    *hash(i) = hash(i - 1) * 65599 + str[i]; what is included below
    * is the faster version used in gawk. [* there is even a faster,
    * duff-device version] the magic constant 65599 was picked out of
    * thin air while experimenting with different constants, and turns
    * out to be a prime. this is one of the algorithms used in
    * berkeley db (see sleepycat) and elsewhere.
    * [1] https://stackoverflow.com/questions/7666509/hash-function-for-string
    * [2] http://www.cse.yorku.ca/~oz/hash.html */

    uint64_t hash = 0;
    uint32_t str_char;
    while ((str_char = *str++)) {
        hash = str_char + (hash << 6) + (hash << 16) - hash;
    }
    return (hash);
}

void tnecs_other_benchmarks() {
    dupprintf(globalf, "\nOther tnecs benchmarks\n");
    double t_0, t_1;

    t_0 = tnecs_get_us();
    for (size_t i = 0; i < ITERATIONS; i++) {
        TNECS_HASH("Position");
        TNECS_HASH("Unit");
    }
    t_1 = tnecs_get_us();
    dupprintf(globalf, "tnecs_hash_djb2: %d iterations \n", ITERATIONS);
    dupprintf(globalf, "%.1f [us] \n", t_1 - t_0);

    t_0 = tnecs_get_us();
    for (size_t i = 0; i < ITERATIONS; i++) {
        tnecs_hash_sdbm("Unit");
        tnecs_hash_sdbm("Position");
    }
    t_1 = tnecs_get_us();
    dupprintf(globalf, "tnecs_hash_sdbm: %d iterations \n", ITERATIONS);
    dupprintf(globalf, "%.1f [us] \n", t_1 - t_0);
}

void tnecs_test_grow() {
    struct tnecs_world *grow_world = NULL;
    tnecs_world_genesis(&grow_world);
    lok(grow_world != NULL);

    lok(grow_world != NULL);

    lok(grow_world->entities.len == TNECS_INIT_ENTITY_LEN);
    lok(grow_world->bytype.len == TNECS_INIT_SYSTEM_LEN);
    lok(grow_world->bytype.num == 1);
    lok(grow_world->systems.len == TNECS_INIT_SYSTEM_LEN);
    lok(grow_world->systems.num == 1);
    lok(grow_world->byphase.len == TNECS_INIT_PHASE_LEN);
    lok(grow_world->byphase.num== 1);
    lok(grow_world->entities_open.num == 0);
    lok(grow_world->entities_open.len == TNECS_INIT_ENTITY_LEN);

    for (size_t i = 0; i < grow_world->entities.len; i++) {
        lok(grow_world->entities.archetypes[i] == 0);
        lok(grow_world->entities.orders[i] == 0);
        lok(grow_world->entities.id[i] == 0);
    }

    for (size_t i = 0; i < grow_world->bytype.len; i++) {
        lok(grow_world->bytype.num_entities[i] == 0);
        lok(grow_world->bytype.len_entities[i] == TNECS_INIT_ENTITY_LEN);
        for (size_t j = 0; j < grow_world->bytype.len_entities[i]; j++) {
            lok(grow_world->bytype.entities[i][j] == 0);
        }
        lok(grow_world->bytype.num_components[i] == 0);
    }

    for (size_t i = 0; i < grow_world->byphase.len; i++) {
        lok(grow_world->byphase.num_systems[i] == 0);
        lok(grow_world->byphase.len_systems[i] == TNECS_INIT_PHASE_LEN);
        for (size_t j = 0; j < grow_world->byphase.len_systems[i]; j++) {
            lok(grow_world->byphase.systems[i][j] == 0);
        }
    }

    tnecs_grow_entity(grow_world);
    lok(grow_world->entities.len == TNECS_INIT_ENTITY_LEN * TNECS_ARRAY_GROWTH_FACTOR);

    for (size_t i = 0; i < grow_world->entities.len; i++) {
        lok(grow_world->entities.archetypes[i] == 0);
        lok(grow_world->entities.orders[i] == 0);
        lok(grow_world->entities.id[i] == 0);
    }

    size_t test_archetypeid = 0;
    tnecs_grow_bytype(grow_world, test_archetypeid);
    lok(grow_world->bytype.num_entities[test_archetypeid] == 0);
    lok(grow_world->bytype.len_entities[test_archetypeid] == TNECS_INIT_ENTITY_LEN *
        TNECS_ARRAY_GROWTH_FACTOR);
    for (size_t j = 0; j < grow_world->bytype.len_entities[test_archetypeid]; j++) {
        lok(grow_world->bytype.entities[test_archetypeid][j] == 0);
    }

    test_archetypeid = 1;
    tnecs_grow_bytype(grow_world, test_archetypeid);
    lok(grow_world->bytype.num_entities[test_archetypeid] == 0);
    lok(grow_world->bytype.len_entities[test_archetypeid] == TNECS_INIT_ENTITY_LEN *
        TNECS_ARRAY_GROWTH_FACTOR);
    for (size_t j = 0; j < grow_world->bytype.len_entities[test_archetypeid]; j++) {
        lok(grow_world->bytype.entities[test_archetypeid][j] == 0);
    }
    for (size_t i = (test_archetypeid + 1); i < grow_world->bytype.len; i++) {
        lok(grow_world->bytype.num_entities[i] == 0);
        lok(grow_world->bytype.len_entities[i] == TNECS_INIT_ENTITY_LEN);
        for (size_t j = 0; j < grow_world->bytype.len_entities[i]; j++) {
            lok(grow_world->bytype.entities[i][j] == 0);
        }
        lok(grow_world->bytype.num_components[i] == 0);
    }

    tnecs_grow_system(grow_world);
    lok(grow_world->systems.len == TNECS_INIT_SYSTEM_LEN * TNECS_ARRAY_GROWTH_FACTOR);
    lok(grow_world->systems.num == 1);
    tnecs_grow_archetype(grow_world);
    lok(grow_world->bytype.len == TNECS_INIT_SYSTEM_LEN * TNECS_ARRAY_GROWTH_FACTOR);
    lok(grow_world->bytype.num == 1);

    for (size_t i = TNECS_INIT_SYSTEM_LEN; i < grow_world->bytype.len; i++) {
        lok(grow_world->bytype.num_entities[i] == 0);
        lok(grow_world->bytype.len_entities[i] == TNECS_INIT_ENTITY_LEN);
        for (size_t j = 0; j < grow_world->bytype.len_entities[i]; j++) {
            lok(grow_world->bytype.entities[i][j] == 0);
        }
        lok(grow_world->bytype.num_components[i] == 0);
    }

    tnecs_grow_phase(grow_world);
    lok(grow_world->byphase.len == TNECS_INIT_PHASE_LEN * TNECS_ARRAY_GROWTH_FACTOR);

    lok(grow_world->byphase.num== 1);
    for (size_t i = TNECS_INIT_PHASE_LEN; i < grow_world->byphase.len; i++) {
        lok(grow_world->byphase.num_systems[i] == 0);
        lok(grow_world->byphase.len_systems[i] == TNECS_INIT_PHASE_LEN);
        for (size_t j = 0; j < grow_world->byphase.len_systems[i]; j++) {
            lok(grow_world->byphase.systems[i][j] == 0);
        }
    }

    tnecs_world_destroy(&grow_world);
}

void tnecs_test_chunk() {
    lok(sizeof(tnecs_chunk) == TNECS_CHUNK_BYTESIZE);
    tnecs_world *chunk_world = NULL;
    tnecs_world_genesis(&chunk_world);

    lok(TNECS_REGISTER_COMPONENT(chunk_world, Velocity));
    lok(TNECS_REGISTER_COMPONENT(chunk_world, Position));
    lok(TNECS_REGISTER_COMPONENT(chunk_world, Sprite));
    lok(TNECS_REGISTER_COMPONENT(chunk_world, Unit));
    lok(chunk_world->components.num == 5);
    tnecs_component type1, type2, type3, type4;
    type1 = TNECS_COMPONENT_NAME2TYPE(chunk_world, Velocity);
    type2 = TNECS_COMPONENT_NAME2TYPE(chunk_world, Position);
    type3 = TNECS_COMPONENT_NAME2TYPE(chunk_world, Sprite);
    type4 = TNECS_COMPONENT_NAME2TYPE(chunk_world, Unit);

    tnecs_component archetype = type1 + type2;
    tnecs_chunk chunk;
    lok(tnecs_chunk_init(&chunk, chunk_world, archetype));
    lok(chunk.num_components == 2);
    size_t *bytesizes = tnecs_chunk_mem(&chunk);
    lok(bytesizes[0] == sizeof(Velocity));
    lok(bytesizes[1] == sizeof(Velocity) + sizeof(Position));
    lok(chunk.len_entities > 0);
    lok(chunk.len_entities == TNECS_CHUNK_COMPONENTS_BYTESIZE / (sizeof(Velocity) + sizeof(Position)));

    archetype = type1 + type2 + type3 + type4;
    lok(tnecs_chunk_init(&chunk, chunk_world, archetype));
    lok(chunk.num_components == 4);
    bytesizes = tnecs_chunk_mem(&chunk);
    lok(bytesizes[0] == sizeof(Velocity));
    lok(bytesizes[1] == sizeof(Velocity) + sizeof(Position));
    lok(bytesizes[2] == sizeof(Velocity) + sizeof(Position) +sizeof(Sprite));
    lok(bytesizes[3] == sizeof(Velocity) + sizeof(Position) +sizeof(Sprite) + sizeof(Unit));
}

void tnecs_benchmarks() {
    printf("world size: %ld bytes\n", sizeof(struct tnecs_world));
    dupprintf(globalf, "\nHomemade tnecs benchmarks\n");

    double t_0;
    double t_1;

    t_0 = tnecs_get_us();
    tnecs_world *bench_world = NULL;
    tnecs_world_genesis(&bench_world);
    t_1 = tnecs_get_us();
    dupprintf(globalf, "tnecs: World Creation time \n");
    dupprintf(globalf, "%.1f [us] \n", t_1 - t_0);

    t_0 = tnecs_get_us();
    TNECS_REGISTER_COMPONENT(bench_world, Position2);
    TNECS_REGISTER_COMPONENT(bench_world, Unit2);
    t_1 = tnecs_get_us();
    dupprintf(globalf, "tnecs: Component Registration \n");
    dupprintf(globalf, "%.1f [us] \n", t_1 - t_0);

    t_0 = tnecs_get_us();
    TNECS_REGISTER_SYSTEM_wEXCL(bench_world, SystemMove2, 0, Position2, Unit2);
    t_1 = tnecs_get_us();

    dupprintf(globalf, "tnecs: System Registration \n");
    dupprintf(globalf, "%.1f [us] \n", t_1 - t_0);

    t_0 = tnecs_get_us();
    for (size_t i = 0; i < ITERATIONS; i++) {
        test_entities[i] = tnecs_entity_create(bench_world);
    }

    t_1 = tnecs_get_us();
    dupprintf(globalf, "tnecs: Entity Creation time: %d iterations \n", ITERATIONS);
    dupprintf(globalf, "%.1f [us] \n", t_1 - t_0);

    t_0 = tnecs_get_us();
    for (size_t i = 0; i < ITERATIONS; i++) {
        tnecs_entity_destroy(bench_world, test_entities[i]);
    }
    t_1 = tnecs_get_us();
    dupprintf(globalf, "tnecs: Entity Destruction time: %d iterations \n", ITERATIONS);
    dupprintf(globalf, "%.1f [us] \n", t_1 - t_0);

    for (size_t i = 0; i < ITERATIONS; i++) {
        test_entities[i] = tnecs_entity_create(bench_world);
    }

    t_0 = tnecs_get_us();
    TNECS_ADD_COMPONENT(bench_world, test_entities[1], Position2);
    TNECS_ADD_COMPONENT(bench_world, test_entities[1], Unit2);
    for (size_t i = 2; i < ITERATIONS; i++) {
        TNECS_ADD_COMPONENT(bench_world, test_entities[i], Position2, false);
        TNECS_ADD_COMPONENT(bench_world, test_entities[i], Unit2, false);
    }
    t_1 = tnecs_get_us();
    dupprintf(globalf, "tnecs: Entity Add Component time: %d iterations \n", ITERATIONS);
    dupprintf(globalf, "%.1f [us] \n", t_1 - t_0);

    t_0 = tnecs_get_us();
    for (size_t i = 0; i < ITERATIONS; i++) {
        TNECS_ENTITY_CREATE_wCOMPONENTS(bench_world, Position2);
    }
    t_1 = tnecs_get_us();
    dupprintf(globalf, "tnecs: Entity Creation wcomponent time: %d iterations \n", ITERATIONS);
    dupprintf(globalf, "%.1f [us] \n", t_1 - t_0);

    tnecs_entity tnecs_entities2[ITERATIONS];
    t_0 = tnecs_get_us();
    for (size_t i = 0; i < ITERATIONS; i++) {
        tnecs_entities2[i] = TNECS_ENTITY_CREATE_wCOMPONENTS(bench_world, Position2, Unit2);
        TNECS_DEBUG_ASSERT(bench_world->entities.id[tnecs_entities2[i]] == tnecs_entities2[i]);
    }
    TNECS_DEBUG_ASSERT(bench_world->bytype.len_entities[3] == 32768);
    TNECS_DEBUG_ASSERT(bench_world->bytype.num_entities[3] == 19999);


    t_1 = tnecs_get_us();
    dupprintf(globalf, "tnecs: Entity Creation wcomponents (2) time: %d iterations \n", ITERATIONS);
    dupprintf(globalf, "%.1f [us] \n", t_1 - t_0);

    for (size_t i = 0; i < ITERATIONS; i++) {
        TNECS_DEBUG_ASSERT(bench_world->entities.id[tnecs_entities2[i]] == tnecs_entities2[i]);
        tnecs_entity_destroy(bench_world, tnecs_entities2[i]);
        TNECS_DEBUG_ASSERT(bench_world->bytype.num_entities[3] == (19999 - i - 1));

    }

    t_0 = tnecs_get_us();
    for (size_t i = 0; i < ITERATIONS; i++) {
        TNECS_GET_COMPONENT(bench_world, test_entities[i], Position2);
        TNECS_GET_COMPONENT(bench_world, test_entities[i], Unit2);
    }
    t_1 = tnecs_get_us();
    dupprintf(globalf, "tnecs: Component Get time: %d iterations \n", ITERATIONS);
    dupprintf(globalf, "%.1f [us] \n", t_1 - t_0);

    t_0 = tnecs_get_us();
    for (size_t i = 0; i < fps_iterations; i++) {
        tnecs_world_step(bench_world, 1, NULL);
    }
    t_1 = tnecs_get_us();
    dupprintf(globalf, "tnecs: World Step time: %d iterations %d entities \n", fps_iterations,
              ITERATIONS);
    dupprintf(globalf, "%.1f [us] \n", t_1 - t_0);
    dupprintf(globalf, "%d frame %d fps \n", fps_iterations, 60);
    dupprintf(globalf, "%.1f [us] \n", fps_iterations / 60.0f * 1e6);

    t_0 = tnecs_get_us();
    tnecs_world_destroy(&bench_world);
    t_1 = tnecs_get_us();
    dupprintf(globalf, "tnecs: World Destroy time: \n", ITERATIONS);
    dupprintf(globalf, "%.1f [us] \n", t_1 - t_0);

}


void test_log2() {
    lok(log2(0.0) == -INFINITY);
    lok(log2(0.0) == -INFINITY);
    lok(log2(0) == -INFINITY);
    lok(log2(0) == -INFINITY);
    lok(log2(1.0) == 0.0);
    lok(log2(1.0) == 0);
    lok(log2(2.0) == 1.0);
    lok(log2(2.0) == 1);
}

int main() {
    globalf = fopen("tnecs_test_results.txt", "w+");
    dupprintf(globalf, "\nHello, World! I am testing tnecs.\n");
    lrun("utilities",  tnecs_test_utilities);
    lrun("log2",       test_log2);
    lrun("c_regis",    tnecs_test_component_registration);
    lrun("s_regis",    tnecs_test_system_registration);
    lrun("chunk",      tnecs_test_chunk);
    lrun("e_create",   tnecs_test_entity_creation);
    lrun("c_add",      tnecs_test_component_add);
    lrun("c_remove",   tnecs_test_component_remove);
    lrun("c_array",    tnecs_test_component_array);
    lrun("grow",       tnecs_test_grow);
    lrun("progress",   tnecs_test_world_progress);
    lresults();

    tnecs_other_benchmarks();
    tnecs_benchmarks();
    tnecs_world_destroy(&test_world);
    dupprintf(globalf, "tnecs Test End \n \n");
    fclose(globalf);
    return (0);
}
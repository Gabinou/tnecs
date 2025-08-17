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
#include "tnecs.c"

typedef unsigned long long int u64;

/********** 0.1 MICROSECOND RESOLUTION CLOCK **********/
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

/** ******************** 0.1 MICROSECOND RESOLUTION CLOCK **********************/
u64 tnecs_get_ns() {
    static u64 is_init = 0;
    #if defined(__APPLE__)
    static mach_timebase_info_data_t info;
    if (0 == is_init) {
        mach_timebase_info(&info);
        is_init = 1;
    }
    u64 now;
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
    u64 now;
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
    return (u64)((1e9 * now.QuadPart) / win_frequency.QuadPart);
    #endif
}
#ifdef MICROSECOND_CLOCK
u64 tnecs_get_us() {
    return (tnecs_get_ns() / 1e3);
}
#else
#  define FAILSAFE_CLOCK
#  define tnecs_get_us() ((clock())/CLOCKS_PER_SEC*1e6) // [us]
#  define tnecs_get_ns() ((clock())/CLOCKS_PER_SEC*1e9) // [ns]
#endif

// TODO:
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
        dupprintf(globalf,"pass:%6d\tfail:%6d\t%4dms\n",\
                  (ltests-ts)-(lfails-fs), lfails-fs,\
                  (int)((clock() - start) * 1000 / CLOCKS_PER_SEC));\
    } while (0)

/* Assert a true statement. */
#define test_true(test) do {\
        ++ltests;\
        if (!(test)) {\
            ++lfails;\
            dupprintf(globalf,"%s:%d error \n", __FILE__, __LINE__);\
        }} while (0)

#endif /*__MINCTEST_H__*/

/* duplicate printf */
void dupprintf(FILE *f, char const *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    va_start(ap, fmt);
    vfprintf(f, fmt, ap);
    va_end(ap);
}

/********************TEST COMPONENTS********************/
typedef struct Position {
    uint32_t x;
    uint32_t y;
    int *arr;
    int  arr_len;
} Position;

typedef struct Unit {
    uint16_t hp;
    uint16_t str;
} Unit;

typedef struct Velocity {
    u64 vx;
    u64 vy;
    u64 vz;
    u64 vw;
} Velocity;

typedef struct Sprite {
    uint32_t texture;
    bool isAnimated;
} Sprite;

struct Unit Unit_default = {.hp = 0, .str = 0 };

typedef struct Position2 {
    u64 x;
    u64 y;
    u64 a;
    u64 b;
    u64 c;
    u64 d;
} Position2;

typedef struct Unit2 {
    u64 hp;
    u64 str;
    u64 mag;
    u64 def;
    u64 res;
} Unit2;

void SystemMove2(struct tnecs_In *input) {
    int     Position2_ID    = 1;
    int     Unit2_ID        = 2;
    struct  Position2   *p = NULL;
    struct  Unit2       *v = NULL;

    p = TNECS_C_ARRAY(input, Position2_ID);
    v = TNECS_C_ARRAY(input, Unit2_ID);

    for (int i = 0; i < input->num_Es; i++) {
        p[i].x += v[i].hp;
        p[i].y += v[i].str;
    }
}

void SystemMovePhase1(struct tnecs_In *input) {
    for (int ent = 0; ent < input->num_Es; ent++) {
        tnecs_E current_ent = input->world->byA.Es[input->E_A_id][ent];
        test_true(current_ent);
        test_true(input->world->Es.id[input->world->byA.Es[input->E_A_id][ent]]
            == input->world->byA.Es[input->E_A_id][ent]);
        test_true(input->E_A_id == tnecs_A_id(input->world,
                                                             input->world->Es.As[current_ent]));
    }
}

void SystemMovePhase4(struct tnecs_In *input) {
    for (int ent = 0; ent < input->num_Es; ent++) {
        tnecs_E current_ent = input->world->byA.Es[input->E_A_id][ent];
        test_true(current_ent);
        test_true(input->world->Es.id[input->world->byA.Es[input->E_A_id][ent]]
            == input->world->byA.Es[input->E_A_id][ent]);
        test_true(input->E_A_id == tnecs_A_id(input->world,
                                                             input->world->Es.As[current_ent]));
    }
}

void SystemMovePhase2(struct tnecs_In *input) {
    for (int ent = 0; ent < input->num_Es; ent++) {
        tnecs_E current_ent = input->world->byA.Es[input->E_A_id][ent];
        test_true(current_ent);
        test_true(input->world->Es.id[input->world->byA.Es[input->E_A_id][ent]]
            == input->world->byA.Es[input->E_A_id][ent]);
        test_true(input->E_A_id == tnecs_A_id(input->world,
                                                             input->world->Es.As[current_ent]));
    }
}

/*****************************TEST CONSTANTS***************************/
#define ITERATIONS 10000
size_t fps_iterations = 10;

/*******************************TEST SYSTEMS***************************/
tnecs_E test_Es[ITERATIONS];
tnecs_E        *Cs_list;
struct Position     *temp_position;
struct Unit         *temp_unit;
struct Sprite       *temp_sprite;
struct tnecs_W  *test_world;

void SystemMove(struct tnecs_In *input) {
    // printf("SystemMove\n");
    int Position_ID = 1;
    int Velocity_ID = 2;
    struct Position *p = NULL;
    struct Velocity *v = NULL;
    p = TNECS_C_ARRAY(input, Position_ID);
    v = TNECS_C_ARRAY(input, Velocity_ID);

    for (int ent = 0; ent < input->num_Es; ent++) {
        tnecs_E current_ent = input->world->byA.Es[input->E_A_id][ent];
        test_true(current_ent);
        test_true(input->world->Es.id[input->world->byA.Es[input->E_A_id][ent]]
            == input->world->byA.Es[input->E_A_id][ent]);
        test_true(input->E_A_id == tnecs_A_id(input->world,
                                                             input->world->Es.As[current_ent]));

        p[ent].x = p[ent].x + v[ent].vx;
        p[ent].y = p[ent].y + v[ent].vy;
    }
}

void SystemMoveDoNothing(struct tnecs_In *input) {
    int doesnotexist_ID = 8;
    void *ptr = NULL;
    ptr = TNECS_C_ARRAY(input, doesnotexist_ID);
    test_true(ptr == NULL);
}

/*******************************ACTUAL TESTS***************************/
const tnecs_Pi pipe0 = 0;

void tnecs_test_utilities() {
    test_true(TNECS_C_T2ID(1 << 0) == 1);
    test_true(TNECS_C_T2ID(1 << 1) == 2);
    test_true(TNECS_C_T2ID(1 << 2) == 3);
    test_true(TNECS_C_T2ID(1 << 3) == 4);
    test_true(TNECS_C_T2ID(1 << 4) == 5);
    test_true(TNECS_C_T2ID(1 << 5) == 6);
    test_true(TNECS_C_T2ID(1 << 6) == 7);
    test_true(TNECS_C_T2ID(1 << 7) == 8);
    test_true(TNECS_C_T2ID(1 << 8) == 9);
    test_true(TNECS_C_T2ID(1 << 9) == 10);
    test_true(TNECS_C_T2ID(1 << 10) == 11);
    test_true(TNECS_C_T2ID(1 << 11) == 12);
    test_true(TNECS_C_T2ID(1 << 12) == 13);
    test_true(TNECS_C_T2ID(1 << 13) == 14);
    test_true(TNECS_C_T2ID(1 << 14) == 15);
    test_true(TNECS_C_T2ID(1 << 15) == 16);
    test_true(TNECS_C_T2ID(1 << 16) == 17);
    test_true(TNECS_C_T2ID(1 << 17) == 18);
    test_true(TNECS_C_T2ID(1 << 18) == 19);
    test_true(TNECS_C_T2ID(1 << 19) == 20);
    test_true(TNECS_C_T2ID(1 << 20) == 21);
    test_true(TNECS_C_T2ID(1 << 21) == 22);
    test_true(TNECS_C_T2ID(1 << 22) == 23);
    test_true(TNECS_C_T2ID(1 << 23) == 24);
    test_true(TNECS_C_T2ID(1 << 24) == 25);
    test_true(TNECS_C_T2ID(1 << 25) == 26);

    test_true(TNECS_C_ID2T(1) == 1);
    test_true(TNECS_C_ID2T(2) == 2);
    test_true(TNECS_C_ID2T(3) == 4);
    test_true(TNECS_C_ID2T(4) == 8);
    test_true(TNECS_C_ID2T(5) == 16);
    test_true(TNECS_C_ID2T(6) == 32);

    test_true(TNECS_A_IS_subA(4, (4 + 8 + 16)));
    test_true(!TNECS_A_IS_subA(2, (4 + 8 + 16)));

    test_true(setBits_KnR(1) == 1);
    test_true(setBits_KnR(2) == 1);
    test_true(setBits_KnR(3) == 2);
    test_true(setBits_KnR(4) == 1);
    test_true(setBits_KnR(5) == 2);
    test_true(setBits_KnR(6) == 2);
    test_true(setBits_KnR(7) == 3);
}

void tnecs_test_C_registration() {
    tnecs_genesis(&test_world);
    test_true(test_world != NULL);
    assert(sizeof(Position) > 0);
    TNECS_REGISTER_C(test_world, Position, NULL, NULL);
    size_t temp_comp_flag       = 1;
    size_t temp_comp_id         = 1;
    size_t temp_comp_order      = 0;
    size_t temp_A_id    = 1;
    size_t temp_archetype       = 1;
    test_true(TNECS_C_ID2T(temp_comp_id) == temp_archetype);
    assert(test_world->byA.Cs_id != NULL);
    assert(test_world->byA.Cs_id[temp_comp_id] != NULL);
    test_true(test_world->byA.Cs_id[temp_A_id][temp_comp_order] == temp_comp_id);
    test_true(test_world->byA.A[0] == 0);
    test_true(test_world->byA.A[1] == (TNECS_NULLSHIFT << 0));
    test_true(test_world->byA.A[1] == temp_comp_flag);
    test_true(test_world->Cs.num == 2);

    TNECS_REGISTER_C(test_world, Unit, NULL, NULL);
    temp_comp_flag = 2;
    temp_comp_id = 2;
    temp_comp_order = 0;
    temp_A_id = 2;
    temp_archetype = 2;
    test_true(TNECS_C_ID2T(temp_comp_id) == temp_archetype);
    test_true(test_world->byA.Cs_id[temp_comp_id][temp_comp_order] == temp_comp_id);
    test_true(test_world->byA.A[0] == 0);
    test_true(test_world->byA.A[1] == (TNECS_NULLSHIFT << 0));
    test_true(test_world->byA.A[2] == (TNECS_NULLSHIFT << 1));
    test_true(test_world->byA.A[2] == temp_comp_flag);
    test_true(test_world->Cs.num == 3);

    TNECS_REGISTER_C(test_world, Sprite, NULL, NULL);
    temp_comp_flag = 4;
    temp_comp_id = 3;
    temp_comp_order = 0;
    temp_A_id = 3;
    temp_archetype = 4;
    test_true(TNECS_C_ID2T(temp_comp_id) == temp_archetype);
    test_true(test_world->byA.A[0] == 0);
    test_true(test_world->byA.A[1] == (TNECS_NULLSHIFT << 0));
    test_true(test_world->byA.A[2] == (TNECS_NULLSHIFT << 1));
    test_true(test_world->byA.A[3] == (TNECS_NULLSHIFT << 2));
    test_true(test_world->byA.A[3] == temp_comp_flag);
    test_true(test_world->Cs.num == 4);

    TNECS_REGISTER_C(test_world, Velocity, NULL, NULL);
    temp_comp_flag = 8;
    temp_comp_id = 4;
    temp_comp_order = 0;
    temp_A_id = 4;
    temp_archetype = 8;
    test_true(TNECS_C_ID2T(temp_comp_id) == temp_archetype);
    test_true(test_world->byA.Cs_id[temp_comp_id][temp_comp_order] == temp_comp_id);

    test_true(test_world->byA.A[0] == 0);
    test_true(test_world->byA.A[1] == (TNECS_NULLSHIFT << 0));
    test_true(test_world->byA.A[2] == (TNECS_NULLSHIFT << 1));
    test_true(test_world->byA.A[3] == (TNECS_NULLSHIFT << 2));
    test_true(test_world->byA.A[4] == (TNECS_NULLSHIFT << 3));
    test_true(test_world->byA.A[4] == temp_comp_flag);
    test_true(test_world->Cs.num == 5);

    int Position_ID = 1;
    int Velocity_ID = 2;
    int Unit_ID     = 3;

    test_true(TNECS_C_IDS2A(1, 2, 3) == (1 + 2 + 4));
    test_true(TNECS_C_IDS2A(Position_ID, Unit_ID, Velocity_ID) == (1 + 2 + 4));
}

void tnecs_test_system_registration() {
    int Position_ID = 1;
    int Velocity_ID = 2;

    TNECS_REGISTER_S(test_world, SystemMove, pipe0, 0, 1, Position_ID, Velocity_ID);
    size_t temp_A_id    = 5;

    test_true(test_world->byA.Cs_id[temp_A_id][0] == Position_ID);
    test_true(test_world->byA.Cs_id[temp_A_id][1] == Velocity_ID);
}

void tnecs_test_E_creation() {
    // dupprintf(globalf, "tnecs_test_E_creation \n");

    int Position_ID = 1;
    int Unit_ID     = 3;
    int Sprite_ID   = 4;

    test_true(test_world->Es.num == TNECS_NULLSHIFT);
    TNECS_REGISTER_C(test_world, Sprite, NULL, NULL);
    tnecs_E Silou = tnecs_E_create(test_world);
    test_true(Silou == TNECS_NULLSHIFT);
    test_true(test_world->Es.num == (TNECS_NULLSHIFT + 1));
    tnecs_E Pirou = tnecs_E_create(test_world);
    test_true(Pirou == (TNECS_NULLSHIFT + 1));
    test_true(test_world->Es.num == (TNECS_NULLSHIFT + 2));
    test_true(Silou != Pirou);
    tnecs_E Perignon = TNECS_E_CREATE_wC(test_world, Position_ID, Unit_ID);
    test_true(Perignon != TNECS_NULL);

    temp_position = tnecs_get_C(test_world, Perignon, Position_ID);
    if (temp_position == NULL) {
        test_true(false);
    } else {
        test_true(temp_position->x == 0);
        test_true(temp_position->y == 0);
        temp_position->x = 3;
        temp_position->y = 6;
    }

    test_true(test_world->Es.As[Perignon] == TNECS_C_ID2T(Position_ID) + TNECS_C_ID2T(Unit_ID));

    temp_position = tnecs_get_C(test_world, Perignon, Sprite_ID);
    test_true(temp_position == NULL);
    temp_unit = tnecs_get_C(test_world, Perignon, Unit_ID);
    if (temp_unit == NULL) {
        test_true(false);
    } else {
        test_true(temp_unit->hp  == 0);
        test_true(temp_unit->str == 0);
    }

    #ifdef NDEBUG
    tnecs_E_destroy(test_world, Silou);
    #else
    assert(tnecs_E_destroy(test_world, Silou));
    #endif
    tnecs_E *open_arr = test_world->Es.open.arr;
    test_true(test_world->Es.open.num == 0);
    test_true(!test_world->Es.id[Silou]);
    tnecs_E_open_find(test_world);
    test_true(test_world->Es.open.num == 1);
    test_true(open_arr[0] != TNECS_NULL);
    test_true(open_arr[0] == Silou);
    tnecs_E_create(test_world);
    test_true(test_world->Es.id[Silou]);

    for (int i = 0; i < 100; i++) {
        if (tnecs_E_create(test_world) <= TNECS_NULL) {
            assert(false);
            test_true(false);
        }
    }

    test_true(test_world->Es.num == 104);
    test_true(tnecs_E_create(test_world));
    test_true(test_world->Es.num == 105);

    tnecs_W *test_world3 = NULL;
    tnecs_genesis(&test_world3);
    tnecs_finale(&test_world3);

    // MORE TESTS FOR COVERAGE
    tnecs_W *test_world2 = NULL;
    tnecs_genesis(&test_world2);

    test_world2->byA.num = TNECS_S_0LEN;
    TNECS_REGISTER_C(test_world2, Position2, NULL, NULL);
    test_true(test_world2->Cs.num == 2);

    // Coverage for if in TNECS_REGISTER_S
    tnecs_finale(&test_world2);
    test_true(test_world2 == NULL);
    tnecs_genesis(&test_world2);

    int Position2_ID = 1;
    int Unit2_ID     = 2;

    TNECS_REGISTER_C(test_world2, Position2, NULL, NULL);
    test_world2->Pis.byPh[0].num_Ss[0] = TNECS_Ph_0LEN;
    TNECS_REGISTER_S(test_world2, SystemMovePhase1, pipe0, 0, 0, Position2_ID);
    tnecs_finale(&test_world2);

    // Coverage for "for" in tnecs_C_del
    tnecs_genesis(&test_world2);
    TNECS_REGISTER_C(test_world2, Unit2_ID, NULL, NULL);
    TNECS_REGISTER_C(test_world2, Position2_ID, NULL, NULL);
    tnecs_E Erwin = TNECS_E_CREATE_wC(test_world2, Position2_ID, Unit2_ID);
    tnecs_C_del(test_world2, Erwin, (1 + 2));
    tnecs_finale(&test_world2);
}

void tnecs_test_C_add() {
    int Position_ID = 1;
    int Unit_ID     = 3;
    int Sprite_ID   = 4;

    tnecs_E Silou = tnecs_E_create(test_world);
    TNECS_ADD_C(test_world, Silou, Position_ID);
    test_true((test_world->Es.As[Silou] & TNECS_C_ID2T(Unit_ID)) == 0);
    TNECS_ADD_C(test_world, Silou, Unit_ID);
    test_true((test_world->Es.As[Silou] & TNECS_C_ID2T(Position_ID)) > 0);
    test_true((test_world->Es.As[Silou] & TNECS_C_ID2T(Unit_ID)) > 0);
    test_true((test_world->Es.As[Silou] & TNECS_C_ID2T(Sprite_ID)) == 0);

    tnecs_E Pirou = tnecs_E_create(test_world);
    TNECS_ADD_C(test_world, Pirou, Position_ID);
    test_true((test_world->Es.As[Pirou] & TNECS_C_ID2T(Position_ID)) > 0);
    test_true((test_world->Es.As[Pirou] & TNECS_C_ID2T(Unit_ID)) == 0);
    TNECS_ADD_C(test_world, Pirou, Unit_ID);
    test_true((test_world->Es.As[Pirou] & TNECS_C_ID2T(Unit_ID)) > 0);
    test_true((test_world->Es.As[Pirou] & TNECS_C_ID2T(Position_ID)) > 0);
    test_true((test_world->Es.As[Pirou] & TNECS_C_ID2T(Sprite_ID)) == 0);

    tnecs_E Chasse = tnecs_E_create(test_world);
    TNECS_ADD_Cs(test_world, Chasse, 1, Sprite_ID, Position_ID);
    test_true((test_world->Es.As[Chasse] & TNECS_C_ID2T(Unit_ID)) == 0);
    test_true((test_world->Es.As[Chasse] & TNECS_C_ID2T(Sprite_ID)) > 0);
    test_true((test_world->Es.As[Chasse] & TNECS_C_ID2T(Position_ID)) > 0);

    temp_position = tnecs_get_C(test_world, Silou, Position_ID);
    test_true(temp_position != NULL);
    test_true(temp_position->x == 0);
    test_true(temp_position->y == 0);
    temp_position->x = 1;
    temp_position->y = 2;
    test_true(temp_position->x == 1);
    test_true(temp_position->y == 2);
    temp_position = tnecs_get_C(test_world, Silou, Position_ID);
    test_true(temp_position->x == 1);
    test_true(temp_position->y == 2);

    temp_unit = tnecs_get_C(test_world, Silou, Unit_ID);
    test_true(temp_unit != NULL);
    test_true(temp_unit->hp == 0);
    test_true(temp_unit->str == 0);
    temp_unit->hp = 3;
    temp_unit->str = 4;
    test_true(temp_unit->hp == 3);
    test_true(temp_unit->str == 4);
    temp_unit = tnecs_get_C(test_world, Silou, Unit_ID);
    test_true(temp_unit->hp == 3);
    test_true(temp_unit->str == 4);
    temp_position = tnecs_get_C(test_world, Silou, Position_ID);
    test_true(temp_position->x == 1);
    test_true(temp_position->y == 2);

    temp_position = tnecs_get_C(test_world, Pirou, Position_ID);
    test_true(temp_position->x == 0);
    test_true(temp_position->y == 0);
    temp_position->x = 5;
    temp_position->y = 6;
    test_true(temp_position->x == 5);
    test_true(temp_position->y == 6);
    temp_position = tnecs_get_C(test_world, Pirou, Position_ID);
    test_true(temp_position->x == 5);
    test_true(temp_position->y == 6);

    temp_unit = tnecs_get_C(test_world, Pirou, Unit_ID);
    test_true(temp_unit->hp == 0);
    test_true(temp_unit->str == 0);
    temp_unit->hp = 7;
    temp_unit->str = 8;
    test_true(temp_unit->hp == 7);
    test_true(temp_unit->str == 8);
    temp_position = tnecs_get_C(test_world, Pirou, Position_ID);
    test_true(temp_position->x == 5);
    test_true(temp_position->y == 6);
    temp_unit = tnecs_get_C(test_world, Pirou, Unit_ID);
    test_true(temp_unit->hp == 7);
    test_true(temp_unit->str == 8);
}

void tnecs_test_E_destroy() {
    /* Check that deleting an entity does NOT change other entity data. */
    int Position_ID = 1;
    int Unit_ID     = 3;

    int Silou_x     = 10;
    int Pirou_x     = 20;
    int Chasse_x    = 30;
    int Michael_x   = 40;

    tnecs_E Silou      = TNECS_E_CREATE_wC(test_world, Position_ID, Unit_ID);
    tnecs_E Pirou      = TNECS_E_CREATE_wC(test_world, Position_ID, Unit_ID);
    tnecs_E Chasse     = TNECS_E_CREATE_wC(test_world, Position_ID, Unit_ID);
    tnecs_E Michael    = TNECS_E_CREATE_wC(test_world, Position_ID, Unit_ID);

    test_true(test_world->Es.Os[Silou]      == 1);
    test_true(test_world->Es.Os[Pirou]      == 2);
    test_true(test_world->Es.Os[Chasse]     == 3);
    test_true(test_world->Es.Os[Michael]    == 4);

    struct Position *position = tnecs_get_C(test_world, Silou, Position_ID);
    position->x = Silou_x;
    position = tnecs_get_C(test_world, Pirou, Position_ID);
    position->x = Pirou_x;
    position = tnecs_get_C(test_world, Chasse, Position_ID);
    position->x = Chasse_x;
    position = tnecs_get_C(test_world, Michael, Position_ID);
    position->x = Michael_x;

    tnecs_E_destroy(test_world, Pirou);

    test_true(test_world->Es.Os[Silou]      == 1);
    test_true(test_world->Es.Os[Michael]    == 2);
    test_true(test_world->Es.Os[Chasse]     == 3);

    position = tnecs_get_C(test_world, Pirou, Position_ID);
    test_true(position == NULL);
    position = tnecs_get_C(test_world, Silou, Position_ID);
    test_true(position->x == Silou_x);
    position = tnecs_get_C(test_world, Chasse, Position_ID);
    test_true(position->x == Chasse_x);
    position = tnecs_get_C(test_world, Michael, Position_ID);
    test_true(position->x == Michael_x);

    tnecs_E_destroy(test_world, Michael);

    test_true(test_world->Es.Os[Silou]  == 1);
    test_true(test_world->Es.Os[Chasse] == 2);

    position = tnecs_get_C(test_world, Pirou, Position_ID);
    test_true(position == NULL);
    position = tnecs_get_C(test_world, Michael, Position_ID);
    test_true(position == NULL);
    position = tnecs_get_C(test_world, Silou, Position_ID);
    test_true(position->x == Silou_x);
    position = tnecs_get_C(test_world, Chasse, Position_ID);
    test_true(position->x == Chasse_x);

}

void tnecs_test_C_remove() {
    int Position_ID = 1;
    int Velocity_ID = 2;
    int Unit_ID     = 3;

    tnecs_E Silou = tnecs_E_create(test_world);
    TNECS_ADD_C(test_world, Silou, Position_ID);
    test_true(TNECS_E_HAS_C(test_world, Silou, Position_ID));
    TNECS_RM_C(test_world, Silou, Position_ID);
    test_true(!TNECS_E_HAS_C(test_world, Silou, Position_ID));

    tnecs_E Perignon = TNECS_E_CREATE_wC(test_world, Position_ID, Velocity_ID);
    test_true(TNECS_E_HAS_C(test_world, Perignon, Position_ID));
    test_true(TNECS_E_HAS_C(test_world, Perignon, Velocity_ID));
    test_true(!TNECS_E_HAS_C(test_world, Perignon, Unit_ID));

    TNECS_RM_C(test_world, Perignon, Velocity_ID);
    test_true(!TNECS_E_HAS_C(test_world, Perignon, Velocity_ID));
    test_true(TNECS_E_HAS_C(test_world, Perignon, Position_ID));
    test_true(!TNECS_E_HAS_C(test_world, Perignon, Unit_ID));

    tnecs_E Pirou = TNECS_E_CREATE_wC(test_world, Position_ID, Velocity_ID, Unit_ID);
    test_true(TNECS_E_HAS_C(test_world, Pirou, Position_ID));
    test_true(TNECS_E_HAS_C(test_world, Pirou, Velocity_ID));
    test_true(TNECS_E_HAS_C(test_world, Pirou, Unit_ID));

    TNECS_RM_C(test_world, Pirou, Position_ID, Velocity_ID);
    test_true(TNECS_E_HAS_C(test_world, Pirou, Unit_ID));
    test_true(!TNECS_E_HAS_C(test_world, Pirou, Position_ID));
    test_true(!TNECS_E_HAS_C(test_world, Pirou, Velocity_ID));
}

void tnecs_test_C_array() {
    int Position_ID = 1;
    int Velocity_ID = 2;
    int Unit_ID     = 3;
    int Sprite_ID   = 4;

    tnecs_W *arr_world = NULL;
    tnecs_genesis(&arr_world);
    
    test_true(TNECS_REGISTER_C(arr_world, Position_ID, NULL, NULL));
    test_true(TNECS_REGISTER_C(arr_world, Velocity_ID, NULL, NULL));
    test_true(TNECS_REGISTER_C(arr_world, Sprite_ID, NULL, NULL));
    test_true(TNECS_REGISTER_C(arr_world, Unit_ID, NULL, NULL));
    TNECS_REGISTER_S(arr_world, SystemMoveDoNothing,  pipe0, 0, 0, Unit_ID); // 4X
    TNECS_REGISTER_S(arr_world, SystemMovePhase1,     pipe0, 0, 0, Unit_ID, Velocity_ID);  // 2X
    TNECS_REGISTER_S(arr_world, SystemMovePhase2,     pipe0, 0, 0, Unit_ID, Position_ID); // 2X
    TNECS_REGISTER_S(arr_world, SystemMovePhase4,     pipe0, 0, 0, Unit_ID, Position_ID, Velocity_ID); // 1X

    tnecs_E temp_ent = TNECS_E_CREATE_wC(arr_world, Unit_ID);
    TNECS_E_CREATE_wC(arr_world, Unit_ID, Velocity_ID);
    TNECS_E_CREATE_wC(arr_world, Unit_ID, Velocity_ID);
    TNECS_E_CREATE_wC(arr_world, Unit_ID, Velocity_ID);
    TNECS_E_CREATE_wC(arr_world, Unit_ID, Position_ID);
    TNECS_E_CREATE_wC(arr_world, Unit_ID, Position_ID);
    TNECS_E_CREATE_wC(arr_world, Unit_ID, Position_ID);
    TNECS_E_CREATE_wC(arr_world, Unit_ID, Position_ID);
    TNECS_E_CREATE_wC(arr_world, Unit_ID, Position_ID, Velocity_ID);
    TNECS_E_CREATE_wC(arr_world, Unit_ID, Position_ID, Velocity_ID);

    size_t temp_archetypeid     = TNECS_C_IDS2AID(arr_world, Unit_ID, Position_ID);

    size_t temp_C_O = tnecs_C_O_byAid(arr_world, Position_ID, temp_archetypeid);
    assert(temp_archetypeid > TNECS_NULL);
    assert(arr_world->byA.Cs != NULL);
    assert(arr_world->byA.Cs[temp_archetypeid] != NULL);

    test_true(arr_world->byA.Cs[temp_archetypeid][temp_C_O].num == 4);
    temp_C_O = tnecs_C_O_byAid(arr_world, Unit_ID, temp_archetypeid);
    test_true(arr_world->byA.Cs[temp_archetypeid][temp_C_O].num == 4);

    temp_archetypeid = TNECS_C_IDS2AID(arr_world, Unit_ID, Velocity_ID);
    temp_C_O = tnecs_C_O_byAid(arr_world, Unit_ID, temp_archetypeid);
    test_true(arr_world->byA.Cs[temp_archetypeid][temp_C_O].num == 3);
    temp_C_O = tnecs_C_O_byAid(arr_world, Velocity_ID, temp_archetypeid);
    test_true(arr_world->byA.Cs[temp_archetypeid][temp_C_O].num == 3);

    temp_archetypeid = TNECS_C_IDS2AID(arr_world, Unit_ID, Velocity_ID, Position_ID);
    temp_C_O = tnecs_C_O_byAid(arr_world, Unit_ID, temp_archetypeid);
    test_true(arr_world->byA.Cs[temp_archetypeid][temp_C_O].num == 2);
    temp_C_O = tnecs_C_O_byAid(arr_world, Position_ID, temp_archetypeid);
    test_true(arr_world->byA.Cs[temp_archetypeid][temp_C_O].num == 2);
    temp_C_O = tnecs_C_O_byAid(arr_world, Velocity_ID, temp_archetypeid);
    test_true(arr_world->byA.Cs[temp_archetypeid][temp_C_O].num == 2);

    size_t old_E_order = arr_world->Es.Os[temp_ent];
    test_true(old_E_order == 0);
    test_true(arr_world->Es.As[temp_ent] == TNECS_C_IDS2A(Unit_ID));
    size_t old_archetypeid = TNECS_C_IDS2AID(arr_world, Unit_ID);
    test_true(arr_world->byA.num_Es[old_archetypeid] == 1);
    test_true(old_archetypeid == Unit_ID);
    size_t old_C_O = tnecs_C_O_byAid(arr_world, Unit_ID, old_archetypeid);

    test_true(old_C_O < TNECS_C_CAP);
    test_true(old_C_O == 0);

    test_true(arr_world->byA.Cs[old_archetypeid][old_C_O].num == 1);

    struct Unit     *temp_unit  = tnecs_get_C(arr_world, temp_ent, Unit_ID);
    struct Position *temp_pos   = tnecs_get_C(arr_world, temp_ent, Position_ID);
    test_true(temp_pos == NULL);
    test_true(temp_unit->hp   == 0);
    test_true(temp_unit->str  == 0);
    temp_unit->hp   = 10;
    temp_unit->str  = 12;
    temp_unit = tnecs_get_C(arr_world, temp_ent + 1, Unit_ID);
    
    for (int i = 0; i < 10; i++) {
        if (tnecs_E_create(arr_world) <= TNECS_NULL) {
            assert(false);
            test_true(false);
        }
    }

    temp_unit = tnecs_get_C(arr_world, temp_ent, Unit_ID);
    test_true(temp_unit->hp   == 10);
    test_true(temp_unit->str  == 12);

    size_t new_archetypeid = TNECS_C_IDS2AID(arr_world, Unit_ID, Position_ID);
    test_true(arr_world->byA.num_Es[new_archetypeid] == 4);
    TNECS_ADD_C(arr_world, temp_ent, Position_ID);
    
    temp_unit = tnecs_get_C(arr_world, temp_ent, Unit_ID);
    test_true(temp_unit->hp   == 10);
    test_true(temp_unit->str  == 12);

    test_true(arr_world->byA.num_Es[old_archetypeid] == 0);
    test_true(arr_world->Es.As[temp_ent] == TNECS_C_IDS2A(Unit_ID,
            Position_ID));
    test_true(arr_world->byA.num_Es[new_archetypeid] == 5);
    temp_unit = tnecs_get_C(arr_world, temp_ent, Unit_ID);
    test_true(temp_unit->hp   == 10);
    test_true(temp_unit->str  == 12);
    size_t new_E_order = arr_world->Es.Os[temp_ent];
    test_true(new_E_order == 4);
    test_true(new_E_order != old_E_order);

    temp_pos = tnecs_get_C(arr_world, temp_ent, Position_ID);
    temp_unit->hp++;
    temp_unit->str++;
    test_true(temp_unit->hp   == 11);
    test_true(temp_unit->str  == 13);
    test_true(temp_pos->x     == 0);
    test_true(temp_pos->y     == 0);

    tnecs_finale(&arr_world);
}

void tnecs_test_world_progress() {
    int Position_ID = 1;
    int Velocity_ID = 2;
    int Unit_ID     = 3;

    struct tnecs_W *inclusive_world = NULL;
    tnecs_genesis(&inclusive_world);
    test_true(inclusive_world != NULL);

    struct tnecs_W *inclusive_world2 = NULL;
    tnecs_genesis(&inclusive_world2);
    test_true(inclusive_world2 != NULL);

    struct Velocity *temp_velocity;
    tnecs_E Perignon = TNECS_E_CREATE_wC(test_world, Position_ID, Velocity_ID);
    test_true(TNECS_E_HAS_C(test_world, Perignon, Position_ID));
    test_true(TNECS_E_HAS_C(test_world, Perignon, Velocity_ID));
    test_true(!TNECS_E_HAS_C(test_world, Perignon, Unit_ID));

    test_true(test_world->Es.As[Perignon] == (1 + 2));
    test_true(test_world->byA.num_Es[tnecs_A_id(test_world, 1 + 2)] == 1);

    temp_position = tnecs_get_C(test_world, Perignon, Position_ID);
    temp_velocity = tnecs_get_C(test_world, Perignon, Velocity_ID);
    temp_position->x = 100;
    temp_position->y = 200;

    tnecs_register_Ph(test_world, pipe0);
    tnecs_register_Ph(test_world, pipe0);
    tnecs_register_Ph(test_world, pipe0);
    tnecs_register_Ph(test_world, pipe0);
    TNECS_REGISTER_S(test_world, SystemMovePhase1, pipe0, 1, 1, Position_ID);
    TNECS_REGISTER_S(test_world, SystemMovePhase2, pipe0, 2, 1, Velocity_ID);
    TNECS_REGISTER_S(test_world, SystemMovePhase2, pipe0, 1, 1, Unit_ID);
    TNECS_REGISTER_S(test_world, SystemMovePhase4, pipe0, 4, 1, Velocity_ID);

    test_true(test_world->Pis.byPh[0].num == 5);
    test_true(test_world->Pis.byPh[0].Ss[1][0] == &SystemMovePhase1);
    test_true(test_world->Pis.byPh[0].Ss[1][1] == &SystemMovePhase2);
    test_true(test_world->Pis.byPh[0].Ss[2][0] == &SystemMovePhase2);
    test_true(test_world->Pis.byPh[0].Ss[4][0] == &SystemMovePhase4);

    temp_velocity->vx = 1;
    temp_velocity->vy = 2;
    tnecs_step(test_world, 1, NULL);
    temp_position = tnecs_get_C(test_world, Perignon, Position_ID);
    temp_velocity = tnecs_get_C(test_world, Perignon, Velocity_ID);
    test_true(test_world->Ss.to_run.num == 5);
    tnecs_S_f *torun_arr = test_world->Ss.to_run.arr;
    test_true(torun_arr[0] == &SystemMove);
    test_true(torun_arr[1] == &SystemMovePhase1);
    test_true(torun_arr[2] == &SystemMovePhase2);
    test_true(torun_arr[3] == &SystemMovePhase2);
    test_true(torun_arr[4] == &SystemMovePhase4);
    test_true(torun_arr[0] != NULL);
    test_true(torun_arr[1] != NULL);
    test_true(torun_arr[2] != NULL);
    test_true(torun_arr[3] != NULL);
    test_true(torun_arr[4] != NULL);
    tnecs_step(test_world, 1, NULL);
    temp_position = tnecs_get_C(test_world, Perignon, Position_ID);
    temp_velocity = tnecs_get_C(test_world, Perignon, Velocity_ID);
    test_true(temp_velocity->vx == 1);
    test_true(temp_velocity->vy == 2);
    test_true(test_world->Ss.to_run.num == 5);
    torun_arr = test_world->Ss.to_run.arr;

    test_true(torun_arr[0] == &SystemMove);
    test_true(torun_arr[1] == &SystemMovePhase1);
    test_true(torun_arr[2] == &SystemMovePhase2);
    test_true(torun_arr[3] == &SystemMovePhase2);
    test_true(torun_arr[4] == &SystemMovePhase4);
    test_true(torun_arr[0] != NULL);
    test_true(torun_arr[1] != NULL);
    test_true(torun_arr[2] != NULL);
    test_true(torun_arr[3] != NULL);
    test_true(torun_arr[4] != NULL);
    test_true(temp_velocity->vx == 1);
    test_true(temp_velocity->vy == 2);
    tnecs_step(test_world, 0, NULL);

    test_true(test_world->Es.As[Perignon] == (1 + 2));
    test_true(test_world->byA.num_Es[tnecs_A_id(test_world, 1 + 2)] == 1);
    tnecs_E_destroy(test_world, Perignon);

    tnecs_grow_Ph(test_world, 0);
    tnecs_grow_S(test_world);
    tnecs_grow_A(test_world);

    TNECS_REGISTER_C(inclusive_world, Position, NULL, NULL);
    TNECS_REGISTER_C(inclusive_world, Velocity, NULL, NULL);
    TNECS_REGISTER_C(inclusive_world, Sprite, NULL, NULL);
    TNECS_REGISTER_C(inclusive_world, Unit, NULL, NULL);
    TNECS_REGISTER_S(inclusive_world, SystemMoveDoNothing, pipe0, 0, 0, Unit_ID); // 4X
    TNECS_REGISTER_S(inclusive_world, SystemMovePhase1,    pipe0, 0, 0, Unit_ID, Velocity_ID);  // 2X
    TNECS_REGISTER_S(inclusive_world, SystemMovePhase2,    pipe0, 0, 0, Unit_ID, Position_ID); // 2X
    TNECS_REGISTER_S(inclusive_world, SystemMovePhase4,    pipe0, 0, 0, Unit_ID, Position_ID, Velocity_ID); // 1X

    int SystemMove_ID       = 1;
    int SystemMovePhase1_ID = 2;
    int SystemMovePhase2_ID = 3;
    int SystemMovePhase4_ID = 4;

    test_true(TNECS_S_ID2A(inclusive_world, SystemMove_ID) == 4);
    test_true(TNECS_S_ID2A(inclusive_world, SystemMovePhase1_ID) == 4 + 2);
    test_true(TNECS_S_ID2A(inclusive_world, SystemMovePhase2_ID) == 4 + 1);
    test_true(TNECS_S_ID2A(inclusive_world, SystemMovePhase4_ID) == 4 + 2 + 1);

    test_true(inclusive_world->byA.num_A_ids[tnecs_A_id(inclusive_world,
                                                            TNECS_S_ID2A(inclusive_world, SystemMove_ID))] == 3);
    test_true(inclusive_world->byA.subA[tnecs_A_id(inclusive_world,
                                                              TNECS_S_ID2A(inclusive_world, SystemMove_ID))][0] == tnecs_A_id(inclusive_world,
                                                                      TNECS_S_ID2A(inclusive_world, SystemMovePhase1_ID)));
    test_true(inclusive_world->byA.subA[tnecs_A_id(inclusive_world,
                                                              TNECS_S_ID2A(inclusive_world, SystemMove_ID))][1] == tnecs_A_id(inclusive_world,
                                                                      TNECS_S_ID2A(inclusive_world, SystemMovePhase2_ID)));
    test_true(inclusive_world->byA.subA[tnecs_A_id(inclusive_world,
                                                              TNECS_S_ID2A(inclusive_world, SystemMove_ID))][2] == tnecs_A_id(inclusive_world,
                                                                      TNECS_S_ID2A(inclusive_world, SystemMovePhase4_ID)));
    test_true(inclusive_world->byA.num_A_ids[tnecs_A_id(inclusive_world,
                                                            TNECS_S_ID2A(inclusive_world, SystemMovePhase1_ID))] == 1);
    test_true(inclusive_world->byA.subA[tnecs_A_id(inclusive_world,
                                                              TNECS_S_ID2A(inclusive_world,
                                                                      SystemMovePhase1_ID))][0] == tnecs_A_id(inclusive_world,
                                                                              TNECS_S_ID2A(inclusive_world, SystemMovePhase4_ID)));
    test_true(inclusive_world->byA.num_A_ids[tnecs_A_id(inclusive_world,
                                                            TNECS_S_ID2A(inclusive_world, SystemMovePhase2_ID))] == 1);
    test_true(inclusive_world->byA.subA[tnecs_A_id(inclusive_world,
                                                              TNECS_S_ID2A(inclusive_world,
                                                                      SystemMovePhase2_ID))][0] == tnecs_A_id(inclusive_world,
                                                                              TNECS_S_ID2A(inclusive_world, SystemMovePhase4_ID)));
    test_true(inclusive_world->byA.num_A_ids[tnecs_A_id(inclusive_world,
                                                            TNECS_S_ID2A(inclusive_world, SystemMovePhase4_ID))] == 0);

    test_true(inclusive_world->byA.num == 8);
    TNECS_E_CREATE_wC(inclusive_world, Unit_ID);
    TNECS_E_CREATE_wC(inclusive_world, Unit_ID);
    TNECS_E_CREATE_wC(inclusive_world, Unit_ID);
    TNECS_E_CREATE_wC(inclusive_world, Unit_ID);
    TNECS_E_CREATE_wC(inclusive_world, Unit_ID);
    TNECS_E_CREATE_wC(inclusive_world, Unit_ID);
    TNECS_E_CREATE_wC(inclusive_world, Unit_ID);
    TNECS_E_CREATE_wC(inclusive_world, Unit_ID);
    TNECS_E_CREATE_wC(inclusive_world, Unit_ID, Velocity_ID);
    TNECS_E_CREATE_wC(inclusive_world, Unit_ID);
    TNECS_E_CREATE_wC(inclusive_world, Unit_ID, Position_ID);
    tnecs_E temp_todestroy = TNECS_E_CREATE_wC(inclusive_world, Unit_ID, Position_ID);
    tnecs_E_destroy(inclusive_world, temp_todestroy);
    TNECS_E_CREATE_wC(inclusive_world, Unit_ID, Position_ID);
    TNECS_E_CREATE_wC(inclusive_world, Unit_ID, Position_ID);
    TNECS_E_CREATE_wC(inclusive_world, Unit_ID, Position_ID);
    TNECS_E_CREATE_wC(inclusive_world, Unit_ID, Velocity_ID);
    TNECS_E_CREATE_wC(inclusive_world, Unit_ID, Position_ID);
    TNECS_E_CREATE_wC(inclusive_world, Unit_ID, Position_ID, Velocity_ID);
    TNECS_E_CREATE_wC(inclusive_world, Unit_ID, Position_ID, Velocity_ID);
    TNECS_E_CREATE_wC(inclusive_world, Unit_ID, Position_ID);
    TNECS_E_CREATE_wC(inclusive_world, Unit_ID, Position_ID, Velocity_ID);
    TNECS_E_CREATE_wC(inclusive_world, Unit_ID, Position_ID, Velocity_ID);
    TNECS_E_CREATE_wC(inclusive_world, Unit_ID, Position_ID, Velocity_ID);
    TNECS_E_CREATE_wC(inclusive_world, Unit_ID, Position_ID, Velocity_ID);
    TNECS_E_CREATE_wC(inclusive_world, Unit_ID, Position_ID, Velocity_ID);
    TNECS_E_CREATE_wC(inclusive_world, Unit_ID, Position_ID, Velocity_ID);
    TNECS_E_CREATE_wC(inclusive_world, Unit_ID, Position_ID, Velocity_ID);
    TNECS_E_CREATE_wC(inclusive_world, Unit_ID, Position_ID, Velocity_ID);
    TNECS_E_CREATE_wC(inclusive_world, Unit_ID, Position_ID, Velocity_ID);
    TNECS_E_CREATE_wC(inclusive_world, Unit_ID, Position_ID, Velocity_ID);
    test_true(inclusive_world->byA.num_Es[tnecs_A_id(inclusive_world,
                                                              TNECS_S_ID2A(inclusive_world, SystemMove_ID))] == 9);
    test_true(inclusive_world->byA.num_Es[tnecs_A_id(inclusive_world,
                                                              TNECS_S_ID2A(inclusive_world, SystemMovePhase1_ID))] == 2);
    test_true(inclusive_world->byA.num_Es[tnecs_A_id(inclusive_world,
                                                              TNECS_S_ID2A(inclusive_world, SystemMovePhase2_ID))] == 6);
    test_true(inclusive_world->byA.num_Es[tnecs_A_id(inclusive_world,
                                                              TNECS_S_ID2A(inclusive_world, SystemMovePhase4_ID))] == 12);
    test_true(inclusive_world->byA.num == 8);
    tnecs_step(inclusive_world, 1, NULL);

    test_true(inclusive_world->Ss.to_run.num == 9);
    torun_arr = inclusive_world->Ss.to_run.arr;
    test_true(torun_arr[0] == &SystemMoveDoNothing);
    test_true(torun_arr[1] == &SystemMoveDoNothing);
    test_true(torun_arr[2] == &SystemMoveDoNothing);
    test_true(torun_arr[3] == &SystemMoveDoNothing);
    test_true(torun_arr[4] == &SystemMovePhase1);
    test_true(torun_arr[5] == &SystemMovePhase1);
    test_true(torun_arr[6] == &SystemMovePhase2);
    test_true(torun_arr[7] == &SystemMovePhase2);
    test_true(torun_arr[8] == &SystemMovePhase4);
    test_true(torun_arr[9] == NULL);
    test_true(torun_arr[10] == NULL);
    test_true(torun_arr[11] == NULL);
    test_true(torun_arr[12] == NULL);
    test_true(torun_arr[13] == NULL);
    test_true(torun_arr[14] == NULL);
    test_true(torun_arr[15] == NULL);

    TNECS_REGISTER_C(inclusive_world2, Position, NULL, NULL);
    TNECS_REGISTER_C(inclusive_world2, Velocity, NULL, NULL);
    TNECS_REGISTER_C(inclusive_world2, Unit, NULL, NULL);
    TNECS_REGISTER_C(inclusive_world2, Sprite, NULL, NULL);

    tnecs_register_Ph(inclusive_world2, pipe0);
    tnecs_register_Ph(inclusive_world2, pipe0);
    tnecs_register_Ph(inclusive_world2, pipe0);
    tnecs_register_Ph(inclusive_world2, pipe0);
    TNECS_REGISTER_S(inclusive_world2, SystemMoveDoNothing, pipe0, 2, 0, Unit_ID);                            // 4X
    TNECS_REGISTER_S(inclusive_world2, SystemMovePhase1,    pipe0, 1, 0, Unit_ID, Velocity_ID);               // 2X
    TNECS_REGISTER_S(inclusive_world2, SystemMovePhase2,    pipe0, 4, 0, Unit_ID, Position_ID);               // 2X
    TNECS_REGISTER_S(inclusive_world2, SystemMovePhase4,    pipe0, 3, 0, Unit_ID, Position_ID, Velocity_ID);  // 1X

    test_true(TNECS_S_ID2A(inclusive_world, SystemMove_ID) == 4);
    test_true(TNECS_S_ID2A(inclusive_world, SystemMovePhase1_ID) == 4 + 2);
    test_true(TNECS_S_ID2A(inclusive_world, SystemMovePhase2_ID) == 4 + 1);
    test_true(TNECS_S_ID2A(inclusive_world, SystemMovePhase4_ID) == 4 + 2 + 1);

    test_true(inclusive_world->byA.num_A_ids[tnecs_A_id(inclusive_world2,
                                                            TNECS_S_ID2A(inclusive_world2, SystemMove_ID))] == 3);
    test_true(inclusive_world->byA.num_A_ids[tnecs_A_id(inclusive_world2,
                                                            TNECS_S_ID2A(inclusive_world2, SystemMovePhase1_ID))] == 1);
    test_true(inclusive_world->byA.num_A_ids[tnecs_A_id(inclusive_world2,
                                                            TNECS_S_ID2A(inclusive_world2, SystemMovePhase2_ID))] == 1);
    test_true(inclusive_world->byA.num_A_ids[tnecs_A_id(inclusive_world2,
                                                            TNECS_S_ID2A(inclusive_world2, SystemMovePhase4_ID))] == 0);

    test_true(inclusive_world2->byA.num == 8);
    TNECS_E_CREATE_wC(inclusive_world2, Unit_ID);
    TNECS_E_CREATE_wC(inclusive_world2, Unit_ID, Velocity_ID);
    TNECS_E_CREATE_wC(inclusive_world2, Unit_ID, Position_ID);
    TNECS_E_CREATE_wC(inclusive_world2, Unit_ID, Position_ID, Velocity_ID);
    test_true(inclusive_world2->byA.num == 8);
    tnecs_step(inclusive_world2, 1, NULL);

    test_true(inclusive_world2->Ss.to_run.num == 9);
    torun_arr = inclusive_world2->Ss.to_run.arr; 
    test_true(torun_arr[0]  == &SystemMovePhase1);
    test_true(torun_arr[1]  == &SystemMovePhase1);
    test_true(torun_arr[2]  == &SystemMoveDoNothing);
    test_true(torun_arr[3]  == &SystemMoveDoNothing);
    test_true(torun_arr[4]  == &SystemMoveDoNothing);
    test_true(torun_arr[5]  == &SystemMoveDoNothing);
    test_true(torun_arr[6]  == &SystemMovePhase4);
    test_true(torun_arr[7]  == &SystemMovePhase2);
    test_true(torun_arr[8]  == &SystemMovePhase2);
    test_true(torun_arr[9]  == NULL);
    test_true(torun_arr[10] == NULL);
    test_true(torun_arr[11] == NULL);
    test_true(torun_arr[12] == NULL);
    test_true(torun_arr[13] == NULL);
    test_true(torun_arr[14] == NULL);
    test_true(torun_arr[15] == NULL);

    tnecs_finale(&inclusive_world2);
    tnecs_finale(&inclusive_world);
}

void tnecs_test_grow() {
    struct tnecs_W *grow_world = NULL;
    tnecs_genesis(&grow_world);
    test_true(grow_world != NULL);

    test_true(grow_world != NULL);

    test_true(grow_world->Es.len    == TNECS_E_0LEN);
    test_true(grow_world->byA.len      == TNECS_S_0LEN);
    test_true(grow_world->byA.num      == 1);
    test_true(grow_world->Ss.len     == TNECS_S_0LEN);
    test_true(grow_world->Ss.num     == 1);
    test_true(grow_world->Pis.byPh[0].len     == TNECS_Ph_0LEN);
    test_true(grow_world->Pis.byPh[0].num     == 1);
    test_true(grow_world->Es.open.num == 0);
    test_true(grow_world->Es.open.len == TNECS_E_0LEN);

    for (size_t i = 0; i < grow_world->Es.len; i++) {
        test_true(grow_world->Es.As[i] == 0);
        test_true(grow_world->Es.Os[i] == 0);
        test_true(grow_world->Es.id[i] == 0);
    }

    for (size_t i = 0; i < grow_world->byA.len; i++) {
        test_true(grow_world->byA.num_Es[i] == 0);
        test_true(grow_world->byA.len_Es[i] == TNECS_E_0LEN);
        for (size_t j = 0; j < grow_world->byA.len_Es[i]; j++) {
            test_true(grow_world->byA.Es[i][j] == 0);
        }
        test_true(grow_world->byA.num_Cs[i] == 0);
    }

    for (size_t i = 0; i < grow_world->Pis.byPh[0].len; i++) {
        test_true(grow_world->Pis.byPh[0].num_Ss[i] == 0);
        test_true(grow_world->Pis.byPh[0].len_Ss[i] == TNECS_Ph_0LEN);
        for (size_t j = 0; j < grow_world->Pis.byPh[0].len_Ss[i]; j++) {
            test_true(grow_world->Pis.byPh[0].Ss[i][j] == 0);
        }
    }

    tnecs_grow_E(grow_world);
    test_true(grow_world->Es.len == TNECS_E_0LEN * TNECS_ARR_GROW);

    for (size_t i = 0; i < grow_world->Es.len; i++) {
        test_true(grow_world->Es.As[i] == 0);
        test_true(grow_world->Es.Os[i] == 0);
        test_true(grow_world->Es.id[i] == 0);
    }

    size_t test_archetypeid = 0;
    tnecs_grow_byA(grow_world, test_archetypeid);
    test_true(grow_world->byA.num_Es[test_archetypeid] == 0);
    test_true(grow_world->byA.len_Es[test_archetypeid] == TNECS_E_0LEN *
        TNECS_ARR_GROW);
    for (size_t j = 0; j < grow_world->byA.len_Es[test_archetypeid]; j++) {
        test_true(grow_world->byA.Es[test_archetypeid][j] == 0);
    }

    test_archetypeid = 1;
    tnecs_grow_byA(grow_world, test_archetypeid);
    test_true(grow_world->byA.num_Es[test_archetypeid] == 0);
    test_true(grow_world->byA.len_Es[test_archetypeid] == TNECS_E_0LEN *
        TNECS_ARR_GROW);
    for (size_t j = 0; j < grow_world->byA.len_Es[test_archetypeid]; j++) {
        test_true(grow_world->byA.Es[test_archetypeid][j] == 0);
    }
    for (size_t i = (test_archetypeid + 1); i < grow_world->byA.len; i++) {
        test_true(grow_world->byA.num_Es[i] == 0);
        test_true(grow_world->byA.len_Es[i] == TNECS_E_0LEN);
        for (size_t j = 0; j < grow_world->byA.len_Es[i]; j++) {
            test_true(grow_world->byA.Es[i][j] == 0);
        }
        test_true(grow_world->byA.num_Cs[i] == 0);
    }

    tnecs_grow_S(grow_world);
    test_true(grow_world->Ss.len == TNECS_S_0LEN * TNECS_ARR_GROW);
    test_true(grow_world->Ss.num == 1);
    tnecs_grow_A(grow_world);
    test_true(grow_world->byA.len == TNECS_S_0LEN * TNECS_ARR_GROW);
    test_true(grow_world->byA.num == 1);

    for (size_t i = TNECS_S_0LEN; i < grow_world->byA.len; i++) {
        test_true(grow_world->byA.num_Es[i] == 0);
        test_true(grow_world->byA.len_Es[i] == TNECS_E_0LEN);
        for (size_t j = 0; j < grow_world->byA.len_Es[i]; j++) {
            test_true(grow_world->byA.Es[i][j] == 0);
        }
        test_true(grow_world->byA.num_Cs[i] == 0);
    }

    tnecs_grow_Ph(grow_world, 0);
    test_true(grow_world->Pis.byPh[0].len == TNECS_Ph_0LEN * TNECS_ARR_GROW);

    test_true(grow_world->Pis.byPh[0].num == 1);
    for (size_t i = TNECS_Ph_0LEN; i < grow_world->Pis.byPh[0].len; i++) {
        test_true(grow_world->Pis.byPh[0].num_Ss[i] == 0);
        test_true(grow_world->Pis.byPh[0].len_Ss[i] == TNECS_Ph_0LEN);
        for (size_t j = 0; j < grow_world->Pis.byPh[0].len_Ss[i]; j++) {
            test_true(grow_world->Pis.byPh[0].Ss[i][j] == 0);
        }
    }

    tnecs_finale(&grow_world);
}

void tnecs_benchmarks(uint64_t num) {
    // printf("tnecs_benchmarks \n");
    u64 t_0;
    u64 t_1;

    uint32_t rand1 = rand() % 100;
    uint32_t rand2 = rand() % 100;

    dupprintf(globalf, " %8llu\t", num);
    t_0 = tnecs_get_us();
    tnecs_W *bench_world = NULL;
    tnecs_genesis(&bench_world);
    t_1 = tnecs_get_us();
    dupprintf(globalf, "%7llu\t", t_1 - t_0);

    t_0 = tnecs_get_us();
    TNECS_REGISTER_C(bench_world, Position2, NULL, NULL);
    TNECS_REGISTER_C(bench_world, Unit2, NULL, NULL);
    t_1 = tnecs_get_us();
    dupprintf(globalf, "%7llu\t", t_1 - t_0);

    int Position2_ID    = 1;
    int Unit2_ID        = 2;
    for (uint64_t i = 0; i < num; i++) {
        TNECS_E_CREATE_wC(bench_world, Position2_ID, Unit2_ID);
    }

    t_0 = tnecs_get_us();
    TNECS_REGISTER_S(bench_world, SystemMove2, pipe0, 0, 0, Position2_ID, Unit2_ID);
    t_1 = tnecs_get_us();

    dupprintf(globalf, "%7llu\t", t_1 - t_0);

    t_0 = tnecs_get_us();
    for (size_t i = 0; i < ITERATIONS; i++) {
        test_Es[i] = tnecs_E_create(bench_world);
    }

    t_1 = tnecs_get_us();
    dupprintf(globalf, "%7llu\t", t_1 - t_0);

    t_0 = tnecs_get_us();
    for (size_t i = 0; i < ITERATIONS; i++) {
        tnecs_E ent = test_Es[i] * (rand2 + 1) % num;
        tnecs_E_destroy(bench_world, ent);
    }
    t_1 = tnecs_get_us();
    dupprintf(globalf, "%7llu\t", t_1 - t_0);

    for (size_t i = 0; i < ITERATIONS; i++) {
        // TODO: benchmark reuse or not
        test_Es[i] = tnecs_E_create(bench_world);
    }

    t_0 = tnecs_get_us();
    TNECS_ADD_C(bench_world, test_Es[1], Position2_ID);
    TNECS_ADD_C(bench_world, test_Es[1], Unit2_ID);
    for (size_t i = 2; i < ITERATIONS; i++) {
        tnecs_E ent = test_Es[i] * (rand1 + 2) % num;
        TNECS_ADD_C(bench_world, ent, Position2_ID, false);
        TNECS_ADD_C(bench_world, ent, Unit2_ID, false);
    }
    t_1 = tnecs_get_us();
    dupprintf(globalf, "%7llu\t", t_1 - t_0);

    t_0 = tnecs_get_us();
    for (size_t i = 0; i < ITERATIONS; i++) {
        TNECS_E_CREATE_wC(bench_world, Position2_ID);
    }
    t_1 = tnecs_get_us();
    dupprintf(globalf, "%7llu\t", t_1 - t_0);

    tnecs_E tnecs_Es2[ITERATIONS];
    t_0 = tnecs_get_us();
    for (size_t i = 0; i < ITERATIONS; i++) {
        TNECS_E_CREATE_wC(bench_world, Position2_ID);
        tnecs_Es2[i] = TNECS_E_CREATE_wC(bench_world, Position2_ID, Unit2_ID);
        assert(bench_world->Es.id[tnecs_Es2[i]] == tnecs_Es2[i]);
    }


    t_1 = tnecs_get_us();
    dupprintf(globalf, "%7llu\t", t_1 - t_0);

    for (size_t i = 0; i < ITERATIONS; i++) {
        // TODO: destroy random entity
        assert(bench_world->Es.id[tnecs_Es2[i]] == tnecs_Es2[i]);
        tnecs_E_destroy(bench_world, tnecs_Es2[i]);
    }

    t_0 = tnecs_get_us();
    for (size_t i = 0; i < ITERATIONS; i++) {
        tnecs_E ent = test_Es[i] * (rand2 + 5) % num;
        struct Position2    *pos   = tnecs_get_C(bench_world, ent, Position2_ID);
        struct Unit2        *unit  = tnecs_get_C(bench_world, ent, Unit2_ID);
        if (pos && unit) {
            unit->hp = pos->x + pos->y;
        }
    }
    t_1 = tnecs_get_us();
    dupprintf(globalf, "%7llu\t", t_1 - t_0);

    t_0 = tnecs_get_us();
    for (size_t i = 0; i < fps_iterations; i++) {
        tnecs_step(bench_world, 1, NULL);
    }
    t_1 = tnecs_get_us();
    dupprintf(globalf, "%7llu\t", t_1 - t_0);

    t_0 = tnecs_get_us();
    tnecs_finale(&bench_world);
    t_1 = tnecs_get_us();
    dupprintf(globalf, "%7llu\n", t_1 - t_0);
}

void test_log2() {
    test_true(log2(0.0) == -INFINITY);
    test_true(log2(0.0) == -INFINITY);
    test_true(log2(0) == -INFINITY);
    test_true(log2(0) == -INFINITY);
    test_true(log2(1.0) == 0.0);
    test_true(log2(1.0) == 0);
    test_true(log2(2.0) == 1.0);
    test_true(log2(2.0) == 1);
}

void tnecs_test_Pis() {
    int Position_ID = 1;
    int Velocity_ID = 2;
    int Unit_ID     = 3;
    int Sprite_ID   = 4;

    tnecs_W *pipe_world = NULL;
    tnecs_genesis(&pipe_world);
    const tnecs_Ph phase0    = 0;
    const tnecs_Ph phase1    = 1;
    const tnecs_Pi pipe1  = 1;
    
    test_true(TNECS_REGISTER_C(pipe_world, Position_ID, NULL, NULL));
    test_true(TNECS_REGISTER_C(pipe_world, Velocity_ID, NULL, NULL));
    test_true(TNECS_REGISTER_C(pipe_world, Sprite_ID, NULL, NULL));
    test_true(TNECS_REGISTER_C(pipe_world, Unit_ID, NULL, NULL));

    // Register Pis
    test_true(tnecs_register_Pi(pipe_world));
    test_true(TNECS_Pi_VALID(pipe_world, pipe0));
    test_true(TNECS_Pi_VALID(pipe_world, pipe1));
    test_true(pipe_world->Pis.num == 2);

    // Register Phs
    test_true(tnecs_register_Ph(pipe_world, pipe0) == 1);
    test_true(tnecs_register_Ph(pipe_world, pipe0) == 2);
    test_true(tnecs_register_Ph(pipe_world, pipe1) == 1);
    test_true(tnecs_register_Ph(pipe_world, pipe1) == 2);
    test_true(pipe_world->Pis.byPh[pipe0].num == 3);
    test_true(pipe_world->Pis.byPh[pipe1].num == 3);

    // Register Ss, all exclusives, all phase0
    TNECS_REGISTER_S(pipe_world, SystemMoveDoNothing,  pipe0, phase0, 1, Unit_ID);                              /* 4X */
    TNECS_REGISTER_S(pipe_world, SystemMovePhase1,     pipe0, phase1, 1, Unit_ID, Velocity_ID);                 /* 2X */
    TNECS_REGISTER_S(pipe_world, SystemMovePhase2,     pipe1, phase0, 1, Unit_ID, Position_ID);                 /* 2X */
    TNECS_REGISTER_S(pipe_world, SystemMovePhase4,     pipe1, phase1, 1, Unit_ID, Position_ID, Velocity_ID);    /* 1X */

    test_true(pipe_world->Ss.Pi[0] == pipe0);
    test_true(pipe_world->Ss.Pi[1] == pipe0);
    test_true(pipe_world->Ss.Pi[2] == pipe0);
    test_true(pipe_world->Ss.Pi[3] == pipe1);
    test_true(pipe_world->Ss.Pi[4] == pipe1);

    test_true(pipe_world->Pis.byPh[pipe0].num_Ss[phase0] == 1);
    test_true(pipe_world->Pis.byPh[pipe0].num_Ss[phase1] == 1);
    test_true(pipe_world->Pis.byPh[pipe1].num_Ss[phase0] == 1);
    test_true(pipe_world->Pis.byPh[pipe1].num_Ss[phase1] == 1);

    // Checking which Ss need to be run for pipe0
    tnecs_step_Pi(pipe_world, 1, NULL, pipe0);
    test_true(pipe_world->Ss.to_run.num == 2);
    tnecs_S_f *system_arr_pipe0 = pipe_world->Ss.to_run.arr;
    test_true(system_arr_pipe0[0] == SystemMoveDoNothing);
    test_true(system_arr_pipe0[1] == SystemMovePhase1);

    // Checking which Ss need to be run for pipe1
    tnecs_step_Pi(pipe_world, 1, NULL, pipe1);
    test_true(pipe_world->Ss.to_run.num == 2);
    tnecs_S_f *system_arr_pipe1 = pipe_world->Ss.to_run.arr;
    test_true(system_arr_pipe1[0] == SystemMovePhase2);
    test_true(system_arr_pipe1[1] == SystemMovePhase4);

    // Checking which Ss need to be run for pipe0, phase0
    pipe_world->Ss.to_run.num = 0;
    tnecs_step_Pi_Ph(pipe_world, 1, NULL, pipe0, phase0);
    test_true(pipe_world->Ss.to_run.num == 1);

    // Checking which Ss need to be run for pipe0, phase1
    pipe_world->Ss.to_run.num = 0;
    tnecs_step_Pi_Ph(pipe_world, 1, NULL, pipe0, phase1);
    test_true(pipe_world->Ss.to_run.num == 1);

    // Checking which Ss need to be run for pipe1, phase0
    pipe_world->Ss.to_run.num = 0;
    tnecs_step_Pi_Ph(pipe_world, 1, NULL, pipe1, phase0);
    test_true(pipe_world->Ss.to_run.num == 1);

    // Checking which Ss need to be run for pipe1, phase1
    pipe_world->Ss.to_run.num = 0;
    tnecs_step_Pi_Ph(pipe_world, 1, NULL, pipe1, phase1);
    test_true(pipe_world->Ss.to_run.num == 1);

    tnecs_finale(&pipe_world);
}

void Position_Init(void *voidpos) {
    struct Position *pos = voidpos;
    pos->arr_len = 10;
    if (pos->arr == NULL) {
        pos->arr = calloc(pos->arr_len, sizeof(*pos->arr));
    }
}

void Position_Free(void *voidpos) {
    struct Position *pos = voidpos;
    if (pos->arr != NULL) {
        free(pos->arr);
        pos->arr = NULL;
    }
    pos->arr_len = 0;
}

void tnecs_test_finit_ffree(void) {
    int Position_ID = 1;

    /* Testing that everything is NULL when creating without functions */
    tnecs_W *nof_world = NULL;
    tnecs_genesis(&nof_world);
   
    TNECS_REGISTER_C(nof_world, Position, NULL, NULL);

    tnecs_E Silou = TNECS_E_CREATE_wC(nof_world, Position_ID);
    struct Position *pos = tnecs_get_C(nof_world, Silou, Position_ID);
    test_true(pos->arr      == NULL);
    test_true(pos->arr_len  == 0);

    /* Testing that everything is NULL when creating without functions */
    tnecs_W *f_world = NULL;
    tnecs_genesis(&f_world);
   
    TNECS_REGISTER_C(f_world, Position, Position_Init, Position_Free);

    Silou = TNECS_E_CREATE_wC(f_world, Position_ID);
    pos = tnecs_get_C(f_world, Silou, Position_ID);
    test_true(pos->arr      != NULL);
    test_true(pos->arr_len  != 0);

    tnecs_E_destroy(f_world, Silou);
    test_true(pos->arr      == NULL);
    test_true(pos->arr_len  == 0);

    tnecs_finale(&nof_world);
    tnecs_finale(&f_world);
}

int main() {
    globalf = fopen("tnecs_test_results.txt", "w+");
    dupprintf(globalf, "\n --- tnecs test start ---\n");
    lrun("utilities",   tnecs_test_utilities);
    lrun("log2",        test_log2);
    lrun("c_regis",     tnecs_test_C_registration);
    lrun("s_regis",     tnecs_test_system_registration);
    lrun("e_create",    tnecs_test_E_creation);
    lrun("e_destroy",   tnecs_test_E_destroy);
    lrun("c_add",       tnecs_test_C_add);
    lrun("c_remove",    tnecs_test_C_remove);
    lrun("c_array",     tnecs_test_C_array);
    lrun("grow",        tnecs_test_grow);
    lrun("progress",    tnecs_test_world_progress);
    lrun("finit_ffree", tnecs_test_finit_ffree);
    lrun("Pis",   tnecs_test_Pis);
    lresults();

    dupprintf(globalf, "\n --- Notes ---\n");
    dupprintf(globalf, "world size: %ld bytes\n", sizeof(struct tnecs_W));
    dupprintf(globalf, "%d frame %d fps, ", fps_iterations, 60);
    dupprintf(globalf, "%.1f [us] \n\n", fps_iterations / 60.0f * 1e6);

    srand(time(NULL));   // Initialization, should only be called once.
    dupprintf(globalf, " --- tnecs benchmarks: %d iterations ---\n", ITERATIONS);
    dupprintf(globalf, "Entities [num]\t");
    dupprintf(globalf, "Genesis\t"); 
    dupprintf(globalf, "cRegist\t");
    dupprintf(globalf, "sRegist\t");
    dupprintf(globalf, "eCreate\t");
    dupprintf(globalf, "eDestro\t");
    dupprintf(globalf, "eAddCom\t");
    dupprintf(globalf, "eCreawC\t");
    dupprintf(globalf, "eCrewCs\t");
    dupprintf(globalf, "compGet\t");
    dupprintf(globalf, "wrlStep\t");
    dupprintf(globalf, "wDestroy [us]\n");
    // for (uint64_t num = 1; num < 2e6; num *= 2)
    //     tnecs_benchmarks(num);

    tnecs_finale(&test_world);
    dupprintf(globalf, "\n --- tnecs test end ---\n\n");
    fclose(globalf);
    return (0);
}
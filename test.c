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
        dupprintf(globalf,"pass:%6d\tfail:%6d\t%4dms\n",\
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

void SystemMove2(struct tnecs_system_input *input) {
    int     Position2_ID    = 1;
    int     Unit2_ID        = 2;
    struct  Position2   *p = NULL;
    struct  Unit2       *v = NULL;

    p = TNECS_COMPONENTS_ARRAY(input, Position2_ID);
    v = TNECS_COMPONENTS_ARRAY(input, Unit2_ID);


    for (int i = 0; i < input->num_entities; i++) {
        p[i].x += v[i].hp;
        p[i].y += v[i].str;
    }
}

void SystemMovePhase1(struct tnecs_system_input *input) {
    for (int ent = 0; ent < input->num_entities; ent++) {
        tnecs_entity current_ent = input->world->bytype.entities[input->entity_archetype_id][ent];
        lok(current_ent);
        lok(input->world->entities.id[input->world->bytype.entities[input->entity_archetype_id][ent]]
            == input->world->bytype.entities[input->entity_archetype_id][ent]);
        lok(input->entity_archetype_id == tnecs_archetypeid(input->world,
                                                             input->world->entities.archetypes[current_ent]));
    }
}

void SystemMovePhase4(struct tnecs_system_input *input) {
    for (int ent = 0; ent < input->num_entities; ent++) {
        tnecs_entity current_ent = input->world->bytype.entities[input->entity_archetype_id][ent];
        lok(current_ent);
        lok(input->world->entities.id[input->world->bytype.entities[input->entity_archetype_id][ent]]
            == input->world->bytype.entities[input->entity_archetype_id][ent]);
        lok(input->entity_archetype_id == tnecs_archetypeid(input->world,
                                                             input->world->entities.archetypes[current_ent]));
    }
}

void SystemMovePhase2(struct tnecs_system_input *input) {
    for (int ent = 0; ent < input->num_entities; ent++) {
        tnecs_entity current_ent = input->world->bytype.entities[input->entity_archetype_id][ent];
        lok(current_ent);
        lok(input->world->entities.id[input->world->bytype.entities[input->entity_archetype_id][ent]]
            == input->world->bytype.entities[input->entity_archetype_id][ent]);
        lok(input->entity_archetype_id == tnecs_archetypeid(input->world,
                                                             input->world->entities.archetypes[current_ent]));
    }
}

/*****************************TEST CONSTANTS***************************/
#define ITERATIONS 10000
size_t fps_iterations = 10;

/*******************************TEST SYSTEMS***************************/
tnecs_entity test_entities[ITERATIONS];
tnecs_entity        *components_list;
struct Position     *temp_position;
struct Unit         *temp_unit;
struct Sprite       *temp_sprite;
struct tnecs_world  *test_world;

void SystemMove(struct tnecs_system_input *input) {
    // printf("SystemMove\n");
    int Position_ID = 1;
    int Velocity_ID = 2;
    struct Position *p = NULL;
    struct Velocity *v = NULL;
    p = TNECS_COMPONENTS_ARRAY(input, Position_ID);
    v = TNECS_COMPONENTS_ARRAY(input, Velocity_ID);

    for (int ent = 0; ent < input->num_entities; ent++) {
        tnecs_entity current_ent = input->world->bytype.entities[input->entity_archetype_id][ent];
        lok(current_ent);
        lok(input->world->entities.id[input->world->bytype.entities[input->entity_archetype_id][ent]]
            == input->world->bytype.entities[input->entity_archetype_id][ent]);
        lok(input->entity_archetype_id == tnecs_archetypeid(input->world,
                                                             input->world->entities.archetypes[current_ent]));

        p[ent].x = p[ent].x + v[ent].vx;
        p[ent].y = p[ent].y + v[ent].vy;
    }
}

void SystemMoveDoNothing(struct tnecs_system_input *input) {
    int doesnotexist_ID = 8;
    void *ptr = NULL;
    ptr = TNECS_COMPONENTS_ARRAY(input, doesnotexist_ID);
    lok(ptr == NULL);
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

    lok(setBits_KnR_u64(1) == 1);
    lok(setBits_KnR_u64(2) == 1);
    lok(setBits_KnR_u64(3) == 2);
    lok(setBits_KnR_u64(4) == 1);
    lok(setBits_KnR_u64(5) == 2);
    lok(setBits_KnR_u64(6) == 2);
    lok(setBits_KnR_u64(7) == 3);
}

void tnecs_test_component_registration() {
    tnecs_world_genesis(&test_world);
    lok(test_world != NULL);
    assert(sizeof(Position) > 0);
    TNECS_REGISTER_COMPONENT(test_world, Position);
    size_t temp_comp_flag       = 1;
    size_t temp_comp_id         = 1;
    size_t temp_comp_order      = 0;
    size_t temp_archetype_id    = 1;
    size_t temp_archetype       = 1;
    lok(TNECS_COMPONENT_ID2TYPE(temp_comp_id) == temp_archetype);
    assert(test_world->bytype.components_id != NULL);
    assert(test_world->bytype.components_id[temp_comp_id] != NULL);
    lok(test_world->bytype.components_id[temp_archetype_id][temp_comp_order] == temp_comp_id);
    lok(test_world->bytype.id[0] == 0);
    lok(test_world->bytype.id[1] == (TNECS_NULLSHIFT << 0));
    lok(test_world->bytype.id[1] == temp_comp_flag);
    lok(test_world->components.num == 2);

    TNECS_REGISTER_COMPONENT(test_world, Unit);
    temp_comp_flag = 2;
    temp_comp_id = 2;
    temp_comp_order = 0;
    temp_archetype_id = 2;
    temp_archetype = 2;
    lok(TNECS_COMPONENT_ID2TYPE(temp_comp_id) == temp_archetype);
    lok(test_world->bytype.components_id[temp_comp_id][temp_comp_order] == temp_comp_id);
    lok(test_world->bytype.id[0] == 0);
    lok(test_world->bytype.id[1] == (TNECS_NULLSHIFT << 0));
    lok(test_world->bytype.id[2] == (TNECS_NULLSHIFT << 1));
    lok(test_world->bytype.id[2] == temp_comp_flag);
    lok(test_world->components.num == 3);

    TNECS_REGISTER_COMPONENT(test_world, Sprite);
    temp_comp_flag = 4;
    temp_comp_id = 3;
    temp_comp_order = 0;
    temp_archetype_id = 3;
    temp_archetype = 4;
    lok(TNECS_COMPONENT_ID2TYPE(temp_comp_id) == temp_archetype);
    lok(test_world->bytype.id[0] == 0);
    lok(test_world->bytype.id[1] == (TNECS_NULLSHIFT << 0));
    lok(test_world->bytype.id[2] == (TNECS_NULLSHIFT << 1));
    lok(test_world->bytype.id[3] == (TNECS_NULLSHIFT << 2));
    lok(test_world->bytype.id[3] == temp_comp_flag);
    lok(test_world->components.num == 4);

    TNECS_REGISTER_COMPONENT(test_world, Velocity);
    temp_comp_flag = 8;
    temp_comp_id = 4;
    temp_comp_order = 0;
    temp_archetype_id = 4;
    temp_archetype = 8;
    lok(TNECS_COMPONENT_ID2TYPE(temp_comp_id) == temp_archetype);
    lok(test_world->bytype.components_id[temp_comp_id][temp_comp_order] == temp_comp_id);

    lok(test_world->bytype.id[0] == 0);
    lok(test_world->bytype.id[1] == (TNECS_NULLSHIFT << 0));
    lok(test_world->bytype.id[2] == (TNECS_NULLSHIFT << 1));
    lok(test_world->bytype.id[3] == (TNECS_NULLSHIFT << 2));
    lok(test_world->bytype.id[4] == (TNECS_NULLSHIFT << 3));
    lok(test_world->bytype.id[4] == temp_comp_flag);
    lok(test_world->components.num == 5);

    int Position_ID = 1;
    int Velocity_ID = 2;
    int Unit_ID     = 3;

    lok(TNECS_COMPONENT_IDS2ARCHETYPE(1, 2, 3) == (1 + 2 + 4));
    lok(TNECS_COMPONENT_IDS2ARCHETYPE(Position_ID, Unit_ID, Velocity_ID) == (1 + 2 + 4));
}

void tnecs_test_system_registration() {
    int Position_ID = 1;
    int Velocity_ID = 2;

    TNECS_REGISTER_SYSTEM(test_world, SystemMove, 0, 1, Position_ID, Velocity_ID);
    size_t temp_archetype_id    = 5;

    lok(test_world->bytype.components_id[temp_archetype_id][0] == Position_ID);
    lok(test_world->bytype.components_id[temp_archetype_id][1] == Velocity_ID);
}

void tnecs_test_entity_creation() {
    // dupprintf(globalf, "tnecs_test_entity_creation \n");

    int Position_ID = 1;
    int Unit_ID     = 3;
    int Sprite_ID   = 4;

    lok(test_world->entities.num == TNECS_NULLSHIFT);
    TNECS_REGISTER_COMPONENT(test_world, Sprite);
    tnecs_entity Silou = tnecs_entity_create(test_world);
    lok(Silou == TNECS_NULLSHIFT);
    lok(test_world->entities.num == (TNECS_NULLSHIFT + 1));
    tnecs_entity Pirou = tnecs_entity_create(test_world);
    lok(Pirou == (TNECS_NULLSHIFT + 1));
    lok(test_world->entities.num == (TNECS_NULLSHIFT + 2));
    lok(Silou != Pirou);
    tnecs_entity Perignon = TNECS_ENTITY_CREATE_wCOMPONENTS(test_world, Position_ID, Unit_ID);
    lok(Perignon != TNECS_NULL);

    temp_position = tnecs_get_component(test_world, Perignon, Position_ID);
    if (temp_position == NULL) {
        lok(false);
    } else {
        lok(temp_position->x == 0);
        lok(temp_position->y == 0);
        temp_position->x = 3;
        temp_position->y = 6;
    }

    lok(test_world->entities.archetypes[Perignon] == TNECS_COMPONENT_ID2TYPE(Position_ID) + TNECS_COMPONENT_ID2TYPE(Unit_ID));

    temp_position = tnecs_get_component(test_world, Perignon, Sprite_ID);
    lok(temp_position == NULL);
    temp_unit = tnecs_get_component(test_world, Perignon, Unit_ID);
    if (temp_unit == NULL) {
        lok(false);
    } else {
        lok(temp_unit->hp  == 0);
        lok(temp_unit->str == 0);
    }

    #ifdef NDEBUG
    tnecs_entity_destroy(test_world, Silou);
    #else
    assert(tnecs_entity_destroy(test_world, Silou));
    #endif
    tnecs_entity *open_arr = test_world->entities_open.arr;
    lok(test_world->entities_open.num == 0);
    lok(!test_world->entities.id[Silou]);
    tnecs_entities_open_reuse(test_world);
    lok(test_world->entities_open.num == 1);
    lok(open_arr[0] != TNECS_NULL);
    lok(open_arr[0] == Silou);
    tnecs_entity_create(test_world);
    lok(test_world->entities.id[Silou]);

    tnecs_entities_create(test_world, 100);
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

    int Position2_ID = 1;
    int Unit2_ID     = 2;

    TNECS_REGISTER_COMPONENT(test_world2, Position2);
    test_world2->byphase.num_systems[0] = TNECS_INIT_PHASE_LEN;
    TNECS_REGISTER_SYSTEM(test_world2, SystemMovePhase1, 0, 0, Position2_ID);
    tnecs_world_destroy(&test_world2);

    // Coverage for "for" in tnecs_component_del
    tnecs_world_genesis(&test_world2);
    TNECS_REGISTER_COMPONENT(test_world2, Unit2_ID);
    TNECS_REGISTER_COMPONENT(test_world2, Position2_ID);
    tnecs_entity Erwin = TNECS_ENTITY_CREATE_wCOMPONENTS(test_world2, Position2_ID, Unit2_ID);
    tnecs_component_del(test_world2, Erwin, (1 + 2));
    tnecs_world_destroy(&test_world2);
}

void tnecs_test_component_add() {
    int Position_ID = 1;
    int Unit_ID     = 3;
    int Sprite_ID   = 4;

    tnecs_entity Silou = tnecs_entity_create(test_world);
    TNECS_ADD_COMPONENT(test_world, Silou, Position_ID);
    lok((test_world->entities.archetypes[Silou] & TNECS_COMPONENT_ID2TYPE(Unit_ID)) == 0);
    TNECS_ADD_COMPONENT(test_world, Silou, Unit_ID);
    lok((test_world->entities.archetypes[Silou] & TNECS_COMPONENT_ID2TYPE(Position_ID)) > 0);
    lok((test_world->entities.archetypes[Silou] & TNECS_COMPONENT_ID2TYPE(Unit_ID)) > 0);
    lok((test_world->entities.archetypes[Silou] & TNECS_COMPONENT_ID2TYPE(Sprite_ID)) == 0);

    tnecs_entity Pirou = tnecs_entity_create(test_world);
    TNECS_ADD_COMPONENT(test_world, Pirou, Position_ID);
    lok((test_world->entities.archetypes[Pirou] & TNECS_COMPONENT_ID2TYPE(Position_ID)) > 0);
    lok((test_world->entities.archetypes[Pirou] & TNECS_COMPONENT_ID2TYPE(Unit_ID)) == 0);
    TNECS_ADD_COMPONENT(test_world, Pirou, Unit_ID);
    lok((test_world->entities.archetypes[Pirou] & TNECS_COMPONENT_ID2TYPE(Unit_ID)) > 0);
    lok((test_world->entities.archetypes[Pirou] & TNECS_COMPONENT_ID2TYPE(Position_ID)) > 0);
    lok((test_world->entities.archetypes[Pirou] & TNECS_COMPONENT_ID2TYPE(Sprite_ID)) == 0);

    tnecs_entity Chasse = tnecs_entity_create(test_world);
    TNECS_ADD_COMPONENTS(test_world, Chasse, 1, Sprite_ID, Position_ID);
    lok((test_world->entities.archetypes[Chasse] & TNECS_COMPONENT_ID2TYPE(Unit_ID)) == 0);
    lok((test_world->entities.archetypes[Chasse] & TNECS_COMPONENT_ID2TYPE(Sprite_ID)) > 0);
    lok((test_world->entities.archetypes[Chasse] & TNECS_COMPONENT_ID2TYPE(Position_ID)) > 0);

    temp_position = tnecs_get_component(test_world, Silou, Position_ID);
    lok(temp_position != NULL);
    lok(temp_position->x == 0);
    lok(temp_position->y == 0);
    temp_position->x = 1;
    temp_position->y = 2;
    lok(temp_position->x == 1);
    lok(temp_position->y == 2);
    temp_position = tnecs_get_component(test_world, Silou, Position_ID);
    lok(temp_position->x == 1);
    lok(temp_position->y == 2);

    temp_unit = tnecs_get_component(test_world, Silou, Unit_ID);
    lok(temp_unit != NULL);
    lok(temp_unit->hp == 0);
    lok(temp_unit->str == 0);
    temp_unit->hp = 3;
    temp_unit->str = 4;
    lok(temp_unit->hp == 3);
    lok(temp_unit->str == 4);
    temp_unit = tnecs_get_component(test_world, Silou, Unit_ID);
    lok(temp_unit->hp == 3);
    lok(temp_unit->str == 4);
    temp_position = tnecs_get_component(test_world, Silou, Position_ID);
    lok(temp_position->x == 1);
    lok(temp_position->y == 2);

    temp_position = tnecs_get_component(test_world, Pirou, Position_ID);
    lok(temp_position->x == 0);
    lok(temp_position->y == 0);
    temp_position->x = 5;
    temp_position->y = 6;
    lok(temp_position->x == 5);
    lok(temp_position->y == 6);
    temp_position = tnecs_get_component(test_world, Pirou, Position_ID);
    lok(temp_position->x == 5);
    lok(temp_position->y == 6);

    temp_unit = tnecs_get_component(test_world, Pirou, Unit_ID);
    lok(temp_unit->hp == 0);
    lok(temp_unit->str == 0);
    temp_unit->hp = 7;
    temp_unit->str = 8;
    lok(temp_unit->hp == 7);
    lok(temp_unit->str == 8);
    temp_position = tnecs_get_component(test_world, Pirou, Position_ID);
    lok(temp_position->x == 5);
    lok(temp_position->y == 6);
    temp_unit = tnecs_get_component(test_world, Pirou, Unit_ID);
    lok(temp_unit->hp == 7);
    lok(temp_unit->str == 8);
}

void tnecs_test_entity_destroy() {
    /* Check that deleting an entity does NOT change other entity data. */
    int Position_ID = 1;
    int Unit_ID     = 3;

    int Silou_x     = 10;
    int Pirou_x     = 20;
    int Chasse_x    = 30;
    int Michael_x   = 40;

    tnecs_entity Silou      = TNECS_ENTITY_CREATE_wCOMPONENTS(test_world, Position_ID, Unit_ID);
    tnecs_entity Pirou      = TNECS_ENTITY_CREATE_wCOMPONENTS(test_world, Position_ID, Unit_ID);
    tnecs_entity Chasse     = TNECS_ENTITY_CREATE_wCOMPONENTS(test_world, Position_ID, Unit_ID);
    tnecs_entity Michael    = TNECS_ENTITY_CREATE_wCOMPONENTS(test_world, Position_ID, Unit_ID);

    lok(test_world->entities.orders[Silou]      == 1);
    lok(test_world->entities.orders[Pirou]      == 2);
    lok(test_world->entities.orders[Chasse]     == 3);
    lok(test_world->entities.orders[Michael]    == 4);

    struct Position *position = tnecs_get_component(test_world, Silou, Position_ID);
    position->x = Silou_x;
    position = tnecs_get_component(test_world, Pirou, Position_ID);
    position->x = Pirou_x;
    position = tnecs_get_component(test_world, Chasse, Position_ID);
    position->x = Chasse_x;
    position = tnecs_get_component(test_world, Michael, Position_ID);
    position->x = Michael_x;

    tnecs_entity_destroy(test_world, Pirou);

    lok(test_world->entities.orders[Silou]      == 1);
    lok(test_world->entities.orders[Michael]    == 2);
    lok(test_world->entities.orders[Chasse]     == 3);

    position = tnecs_get_component(test_world, Pirou, Position_ID);
    lok(position == NULL);
    position = tnecs_get_component(test_world, Silou, Position_ID);
    lok(position->x == Silou_x);
    position = tnecs_get_component(test_world, Chasse, Position_ID);
    lok(position->x == Chasse_x);
    position = tnecs_get_component(test_world, Michael, Position_ID);
    lok(position->x == Michael_x);

    tnecs_entity_destroy(test_world, Michael);

    lok(test_world->entities.orders[Silou]  == 1);
    lok(test_world->entities.orders[Chasse] == 2);

    position = tnecs_get_component(test_world, Pirou, Position_ID);
    lok(position == NULL);
    position = tnecs_get_component(test_world, Michael, Position_ID);
    lok(position == NULL);
    position = tnecs_get_component(test_world, Silou, Position_ID);
    lok(position->x == Silou_x);
    position = tnecs_get_component(test_world, Chasse, Position_ID);
    lok(position->x == Chasse_x);

}

void tnecs_test_component_remove() {
    int Position_ID = 1;
    int Velocity_ID = 2;
    int Unit_ID     = 3;

    tnecs_entity Silou = tnecs_entity_create(test_world);
    TNECS_ADD_COMPONENT(test_world, Silou, Position_ID);
    lok(TNECS_ENTITY_HASCOMPONENT(test_world, Silou, Position_ID));
    TNECS_REMOVE_COMPONENTS(test_world, Silou, Position_ID);
    lok(!TNECS_ENTITY_HASCOMPONENT(test_world, Silou, Position_ID));

    tnecs_entity Perignon = TNECS_ENTITY_CREATE_wCOMPONENTS(test_world, Position_ID, Velocity_ID);
    lok(TNECS_ENTITY_HASCOMPONENT(test_world, Perignon, Position_ID));
    lok(TNECS_ENTITY_HASCOMPONENT(test_world, Perignon, Velocity_ID));
    lok(!TNECS_ENTITY_HASCOMPONENT(test_world, Perignon, Unit_ID));

    TNECS_REMOVE_COMPONENTS(test_world, Perignon, Velocity_ID);
    lok(!TNECS_ENTITY_HASCOMPONENT(test_world, Perignon, Velocity_ID));
    lok(TNECS_ENTITY_HASCOMPONENT(test_world, Perignon, Position_ID));
    lok(!TNECS_ENTITY_HASCOMPONENT(test_world, Perignon, Unit_ID));

    tnecs_entity Pirou = TNECS_ENTITY_CREATE_wCOMPONENTS(test_world, Position_ID, Velocity_ID, Unit_ID);
    lok(TNECS_ENTITY_HASCOMPONENT(test_world, Pirou, Position_ID));
    lok(TNECS_ENTITY_HASCOMPONENT(test_world, Pirou, Velocity_ID));
    lok(TNECS_ENTITY_HASCOMPONENT(test_world, Pirou, Unit_ID));

    TNECS_REMOVE_COMPONENTS(test_world, Pirou, Position_ID, Velocity_ID);
    lok(TNECS_ENTITY_HASCOMPONENT(test_world, Pirou, Unit_ID));
    lok(!TNECS_ENTITY_HASCOMPONENT(test_world, Pirou, Position_ID));
    lok(!TNECS_ENTITY_HASCOMPONENT(test_world, Pirou, Velocity_ID));
}

void tnecs_test_component_array() {
    int Position_ID = 1;
    int Velocity_ID = 2;
    int Unit_ID     = 3;
    int Sprite_ID   = 4;

    tnecs_world *arr_world = NULL;
    tnecs_world_genesis(&arr_world);
    
    lok(TNECS_REGISTER_COMPONENT(arr_world, Position_ID));
    lok(TNECS_REGISTER_COMPONENT(arr_world, Velocity_ID));
    lok(TNECS_REGISTER_COMPONENT(arr_world, Sprite_ID));
    lok(TNECS_REGISTER_COMPONENT(arr_world, Unit_ID));
    TNECS_REGISTER_SYSTEM(arr_world, SystemMoveDoNothing, 0, 0, Unit_ID); // 4X
    TNECS_REGISTER_SYSTEM(arr_world, SystemMovePhase1, 0, 0, Unit_ID, Velocity_ID);  // 2X
    TNECS_REGISTER_SYSTEM(arr_world, SystemMovePhase2, 0, 0, Unit_ID, Position_ID); // 2X
    TNECS_REGISTER_SYSTEM(arr_world, SystemMovePhase4, 0, 0, Unit_ID, Position_ID, Velocity_ID); // 1X

    tnecs_entity temp_ent = TNECS_ENTITY_CREATE_wCOMPONENTS(arr_world, Unit_ID);
    TNECS_ENTITY_CREATE_wCOMPONENTS(arr_world, Unit_ID, Velocity_ID);
    TNECS_ENTITY_CREATE_wCOMPONENTS(arr_world, Unit_ID, Velocity_ID);
    TNECS_ENTITY_CREATE_wCOMPONENTS(arr_world, Unit_ID, Velocity_ID);
    TNECS_ENTITY_CREATE_wCOMPONENTS(arr_world, Unit_ID, Position_ID);
    TNECS_ENTITY_CREATE_wCOMPONENTS(arr_world, Unit_ID, Position_ID);
    TNECS_ENTITY_CREATE_wCOMPONENTS(arr_world, Unit_ID, Position_ID);
    TNECS_ENTITY_CREATE_wCOMPONENTS(arr_world, Unit_ID, Position_ID);
    TNECS_ENTITY_CREATE_wCOMPONENTS(arr_world, Unit_ID, Position_ID, Velocity_ID);
    TNECS_ENTITY_CREATE_wCOMPONENTS(arr_world, Unit_ID, Position_ID, Velocity_ID);

    size_t temp_archetypeid     = TNECS_COMPONENT_IDS2ARCHETYPEID(arr_world, Unit_ID, Position_ID);

    size_t temp_component_order = tnecs_component_order_bytypeid(arr_world, Position_ID, temp_archetypeid);
    assert(temp_archetypeid > TNECS_NULL);
    assert(arr_world->bytype.components != NULL);
    assert(arr_world->bytype.components[temp_archetypeid] != NULL);

    lok(arr_world->bytype.components[temp_archetypeid][temp_component_order].num == 4);
    temp_component_order = tnecs_component_order_bytypeid(arr_world, Unit_ID, temp_archetypeid);
    lok(arr_world->bytype.components[temp_archetypeid][temp_component_order].num == 4);

    temp_archetypeid = TNECS_COMPONENT_IDS2ARCHETYPEID(arr_world, Unit_ID, Velocity_ID);
    temp_component_order = tnecs_component_order_bytypeid(arr_world, Unit_ID, temp_archetypeid);
    lok(arr_world->bytype.components[temp_archetypeid][temp_component_order].num == 3);
    temp_component_order = tnecs_component_order_bytypeid(arr_world, Velocity_ID, temp_archetypeid);
    lok(arr_world->bytype.components[temp_archetypeid][temp_component_order].num == 3);

    temp_archetypeid = TNECS_COMPONENT_IDS2ARCHETYPEID(arr_world, Unit_ID, Velocity_ID, Position_ID);
    temp_component_order = tnecs_component_order_bytypeid(arr_world, Unit_ID, temp_archetypeid);
    lok(arr_world->bytype.components[temp_archetypeid][temp_component_order].num == 2);
    temp_component_order = tnecs_component_order_bytypeid(arr_world, Position_ID, temp_archetypeid);
    lok(arr_world->bytype.components[temp_archetypeid][temp_component_order].num == 2);
    temp_component_order = tnecs_component_order_bytypeid(arr_world, Velocity_ID, temp_archetypeid);
    lok(arr_world->bytype.components[temp_archetypeid][temp_component_order].num == 2);

    size_t old_entity_order = arr_world->entities.orders[temp_ent];
    lok(old_entity_order == 0);
    lok(arr_world->entities.archetypes[temp_ent] == TNECS_COMPONENT_IDS2ARCHETYPE(Unit_ID));
    size_t old_archetypeid = TNECS_COMPONENT_IDS2ARCHETYPEID(arr_world, Unit_ID);
    lok(arr_world->bytype.num_entities[old_archetypeid] == 1);
    lok(old_archetypeid == Unit_ID);
    size_t old_component_order = tnecs_component_order_bytypeid(arr_world, Unit_ID, old_archetypeid);

    lok(old_component_order < TNECS_COMPONENT_CAP);
    lok(old_component_order == 0);

    lok(arr_world->bytype.components[old_archetypeid][old_component_order].num == 1);

    struct Unit     *temp_unit  = tnecs_get_component(arr_world, temp_ent, Unit_ID);
    struct Position *temp_pos   = tnecs_get_component(arr_world, temp_ent, Position_ID);
    lok(temp_pos == NULL);
    lok(temp_unit->hp   == 0);
    lok(temp_unit->str  == 0);
    temp_unit->hp   = 10;
    temp_unit->str  = 12;
    temp_unit = tnecs_get_component(arr_world, temp_ent + 1, Unit_ID);
    
    tnecs_entities_create(arr_world, 10);
    temp_unit = tnecs_get_component(arr_world, temp_ent, Unit_ID);
    lok(temp_unit->hp   == 10);
    lok(temp_unit->str  == 12);

    size_t new_archetypeid = TNECS_COMPONENT_IDS2ARCHETYPEID(arr_world, Unit_ID, Position_ID);
    lok(arr_world->bytype.num_entities[new_archetypeid] == 4);
    TNECS_ADD_COMPONENT(arr_world, temp_ent, Position_ID);
    
    temp_unit = tnecs_get_component(arr_world, temp_ent, Unit_ID);
    lok(temp_unit->hp   == 10);
    lok(temp_unit->str  == 12);

    lok(arr_world->bytype.num_entities[old_archetypeid] == 0);
    lok(arr_world->entities.archetypes[temp_ent] == TNECS_COMPONENT_IDS2ARCHETYPE(Unit_ID,
            Position_ID));
    lok(arr_world->bytype.num_entities[new_archetypeid] == 5);
    temp_unit = tnecs_get_component(arr_world, temp_ent, Unit_ID);
    lok(temp_unit->hp   == 10);
    lok(temp_unit->str  == 12);
    size_t new_entity_order = arr_world->entities.orders[temp_ent];
    lok(new_entity_order == 4);
    lok(new_entity_order != old_entity_order);

    temp_pos = tnecs_get_component(arr_world, temp_ent, Position_ID);
    temp_unit->hp++;
    temp_unit->str++;
    lok(temp_unit->hp   == 11);
    lok(temp_unit->str  == 13);
    lok(temp_pos->x     == 0);
    lok(temp_pos->y     == 0);

    tnecs_world_destroy(&arr_world);
}

void tnecs_test_world_progress() {
    int Position_ID = 1;
    int Velocity_ID = 2;
    int Unit_ID     = 3;

    struct tnecs_world *inclusive_world = NULL;
    tnecs_world_genesis(&inclusive_world);
    lok(inclusive_world != NULL);


    struct tnecs_world *inclusive_world2 = NULL;
    tnecs_world_genesis(&inclusive_world2);
    lok(inclusive_world2 != NULL);

    struct Velocity *temp_velocity;
    tnecs_entity Perignon = TNECS_ENTITY_CREATE_wCOMPONENTS(test_world, Position_ID, Velocity_ID);
    lok(TNECS_ENTITY_HASCOMPONENT(test_world, Perignon, Position_ID));
    lok(TNECS_ENTITY_HASCOMPONENT(test_world, Perignon, Velocity_ID));
    lok(!TNECS_ENTITY_HASCOMPONENT(test_world, Perignon, Unit_ID));

    lok(test_world->entities.archetypes[Perignon] == (1 + 2));
    lok(test_world->bytype.num_entities[tnecs_archetypeid(test_world, 1 + 2)] == 1);

    temp_position = tnecs_get_component(test_world, Perignon, Position_ID);
    temp_velocity = tnecs_get_component(test_world, Perignon, Velocity_ID);
    temp_position->x = 100;
    temp_position->y = 200;

    TNECS_REGISTER_SYSTEM(test_world, SystemMovePhase1, 1, 1, Position_ID);
    TNECS_REGISTER_SYSTEM(test_world, SystemMovePhase2, 2, 1, Velocity_ID);
    TNECS_REGISTER_SYSTEM(test_world, SystemMovePhase2, 1, 1, Unit_ID);
    TNECS_REGISTER_SYSTEM(test_world, SystemMovePhase4, 4, 1, Velocity_ID);

    lok(test_world->byphase.num == 5);
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
    temp_position = tnecs_get_component(test_world, Perignon, Position_ID);
    temp_velocity = tnecs_get_component(test_world, Perignon, Velocity_ID);

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
    temp_position = tnecs_get_component(test_world, Perignon, Position_ID);
    temp_velocity = tnecs_get_component(test_world, Perignon, Velocity_ID);
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

    lok(test_world->entities.archetypes[Perignon] == (1 + 2));
    lok(test_world->bytype.num_entities[tnecs_archetypeid(test_world, 1 + 2)] == 1);
    tnecs_entity_destroy(test_world, Perignon);

    tnecs_grow_phase(test_world);
    tnecs_grow_system(test_world);
    tnecs_grow_archetype(test_world);

    TNECS_REGISTER_COMPONENT(inclusive_world, Position);
    TNECS_REGISTER_COMPONENT(inclusive_world, Velocity);
    TNECS_REGISTER_COMPONENT(inclusive_world, Sprite);
    TNECS_REGISTER_COMPONENT(inclusive_world, Unit);
    TNECS_REGISTER_SYSTEM(inclusive_world, SystemMoveDoNothing, 0, 0, Unit_ID); // 4X
    TNECS_REGISTER_SYSTEM(inclusive_world, SystemMovePhase1, 0, 0, Unit_ID, Velocity_ID);  // 2X
    TNECS_REGISTER_SYSTEM(inclusive_world, SystemMovePhase2, 0, 0, Unit_ID, Position_ID); // 2X
    TNECS_REGISTER_SYSTEM(inclusive_world, SystemMovePhase4, 0, 0, Unit_ID, Position_ID, Velocity_ID); // 1X

    int SystemMove_ID       = 1;
    int SystemMovePhase1_ID = 2;
    int SystemMovePhase2_ID = 3;
    int SystemMovePhase4_ID = 4;

    lok(TNECS_SYSTEM_ID2ARCHETYPE(inclusive_world, SystemMove_ID) == 4);
    lok(TNECS_SYSTEM_ID2ARCHETYPE(inclusive_world, SystemMovePhase1_ID) == 4 + 2);
    lok(TNECS_SYSTEM_ID2ARCHETYPE(inclusive_world, SystemMovePhase2_ID) == 4 + 1);
    lok(TNECS_SYSTEM_ID2ARCHETYPE(inclusive_world, SystemMovePhase4_ID) == 4 + 2 + 1);

    lok(inclusive_world->bytype.num_archetype_ids[tnecs_archetypeid(inclusive_world,
                                                            TNECS_SYSTEM_ID2ARCHETYPE(inclusive_world, SystemMove_ID))] == 3);
    lok(inclusive_world->bytype.archetype_id[tnecs_archetypeid(inclusive_world,
                                                              TNECS_SYSTEM_ID2ARCHETYPE(inclusive_world, SystemMove_ID))][0] == tnecs_archetypeid(inclusive_world,
                                                                      TNECS_SYSTEM_ID2ARCHETYPE(inclusive_world, SystemMovePhase1_ID)));
    lok(inclusive_world->bytype.archetype_id[tnecs_archetypeid(inclusive_world,
                                                              TNECS_SYSTEM_ID2ARCHETYPE(inclusive_world, SystemMove_ID))][1] == tnecs_archetypeid(inclusive_world,
                                                                      TNECS_SYSTEM_ID2ARCHETYPE(inclusive_world, SystemMovePhase2_ID)));
    lok(inclusive_world->bytype.archetype_id[tnecs_archetypeid(inclusive_world,
                                                              TNECS_SYSTEM_ID2ARCHETYPE(inclusive_world, SystemMove_ID))][2] == tnecs_archetypeid(inclusive_world,
                                                                      TNECS_SYSTEM_ID2ARCHETYPE(inclusive_world, SystemMovePhase4_ID)));
    lok(inclusive_world->bytype.num_archetype_ids[tnecs_archetypeid(inclusive_world,
                                                            TNECS_SYSTEM_ID2ARCHETYPE(inclusive_world, SystemMovePhase1_ID))] == 1);
    lok(inclusive_world->bytype.archetype_id[tnecs_archetypeid(inclusive_world,
                                                              TNECS_SYSTEM_ID2ARCHETYPE(inclusive_world,
                                                                      SystemMovePhase1_ID))][0] == tnecs_archetypeid(inclusive_world,
                                                                              TNECS_SYSTEM_ID2ARCHETYPE(inclusive_world, SystemMovePhase4_ID)));
    lok(inclusive_world->bytype.num_archetype_ids[tnecs_archetypeid(inclusive_world,
                                                            TNECS_SYSTEM_ID2ARCHETYPE(inclusive_world, SystemMovePhase2_ID))] == 1);
    lok(inclusive_world->bytype.archetype_id[tnecs_archetypeid(inclusive_world,
                                                              TNECS_SYSTEM_ID2ARCHETYPE(inclusive_world,
                                                                      SystemMovePhase2_ID))][0] == tnecs_archetypeid(inclusive_world,
                                                                              TNECS_SYSTEM_ID2ARCHETYPE(inclusive_world, SystemMovePhase4_ID)));
    lok(inclusive_world->bytype.num_archetype_ids[tnecs_archetypeid(inclusive_world,
                                                            TNECS_SYSTEM_ID2ARCHETYPE(inclusive_world, SystemMovePhase4_ID))] == 0);

    lok(inclusive_world->bytype.num == 8);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit_ID);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit_ID);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit_ID);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit_ID);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit_ID);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit_ID);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit_ID);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit_ID);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit_ID, Velocity_ID);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit_ID);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit_ID, Position_ID);
    tnecs_entity temp_todestroy = TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit_ID, Position_ID);
    tnecs_entity_destroy(inclusive_world, temp_todestroy);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit_ID, Position_ID);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit_ID, Position_ID);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit_ID, Position_ID);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit_ID, Velocity_ID);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit_ID, Position_ID);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit_ID, Position_ID, Velocity_ID);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit_ID, Position_ID, Velocity_ID);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit_ID, Position_ID);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit_ID, Position_ID, Velocity_ID);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit_ID, Position_ID, Velocity_ID);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit_ID, Position_ID, Velocity_ID);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit_ID, Position_ID, Velocity_ID);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit_ID, Position_ID, Velocity_ID);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit_ID, Position_ID, Velocity_ID);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit_ID, Position_ID, Velocity_ID);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit_ID, Position_ID, Velocity_ID);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit_ID, Position_ID, Velocity_ID);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world, Unit_ID, Position_ID, Velocity_ID);
    lok(inclusive_world->bytype.num_entities[tnecs_archetypeid(inclusive_world,
                                                              TNECS_SYSTEM_ID2ARCHETYPE(inclusive_world, SystemMove_ID))] == 9);
    lok(inclusive_world->bytype.num_entities[tnecs_archetypeid(inclusive_world,
                                                              TNECS_SYSTEM_ID2ARCHETYPE(inclusive_world, SystemMovePhase1_ID))] == 2);
    lok(inclusive_world->bytype.num_entities[tnecs_archetypeid(inclusive_world,
                                                              TNECS_SYSTEM_ID2ARCHETYPE(inclusive_world, SystemMovePhase2_ID))] == 6);
    lok(inclusive_world->bytype.num_entities[tnecs_archetypeid(inclusive_world,
                                                              TNECS_SYSTEM_ID2ARCHETYPE(inclusive_world, SystemMovePhase4_ID))] == 12);
    lok(inclusive_world->bytype.num == 8);
    tnecs_world_step(inclusive_world, 1, NULL);

    lok(inclusive_world->systems_torun.num == 9);
    torun_arr = inclusive_world->systems_torun.arr;
    lok(torun_arr[0] == &SystemMoveDoNothing);
    lok(torun_arr[1] == &SystemMoveDoNothing);
    lok(torun_arr[2] == &SystemMoveDoNothing);
    lok(torun_arr[3] == &SystemMoveDoNothing);
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

    TNECS_REGISTER_COMPONENT(inclusive_world2, Position);
    TNECS_REGISTER_COMPONENT(inclusive_world2, Velocity);
    TNECS_REGISTER_COMPONENT(inclusive_world2, Unit);
    TNECS_REGISTER_COMPONENT(inclusive_world2, Sprite);

    TNECS_REGISTER_SYSTEM(inclusive_world2, SystemMoveDoNothing, 2, 0, Unit_ID); // 4X
    TNECS_REGISTER_SYSTEM(inclusive_world2, SystemMovePhase1, 1, 0, Unit_ID, Velocity_ID);  // 2X
    TNECS_REGISTER_SYSTEM(inclusive_world2, SystemMovePhase2, 4, 0, Unit_ID, Position_ID); // 2X
    TNECS_REGISTER_SYSTEM(inclusive_world2, SystemMovePhase4, 3, 0, Unit_ID, Position_ID, Velocity_ID); // 1X

    lok(TNECS_SYSTEM_ID2ARCHETYPE(inclusive_world, SystemMove_ID) == 4);
    lok(TNECS_SYSTEM_ID2ARCHETYPE(inclusive_world, SystemMovePhase1_ID) == 4 + 2);
    lok(TNECS_SYSTEM_ID2ARCHETYPE(inclusive_world, SystemMovePhase2_ID) == 4 + 1);
    lok(TNECS_SYSTEM_ID2ARCHETYPE(inclusive_world, SystemMovePhase4_ID) == 4 + 2 + 1);

    lok(inclusive_world->bytype.num_archetype_ids[tnecs_archetypeid(inclusive_world2,
                                                            TNECS_SYSTEM_ID2ARCHETYPE(inclusive_world2, SystemMove_ID))] == 3);
    lok(inclusive_world->bytype.num_archetype_ids[tnecs_archetypeid(inclusive_world2,
                                                            TNECS_SYSTEM_ID2ARCHETYPE(inclusive_world2, SystemMovePhase1_ID))] == 1);
    lok(inclusive_world->bytype.num_archetype_ids[tnecs_archetypeid(inclusive_world2,
                                                            TNECS_SYSTEM_ID2ARCHETYPE(inclusive_world2, SystemMovePhase2_ID))] == 1);
    lok(inclusive_world->bytype.num_archetype_ids[tnecs_archetypeid(inclusive_world2,
                                                            TNECS_SYSTEM_ID2ARCHETYPE(inclusive_world2, SystemMovePhase4_ID))] == 0);

    lok(inclusive_world2->bytype.num == 8);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world2, Unit_ID);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world2, Unit_ID, Velocity_ID);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world2, Unit_ID, Position_ID);
    TNECS_ENTITY_CREATE_wCOMPONENTS(inclusive_world2, Unit_ID, Position_ID, Velocity_ID);
    lok(inclusive_world2->bytype.num == 8);
    tnecs_world_step(inclusive_world2, 1, NULL);

    lok(inclusive_world2->systems_torun.num == 9);
    torun_arr = inclusive_world2->systems_torun.arr; 
    lok(torun_arr[0]  == &SystemMovePhase1);
    lok(torun_arr[1]  == &SystemMovePhase1);
    lok(torun_arr[2]  == &SystemMoveDoNothing);
    lok(torun_arr[3]  == &SystemMoveDoNothing);
    lok(torun_arr[4]  == &SystemMoveDoNothing);
    lok(torun_arr[5]  == &SystemMoveDoNothing);
    lok(torun_arr[6]  == &SystemMovePhase4);
    lok(torun_arr[7]  == &SystemMovePhase2);
    lok(torun_arr[8]  == &SystemMovePhase2);
    lok(torun_arr[9]  == NULL);
    lok(torun_arr[10] == NULL);
    lok(torun_arr[11] == NULL);
    lok(torun_arr[12] == NULL);
    lok(torun_arr[13] == NULL);
    lok(torun_arr[14] == NULL);
    lok(torun_arr[15] == NULL);

    tnecs_world_destroy(&inclusive_world2);
    tnecs_world_destroy(&inclusive_world);
}

void tnecs_test_grow() {
    struct tnecs_world *grow_world = NULL;
    tnecs_world_genesis(&grow_world);
    lok(grow_world != NULL);

    lok(grow_world != NULL);

    lok(grow_world->entities.len    == TNECS_INIT_ENTITY_LEN);
    lok(grow_world->bytype.len      == TNECS_INIT_SYSTEM_LEN);
    lok(grow_world->bytype.num      == 1);
    lok(grow_world->systems.len     == TNECS_INIT_SYSTEM_LEN);
    lok(grow_world->systems.num     == 1);
    lok(grow_world->byphase.len     == TNECS_INIT_PHASE_LEN);
    lok(grow_world->byphase.num     == 1);
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

    lok(grow_world->byphase.num == 1);
    for (size_t i = TNECS_INIT_PHASE_LEN; i < grow_world->byphase.len; i++) {
        lok(grow_world->byphase.num_systems[i] == 0);
        lok(grow_world->byphase.len_systems[i] == TNECS_INIT_PHASE_LEN);
        for (size_t j = 0; j < grow_world->byphase.len_systems[i]; j++) {
            lok(grow_world->byphase.systems[i][j] == 0);
        }
    }

    tnecs_world_destroy(&grow_world);
}

void tnecs_benchmarks(uint64_t num) {
    // printf("tnecs_benchmarks \n");
    u64 t_0;
    u64 t_1;

    uint32_t rand1 = rand() % 100;
    uint32_t rand2 = rand() % 100;

    dupprintf(globalf, " %8llu\t", num);
    t_0 = tnecs_get_us();
    tnecs_world *bench_world = NULL;
    tnecs_world_genesis(&bench_world);
    t_1 = tnecs_get_us();
    dupprintf(globalf, "%7llu\t", t_1 - t_0);

    t_0 = tnecs_get_us();
    TNECS_REGISTER_COMPONENT(bench_world, Position2);
    TNECS_REGISTER_COMPONENT(bench_world, Unit2);
    t_1 = tnecs_get_us();
    dupprintf(globalf, "%7llu\t", t_1 - t_0);

    int Position2_ID    = 1;
    int Unit2_ID        = 2;
    for (uint64_t i = 0; i < num; i++) {
        TNECS_ENTITY_CREATE_wCOMPONENTS(bench_world, Position2_ID, Unit2_ID);
    }

    t_0 = tnecs_get_us();
    TNECS_REGISTER_SYSTEM(bench_world, SystemMove2, 0, 0, Position2_ID, Unit2_ID);
    t_1 = tnecs_get_us();

    dupprintf(globalf, "%7llu\t", t_1 - t_0);

    t_0 = tnecs_get_us();
    for (size_t i = 0; i < ITERATIONS; i++) {
        test_entities[i] = tnecs_entity_create(bench_world);
    }

    t_1 = tnecs_get_us();
    dupprintf(globalf, "%7llu\t", t_1 - t_0);

    t_0 = tnecs_get_us();
    for (size_t i = 0; i < ITERATIONS; i++) {
        tnecs_entity ent = test_entities[i] * (rand2 + 1) % num;
        tnecs_entity_destroy(bench_world, ent);
    }
    t_1 = tnecs_get_us();
    dupprintf(globalf, "%7llu\t", t_1 - t_0);

    for (size_t i = 0; i < ITERATIONS; i++) {
        // TODO: benchmark reuse or not
        test_entities[i] = tnecs_entity_create(bench_world);
    }

    t_0 = tnecs_get_us();
    TNECS_ADD_COMPONENT(bench_world, test_entities[1], Position2_ID);
    TNECS_ADD_COMPONENT(bench_world, test_entities[1], Unit2_ID);
    for (size_t i = 2; i < ITERATIONS; i++) {
        tnecs_entity ent = test_entities[i] * (rand1 + 2) % num;
        TNECS_ADD_COMPONENT(bench_world, ent, Position2_ID, false);
        TNECS_ADD_COMPONENT(bench_world, ent, Unit2_ID, false);
    }
    t_1 = tnecs_get_us();
    dupprintf(globalf, "%7llu\t", t_1 - t_0);

    t_0 = tnecs_get_us();
    for (size_t i = 0; i < ITERATIONS; i++) {
        TNECS_ENTITY_CREATE_wCOMPONENTS(bench_world, Position2_ID);
    }
    t_1 = tnecs_get_us();
    dupprintf(globalf, "%7llu\t", t_1 - t_0);

    tnecs_entity tnecs_entities2[ITERATIONS];
    t_0 = tnecs_get_us();
    for (size_t i = 0; i < ITERATIONS; i++) {
        TNECS_ENTITY_CREATE_wCOMPONENTS(bench_world, Position2_ID);
        tnecs_entities2[i] = TNECS_ENTITY_CREATE_wCOMPONENTS(bench_world, Position2_ID, Unit2_ID);
        assert(bench_world->entities.id[tnecs_entities2[i]] == tnecs_entities2[i]);
    }


    t_1 = tnecs_get_us();
    dupprintf(globalf, "%7llu\t", t_1 - t_0);

    for (size_t i = 0; i < ITERATIONS; i++) {
        // TODO: destroy random entity
        assert(bench_world->entities.id[tnecs_entities2[i]] == tnecs_entities2[i]);
        tnecs_entity_destroy(bench_world, tnecs_entities2[i]);
    }

    t_0 = tnecs_get_us();
    for (size_t i = 0; i < ITERATIONS; i++) {
        tnecs_entity ent = test_entities[i] * (rand2 + 5) % num;
        struct Position2    *pos   = tnecs_get_component(bench_world, ent, Position2_ID);
        struct Unit2        *unit  = tnecs_get_component(bench_world, ent, Unit2_ID);
        if (pos && unit) {
            unit->hp = pos->x + pos->y;
        }
    }
    t_1 = tnecs_get_us();
    dupprintf(globalf, "%7llu\t", t_1 - t_0);

    t_0 = tnecs_get_us();
    for (size_t i = 0; i < fps_iterations; i++) {
        tnecs_world_step(bench_world, 1, NULL);
    }
    t_1 = tnecs_get_us();
    dupprintf(globalf, "%7llu\t", t_1 - t_0);

    t_0 = tnecs_get_us();
    tnecs_world_destroy(&bench_world);
    t_1 = tnecs_get_us();
    dupprintf(globalf, "%7llu\n", t_1 - t_0);
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
    dupprintf(globalf, "\n --- tnecs test start ---\n");
    lrun("utilities",  tnecs_test_utilities);
    lrun("log2",       test_log2);
    lrun("c_regis",    tnecs_test_component_registration);
    lrun("s_regis",    tnecs_test_system_registration);
    lrun("e_create",   tnecs_test_entity_creation);
    lrun("e_destroy",  tnecs_test_entity_destroy);
    lrun("c_add",      tnecs_test_component_add);
    lrun("c_remove",   tnecs_test_component_remove);
    lrun("c_array",    tnecs_test_component_array);
    lrun("grow",       tnecs_test_grow);
    lrun("progress",   tnecs_test_world_progress);
    lresults();

    dupprintf(globalf, "\n --- Notes ---\n");
    dupprintf(globalf, "world size: %ld bytes\n", sizeof(struct tnecs_world));
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

    tnecs_world_destroy(&test_world);
    dupprintf(globalf, "\n --- tnecs test end ---\n\n");
    fclose(globalf);
    return (0);
}
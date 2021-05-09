#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <string.h>

#include "us_clock.h"
#include "tnecs.h"
#include "flecs.h"

/* MINCTEST - Minimal C Test Library - 0.2.0
*******************************MODIFIED FOR TNECS*************************
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

/* How far apart can floats be before we consider them unequal. */
#ifndef LTEST_FLOAT_TOLERANCE
#define LTEST_FLOAT_TOLERANCE 0.001
#endif

/* Track the number of passes, fails. */
/* NB this is made for all tests to be in one file. */
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

/* Prototype to assert equal. */
#define lequal_base(equality, a, b, format) do {\
    ++ltests;\
    if (!(equality)) {\
        ++lfails;\
        dupprintf(globalf,"%s:%d ("format " != " format")\n", __FILE__, __LINE__, (a), (b));\
    }} while (0)

/* Assert two integers are equal. */
#define lequal(a, b)\
    lequal_base((a) == (b), a, b, "%d")

/* Assert two floats are equal (Within LTEST_FLOAT_TOLERANCE). */
#define lfequal(a, b)\
    lequal_base(fabs((double)(a)-(double)(b)) <= LTEST_FLOAT_TOLERANCE\
     && fabs((double)(a)-(double)(b)) == fabs((double)(a)-(double)(b)), (double)(a), (double)(b), "%f")

/* Assert two strings are equal. */
#define lsequal(a, b)\
    lequal_base(strcmp(a, b) == 0, a, b, "%s")

#endif /*__MINCTEST_H__*/

void dupprintf(FILE * f, char const * fmt, ...) { // duplicate printf
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
    uint32_t hp;
    uint32_t str;
} Unit;


typedef struct Sprite {
    uint32_t texture;
    bool isAnimated;
} Sprite;

struct Unit Unit_default = {
    .hp = 0,
    .str = 0,
};

typedef struct Position2 {
    uint32_t x;
    uint32_t y;
} Position2;

typedef struct Unit2 {
    uint32_t hp;
    uint32_t str;
} Unit2;


void tnecs_Move(struct tnecs_System_Input in_input) {
    // Position *p = TNECS_COMPONENTS_LIST(entity_list, Position);
    // Unit *v = TNECS_COMPONENTS_LIST(entity_list, Unit);

    // for (int i = 0; i < entity_num; i++) {
    //     p[i].x += 2;
    //     p[i].y += 4;
    // }
}

void flecs_Move(ecs_iter_t * it) {
    // Position *p = TNECS_COMPONENTS_LIST(entity_list, Position);
    // Unit *v = TNECS_COMPONENTS_LIST(entity_list, Unit);

    // for (int i = 0; i < entity_num; i++) {
    //     p[i].x += 2;
    //     p[i].y += 4;
    // }
}

/*****************************TEST GLOBALS*****************************/
FILE * globalf;
/*****************************TEST CONSTANTS***************************/
#define ITERATIONS 10000
#define ITERATIONS_SMALL 1000
#define ARRAY_LEN 100

/*******************************TEST SYSTEMS***************************/
tnecs_entity_t tnecs_entities[ITERATIONS];
struct Unit unit_array[ARRAY_LEN];
tnecs_entity_t * components_list;
struct Position * temp_position;
struct Unit * temp_unit;
struct tnecs_World * test_world;


void flecs_benchmarks() {
    dupprintf(globalf, "\nHomemade flecs benchmarks\n");
    double t_0;
    double t_1;
    t_0 = get_us();
    ecs_world_t * world = ecs_init();
    t_1 = get_us();
    dupprintf(globalf, "flecs: world init\n");
    dupprintf(globalf, "%.1f [us] \n\n", t_1 - t_0);

    t_0 = get_us();
    ecs_entity_t flecs_entities[ITERATIONS];
    ecs_entity_t flecs_temp_ent;
    for (size_t i = 0; i < ITERATIONS; i++) {
        flecs_temp_ent = ecs_new(world, 0);
        flecs_entities[i] = flecs_temp_ent;
    }
    t_1 = get_us();
    dupprintf(globalf, "flecs: Entity Creation time: %d iterations \n", ITERATIONS);
    dupprintf(globalf, "%.1f [us] \n\n", t_1 - t_0);


    t_0 = get_us();
    ECS_COMPONENT(world, Position);
    ECS_COMPONENT(world, Unit);
    t_1 = get_us();
    dupprintf(globalf, "flecs: Component registration \n");
    dupprintf(globalf, "%.1f [us] \n\n", t_1 - t_0);

    t_0 = get_us();
    for (size_t i = 0; i < ITERATIONS; i++) {
        ecs_add(world, flecs_entities[i], Position);
        ecs_add(world, flecs_entities[i], Unit);
    }
    t_1 = get_us();
    dupprintf(globalf, "flecs: Adding components: %d iterations \n", ITERATIONS);
    dupprintf(globalf, "%.1f [us] \n\n", t_1 - t_0);

    t_0 = get_us();
    for (size_t i = 0; i < ITERATIONS; i++) {
        flecs_temp_ent = ecs_new(world, Position);
    }
    t_1 = get_us();
    dupprintf(globalf, "flecs: Creating entities with a single component: %d iterations \n", ITERATIONS);
    dupprintf(globalf, "%.1f [us] \n\n", t_1 - t_0);

    t_0 = get_us();
    ECS_SYSTEM(world, flecs_Move, EcsOnUpdate, Position, Unit);
    t_1 = get_us();
    dupprintf(globalf, "flecs: System registration\n");
    dupprintf(globalf, "%.1f [us] \n\n", t_1 - t_0);

    t_0 = get_us();
    const Position * position_ptr;
    const Unit * unit_ptr;
    for (size_t i = 0; i < ITERATIONS; i++) {
        position_ptr = ecs_get(world, flecs_entities[i], Position);
        unit_ptr = ecs_get(world, flecs_entities[i], Unit);
    }
    t_1 = get_us();
    dupprintf(globalf, "flecs: get component const: %d iterations \n", ITERATIONS);
    dupprintf(globalf, "%.1f [us] \n\n", t_1 - t_0);

    t_0 = get_us();
    Position * position_mptr;
    Unit * unit_mptr;
    for (size_t i = 0; i < ITERATIONS; i++) {
        position_mptr = ecs_get_mut(world, flecs_entities[i], Position, NULL);
        unit_mptr = ecs_get_mut(world, flecs_entities[i], Unit, NULL);
        ecs_modified(world, flecs_entities[i], Unit);
        ecs_modified(world, flecs_entities[i], Position);
    }
    t_1 = get_us();
    dupprintf(globalf, "flecs: get component mut: %d iterations \n", ITERATIONS);
    dupprintf(globalf, "%.1f [us] \n\n", t_1 - t_0);


    t_0 = get_us();
    for (size_t i = 0; i < ITERATIONS; i++) {
        ecs_progress(world, 0);
    }
    t_1 = get_us();
    dupprintf(globalf, "flecs: progress the world: %d iterations \n", ITERATIONS);
    dupprintf(globalf, "%.1f [us] \n\n", t_1 - t_0);

    t_0 = get_us();
    ecs_fini(world);
    t_1 = get_us();
    dupprintf(globalf, "flecs: world deinit\n");
    dupprintf(globalf, "%.1f [us] \n\n", t_1 - t_0);
}


void tnecs_benchmarks() {
    dupprintf(globalf, "\nHomemade tnecs benchmarks\n");
    double t_0;
    double t_1;

    t_0 = get_us();
    struct tnecs_World * bench_world = tnecs_init();
    t_1 = get_us();
    dupprintf(globalf, "tnecs: World Creation time \n");
    dupprintf(globalf, "%.1f [us] \n\n", t_1 - t_0);

    t_0 = get_us();
    tnecs_entity_t tnecs_temp_ent;
    for (size_t i = 0; i < ITERATIONS; i++) {
        // printf("i %d \n", i);
        tnecs_temp_ent = tnecs_new_entity(bench_world);
        tnecs_entities[i] = tnecs_temp_ent;
    }
    t_1 = get_us();
    dupprintf(globalf, "tnecs: Entity Creation time: %d iterations \n", ITERATIONS);
    dupprintf(globalf, "%.1f [us] \n\n", t_1 - t_0);

    t_0 = get_us();
    TNECS_REGISTER_COMPONENT(bench_world, Position2);
    TNECS_REGISTER_COMPONENT(bench_world, Unit2);
    t_1 = get_us();
    dupprintf(globalf, "tnecs: Component Registration \n");
    dupprintf(globalf, "%.1f [us] \n\n", t_1 - t_0);


    t_0 = get_us();
    TNECS_ADD_COMPONENT(bench_world, tnecs_entities[1], Position2);
    TNECS_ADD_COMPONENT(bench_world, tnecs_entities[1], Unit2);
    for (size_t i = 2; i < ITERATIONS; i++) {
        printf("i %d\n", i);
        TNECS_ADD_COMPONENT(bench_world, tnecs_entities[i], false, Position2);
        TNECS_ADD_COMPONENT(bench_world, tnecs_entities[i], false, Unit2);
    }
    t_1 = get_us();
    dupprintf(globalf, "tnecs: Component adding time: %d iterations \n", ITERATIONS);
    dupprintf(globalf, "%.1f [us] \n\n", t_1 - t_0);

}


int main() {
    globalf = fopen("test_flecs.txt", "w+");

    flecs_benchmarks();
    tnecs_benchmarks();
    dupprintf(globalf, "tnecs Test End \n \n");
    return (0);
}
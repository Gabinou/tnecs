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


/*****************************TEST GLOBALS*****************************/
FILE * globalf;
/*****************************TEST CONSTANTS***************************/
#define ITERATIONS 100000
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

}


void tnecs_benchmarks() {
    dupprintf(globalf, "\nHomemade tnecs benchmarks\n");
    double t_0;
    double t_1;

    // struct Unit_Hash * unit_hash = NULL;

    // t_0 = get_us();
    // for (size_t i = 0; i < ARRAY_LEN; i++) {
    //     unit_array[i].hp = Unit_default.hp;
    //     unit_array[i].str = Unit_default.str;
    // }
    // t_1 = get_us();
    // dupprintf(globalf,"unit_array init: %d iterations \n", ARRAY_LEN);
    // dupprintf(globalf,"%.1f [us] \n", t_1 - t_0);

    // struct Unit temp_unitnp;
    // t_0 = get_us();
    // for (size_t i = 0; i < ARRAY_LEN; i++) {
    //     temp_unitnp.hp = i;
    //     temp_unitnp.str = i * 2;
    //     hmput(unit_hash, i, temp_unitnp);
    // }
    // t_1 = get_us();
    // dupprintf(globalf,"unit_hash init: %d iterations \n", ARRAY_LEN);
    // dupprintf(globalf,"%.1f [us] \n", t_1 - t_0);

    // t_0 = get_us();
    // for (size_t i = 0; i < ITERATIONS; i++) {
    //     unit_array[(i % ARRAY_LEN)].hp++;
    //     unit_array[(i % ARRAY_LEN)].str++;
    // }
    // t_1 = get_us();
    // dupprintf(globalf,"unit_array operations: %d iterations \n", ITERATIONS);
    // dupprintf(globalf,"%.1f [us] \n", t_1 - t_0);



    uint64_t res_hash;
    t_0 = get_us();
    for (size_t i = 0; i < ITERATIONS; i++) {
        res_hash = hash_djb2("Position");
        res_hash = hash_djb2("Unit");
    }
    t_1 = get_us();
    dupprintf(globalf, "hash_djb2: %d iterations \n", ITERATIONS);
    dupprintf(globalf, "%.1f [us] \n", t_1 - t_0);

    t_0 = get_us();
    for (size_t i = 0; i < ITERATIONS; i++) {
        res_hash = hash_sdbm("Unit");
        res_hash = hash_sdbm("Position");
    }
    t_1 = get_us();
    dupprintf(globalf, "hash_sdbm: %d iterations \n", ITERATIONS);
    dupprintf(globalf, "%.1f [us] \n", t_1 - t_0);


    // t_0 = get_us();
    // size_t index;
    // for (size_t i = 0; i < ITERATIONS; i++) {
    //     index = (i % ARRAY_LEN);
    //     temp_unitnp = hmget(unit_hash, index);
    //     temp_unitnp.hp++;
    //     temp_unitnp.str++;
    //     hmput(unit_hash, index, temp_unitnp);
    // }
    // t_1 = get_us();
    // dupprintf(globalf,"unit_hash operations: %d iterations \n", ITERATIONS);
    // dupprintf(globalf,"%.1f [us] \n", t_1 - t_0);

    t_0 = get_us();
    struct tnecs_World * bench_world = tnecs_init();
    t_1 = get_us();
    dupprintf(globalf, "tnecs: World Creation time \n");
    dupprintf(globalf, "%.1f [us] \n", t_1 - t_0);

    t_0 = get_us();
    TNECS_REGISTER_COMPONENT(bench_world, Position2);
    TNECS_REGISTER_COMPONENT(bench_world, Unit2);
    t_1 = get_us();
    dupprintf(globalf, "tnecs: Component Registration \n");
    dupprintf(globalf, "%.1f [us] \n", t_1 - t_0);

    t_0 = get_us();
    tnecs_entity_t tnecs_temp_ent;
    for (size_t i = 0; i < ITERATIONS; i++) {
        tnecs_temp_ent = tnecs_new_entity(bench_world);
        tnecs_entities[i] = tnecs_temp_ent;
    }
    t_1 = get_us();
    dupprintf(globalf, "tnecs: Entity Creation time: %d iterations \n", ITERATIONS);
    dupprintf(globalf, "%.1f [us] \n", t_1 - t_0);

    t_0 = get_us();
    TNECS_ADD_COMPONENT(bench_world, tnecs_entities[1], Position2);
    TNECS_ADD_COMPONENT(bench_world, tnecs_entities[1], Unit2);
    for (size_t i = 2; i < ITERATIONS; i++) {
        TNECS_ADD_COMPONENT(bench_world, tnecs_entities[i], false, Position2);
        TNECS_ADD_COMPONENT(bench_world, tnecs_entities[i], false, Unit2);
    }
    t_1 = get_us();
    dupprintf(globalf, "tnecs: Component adding time: %d iterations \n", ITERATIONS);
    dupprintf(globalf, "%.1f [us] \n", t_1 - t_0);

}


int main() {
    globalf = fopen("flecs_benchmarks.txt", "w+");

    tnecs_benchmarks();
    flecs_benchmarks();
    dupprintf(globalf, "tnecs Test End \n \n");
    return (0);
}
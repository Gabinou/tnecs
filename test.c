#include <stdio.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <string.h>

#include "us_clock.h"
#include "tnecs.h"

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

typedef struct Velocity {
    uint64_t vx;
    uint64_t vy;
} Velocity;


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
struct Sprite * temp_sprite;
struct tnecs_World * test_world;

void SystemMove(struct tnecs_System_Input * in_input) {
    // printf("SystemMove\n");
    struct Position * p = TNECS_COMPONENTS_LIST(in_input, Position);
    struct Velocity * v = TNECS_COMPONENTS_LIST(in_input, Velocity);
    for (int i = 0; i < in_input->num_entities; i++) {
        p[i].x = p[i].x + v[i].vx;
        p[i].y = p[i].y + v[i].vy;
    }
}

/*******************************ACTUAL TESTS***************************/
void tnecs_test_utilities() {
    lok(TNECS_COMPONENT_TYPE2ID(1) == 1);
    lok(TNECS_COMPONENT_TYPE2ID(2) == 2);
    lok(TNECS_COMPONENT_TYPE2ID(4) == 3);
    lok(TNECS_COMPONENT_TYPE2ID(8) == 4);
    lok(TNECS_COMPONENT_TYPE2ID(16) == 5);
    lok(TNECS_COMPONENT_TYPE2ID(32) == 6);
    lok(TNECS_COMPONENT_TYPE2ID(64) == 7);
    lok(TNECS_COMPONENT_TYPE2ID(128) == 8);
    lok(TNECS_COMPONENT_TYPE2ID(256) == 9);

    lok(TNECS_COMPONENT_ID2TYPE(1) == 1);
    lok(TNECS_COMPONENT_ID2TYPE(2) == 2);
    lok(TNECS_COMPONENT_ID2TYPE(3) == 4);
    lok(TNECS_COMPONENT_ID2TYPE(4) == 8);
    lok(TNECS_COMPONENT_ID2TYPE(5) == 16);
    lok(TNECS_COMPONENT_ID2TYPE(6) == 32);


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
}

void tnecs_test_component_registration() {
    TNECS_REGISTER_COMPONENT(test_world, Position);
    size_t temp_comp_flag = 1;
    size_t temp_comp_id = 1;
    size_t temp_comp_order = 0;
    size_t temp_typeflag_id = 1;
    size_t temp_typeflag = 1;
    lok(TNECS_COMPONENT_NAME2ID(test_world, Position) == temp_comp_id);
    lok(TNECS_COMPONENT_ID2TYPEFLAG(temp_comp_id) == temp_typeflag);
    lok(test_world->components_idbytype[temp_comp_id][temp_comp_order] == temp_comp_id);
    lok(test_world->components_flagbytype[temp_comp_id][temp_comp_order] == temp_comp_flag);
    lok(test_world->component_hashes[TNECS_COMPONENT_NAME2ID(test_world, Position)] == TNECS_HASH("Position"));
    lok(test_world->typeflags[0] == 0);
    lok(test_world->typeflags[1] == (TNECS_NULLSHIFT << 0));
    lok(test_world->typeflags[1] == temp_comp_flag);
    lok(test_world->num_components == 2);
    lok(TNECS_COMPONENT_TYPE(test_world, Position) == (TNECS_NULLSHIFT << 0));
    lok(TNECS_COMPONENT_TYPE(test_world, Position) == test_world->typeflags[temp_typeflag_id]);

    TNECS_REGISTER_COMPONENT(test_world, Unit);
    temp_comp_flag = 2;
    temp_comp_id = 2;
    temp_comp_order = 0;
    temp_typeflag_id = 2;
    temp_typeflag = 2;
    lok(TNECS_COMPONENT_NAME2ID(test_world, Unit) == temp_comp_id);
    lok(TNECS_COMPONENT_ID2TYPEFLAG(temp_comp_id) == temp_typeflag);
    lok(test_world->components_idbytype[temp_comp_id][temp_comp_order] == temp_comp_id);
    lok(test_world->components_flagbytype[temp_comp_id][temp_comp_order] == temp_comp_flag);
    lok(test_world->component_hashes[TNECS_COMPONENT_NAME2ID(test_world, Unit)] == TNECS_HASH("Unit"));
    lok(test_world->typeflags[0] == 0);
    lok(test_world->typeflags[1] == (TNECS_NULLSHIFT << 0));
    lok(test_world->typeflags[2] == (TNECS_NULLSHIFT << 1));
    lok(test_world->typeflags[2] == temp_comp_flag);
    lok(test_world->num_components == 3);
    lok(TNECS_COMPONENT_TYPE(test_world, Unit) == (TNECS_NULLSHIFT << 1));
    lok(TNECS_COMPONENT_TYPE(test_world, Unit) == test_world->typeflags[temp_typeflag_id]);
    lok(test_world->component_hashes[TNECS_COMPONENT_NAME2ID(test_world, Unit)] == TNECS_HASH("Unit"));

    TNECS_REGISTER_COMPONENT(test_world, Sprite);
    temp_comp_flag = 4;
    temp_comp_id = 3;
    temp_comp_order = 0;
    temp_typeflag_id = 3;
    temp_typeflag = 4;
    lok(TNECS_COMPONENT_NAME2ID(test_world, Sprite) == temp_comp_id);
    lok(TNECS_COMPONENT_ID2TYPEFLAG(temp_comp_id) == temp_typeflag);
    lok(test_world->components_idbytype[temp_comp_id][temp_comp_order] == temp_comp_id);
    lok(test_world->components_flagbytype[temp_comp_id][temp_comp_order] == temp_comp_flag);
    lok(test_world->component_hashes[TNECS_COMPONENT_NAME2ID(test_world, Sprite)] == TNECS_HASH("Sprite"));
    lok(test_world->typeflags[0] == 0);
    lok(test_world->typeflags[1] == (TNECS_NULLSHIFT << 0));
    lok(test_world->typeflags[2] == (TNECS_NULLSHIFT << 1));
    lok(test_world->typeflags[3] == (TNECS_NULLSHIFT << 2));
    lok(test_world->typeflags[3] == temp_comp_flag);
    lok(test_world->num_components == 4);
    lok(TNECS_COMPONENT_TYPE(test_world, Sprite) == (TNECS_NULLSHIFT << 2));
    lok(TNECS_COMPONENT_TYPE(test_world, Sprite) == test_world->typeflags[temp_typeflag_id]);
    lok(test_world->component_hashes[TNECS_COMPONENT_NAME2ID(test_world, Sprite)] == TNECS_HASH("Sprite"));

    TNECS_REGISTER_COMPONENT(test_world, Velocity);
    temp_comp_flag = 8;
    temp_comp_id = 4;
    temp_comp_order = 0;
    temp_typeflag_id = 4;
    temp_typeflag = 8;
    lok(TNECS_COMPONENT_NAME2ID(test_world, Velocity) == temp_comp_id);
    lok(TNECS_COMPONENT_ID2TYPEFLAG(temp_comp_id) == temp_typeflag);
    lok(test_world->components_idbytype[temp_comp_id][temp_comp_order] == temp_comp_id);
    lok(test_world->components_flagbytype[temp_comp_id][temp_comp_order] == temp_comp_flag);
    lok(test_world->component_hashes[TNECS_COMPONENT_NAME2ID(test_world, Velocity)] == TNECS_HASH("Velocity"));
    lok(test_world->typeflags[0] == 0);
    lok(test_world->typeflags[1] == (TNECS_NULLSHIFT << 0));
    lok(test_world->typeflags[2] == (TNECS_NULLSHIFT << 1));
    lok(test_world->typeflags[3] == (TNECS_NULLSHIFT << 2));
    lok(test_world->typeflags[4] == (TNECS_NULLSHIFT << 3));
    lok(test_world->typeflags[4] == temp_comp_flag);
    lok(test_world->num_components == 5);
    lok(TNECS_COMPONENT_TYPE(test_world, Velocity) == (TNECS_NULLSHIFT << 3));
    lok(TNECS_COMPONENT_TYPE(test_world, Velocity) == test_world->typeflags[temp_typeflag_id]);
    lok(test_world->component_hashes[TNECS_COMPONENT_NAME2ID(test_world, Velocity)] == TNECS_HASH("Velocity"));


}

void tnecs_test_system_registration() {
    TNECS_REGISTER_SYSTEM(test_world, SystemMove, Position, Velocity);
    size_t temp_comp_flag = 1;
    size_t temp_comp_id = 1;
    size_t temp_comp_order = 0;
    size_t temp_typeflag_id = 5;
    size_t temp_typeflag = 1 + 8;

    lok(TNECS_SYSTEM_ID(test_world, SystemMove) == 1);

    lok(TNECS_HASH("SystemMove") == tnecs_hash_djb2("SystemMove"));
    lok(test_world->system_hashes[TNECS_SYSTEM_ID(test_world, SystemMove)] == TNECS_HASH("SystemMove"));

    lok(test_world->components_idbytype[temp_typeflag_id][0] == TNECS_COMPONENT_NAME2ID(test_world, Position));
    lok(test_world->components_idbytype[temp_typeflag_id][1] == TNECS_COMPONENT_NAME2ID(test_world, Velocity));
    lok(test_world->components_flagbytype[temp_typeflag_id][0] == TNECS_COMPONENT_TYPE(test_world, Position));
    lok(test_world->components_flagbytype[temp_typeflag_id][1] == TNECS_COMPONENT_TYPE(test_world, Velocity));
}

void tnecs_test_entity_creation() {
    // dupprintf(globalf, "tnecs_test_entity_creation \n");
    lok(test_world->next_entity_id == TNECS_NULLSHIFT);
    tnecs_entity_t Silou = tnecs_new_entity(test_world);
    lok(Silou == TNECS_NULLSHIFT);
    lok(test_world->next_entity_id == (TNECS_NULLSHIFT + 1));
    tnecs_entity_t Pirou = TNECS_NEW_ENTITY(test_world);
    lok(Pirou == (TNECS_NULLSHIFT + 1));
    lok(test_world->next_entity_id == (TNECS_NULLSHIFT + 2));
    lok(Silou != Pirou);
    tnecs_entity_t Perignon = TNECS_NEW_ENTITY_WCOMPONENTS(test_world, Position, Unit);
    temp_position = TNECS_GET_COMPONENT(test_world, Perignon, Position);
    if (temp_position != NULL) {
        lok(temp_position->x == 0);
        lok(temp_position->y == 0);
        temp_position->x = 3;
        temp_position->y = 6;
    } else {
        lok(false);
    }

    lok(TNECS_COMPONENT_NAME2ID(test_world, Position) == 1);
    lok(TNECS_COMPONENT_NAME2ID(test_world, Unit) == 2);
    lok((TNECS_COMPONENT_NAME2ID(test_world, Unit) + TNECS_COMPONENT_NAME2ID(test_world, Position)) == 3);
    lok(test_world->entity_typeflags[Perignon] == (TNECS_COMPONENT_NAME2ID(test_world, Position) + TNECS_COMPONENT_NAME2ID(test_world, Unit)));

    temp_sprite = TNECS_GET_COMPONENT(test_world, Perignon, Sprite);
    lok(temp_sprite == NULL);
    temp_unit = TNECS_GET_COMPONENT(test_world, Perignon, Unit);
    if (temp_unit != NULL) {
        lok(temp_unit->hp  == 0);
        lok(temp_unit->str == 0);
    } else {
        lok(false);
    }


    tnecs_entity_t Chasse = tnecs_new_entity(test_world);

    tnecs_entity_destroy(test_world, Silou);

}

void tnecs_test_component_add() {

    tnecs_entity_t Silou = tnecs_new_entity(test_world);
    TNECS_ADD_COMPONENT(test_world, Silou, Position);
    lok((test_world->entity_typeflags[Silou] & TNECS_COMPONENT_TYPE(test_world, Unit)) == 0);
    TNECS_ADD_COMPONENT(test_world, Silou, Unit);
    lok((test_world->entity_typeflags[Silou] & TNECS_COMPONENT_TYPE(test_world, Position)) > 0);
    lok((test_world->entity_typeflags[Silou] & TNECS_COMPONENT_TYPE(test_world, Unit)) > 0);
    lok((test_world->entity_typeflags[Silou] & TNECS_COMPONENT_TYPE(test_world, Sprite)) == 0);

    tnecs_entity_t Pirou = tnecs_new_entity(test_world);
    TNECS_ADD_COMPONENT(test_world, Pirou, Position);
    lok((test_world->entity_typeflags[Pirou] & TNECS_COMPONENT_TYPE(test_world, Position)) > 0);
    lok((test_world->entity_typeflags[Pirou] & TNECS_COMPONENT_TYPE(test_world, Unit)) == 0);
    TNECS_ADD_COMPONENT(test_world, Pirou, Unit);
    lok((test_world->entity_typeflags[Pirou] & TNECS_COMPONENT_TYPE(test_world, Unit)) > 0);
    lok((test_world->entity_typeflags[Pirou] & TNECS_COMPONENT_TYPE(test_world, Position)) > 0);
    lok((test_world->entity_typeflags[Pirou] & TNECS_COMPONENT_TYPE(test_world, Sprite)) == 0);


    tnecs_entity_t Chasse = tnecs_new_entity(test_world);
    TNECS_ADD_COMPONENTS(test_world, Chasse, 1, Sprite, Position);
    lok((test_world->entity_typeflags[Chasse] & TNECS_COMPONENT_TYPE(test_world, Unit)) == 0);
    lok((test_world->entity_typeflags[Chasse] & TNECS_COMPONENT_TYPE(test_world, Sprite)) > 0);
    lok((test_world->entity_typeflags[Chasse] & TNECS_COMPONENT_TYPE(test_world, Position)) > 0);

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

void tnecs_test_hashing() {

}

void tnecs_test_world_progress() {
    struct Velocity * temp_velocity;
    tnecs_entity_t Perignon = TNECS_NEW_ENTITY_WCOMPONENTS(test_world, Position, Velocity);
    temp_position = TNECS_GET_COMPONENT(test_world, Perignon, Position);
    temp_velocity = TNECS_GET_COMPONENT(test_world, Perignon, Velocity);
    temp_position->x = 100;
    temp_position->y = 200;

    temp_velocity->vx = 1;
    temp_velocity->vy = 2;
    tnecs_world_progress(test_world, 1);
    temp_position = TNECS_GET_COMPONENT(test_world, Perignon, Position);
    temp_velocity = TNECS_GET_COMPONENT(test_world, Perignon, Velocity);
    lok(temp_position->x == 101);
    lok(temp_position->y == 202);
    lok(temp_velocity->vx == 1);
    lok(temp_velocity->vy == 2);
    tnecs_world_progress(test_world, 1);
    temp_position = TNECS_GET_COMPONENT(test_world, Perignon, Position);
    temp_velocity = TNECS_GET_COMPONENT(test_world, Perignon, Velocity);
    lok(temp_position->x == 102);
    lok(temp_position->y == 204);
    lok(temp_velocity->vx == 1);
    lok(temp_velocity->vy == 2);
}

void tnecs_benchmarks() {
    dupprintf(globalf, "\nHomemade tnecs benchmarks\n");

    double t_0;
    double t_1;

    uint64_t res_hash;
    t_0 = get_us();
    for (size_t i = 0; i < ITERATIONS; i++) {
        res_hash = TNECS_HASH("Position");
        res_hash = TNECS_HASH("Unit");
    }
    t_1 = get_us();
    dupprintf(globalf, "tnecs_hash_djb2: %d iterations \n", ITERATIONS);
    dupprintf(globalf, "%.1f [us] \n", t_1 - t_0);

    t_0 = get_us();
    for (size_t i = 0; i < ITERATIONS; i++) {
        res_hash = tnecs_hash_sdbm("Unit");
        res_hash = tnecs_hash_sdbm("Position");
    }
    t_1 = get_us();
    dupprintf(globalf, "tnecs_hash_sdbm: %d iterations \n", ITERATIONS);
    dupprintf(globalf, "%.1f [us] \n", t_1 - t_0);

    t_0 = get_us();
    struct tnecs_World * bench_world = tnecs_world_genesis();
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
        tnecs_entities[i] = tnecs_new_entity(bench_world);
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
    tnecs_world_destroy(bench_world);

}

int main() {
    globalf = fopen("tnecs_test_results.txt", "w+");
    dupprintf(globalf, "\nHello, World! I am testing tnecs.\n");
    lrun("utilities", tnecs_test_utilities);
    test_world = tnecs_world_genesis();
    lrun("c_regis", tnecs_test_component_registration);
    lrun("s_regis", tnecs_test_system_registration);
    lrun("e_create", tnecs_test_entity_creation);
    lrun("c_add", tnecs_test_component_add);
    lrun("progress", tnecs_test_world_progress);
    lrun("hashing", tnecs_test_hashing);
    lresults();

    tnecs_benchmarks();
    tnecs_world_destroy(test_world);
    dupprintf(globalf, "tnecs Test End \n \n");
    fclose(globalf);
    return (0);
}
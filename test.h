
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <string.h>

#include "us_clock.h"
#include "tnecs.h"


void tnecs_benchmarks(FILE *) {
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
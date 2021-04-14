#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <stdbool.h>

#include "stb_ds.h"
#define STB_DS_IMPLEMENTATION
#include "simplecs.h"

// TIMER. get_time() resolution: 0.1 [us]
#ifdef __TINYC__
// very low resolution
#define get_time() ((double)clock())/CLOCKS_PER_SEC*1e6
// #define get_time() (double)(sc_time_ns() * 1e3)
// #define get_time() (double)clock()/CLOCKS_PER_SEC*1e6
#else
#ifdef WIN32
#include <windows.h>
double get_time() {
    LARGE_INTEGER t, f; // [us]
    QueryPerformanceCounter(&t);
    QueryPerformanceFrequency(&f);
    return 1e6 * (double)t.QuadPart / (double)f.QuadPart;
}
#else
#include <sys/time.h>
double get_time() { // [s]
    struct timeval t;
    struct timezone tzp;
    gettimeofday(&t, &tzp);
    return 1e6 * t.tv_sec + t.tv_usec;
}
#endif
#endif


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



#define ITERATIONS 100000
#define ITERATIONS_SMALL 1000
simplecs_entity_t simplecs_entities[ITERATIONS];

#define ARRAY_LEN 100
struct Unit unit_array[ARRAY_LEN];

struct Unit_Hash {
    simplecs_entity_t key; // id
    struct Unit value; // components_list
};


void Simplecs_SystemMove(struct Simplecs_System_Input in_input) {
    // Position *p = SIMPLECS_COMPONENTS_LIST(entity_list, Position);
    // Unit *v = SIMPLECS_COMPONENTS_LIST(entity_list, Unit);

    // for (int i = 0; i < entity_num; i++) {
    //     p[i].x += 2;
    //     p[i].y += 4;
    // }
}

int main() {
    printf("Hello, World! I am testing Simplecs. \n\n");
    simplecs_entity_t * components_list;
    struct Position * temp_position;
    struct Unit * temp_unit;

    printf("simplecs_init\n");
    struct Simplecs_World * test_world = simplecs_init();
    printf("\n");

    printf("Component registration\n");
    printf("Registering Position Component \n");
    SIMPLECS_REGISTER_COMPONENT(test_world, Position); // component id is 1
    SIMPLECS_NEW_ENTITY_WCOMPONENTS(test_world, Position, Unit);
    // printf("Component_Position_id %d \n", Component_Position_id);
    // assert(Component_Position_id == COMPONENT_ID_START);
    // printf("Registering Position Unit \n");
    // SIMPLECS_REGISTER_COMPONENT(test_world, Unit);
    // assert(Component_Unit_id == (COMPONENT_ID_START << 1));
    // printf("Registering Position Sprite \n");
    // SIMPLECS_REGISTER_COMPONENT(test_world, Sprite);
    // assert(Component_Sprite_id == (COMPONENT_ID_START << 2));

    // printf("\n");

    // printf("System registration\n");
    // // SIMPLECS_REGISTER_SYSTEM(test_world, Simplecs_SystemMove, SIMPLECS_PHASE_PREUPDATE, Position, Unit);
    // printf("\n");

    // printf("Entity Creation/Destruction\n");
    // assert(test_world->next_entity_id == ENTITY_ID_START);
    // printf("Making Silou Entity \n");
    // simplecs_entity_t Silou = simplecs_new_entity(test_world);
    // assert(Silou == ENTITY_ID_START);
    // assert(test_world->next_entity_id == (ENTITY_ID_START + 1));
    // printf("Making Pirou Entity \n");
    // simplecs_entity_t Pirou = simplecs_new_entity(test_world);
    // assert(Pirou == (ENTITY_ID_START + 1));
    // assert(test_world->next_entity_id == (ENTITY_ID_START + 2));
    // assert(Silou != Pirou);
    // printf("\n");

    // printf("Adding Components to Entities\n");
    // SIMPLECS_ADD_COMPONENT(test_world, Position, Silou);
    // components_list = hmget(test_world->entities_table, Silou);
    // assert(arrlen(components_list) == 1);
    // assert(components_list[0] == Component_Position_id);
    // SIMPLECS_ADD_COMPONENT(test_world, Unit, Silou);
    // components_list = hmget(test_world->entities_table, Silou);
    // assert(arrlen(components_list) == 2);
    // assert(components_list[0] == Component_Position_id);
    // assert(components_list[1] == Component_Unit_id);

    // SIMPLECS_ADD_COMPONENT(test_world, Position, Pirou);
    // components_list = hmget(test_world->entities_table, Pirou);
    // assert(arrlen(components_list) == 1);
    // assert(components_list[0] == Component_Position_id);
    // SIMPLECS_ADD_COMPONENT(test_world, Unit, Pirou);
    // components_list = hmget(test_world->entities_table, Pirou);
    // assert(arrlen(components_list) == 2);
    // assert(components_list[0] == Component_Position_id);
    // assert(components_list[1] == Component_Unit_id);
    // printf("\n");

    // printf("Getting Components from Entities\n");
    // Position * temp_Pirou = hmget(component_Position, Pirou);
    // Position * temp_Silou = hmget(component_Position, Silou);
    // assert(temp_Silou != temp_Pirou);
    // assert(SIMPLECS_GET_COMPONENT(Position, Silou) != SIMPLECS_GET_COMPONENT(Position, Pirou));
    // temp_position = SIMPLECS_GET_COMPONENT(Position, Silou);
    // assert(temp_position->x == 0);
    // assert(temp_position->y == 0);
    // temp_position->x = 1;
    // temp_position->y = 2;
    // assert(temp_position->x == 1);
    // assert(temp_position->y == 2);
    // temp_position = SIMPLECS_GET_COMPONENT(Position, Silou);
    // assert(temp_position->x == 1);
    // assert(temp_position->y == 2);

    // temp_unit = SIMPLECS_GET_COMPONENT(Unit, Silou);
    // assert(temp_unit->hp == 0);
    // assert(temp_unit->str == 0);
    // temp_unit->hp = 3;
    // temp_unit->str = 4;
    // assert(temp_unit->hp == 3);
    // assert(temp_unit->str == 4);
    // temp_position = SIMPLECS_GET_COMPONENT(Position, Silou);
    // assert(temp_position->x == 1);
    // assert(temp_position->y == 2);
    // temp_unit = SIMPLECS_GET_COMPONENT(Unit, Silou);
    // assert(temp_unit->hp == 3);
    // assert(temp_unit->str == 4);

    // temp_position = SIMPLECS_GET_COMPONENT(Position, Pirou);
    // assert(temp_position->x == 0);
    // assert(temp_position->y == 0);
    // temp_position->x = 5;
    // temp_position->y = 6;
    // assert(temp_position->x == 5);
    // assert(temp_position->y == 6);
    // temp_position = SIMPLECS_GET_COMPONENT(Position, Pirou);
    // assert(temp_position->x == 5);
    // assert(temp_position->y == 6);

    // temp_unit = SIMPLECS_GET_COMPONENT(Unit, Pirou);
    // assert(temp_unit->hp == 0);
    // assert(temp_unit->str == 0);
    // temp_unit->hp = 7;
    // temp_unit->str = 8;
    // assert(temp_unit->hp == 7);
    // assert(temp_unit->str == 8);
    // temp_position = SIMPLECS_GET_COMPONENT(Position, Pirou);
    // assert(temp_position->x == 5);
    // assert(temp_position->y == 6);
    // temp_unit = SIMPLECS_GET_COMPONENT(Unit, Pirou);
    // assert(temp_unit->hp == 7);
    // assert(temp_unit->str == 8);
    // printf("\n");

    // printf("Destroying Entities\n");
    // simplecs_entity_destroy(test_world, Pirou);
    // Pirou = simplecs_new_entity(test_world);
    // assert(Pirou == (ENTITY_ID_START + 1));
    // assert(test_world->next_entity_id == (ENTITY_ID_START + 2));
    // assert(Silou != Pirou);
    // SIMPLECS_ADD_COMPONENT(test_world, Position, Pirou);
    // components_list = hmget(test_world->entities_table, Pirou);
    // assert(arrlen(components_list) == 1);
    // assert(components_list[0] == Component_Position_id);
    // SIMPLECS_ADD_COMPONENT(test_world, Unit, Pirou);
    // components_list = hmget(test_world->entities_table, Pirou);
    // assert(arrlen(components_list) == 2);
    // assert(components_list[0] == Component_Position_id);
    // assert(components_list[1] == Component_Unit_id);
    // printf("\n");

    // printf("Homemade simplecs benchmarks\n");
    // double t_0;
    // double t_1;

    // struct Unit_Hash * unit_hash = NULL;

    // t_0 = get_time();
    // for (size_t i = 0; i < ARRAY_LEN; i++) {
    //     unit_array[i].hp = Unit_default.hp;
    //     unit_array[i].str = Unit_default.str;
    // }
    // t_1 = get_time();
    // printf("unit_array init: %d iterations \n", ARRAY_LEN);
    // printf("%.1f [us] \n", t_1 - t_0);

    // struct Unit temp_unitnp;
    // t_0 = get_time();
    // for (size_t i = 0; i < ARRAY_LEN; i++) {
    //     temp_unitnp.hp = i;
    //     temp_unitnp.str = i * 2;
    //     hmput(unit_hash, i, temp_unitnp);
    // }
    // t_1 = get_time();
    // printf("unit_hash init: %d iterations \n", ARRAY_LEN);
    // printf("%.1f [us] \n", t_1 - t_0);

    // t_0 = get_time();
    // for (size_t i = 0; i < ITERATIONS; i++) {
    //     unit_array[(i % ARRAY_LEN)].hp++;
    //     unit_array[(i % ARRAY_LEN)].str++;
    // }
    // t_1 = get_time();
    // printf("unit_array operations: %d iterations \n", ITERATIONS);
    // printf("%.1f [us] \n", t_1 - t_0);

    // t_0 = get_time();
    // size_t index;
    // for (size_t i = 0; i < ITERATIONS; i++) {
    //     index = (i % ARRAY_LEN);
    //     temp_unitnp = hmget(unit_hash, index);
    //     temp_unitnp.hp++;
    //     temp_unitnp.str++;
    //     hmput(unit_hash, index, temp_unitnp);
    // }
    // t_1 = get_time();
    // printf("unit_hash operations: %d iterations \n", ITERATIONS);
    // printf("%.1f [us] \n", t_1 - t_0);

    // t_0 = get_time();
    // struct Simplecs_World * bench_world = simplecs_init();
    // t_1 = get_time();
    // printf("simplecs: World Creation time \n");
    // printf("%.1f [us] \n", t_1 - t_0);

    // t_0 = get_time();
    // SIMPLECS_REGISTER_COMPONENT(bench_world, Position2)
    // SIMPLECS_REGISTER_COMPONENT(bench_world, Unit2)
    // t_1 = get_time();
    // printf("simplecs: Component Registration \n");
    // printf("%.1f [us] \n", t_1 - t_0);

    // t_0 = get_time();
    // simplecs_entity_t simplecs_temp_ent;
    // for (size_t i = 0; i < ITERATIONS; i++) {
    //     simplecs_temp_ent = simplecs_new_entity(bench_world);
    //     simplecs_entities[i] = simplecs_temp_ent;
    // }
    // t_1 = get_time();
    // printf("simplecs: Entity Creation time: %d iterations \n", ITERATIONS);
    // printf("%.1f [us] \n", t_1 - t_0);

    // t_0 = get_time();
    // SIMPLECS_ADD_COMPONENT(bench_world, Position2, simplecs_entities[1]);
    // SIMPLECS_ADD_COMPONENT(bench_world, Unit2, simplecs_entities[1]);
    // for (size_t i = 2; i < ITERATIONS; i++) {
    //     // printf("%i \n", i);
    //     SIMPLECS_ADD_COMPONENT(bench_world, Position2, simplecs_entities[i], false);
    //     SIMPLECS_ADD_COMPONENT(bench_world, Unit2, simplecs_entities[i], false);
    // }
    // t_1 = get_time();
    // printf("simplecs: Component adding time: %d iterations \n", ITERATIONS);
    // printf("%.1f [us] \n", t_1 - t_0);

    printf("Simplecs Test End");
    return (0);
}
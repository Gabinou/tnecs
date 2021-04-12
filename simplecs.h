#ifndef SIMPLECS_H
#define SIMPLECS_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
// #define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

typedef uint64_t simplecs_entity_t;
typedef uint64_t simplecs_component_t;
typedef uint64_t simplecs_components_t;
typedef uint16_t simplecs_system_t;
#define SIMPLECS_NULLENTITY 0
#define OPEN_IDS_BUFFER 128
#define MAX_COMPONENT 63
#define COMPONENT_ID_START 1
#define ENTITY_ID_START 1
#define DEFAULT_COMPONENT_NUM 4
#define DEFAULT_SYSTEM_CAP 16
#define DEFAULT_COMPONENT_CAP 64
#define DEFAULT_ENTITY_CAP 128
#define ENTITY_MAX_COMPONENT_NUM 10
// IDEA component IDS are integer bitflags.
//  -> MAX 64 components
// This makes the system flag just a sum
// -> Still want to keep posibility of multiple systems having same bitflag, but it makes everything easier to store.


#define CONCATENATE(arg1, arg2) CONCATENATE1(arg1, arg2)
#define CONCATENATE1(arg1, arg2) CONCATENATE2(arg1, arg2)
#define CONCATENATE2(arg1, arg2)  arg1##arg2

#define FOR_EACH_1(macro, x)\
    macro(x)

#define FOR_EACH_2(macro, x, ...)\
    macro(x)+\
    FOR_EACH_1(macro, __VA_ARGS__)

#define FOR_EACH_3(macro, x, ...)\
    macro(x)+\
    FOR_EACH_2(macro, __VA_ARGS__)

#define FOR_EACH_4(macro, x, ...)\
    macro(x)+\
    FOR_EACH_3(macro,  __VA_ARGS__)

#define FOR_EACH_5(macro, x, ...)\
    macro(x)+\
    FOR_EACH_4(macro,  __VA_ARGS__)

#define FOR_EACH_6(macro, x, ...)\
    macro(x)+\
    FOR_EACH_5(macro,  __VA_ARGS__)

#define FOR_EACH_7(macro, x, ...)\
    macro(x)+\
    FOR_EACH_6(macro,  __VA_ARGS__)

#define FOR_EACH_8(macro, x, ...)\
    macro(x)+\
    FOR_EACH_7(macro,  __VA_ARGS__)

#define FOR_EACH_NARG(...) FOR_EACH_NARG_(__VA_ARGS__, FOR_EACH_RSEQ_N())
#define FOR_EACH_NARG_(...) FOR_EACH_ARG_N(__VA_ARGS__)
#define FOR_EACH_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N
#define FOR_EACH_RSEQ_N() 8, 7, 6, 5, 4, 3, 2, 1, 0
#define FOR_EACH_(N, macro, ...) CONCATENATE(FOR_EACH_, N)(macro, __VA_ARGS__)
#define FOR_EACH(macro, ...) FOR_EACH_(FOR_EACH_NARG(__VA_ARGS__), macro, __VA_ARGS__)

enum RUN_PHASES {
    SIMPLECS_PHASE_PREUPDATE = 0,
    SIMPLECS_PHASE_ONUPDATE = 1,
    SIMPLECS_PHASE_POSTUPDATE = 2,
};

struct Components_byType {
    \
    simplecs_components_t type;
    void * components;
};

struct Simplecs_System_Input {
    simplecs_entity_t * entities;       
    simplecs_components_t typeflag;
    size_t num;
    size_t * components_order; // Always equal to the total length of components I guess.
    void ** components_lists;
};

struct Simplecs_World {
    simplecs_entity_t * entities;                    // Always maintain contiguous as possible.
    simplecs_components_t * entity_component_flags;  // Same order as entities
    simplecs_components_t * system_typeflags;
    bool * system_isExclusive;
    void (** systems)(struct Simplecs_System_Input);

    simplecs_components_t * all_typeflags;           // created on ADD_COMPONENT
    simplecs_entity_t ** entitiesbytype_lists;       // 2D list. Same order as system_typeflags
    size_t * num_componentsbytype;       // For reference I guess?
    size_t * num_entitiesbytype;
    size_t num_components;
    size_t num_system_typeflags;
    size_t num_all_typeflags;
    size_t num_typeflags_bybitcount;
    struct Components_byType *** components_bytype;  // 3D. Each entity has multiple components

    simplecs_entity_t next_entity_id; // ]0,  UINT64_MAX]
    simplecs_system_t next_system_id; // [0, ...]

    simplecs_entity_t opened_entity_ids[OPEN_IDS_BUFFER];
    uint8_t num_opened_entity_ids;
    // Systems don't get destroyed
};

struct Simplecs_World * simplecs_init();


// -> system should be able to get 1D component arrays associated with all entities.
#define SIMPLECS_SYSTEMS_COMPONENTLIST(input, name) (* name)input->components_lists[input->components_order[Component_##name##_id]]


#define SIMPLECS_REGISTER_COMPONENT(world, name) _SIMPLECS_REGISTER_COMPONENT(world, name)
#define _SIMPLECS_REGISTER_COMPONENT(world, name) const simplecs_component_t Component_##name##_flag = (1 << world->num_components);\
arrput(world->all_typeflags, Component_##name##_flag);\
world->num_all_typeflags++;\
const simplecs_component_t Component_##name##_id = world->num_components++;
// Error if component registered twice -> user responsibility

#define SIMPLECS_GET_COMPONENT_HASH(name)  (struct Component_##name *)(in_world->component_hashes[Component_##name##_id])
#define SIMPLECS_COMPONENT_ID(name) Component_##name##_id
#define SIMPLECS_COMPONENT_FLAG(name) Component_##name##_flag

#define SIMPLECS_ADD_COMPONENT(world, name, entity_id) if (!simplecs_type_exists(world->all_typeflags, world->num_all_typeflags, Component_##name##_flag + world->entity_component_flags[entity_id])) {\
arrput(world->all_typeflags, world->entity_component_flags[entity_id]); \
world->num_all_typeflags++;\
}\
simplecs_entity_typeflag_change(world, entity_id, Component_##name##_flag);
// Components are never removed.

simplecs_entity_t simplecs_new_entity(struct Simplecs_World * in_world);
simplecs_entity_t simplecs_entity_destroy(struct Simplecs_World * in_world, simplecs_entity_t in_entity);

#define SIMPLECS_REGISTER_SYSTEM(world, pfunc, phase, isexcl, ...) simplecs_register_system(world, pfunc, phase, isexcl, FOR_EACH_NARG(__VA_ARGS__), FOR_EACH(SIMPLECS_COMPONENT_ID, __VA_ARGS__))

void simplecs_register_system(struct Simplecs_World * in_world, simplecs_entity_t * entities_list, uint8_t in_run_phase, bool isexclusive, size_t component_num, simplecs_components_t component_typeflag);

void simplecs_entity_typeflag_change(struct Simplecs_World * in_world, simplecs_entity_t in_entity, simplecs_components_t new_flag);
bool simplecs_type_exists(simplecs_components_t * in_typelist, size_t len, simplecs_components_t in_flag);
size_t simplecs_issubtype(simplecs_components_t * in_typelist, size_t len, simplecs_components_t in_flag);

#define SIMPLECS_COMPONENTS_LIST(entity_list, Position)

#endif // SIMPLECS
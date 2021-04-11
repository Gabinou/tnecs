
#include "simplecs.h"

// Simplecs (pronounced simplex) is a very simple implementation of an entity-component-system (ECS)
// ECS is very useful in game programming.
// OOP: objects and methods, children inheriting from parents, etc.
// ECS: Entities can have any number of independent components, acted upon by systems
// Example: Videogame
//      -> main character: Physics component, PlayerControlled Component
//      -> enemies: Physics component, AIControlled Component
//      -> environment tiles: Destroyable Component
// Entities are indices (uint64_t)
// Component are structures
// Systems are functions
// The main loop iterates over systems
// There can be only one world.

struct Simplecs_World * simplecs_init() {
    struct Simplecs_World * simplecs_world = (struct Simplecs_World *)calloc(sizeof(struct Simplecs_World), 1);
    simplecs_world->entities = NULL;
    arrsetcap(simplecs_world->entities, DEFAULT_ENTITY_CAP)
    arrput(simplecs_world->entities, 0);
    simplecs_world->entity_component_flags = NULL;
    arrsetcap(simplecs_world->entity_component_flags, DEFAULT_ENTITY_CAP)
    arrput(simplecs_world->entity_component_flags, 0);
    simplecs_world->system_typeflags = NULL;
    arrsetcap(simplecs_world->system_typeflags, DEFAULT_SYSTEM_CAP)
    simplecs_world->num_system_typeflags = 0;
    arrsetcap(simplecs_world->system_typeflags, DEFAULT_SYSTEM_CAP)
    simplecs_world->entitiesbytype_lists = NULL;
    arrsetcap(simplecs_world->entitiesbytype_lists, DEFAULT_SYSTEM_CAP)

    simplecs_world->next_entity_id = ENTITY_ID_START;
    simplecs_world->next_system_id = 0;
    simplecs_world->num_components = 0;
    simplecs_world->entities_typeflags = NULL;

    return (simplecs_world);
}

simplecs_entity_t simplecs_new_entity(struct Simplecs_World * in_world) {
    simplecs_entity_t out = SIMPLECS_NULLENTITY;
    simplecs_component_t component_flag;
    while ((out == SIMPLECS_NULLENTITY) && (in_world->num_opened_entity_ids > 0)) {
        out = in_world->opened_entity_ids[--in_world->num_opened_entity_ids];
        in_world->opened_entity_ids[in_world->num_opened_entity_ids] = SIMPLECS_NULLENTITY;
    }
    if (out == SIMPLECS_NULLENTITY) {
        out = in_world->next_entity_id++;
    }
    return (out);
}

simplecs_entity_t simplecs_entity_destroy(struct Simplecs_World * in_world, simplecs_entity_t in_entity) {
    simplecs_component_t previous_flag = in_world->entity_component_flags[in_entity];
    
    for (size_t i =0 ; i < in_world->num_system_typeflags; i++) {
        if (previous_flag == in_world->system_typeflags[i]) {
            for (size_t j =0 ; j < in_world->num_entitiesbytype[i]; j++) {
                if (entitiesbytype_lists[i][j] == in_entity) {
                    arrdel(entitiesbytype_lists[i], j);
                break;
            }
        }
    }

    in_world->entity_component_flags[in_entity] = 0;
    if (in_world->num_opened_entity_ids < OPEN_IDS_BUFFER) {
        in_world->opened_entity_ids[in_world->num_opened_entity_ids++] = in_entity;
    }
}

void simplecs_register_system(struct Simplecs_World * in_world, void (*in_system)(struct Simplecs_System_Input system_input), uint8_t in_run_phase, size_t num_components, ...) {
    printf("I'M IN");
    arrput(in_world->systems_table->systems_list, in_system);
    arrput(in_world->systems_table->components_num, num_components);
    simplecs_entity_t * components_list = malloc(num_components * sizeof(simplecs_entity_t));
    va_list ap;
    va_start(ap, num_components);
    for (size_t i = 0; i < num_components; i++) {
        components_list[i] = va_arg(ap, simplecs_entity_t);
    }
    arrput(in_world->systems_table->components_lists, components_list);
    in_world->next_system_id++;
}


/* PROTOTYPE SYSTEM
void Simplecs_SystemMove(struct Simplecs_System_Input system_input) {

    Position *p = SIMPLECS_COMPONENTS_LIST(entity_list, Position);

    Unit *v = SIMPLECS_COMPONENTS_LIST(entity_list, Unit);

    for (int i = 0; i < entity_num; i++) {
        p[i].x += 2;
        p[i].y += 4;
    }
}
*/
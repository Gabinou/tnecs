
#include "tnecs2.h"

/************************* PRIVATE DECLARATIONS ******************************/
/* --- WORLD FUNCTIONS --- */
static b32 _tnecs_world_breath_arrays(      tnecs_world *w);
static b32 _tnecs_world_breath_phases(      tnecs_world *w);
static b32 _tnecs_world_breath_systems(     tnecs_world *w);
static b32 _tnecs_world_breath_entities(    tnecs_world *w);
static b32 _tnecs_world_breath_archetype(   tnecs_world *w);
static b32 _tnecs_world_breath_components(  tnecs_world *w);

/* --- REGISTRATION  --- */
static size_t _tnecs_register_archetype(tnecs_world *w, size_t num_components,
                                       tnecs_component archetype);

/**************************** WORLD FUNCTIONS ********************************/
b32 tnecs_world_genesis(tnecs_world **world) {
    /* Allocate world itself */
    if (*world != NULL) {
        TNECS_CHECK_CALL(tnecs_world_destroy(world));   
    }

    /* Allocate enough to contain all arenas in memory */
    *world = calloc(1, sizeof(tnecs_world) + TNECS_INIT_WORLD_BYTESIZE);
    TNECS_CHECK_ALLOC(*world);

    /* Create parent arena inside world mem */
    tnecs_arena *world_arena = tnecs_world_arena(*world);
    world_arena->size = TNECS_INIT_WORLD_BYTESIZE;
    world_arena->fill = TNECS_NULLSHIFT; // 0 always NULL;

    /* Create children arena inside parent arent */
    TNECS_CHECK_CALL(_tnecs_world_breath_arrays(*world));
    TNECS_CHECK_CALL(_tnecs_world_breath_phases(*world));
    TNECS_CHECK_CALL(_tnecs_world_breath_systems(*world));
    TNECS_CHECK_CALL(_tnecs_world_breath_entities(*world));
    TNECS_CHECK_CALL(_tnecs_world_breath_archetype(*world));
    TNECS_CHECK_CALL(_tnecs_world_breath_components(*world));

    return(1);
}

tnecs_arena *tnecs_world_arena(tnecs_world *world) {
    return((tnecs_arena *)world->mem);
}


// b32 tnecs_world_destroy(tnecs_world **world) {
//     free(*world);
//     *world = NULL;
//     return(1);
// }

// b32 tnecs_world_step(tnecs_world *world, tnecs_ns deltat, void *data) {
//     world->num_systems_torun = 0;
//     for (size_t phase = 0; phase < world->num_phases; phase++) {
//         if (!tnecs_world_step_phase(world, phase, deltat, data)) {
//             printf("tnecs: Could not run phase %zu \n", phase);
//         }
//     }
//     return(1);
// }

// b32 tnecs_world_step_phase(tnecs_world *world,  tnecs_phase  phase,
//                            tnecs_ns     deltat, void        *data) {
//     if (phase != world->phases[phase]) {
//         printf("tnecs: Invalid phase '%d' \n", phase);
//         return(0);
//     }

//     for (size_t sorder = 0; sorder < world->num_systems_byphase[phase]; sorder++) {
//         size_t system_id = world->systems_idbyphase[phase][sorder];
//         TNECS_CHECK_CALL(tnecs_system_run(world, system_id, deltat, data));
//     }
//     return(1);
// }

b32 _tnecs_world_breath_arrays(tnecs_world *world) {
    size_t bytesize = 0;
    tnecs_arena *world_arena = tnecs_world_arena(world);
    
    world->entities_open.len        = TNECS_INIT_ENTITY_LEN;
    bytesize                        = world->entities_open.len * sizeof(tnecs_entity);
    world->entities_open.handle     = tnecs_arena_push(world_arena, bytesize);
    TNECS_CHECK_ALLOC(world->entities_open.handle);

    world->system_torun.len         = TNECS_INIT_SYSTEM_LEN;
    bytesize                        = world->system_torun.len * sizeof(tnecs_system_ptr);
    world->system_torun.handle      = tnecs_arena_push(world_arena, bytesize);
    TNECS_CHECK_ALLOC(world->system_torun.handle);

    return(1);
}

b32 _tnecs_world_breath_entities(tnecs_world *world) {
    /* Compute all bytesizes */
    world->entities.len         = TNECS_INIT_ENTITY_LEN;
    world->entities.num         = TNECS_NULLSHIFT;
    size_t bytesize_id          = world->entities.len * sizeof(tnecs_entity);
    size_t bytesize_order       = world->entities.len * sizeof(size_t);
    size_t bytesize_archetype   = world->entities.len * sizeof(tnecs_component);
    size_t bytesize_total       = bytesize_id + bytesize_order + bytesize_archetype + TNECS_NULLSHIFT;
    bytesize_total              = tnecs_round_up(bytesize_total);

    /* Alloc child arena in parent arena */
    tnecs_arena *world_arena    = tnecs_world_arena(world);
    world->entities.arena       = tnecs_arena_push(world_arena, bytesize_total);
    TNECS_CHECK_ALLOC(world->entities.arena);

    /* Alloc entities array into entity_arena */
    tnecs_arena *entity_arena   = tnecs_arena_ptr(world_arena, world->entities.arena);
    entity_arena->fill          = TNECS_NULLSHIFT;

    world->entities.id          = tnecs_arena_push(entity_arena, bytesize_id);
    world->entities.order       = tnecs_arena_push(entity_arena, bytesize_order);
    world->entities.archetype   = tnecs_arena_push(entity_arena, bytesize_archetype);

    TNECS_CHECK_ALLOC(world->entities.id);
    TNECS_CHECK_ALLOC(world->entities.archetype);
    TNECS_CHECK_ALLOC(world->entities.order);

    return(1);
}

b32 _tnecs_world_breath_components(tnecs_world *world) {
    // /* NULL component always exists! */
    // world->num_components               = TNECS_NULLSHIFT;
    // world->component_hashes[TNECS_NULL] = TNECS_NULL;

    // /* Set name of first component */
    // strncpy(world->component_names[TNECS_NULL], "NULL\0", namelen);
    return(1);
}

b32 _tnecs_world_breath_systems(tnecs_world *world) {
    /* Compute all bytesizes */
    world->systems.len          = TNECS_INIT_SYSTEM_LEN;
    world->systems.num          = TNECS_NULLSHIFT;
    size_t bytesize_archetype   = world->systems.len * sizeof(tnecs_component);
    size_t bytesize_order       = world->systems.len * sizeof(size_t);
    size_t bytesize_name        = world->systems.len * sizeof(i64);
    size_t bytesize_hash        = world->systems.len * sizeof(tnecs_hash);
    size_t bytesize_phase       = world->systems.len * sizeof(tnecs_phase);
    size_t bytesize_exclusive   = world->systems.len * sizeof(b32);
    size_t bytesize_total       = bytesize_archetype + bytesize_order + bytesize_name + bytesize_hash + bytesize_phase + bytesize_exclusive + TNECS_NULLSHIFT;
    bytesize_total              = tnecs_round_up(bytesize_total);

    /* Alloc child arena in parent arena */
    tnecs_arena *world_arena   = tnecs_world_arena(world);
    world->systems.arena      = tnecs_arena_push(world_arena, bytesize_total);
    TNECS_CHECK_ALLOC(world->systems.arena);

    tnecs_arena *systems_arena  = tnecs_arena_ptr(world_arena, world->systems.arena);
    systems_arena->fill          = TNECS_NULLSHIFT;

    world->systems.hash         = tnecs_arena_push(systems_arena, bytesize_hash);
    world->systems.name         = tnecs_arena_push(systems_arena, bytesize_name);
    world->systems.order        = tnecs_arena_push(systems_arena, bytesize_order);
    world->systems.phase        = tnecs_arena_push(systems_arena, bytesize_phase);
    world->systems.archetype    = tnecs_arena_push(systems_arena, bytesize_archetype);
    world->systems.exclusive    = tnecs_arena_push(systems_arena, bytesize_exclusive);
    TNECS_CHECK_ALLOC(world->systems.archetype);
    TNECS_CHECK_ALLOC(world->systems.order);
    TNECS_CHECK_ALLOC(world->systems.name);
    TNECS_CHECK_ALLOC(world->systems.hash);
    TNECS_CHECK_ALLOC(world->systems.phase);
    TNECS_CHECK_ALLOC(world->systems.exclusive);


    /* Set name of NULL system */
    i64 *system_names_array = tnecs_arena_ptr(systems_arena, world->systems.name);
    // TODO: name adding utility
    size_t namelen = 5;
    system_names_array[0]   = tnecs_arena_push(systems_arena, namelen);
    char *null_system_name  = tnecs_arena_ptr(systems_arena, system_names_array[0]);
    strncpy(null_system_name, "NULL\0", namelen);

    return(1);
}

b32 _tnecs_world_breath_phases(tnecs_world *world) {
    world->phases.len   = TNECS_INIT_PHASE_LEN;
    world->phases.num   = TNECS_NULLSHIFT;

    size_t bytesize_byphase     = world->phases.len * sizeof(i64);
    size_t bytesize_idbyphase   = world->phases.len * sizeof(i64);
    size_t bytesize_numbyphase  = world->phases.len * sizeof(i64);
    size_t bytesize_lenbyphase  = world->phases.len * sizeof(i64);
    size_t bytesize_total       = bytesize_byphase + bytesize_idbyphase + bytesize_numbyphase + bytesize_lenbyphase + TNECS_NULLSHIFT;              
    bytesize_total= tnecs_round_up(bytesize_total);

    /* Alloc child arena in parent arena */
    tnecs_arena *world_arena    = tnecs_world_arena(world);
    world->phases.arena         = tnecs_arena_push(world_arena, bytesize_total);
    TNECS_CHECK_ALLOC(world->phases.arena);

    tnecs_arena *phases_arena       = tnecs_arena_ptr(world_arena, world->phases.arena);
    phases_arena->fill               = TNECS_NULLSHIFT;

    world->phases.num_byphase       = tnecs_arena_push(phases_arena, bytesize_numbyphase);
    world->phases.len_byphase       = tnecs_arena_push(phases_arena, bytesize_byphase);
    world->phases.system_byphase    = tnecs_arena_push(phases_arena, bytesize_byphase);
    world->phases.system_idbyphase  = tnecs_arena_push(phases_arena, bytesize_idbyphase);
    TNECS_CHECK_ALLOC(world->phases.system_byphase);
    TNECS_CHECK_ALLOC(world->phases.system_idbyphase);


    /* Alloc & check for entities_byphase elements */
    i64 *system_byphase     = tnecs_arena_ptr(phases_arena, world->phases.system_byphase);
    i64 *system_idbyphase   = tnecs_arena_ptr(phases_arena, world->phases.system_idbyphase);
    size_t *len_byphase     = tnecs_arena_ptr(phases_arena, world->phases.len_byphase);
    size_t bytesize_byphase_elem    = world->phases.len * sizeof(tnecs_system_ptr);
    size_t bytesize_idbyphase_elem  = world->phases.len * sizeof(size_t);
    for (size_t i = 0; i < world->phases.len; i++) {
        len_byphase[i] = world->phases.len;

        system_byphase[i]   = tnecs_arena_push(phases_arena, bytesize_byphase_elem);
        system_idbyphase[i] = tnecs_arena_push(phases_arena, bytesize_idbyphase_elem);

        TNECS_CHECK_ALLOC(system_byphase[i]);
        TNECS_CHECK_ALLOC(system_idbyphase[i]);
    }

    return(1);
}

b32 _tnecs_world_breath_archetype(tnecs_world *world) {

    /* Variables */
    // world->num_archetypes = TNECS_NULLSHIFT;
    // world->len_archetypes = TNECS_INIT_SYSTEM_LEN;

    return(1);
}

// /**************************** SYSTEM FUNCTIONS ********************************/
// b32 tnecs_custom_system_run(tnecs_world *world, tnecs_system_ptr custom_system,
//                              tnecs_component archetype, tnecs_ns deltat, void *data) {
//     /* Building the systems input */
//     tnecs_system_input input = {.world = world, .deltat = deltat, .data = data};
//     size_t tID = tnecs_archetypeid(world, archetype);
//     if (tID == TNECS_NULL) {
//         printf("tnecs: Input archetype is unknown.\n");
//         return(0);
//     }

//     /* Running the exclusive custom system */
//     input.entity_archetype_id    = tID;
//     input.num_entities          = world->num_entities_bytype[input.entity_archetype_id];
//     custom_system(&input);

//     /* Running the non-exclusive/inclusive custom system */
//     for (size_t tsub = 0; tsub < world->num_archetype_ids[tID]; tsub++) {
//         input.entity_archetype_id    = world->archetype_id_bytype[tID][tsub];
//         input.num_entities          = world->num_entities_bytype[input.entity_archetype_id];
//         custom_system(&input);
//     }
//     return(1);
// }

// b32 tnecs_growArray_torun(tnecs_world *world) {
//     /* Realloc systems_to_run if too many */
//         size_t old_len              = world->len_systems_torun;
//         size_t new_len              = old_len * TNECS_ARRAY_GROWTH_FACTOR;
//         world->len_systems_torun    = new_len;
//         size_t bytesize             = sizeof(*world->systems_torun);

//         world->systems_torun = tnecs_realloc(world->systems_torun, old_len, new_len, bytesize);
//         TNECS_CHECK_ALLOC(world->systems_torun);
//     return(1);
// }

// b32 tnecs_system_run(tnecs_world *world, size_t in_system_id,
//                     tnecs_ns     deltat, void *data) {
//     /* Building the systems input */
//     tnecs_system_input input = {.world = world, .deltat = deltat, .data = data};
//     size_t sorder               = world->system_orders[in_system_id];
//     tnecs_phase phase           = world->system_phases[in_system_id];
//     size_t system_archetype_id   = tnecs_archetypeid(world, world->system_archetypes[in_system_id]);

//     input.entity_archetype_id    = system_archetype_id;
//     input.num_entities          = world->num_entities_bytype[input.entity_archetype_id];

//     /* Running the exclusive systems in current phase */
//     while (world->num_systems_torun >= (world->len_systems_torun - 1)) {
//         TNECS_CHECK_CALL(tnecs_growArray_torun(world));
//     }
    
//     tnecs_system_ptr system             = world->systems_byphase[phase][sorder];
//     size_t system_num                   = world->num_systems_torun++;
//     world->systems_torun[system_num]    = world->systems_byphase[phase][sorder];
//     system(&input);

//     if (world->system_exclusive[in_system_id])
//         return(1);

//     /* Running the inclusive systems in current phase */
//     for (size_t tsub = 0; tsub < world->num_archetype_ids[system_archetype_id]; tsub++) {
//         input.entity_archetype_id    = world->archetype_id_bytype[system_archetype_id][tsub];
//         input.num_entities          = world->num_entities_bytype[input.entity_archetype_id];
//         while (world->num_systems_torun >= (world->len_systems_torun - 1)) {
//             TNECS_CHECK_CALL(tnecs_growArray_torun(world));
//         }
//         tnecs_system_ptr system             = world->systems_byphase[phase][sorder];
//         size_t system_num                   = world->num_systems_torun++;
//         world->systems_torun[system_num]    = system;
//         system(&input);
//     }
//     return(1);
// }

// /***************************** REGISTRATION **********************************/
// size_t tnecs_register_system(tnecs_world *world, const char *name,
//                              tnecs_system_ptr in_system, tnecs_phase phase,
//                              b32 isExclusive, size_t num_components, tnecs_component components_archetype) {
//     /* Compute new id */
//     size_t system_id = world->num_systems++;

//     /* Realloc systems if too many */
//     if (world->num_systems >= world->len_systems)
//         TNECS_CHECK_CALL(tnecs_growArray_system(world));

//     /* Realloc systems_byphase if too many */
//     if (world->num_systems_byphase[phase] >= world->len_systems_byphase[phase]) {
//         size_t olen                         = world->len_systems_byphase[phase];
//         size_t nlen                         = olen * TNECS_ARRAY_GROWTH_FACTOR;
//         world->len_systems_byphase[phase]   = nlen;
//         size_t bs                           = sizeof(**world->systems_byphase);
//         size_t bsid                         = sizeof(**world->systems_idbyphase);

//         tnecs_system_ptr *systems       = world->systems_byphase[phase];
//         size_t *system_id               = world->systems_idbyphase[phase];
//         world->systems_byphase[phase]   = tnecs_realloc(systems, olen, nlen, bs);
//         TNECS_CHECK_ALLOC(world->systems_byphase[phase]);   
//         world->systems_idbyphase[phase] = tnecs_realloc(system_id, olen, nlen, bsid);
//         TNECS_CHECK_ALLOC(world->systems_idbyphase[phase]);
//     }

//     /* -- Actual registration -- */
//     /* Saving name and hash */
//     world->system_names[system_id]  = malloc(strlen(name) + 1);
//     TNECS_CHECK_ALLOC(world->system_names[system_id]);

//     tnecs_hash hash                 = tnecs_hash_djb2(name);
//     strncpy(world->system_names[system_id], name, strlen(name) + 1);

//     /* Register new phase if didn't exist */
//     if (!world->phases[phase])
//         TNECS_CHECK_CALL(tnecs_register_phase(world, phase));

//     world->system_exclusive[system_id]  = isExclusive;
//     world->system_phases[system_id]     = phase;
//     world->system_hashes[system_id]     = hash;
//     world->system_archetypes[system_id]  = components_archetype;

//     /* System order */
//     size_t system_order                             = world->num_systems_byphase[phase]++;
//     world->system_orders[system_id]                 = system_order;
//     world->systems_byphase[phase][system_order]     = in_system;
//     world->systems_idbyphase[phase][system_order]   = system_id;
//     TNECS_CHECK_CALL(_tnecs_register_archetype(world, num_components, components_archetype));
//     return (system_id);
// }

// tnecs_component tnecs_register_component(tnecs_world *world,
//                                          const char *name,
//                                          size_t bytesize) {
//     /* Checks */
//     if (bytesize <= 0) {
//         printf("tnecs: Component should have >0 bytesize.\n");
//         return(TNECS_NULL);
//     }
//     if (world->num_components >= TNECS_COMPONENT_CAP) {
//         printf("tnecs: Component capacity reached.\n");
//         return(TNECS_NULL);
//     }


//     /* Registering */
//     tnecs_component new_component_id                = world->num_components++;
//     world->component_hashes[new_component_id]       = tnecs_hash_djb2(name);
//     tnecs_component new_component_flag              = TNECS_COMPONENT_ID2TYPE(new_component_id);
//     world->component_bytesizes[new_component_id]    = bytesize;

//     /* Setting component name */
//     world->component_names[new_component_id] = malloc(strlen(name) + 1);
//     TNECS_CHECK_ALLOC(world->component_names[new_component_id]);

//     strncpy(world->component_names[new_component_id], name, strlen(name) + 1);
//     TNECS_CHECK_CALL(_tnecs_register_archetype(world, 1, new_component_flag));
//     return (new_component_id);
// }

// size_t _tnecs_register_archetype(tnecs_world *world, size_t num_components,
//                                 tnecs_component archetype_new) {
//     // 0- Check if archetype exists, return
//     size_t tID = 0;
//     for (size_t i = 0 ; i < world->num_archetypes; i++) {
//         if (archetype_new == world->archetypes[i]) {
//             tID = i;
//             break;
//         }
//     }
//     if (tID)
//         return (tID);

//     // 1- Add new components_bytype at [tID]
//     if ((world->num_archetypes + 1) >= world->len_archetypes)
//         tnecs_growArray_archetype(world);
//     world->archetypes[world->num_archetypes++] = archetype_new;
//     tID = tnecs_archetypeid(world, archetype_new);
//     TNECS_DEBUG_ASSERT(tID == (world->num_archetypes - 1));
//     world->num_components_bytype[tID] = num_components;

//     // 2- Add arrays to components_bytype[tID] for each component
//     tnecs_component_array_new(world, num_components, archetype_new);

//     // 3- Add all components to components_idbytype
//     tnecs_component component_id_toadd, component_type_toadd;
//     tnecs_component archetype_reduced = archetype_new, archetype_added = 0;
//     size_t bytesize1 =  sizeof(**world->components_idbytype);
//     size_t bytesize2 =  sizeof(**world->components_orderbytype);
//     world->components_idbytype[tID]     = calloc(num_components,      bytesize1);
//     TNECS_CHECK_ALLOC(world->components_idbytype[tID]);
//     world->components_orderbytype[tID]  = calloc(TNECS_COMPONENT_CAP, bytesize2);
//     TNECS_CHECK_ALLOC(world->components_orderbytype[tID]);

//     size_t i = 0;
//     while (archetype_reduced) {
//         archetype_reduced &= (archetype_reduced - 1);

//         component_type_toadd = (archetype_reduced + archetype_added) ^ archetype_new;
//         archetype_added      += component_type_toadd;
//         component_id_toadd   = TNECS_COMPONENT_TYPE2ID(component_type_toadd);

//         world->components_idbytype[tID][i]      = component_id_toadd;

//         world->components_orderbytype[tID][component_id_toadd] = i++;
//     }

//     // 4- Check archetypes.
//     for (size_t i = 1 ; i < world->num_archetypes; i++) {
//         world->num_archetype_ids[i] = 0;
//         for (size_t j = 1 ; j < (world->num_archetypes); j++) {
//             if (i == j)
//                 continue;

//             if (!TNECS_TYPEFLAG_IS_ARCHETYPE(world->archetypes[i], world->archetypes[j]))
//                 continue;

//             // j is an archetype of i
//             world->archetype_id_bytype[i][world->num_archetype_ids[i]++] = j;
//         }
//     }

//     return (tID);
// }

// size_t tnecs_register_phase(tnecs_world *world, tnecs_phase phase) {
//     if (phase <= 0)
//         return(1);

//     while (phase >= world->len_phases) {
//         TNECS_CHECK_CALL(tnecs_growArray_phase(world));
//     }
//     world->phases[phase]    = phase;
//     world->num_phases       = (phase >= world->num_phases) ? (phase + 1) : world->num_phases;
//     return (phase);
// }

// /***************************** ENTITY MANIPULATION ***************************/
// tnecs_entity tnecs_entity_create(tnecs_world *world) {
//     tnecs_entity out = TNECS_NULL;
    
//     /* Check if an open entity exists */
//     if (world->reuse_entities) {
//         while ((out == TNECS_NULL) && (world->num_entities_open > 0)) {
//             out = world->entities_open[--world->num_entities_open];
//             world->entities_open[world->num_entities_open] = TNECS_NULL;
//         }
//     }

//     /* If no open entity existed, create one */
//     if (out == TNECS_NULL) {
//         do {
//             if (world->entity_next >= world->len_entities) {
//                 if (!tnecs_growArray_entity(world)) {
//                     printf("tnecs: Could not allocate more memory for entities.\n");
//                     return(TNECS_NULL);
//                 }
//             }
//         } while (world->entities[out = world->entity_next++] != TNECS_NULL);
//     }
//     TNECS_DEBUG_ASSERT(out != TNECS_NULL);

//     /* Set entity and checks  */
//     world->entities[out] = out;
//     tnecs_entitiesbytype_add(world, out, TNECS_NULL);
//     TNECS_DEBUG_ASSERT(world->entities[out] == out);
//     TNECS_DEBUG_ASSERT(world->entities_bytype[TNECS_NULL][world->entity_orders[out]] == out);
//     return (out);
// }

// tnecs_entity tnecs_entity_create_wID(tnecs_world *world, tnecs_entity entity) {
//     // TODO: What to do if entity existed before?
//     tnecs_entity out = 0;
//     while (entity >= world->len_entities) {
//         if (!tnecs_growArray_entity(world)) {
//             printf("tnecs: Could not allocate more memory for entities.\n");
//             return(TNECS_NULL);
//         }
//     }

//     if ((!world->entities[entity]) & (entity > 0)) {
//         out = world->entities[entity] = entity;
//         tnecs_entitiesbytype_add(world, out, TNECS_NULL);
//     }
//     return (out);
// }

// tnecs_entity tnecs_entities_create(tnecs_world *world, size_t num) {
//     for (int i = 0; i < num; i++) {
//         if (tnecs_entity_create(world) <= TNECS_NULL) {
//             printf("tnecs: Could not create another entity.\n");
//             return(TNECS_NULL);            
//         }
//     }
//     return (num);
// }

// tnecs_entity tnecs_entities_create_wID(tnecs_world *world, size_t num, tnecs_entity *ents) {
//     for (int i = 0; i < num; i++) {
//         if (tnecs_entity_create_wID(world, ents[i]) <= TNECS_NULL) {
//             printf("tnecs: Could not create another entity_wID.\n");
//             return(TNECS_NULL);            
//         }
//     }
//     return (num);
// }

// tnecs_entity tnecs_entity_create_wcomponents(tnecs_world *world, size_t argnum, ...) {
//     /* Get archetype of all vararg components */
//     va_list ap;
//     va_start(ap, argnum);
//     tnecs_component archetype = 0;
//     for (size_t i = 0; i < argnum; i++) {
//         tnecs_hash hash = va_arg(ap, tnecs_hash);
//         archetype += tnecs_component_hash2type(world, hash);
//     }
//     va_end(ap);

//     /* Create entity with all components */
//     tnecs_entity new_entity = tnecs_entity_create(world);
//     if (new_entity == TNECS_NULL) {
//         printf("tnecs: could not create new entity\n");
//         return(TNECS_NULL);
//     }
//     TNECS_CHECK_CALL(tnecs_entity_add_components(world, new_entity, argnum, archetype, 1));

//     /* Check */
//     size_t tID      = TNECS_TYPEFLAGID(world, archetype);
//     size_t order    = world->entity_orders[new_entity];
//     TNECS_DEBUG_ASSERT(world->entities_bytype[tID][order] == new_entity);
//     TNECS_DEBUG_ASSERT(world->entities[new_entity] == new_entity);
//     return (new_entity);
// }

// b32 tnecs_entity_destroy(tnecs_world *world, tnecs_entity entity) {
//     if (entity <= TNECS_NULL) {
//         return(1);
//     }

//     if (world->entities[entity] <= TNECS_NULL) {
//         return(1);
//     }

//     /* Preliminaries */
//     tnecs_component archetype =  world->entity_archetypes[entity];
//     size_t tID =                  TNECS_TYPEFLAGID(world, archetype);
//     size_t entity_order =         world->entity_orders[entity];
//     TNECS_DEBUG_ASSERT(world->num_entities_bytype[tID] > TNECS_NULL);
//     /* Delete components */
//     tnecs_component_del(world, entity, archetype);
//     TNECS_DEBUG_ASSERT(world->len_entities_bytype[tID] >= entity_order);
//     TNECS_DEBUG_ASSERT(world->num_entities_bytype[tID] > TNECS_NULL);
//     /* Delete entitiesbytype */
//     tnecs_entitiesbytype_del(world, entity, archetype);

//     /* Realloc entities_open if too many */
//     world->entities[entity]         = TNECS_NULL;
//     world->entity_orders[entity]    = TNECS_NULL;
//     world->entity_archetypes[entity] = TNECS_NULL;
//     if ((world->num_entities_open + 1) >= world->len_entities_open) {
//         size_t old_len              = world->len_entities_open;
//         size_t new_len              = old_len * TNECS_ARRAY_GROWTH_FACTOR;
//         size_t bytesize             = sizeof(*world->entities_open);
//         world->len_entities_open    = new_len;

//         world->entities_open = tnecs_realloc(world->entities_open, old_len, new_len, bytesize);
//         TNECS_CHECK_ALLOC(world->entities_open);
//     }

//     /* Add deleted entity to open entities */
//     world->entities_open[world->num_entities_open++] = entity;
//     TNECS_DEBUG_ASSERT(world->entities[entity]            == TNECS_NULL);
//     TNECS_DEBUG_ASSERT(world->entity_archetypes[entity]    == TNECS_NULL);
//     TNECS_DEBUG_ASSERT(world->entity_orders[entity]       == TNECS_NULL);
//     TNECS_DEBUG_ASSERT(world->entity_orders[entity_order] != entity);
//     return (world->entities[entity] == TNECS_NULL);
// }

// /*****************************************************************************/
// /***************************** TNECS INTERNALS *******************************/
// /*****************************************************************************/
// tnecs_entity tnecs_entity_add_components(tnecs_world *world, tnecs_entity entity,
//                                          size_t num_components_toadd, tnecs_component archetype_toadd, b32 isNew) {
//     if (num_components_toadd <= 0) {
//         return(TNECS_NULL);
//     }
//     if (archetype_toadd <= 0) {
//         return(TNECS_NULL);
//     }
//     tnecs_component archetype_old = world->entity_archetypes[entity];
//     TNECS_DEBUG_ASSERT(!(archetype_toadd & archetype_old));
//     tnecs_component archetype_new = archetype_toadd + archetype_old;
//     TNECS_DEBUG_ASSERT(archetype_new != archetype_old);
//     if (isNew)
//         TNECS_CHECK_CALL(_tnecs_register_archetype(world, setBits_KnR_uint64_t(archetype_new), archetype_new));

//     size_t tID_new = tnecs_archetypeid(world, archetype_new);

//     TNECS_CHECK_CALL(tnecs_component_migrate(world,      entity, archetype_old, archetype_new));
//     TNECS_CHECK_CALL(tnecs_entitiesbytype_migrate(world, entity, archetype_old, archetype_new));

//     size_t new_order = world->num_entities_bytype[tID_new] - 1;
//     TNECS_DEBUG_ASSERT(world->entity_archetypes[entity]            == archetype_new);
//     TNECS_DEBUG_ASSERT(world->entities_bytype[tID_new][new_order] == entity);
//     TNECS_DEBUG_ASSERT(world->entity_orders[entity]               == new_order);
//     return (world->entities[entity]);
// }

// b32 tnecs_entity_remove_components(tnecs_world *world, tnecs_entity entity,
//                                     size_t num_components, tnecs_component archetype) {
//     /* Get new archetype. Since it is a archetype, just need to substract. */
//     tnecs_component archetype_old = world->entity_archetypes[entity];
//     tnecs_component archetype_new = archetype_old - archetype;

//     if (archetype_new != TNECS_NULL) {
//         /* Migrate remaining components to new archetype array. */
//         TNECS_CHECK_CALL(_tnecs_register_archetype(world, setBits_KnR_uint64_t(archetype_new), archetype_new));
//         TNECS_CHECK_CALL(tnecs_component_migrate(world, entity, archetype_old, archetype_new));
//     } else {
//         /* No remaining component, delete everything. */
//         TNECS_CHECK_CALL(tnecs_component_del(world, entity, archetype_old));
//     }
//     /* Migrate entity to new bytype array. */
//     TNECS_CHECK_CALL(tnecs_entitiesbytype_migrate(world, entity, archetype_old, archetype_new));
//     TNECS_DEBUG_ASSERT(archetype_new == world->entity_archetypes[entity]);
//     return(1);
// }


// void *tnecs_entity_get_component(tnecs_world *world, tnecs_entity eID,
//                                  tnecs_component cID) {

//     tnecs_component component_flag =   TNECS_COMPONENT_ID2TYPE(cID);
//     tnecs_component entity_archetype =  TNECS_ENTITY_TYPEFLAG(world, eID);
//     void *out = NULL;
//     // If entity has component, get output it. If not output NULL.
//     if ((component_flag & entity_archetype) == 0)
//         return (out);

//     size_t tID = tnecs_archetypeid(world, entity_archetype);
//     size_t component_order = tnecs_component_order_bytype(world, cID, entity_archetype);
//     TNECS_DEBUG_ASSERT(component_order <= world->num_components_bytype[tID]);
//     size_t entity_order = world->entity_orders[eID];
//     size_t bytesize = world->component_bytesizes[cID];
//     tnecs_component_array *comp_array;
//     comp_array = &world->components_bytype[tID][component_order];
//     tnecs_byte *temp_component_bytesptr = (tnecs_byte *)(comp_array->components);
//     out = (temp_component_bytesptr + (bytesize * entity_order));
//     return (out);
// }

// b32 tnecs_entitiesbytype_add(tnecs_world *world, tnecs_entity entity,
//                                 tnecs_component archetype_new) {
//     size_t tID_new = tnecs_archetypeid(world, archetype_new);
//     if ((world->num_entities_bytype[tID_new] + 1) >= world->len_entities_bytype[tID_new]) {
//         TNECS_CHECK_CALL(tnecs_growArray_bytype(world, tID_new));
//     }
//     size_t new_order =                               world->num_entities_bytype[tID_new]++;
//     world->entity_orders[entity] =                new_order;
//     world->entity_archetypes[entity] =             archetype_new;
//     world->entities_bytype[tID_new][new_order] =  entity;
//     return(1);
// }

// b32 tnecs_entitiesbytype_del(tnecs_world *world, tnecs_entity entity,
//                                 tnecs_component archetype_old) {

//     if (entity <= TNECS_NULL) {
//         return(1);
//     }

//     if (world->entities[entity] != entity) {
//         return(1);
//     }

//     if (entity >= world->len_entities) {
//         return(1);
//     }

//     size_t archetype_old_id = tnecs_archetypeid(world, archetype_old);
//     size_t old_num = world->num_entities_bytype[archetype_old_id];
//     size_t entity_order_old = world->entity_orders[entity];
//     TNECS_DEBUG_ASSERT(old_num > 0);
//     TNECS_DEBUG_ASSERT(entity_order_old < world->len_entities_bytype[archetype_old_id]);
//     TNECS_DEBUG_ASSERT(world->entities_bytype[archetype_old_id][entity_order_old] == entity);

//     tnecs_entity top_entity = world->entities_bytype[archetype_old_id][old_num - 1];

//     /* components scrambles -> entitiesbytype too */
//     tnecs_arrdel_scramble(world->entities_bytype[archetype_old_id], entity_order_old, old_num,
//                           sizeof(**world->entities_bytype));

//     if (top_entity != entity) {
//         world->entity_orders[top_entity] = entity_order_old;
//         TNECS_DEBUG_ASSERT(world->entities_bytype[archetype_old_id][entity_order_old] == top_entity);
//     }

//     world->entity_orders[entity]    = TNECS_NULL;
//     world->entity_archetypes[entity] = TNECS_NULL;

//     --world->num_entities_bytype[archetype_old_id];
//     return(1);
// }

// b32 tnecs_entitiesbytype_migrate(tnecs_world *world, tnecs_entity entity,
//                                     tnecs_component archetype_old, tnecs_component archetype_new) {
//     /* Migrate entities into correct bytype array */
//     TNECS_CHECK_CALL(tnecs_entitiesbytype_del(world, entity, archetype_old));
//     TNECS_DEBUG_ASSERT(world->entity_archetypes[entity]  == TNECS_NULL);
//     TNECS_DEBUG_ASSERT(world->entity_orders[entity]     == TNECS_NULL);
//     TNECS_CHECK_CALL(tnecs_entitiesbytype_add(world, entity, archetype_new));

//     /* Checks */
//     size_t tID_new      = tnecs_archetypeid(world, archetype_new);
//     size_t order_new    = world->entity_orders[entity];
//     TNECS_DEBUG_ASSERT(world->entity_archetypes[entity]            == archetype_new);
//     TNECS_DEBUG_ASSERT(world->num_entities_bytype[tID_new] - 1    == order_new);
//     TNECS_DEBUG_ASSERT(world->entities_bytype[tID_new][order_new] == entity);
//     return (true);
// }

// b32 tnecs_component_add(tnecs_world *world, tnecs_component archetype) {
//     /* Check if need to grow component array after adding new component */
//     size_t tID = tnecs_archetypeid(world, archetype);
//     size_t new_comp_num = world->num_components_bytype[tID];
//     size_t new_order = world->num_entities_bytype[tID];

//     for (size_t corder = 0; corder < new_comp_num; corder++) {
//         // Take component array of current archetype_id
//         tnecs_component_array *comp_arr = &world->components_bytype[tID][corder];
//         // check if it need to grow after adding new component
//         TNECS_DEBUG_ASSERT(new_order == comp_arr->num_components);

//         if (++comp_arr->num_components >= comp_arr->len_components) {
//             size_t old_len      = comp_arr->len_components;
//             size_t new_len      = old_len * TNECS_ARRAY_GROWTH_FACTOR;
//             size_t new_comp_num = world->num_components_bytype[tID];
//             comp_arr->len_components = new_len;

//             size_t cID = world->components_idbytype[tID][corder];

//             size_t bytesize         = world->component_bytesizes[cID];
//             comp_arr->components    = tnecs_realloc(comp_arr->components, old_len, new_len, bytesize);
//             TNECS_CHECK_ALLOC(comp_arr->components);
//         }
//     }

//     return(1);
// }

// b32 tnecs_component_copy(tnecs_world *world, tnecs_entity entity,
//                           tnecs_component old_archetype, tnecs_component new_archetype) {
//     /* Copy components from old order unto top of new type component array */

//     TNECS_DEBUG_ASSERT(old_archetype != TNECS_NULL);
//     size_t old_tID =            tnecs_archetypeid(world, old_archetype);
//     size_t new_tID =            tnecs_archetypeid(world, new_archetype);
//     size_t old_entity_order =   world->entity_orders[entity];
//     size_t new_entity_order =   world->num_entities_bytype[new_tID];

//     size_t num_comp_new =          world->num_components_bytype[new_tID];
//     size_t num_comp_old =          world->num_components_bytype[old_tID];

//     for (int i = 0; i < num_comp_new; ++i) {
//         size_t num = world->components_bytype[new_tID][i].num_components;
//         TNECS_DEBUG_ASSERT((num - 1) == new_entity_order);
//     }
//     TNECS_DEBUG_ASSERT(old_archetype != TNECS_NULL);
//     TNECS_DEBUG_ASSERT(old_archetype != new_archetype);
//     size_t old_component_id, new_component_id, component_bytesize;
//     tnecs_component_array *old_array,  *new_array;
//     tnecs_byte *old_component_ptr,        *new_component_ptr;
//     tnecs_byte *old_component_bytesptr,   *new_component_bytesptr;
//     for (size_t old_corder = 0; old_corder < num_comp_old; old_corder++) {
//         old_component_id = world->components_idbytype[old_tID][old_corder];
//         for (size_t new_corder = 0; new_corder < num_comp_new; new_corder++) {
//             new_component_id = world->components_idbytype[new_tID][new_corder];
//             if (old_component_id != new_component_id)
//                 continue;

//             TNECS_DEBUG_ASSERT(old_component_id == new_component_id);
//             new_array = &world->components_bytype[new_tID][new_corder];
//             old_array = &world->components_bytype[old_tID][old_corder];
//             TNECS_DEBUG_ASSERT(old_array->type == new_array->type);
//             TNECS_DEBUG_ASSERT(old_array != new_array);
//             component_bytesize = world->component_bytesizes[old_component_id];
//             TNECS_DEBUG_ASSERT(component_bytesize > 0);
//             old_component_bytesptr = (tnecs_byte *)(old_array->components);
//             TNECS_DEBUG_ASSERT(old_component_bytesptr != NULL);
//             old_component_ptr = (old_component_bytesptr + (component_bytesize * old_entity_order));
//             TNECS_DEBUG_ASSERT(old_component_ptr != NULL);
//             new_component_bytesptr = (tnecs_byte *)(new_array->components);
//             TNECS_DEBUG_ASSERT(new_component_bytesptr != NULL);
//             new_component_ptr = (new_component_bytesptr + (component_bytesize * new_entity_order));
//             TNECS_DEBUG_ASSERT(new_component_ptr != NULL);
//             TNECS_DEBUG_ASSERT(new_component_ptr != old_component_ptr);
//             void *out =  memcpy(new_component_ptr, old_component_ptr, component_bytesize);
//             TNECS_DEBUG_ASSERT(out == new_component_ptr);
//             break;
//         }
//     }
//     return(1);
// }

// b32 tnecs_component_del(tnecs_world *world, tnecs_entity entity,
//                          tnecs_component old_archetype) {
//     /* Delete ALL components from componentsbytype at old entity order */
//     size_t old_tID      = tnecs_archetypeid(world, old_archetype);
//     size_t order_old    = world->entity_orders[entity];
//     size_t old_comp_num = world->num_components_bytype[old_tID];
//     for (size_t corder = 0; corder < old_comp_num; corder++) {
//         size_t current_component_id = world->components_idbytype[old_tID][corder];
//         tnecs_component_array *old_array = &world->components_bytype[old_tID][corder];
//         tnecs_byte *comp_ptr =             old_array->components;
//         TNECS_DEBUG_ASSERT(comp_ptr != NULL);

//         /* Scramble components too */
//         size_t comp_by       = world->component_bytesizes[current_component_id];
//         size_t new_comp_num  = world->num_entities_bytype[old_tID];
//         tnecs_byte *scramble = tnecs_arrdel_scramble(comp_ptr, order_old, new_comp_num, comp_by);
//         TNECS_CHECK_ALLOC(scramble);

//         old_array->num_components--;
//     }
//     return(1);
// }

// b32 tnecs_component_migrate(tnecs_world *world, tnecs_entity entity,
//                              tnecs_component old_archetype, tnecs_component new_archetype) {
//     TNECS_DEBUG_ASSERT(old_archetype == world->entity_archetypes[entity]);
//     TNECS_CHECK_CALL(tnecs_component_add(world,  new_archetype));
//     if (old_archetype > TNECS_NULL) {
//         TNECS_CHECK_CALL(tnecs_component_copy(world, entity, old_archetype, new_archetype));
//         TNECS_CHECK_CALL(tnecs_component_del( world, entity, old_archetype));
//     }
//     return(1);
// }

// b32 tnecs_component_array_new(tnecs_world *world, size_t num_components,
//                                tnecs_component archetype) {
//     tnecs_component_array *temp_comparray;
//     temp_comparray = calloc(num_components, sizeof(tnecs_component_array));
//     TNECS_CHECK_ALLOC(temp_comparray);

//     tnecs_component archetype_reduced = archetype, archetype_added = 0, type_toadd;
//     tnecs_component tID = tnecs_archetypeid(world, archetype);
//     size_t id_toadd, num_flags = 0;

//     while (archetype_reduced) {
//         archetype_reduced &= (archetype_reduced - 1);
//         type_toadd = (archetype_reduced + archetype_added) ^ archetype;
//         id_toadd = TNECS_COMPONENT_TYPE2ID(type_toadd);
//         TNECS_DEBUG_ASSERT(id_toadd > 0);
//         TNECS_DEBUG_ASSERT(id_toadd < world->num_components);
//         tnecs_component_array_init(world, &temp_comparray[num_flags], id_toadd);
//         num_flags++;
//         archetype_added += type_toadd;
//     }
//     world->components_bytype[tID] = temp_comparray;
//     TNECS_DEBUG_ASSERT(id_toadd < world->num_components);
//     return ((archetype_added == archetype) && (num_flags == num_components));
// }

// b32 tnecs_component_array_init(tnecs_world *world, tnecs_component_array *in_array,
//                                 size_t cID) {
//     TNECS_DEBUG_ASSERT(cID > 0);
//     TNECS_DEBUG_ASSERT(cID < world->num_components);
//     tnecs_component in_type = TNECS_COMPONENT_ID2TYPE(cID);
//     TNECS_DEBUG_ASSERT(in_type <= (1 << world->num_components));
//     size_t bytesize = world->component_bytesizes[cID];
//     TNECS_DEBUG_ASSERT(bytesize > 0);

//     in_array->type = in_type;
//     in_array->num_components    = 0;
//     in_array->len_components    = TNECS_INIT_COMPONENT_LEN;
//     in_array->components        = calloc(TNECS_INIT_COMPONENT_LEN, bytesize);
//     TNECS_CHECK_ALLOC(in_array->components);
//     return(1);
// }

// b32 tnecs_system_order_switch(tnecs_world *world, tnecs_phase phase,
//                                size_t order1, size_t order2) {
//     TNECS_DEBUG_ASSERT(world->num_phases > phase);
//     TNECS_DEBUG_ASSERT(world->phases[phase]);
//     TNECS_DEBUG_ASSERT(world->num_systems_byphase[phase] > order1);
//     TNECS_DEBUG_ASSERT(world->num_systems_byphase[phase] > order2);
//     TNECS_DEBUG_ASSERT(world->systems_byphase[phase][order1]);
//     TNECS_DEBUG_ASSERT(world->systems_byphase[phase][order2]);
//     tnecs_system_ptr systems_temp           = world->systems_byphase[phase][order1];
//     world->systems_byphase[phase][order1]   = world->systems_byphase[phase][order2];
//     world->systems_byphase[phase][order2]   = systems_temp;
//     b32 out1 = (world->systems_byphase[phase][order1] != NULL);
//     b32 out2 = (world->systems_byphase[phase][order2] != NULL);
//     return (out1 && out2);
// }

// /************************ UTILITY FUNCTIONS/MACROS ***************************/
// size_t tnecs_round_up(size_t to_round) {
//     // Round up to nearest multiple of TNECS_ROUNDING_MULTIPLE
//     size_t remainder = to_round % TNECS_ROUNDING_MULTIPLE;
//     if (remainder == 0)
//         return to_round;

//     return numToRound + multiple - remainder;
// }

// size_t tnecs_component_name2id(tnecs_world *world,
//                                const char *name) {
//     return (tnecs_component_hash2id(world, tnecs_hash_djb2(name)));
// }

// size_t tnecs_component_hash2id(tnecs_world *world, tnecs_hash hash) {
//     size_t out;
//     for (size_t i = 0; i < world->num_components; i++) {
//         if (world->component_hashes[i] == hash) {
//             out = i;
//             break;
//         }
//     }
//     return (out);
// }

// size_t tnecs_component_order_bytype(tnecs_world *world, size_t cID, tnecs_component flag) {
//     tnecs_component tID = tnecs_archetypeid(world, flag);
//     return (tnecs_component_order_bytypeid(world, cID, tID));
// }

// size_t tnecs_component_order_bytypeid(tnecs_world *world, size_t cID, size_t tID) {
//     size_t order = TNECS_COMPONENT_CAP;
//     for (size_t i = 0; i < world->num_components_bytype[tID]; i++) {
//         if (world->components_idbytype[tID][i] == cID) {
//             order = i;
//             break;
//         }
//     }
//     return (order);
// }

// tnecs_component tnecs_component_names2archetype(tnecs_world *world, size_t argnum, ...) {
//     va_list ap;
//     tnecs_component archetype = 0;
//     va_start(ap, argnum);
//     for (size_t i = 0; i < argnum; i++) {
//         archetype += world->archetypes[tnecs_component_name2id(world, va_arg(ap, const char *))];
//     }
//     va_end(ap);
//     return (archetype);
// }

// void tnecs_component_names_print(tnecs_world *world, tnecs_entity entity) {
//     tnecs_component archetype = world->entity_archetypes[entity];
//     size_t tID               = tnecs_archetypeid(world, archetype);
//     size_t comp_num          = world->num_components_bytype[tID];
//     printf("Entity %llu: ", entity);
//     for (size_t corder = 0; corder < comp_num; corder++) {
//         size_t component_id = world->components_idbytype[tID][corder];
//         printf("%s, ", world->component_names[component_id]);
//     }
//     printf("\n");
// }


// tnecs_component tnecs_component_ids2archetype(size_t argnum, ...) {
//     tnecs_component out = 0;
//     va_list ap;
//     va_start(ap, argnum);
//     for (size_t i = 0; i < argnum; i++)
//         out += TNECS_COMPONENT_ID2TYPE(va_arg(ap, size_t));
//     va_end(ap);
//     return (out);
// }

// tnecs_component tnecs_component_hash2type(tnecs_world *world, tnecs_hash hash) {
//     return (TNECS_COMPONENT_ID2TYPE(tnecs_component_hash2id(world, hash)));
// }

// size_t tnecs_system_name2id(tnecs_world *world, const char *name) {
//     tnecs_hash hash = tnecs_hash_djb2(name);
//     size_t found = 0;
//     for (size_t i = 0; i < world->num_systems; i++) {
//         if (world->system_hashes[i] == hash) {
//             found = i;
//             break;
//         }
//     }
//     return (found);
// }

// tnecs_component tnecs_system_name2archetype(tnecs_world *world,
//                                            const char *name) {
//     size_t id = tnecs_system_name2id(world, name);
//     return (world->system_archetypes[id]);
// }

// size_t tnecs_archetypeid(tnecs_world *world, tnecs_component archetype) {
//     size_t id = 0;
//     for (size_t i = 0; i < world->num_archetypes; i++) {
//         if (archetype == world->archetypes[i]) {
//             id = i;
//             break;
//         }
//     }
//     return (id);
// }

// /***************************** "DYNAMIC" ARRAYS ******************************/
// void *tnecs_realloc(void *ptr, size_t old_len, size_t new_len, size_t elem_bytesize) {
//     TNECS_DEBUG_ASSERT(ptr);
//     TNECS_DEBUG_ASSERT(new_len > old_len);
//     assert(new_len > old_len);
//     void *realloced = (void *)calloc(new_len, elem_bytesize);
//     TNECS_CHECK_ALLOC(realloced);
//     memcpy(realloced, ptr, old_len * elem_bytesize);
//     free(ptr);
//     return (realloced);
// }

// void *tnecs_arrdel(void *arr, size_t elem, size_t len, size_t bytesize) {
//     void *out;
//     tnecs_byte *bytes = arr;
//     if (elem < (len - 1)) {
//         tnecs_byte *dst = bytes + (elem * bytesize);
//         tnecs_byte *src = bytes + ((elem + 1) * bytesize);
//         out = memmove(dst, src, bytesize * (len - elem - 1));
//     } else
//         out = memset(bytes + (elem * bytesize), TNECS_NULL, bytesize);
//     return (out);
// }

// void *tnecs_arrdel_scramble(void *arr, size_t elem, size_t len, size_t bytesize) {
//     tnecs_byte *bytes = arr;
//     if (elem != (len - 1))
//         memmove(bytes + (elem * bytesize), bytes + ((len - 1) * bytesize), bytesize);

//     memset(bytes + ((len - 1) * bytesize), TNECS_NULL, bytesize);
//     return (arr);
// }

// b32 tnecs_growArray_entity(tnecs_world *world) {
//     size_t olen = world->len_entities;
//     size_t nlen = world->len_entities * TNECS_ARRAY_GROWTH_FACTOR;
//     world->len_entities = nlen;
//     if (nlen >= TNECS_ENTITIES_CAP) {
//         printf("tnecs: entities cap reached\n");
//         return(TNECS_NULL);
//     }

//     world->entities         = tnecs_realloc(world->entities,         olen, nlen, sizeof(*world->entities));
//     TNECS_CHECK_ALLOC(world->entities);
//     world->entity_orders    = tnecs_realloc(world->entity_orders,    olen, nlen, sizeof(*world->entity_orders));
//     TNECS_CHECK_ALLOC(world->entity_orders);
//     world->entity_archetypes = tnecs_realloc(world->entity_archetypes, olen, nlen, sizeof(*world->entity_archetypes));
//     TNECS_CHECK_ALLOC(world->entity_archetypes);

//     return(1);
// }

// b32 tnecs_growArray_system(tnecs_world *world) {
//     size_t olen = world->len_systems;
//     size_t nlen = olen * TNECS_ARRAY_GROWTH_FACTOR;
//     TNECS_DEBUG_ASSERT(olen > 0);
//     world->len_systems          = nlen;

//     world->system_names     = tnecs_realloc(world->system_names,     olen,       nlen,       sizeof(*world->system_names));
//     TNECS_CHECK_ALLOC(world->system_names);
//     world->system_phases    = tnecs_realloc(world->system_phases,    olen,       nlen,       sizeof(*world->system_phases));
//     TNECS_CHECK_ALLOC(world->system_phases);
//     world->system_orders    = tnecs_realloc(world->system_orders,    olen,       nlen,       sizeof(*world->system_orders));
//     TNECS_CHECK_ALLOC(world->system_orders);
//     world->system_hashes    = tnecs_realloc(world->system_hashes,    olen,       nlen,       sizeof(*world->system_hashes));
//     TNECS_CHECK_ALLOC(world->system_hashes);
//     world->system_exclusive = tnecs_realloc(world->system_exclusive, olen,       nlen,       sizeof(*world->system_exclusive));
//     TNECS_CHECK_ALLOC(world->system_exclusive);
//     world->system_archetypes = tnecs_realloc(world->system_archetypes, olen,       nlen,       sizeof(*world->system_archetypes));
//     TNECS_CHECK_ALLOC(world->system_archetypes);

//     return(1);
// }

// b32 tnecs_growArray_archetype(tnecs_world *world) {
//     size_t olen = world->len_archetypes;
//     size_t nlen = olen * TNECS_ARRAY_GROWTH_FACTOR;
//     world->len_archetypes = nlen;

//     world->archetypes                = tnecs_realloc(world->archetypes,              olen, nlen, sizeof(*world->archetypes));
//     TNECS_CHECK_ALLOC(world->archetypes);
//     world->entities_bytype          = tnecs_realloc(world->entities_bytype,        olen, nlen, sizeof(*world->entities_bytype));
//     TNECS_CHECK_ALLOC(world->entities_bytype);
//     world->components_bytype        = tnecs_realloc(world->components_bytype,      olen, nlen, sizeof(*world->components_bytype));
//     TNECS_CHECK_ALLOC(world->components_bytype);
//     world->num_archetype_ids        = tnecs_realloc(world->num_archetype_ids,      olen, nlen, sizeof(*world->num_archetype_ids));
//     TNECS_CHECK_ALLOC(world->num_archetype_ids);
//     world->num_entities_bytype      = tnecs_realloc(world->num_entities_bytype,    olen, nlen, sizeof(*world->num_entities_bytype));
//     TNECS_CHECK_ALLOC(world->num_entities_bytype);
//     world->len_entities_bytype      = tnecs_realloc(world->len_entities_bytype,    olen, nlen, sizeof(*world->len_entities_bytype));
//     TNECS_CHECK_ALLOC(world->len_entities_bytype);
//     world->components_idbytype      = tnecs_realloc(world->components_idbytype,    olen, nlen, sizeof(*world->components_idbytype));
//     TNECS_CHECK_ALLOC(world->components_idbytype);
//     world->archetype_id_bytype      = tnecs_realloc(world->archetype_id_bytype,    olen, nlen, sizeof(*world->archetype_id_bytype));
//     TNECS_CHECK_ALLOC(world->archetype_id_bytype);
//     world->num_components_bytype    = tnecs_realloc(world->num_components_bytype,  olen, nlen, sizeof(*world->num_components_bytype));
//     TNECS_CHECK_ALLOC(world->num_components_bytype);
//     world->components_orderbytype   = tnecs_realloc(world->components_orderbytype, olen, nlen, sizeof(*world->components_orderbytype));
//     TNECS_CHECK_ALLOC(world->components_orderbytype);

//     for (size_t i = olen; i < world->len_archetypes; i++) {
//         world->entities_bytype[i]       = calloc(TNECS_INIT_ENTITY_LEN, sizeof(**world->entities_bytype));
//         TNECS_CHECK_ALLOC(world->entities_bytype[i]);
//         world->archetype_id_bytype[i]   = calloc(TNECS_COMPONENT_CAP, sizeof(**world->archetype_id_bytype));
//         TNECS_CHECK_ALLOC(world->archetype_id_bytype[i]);

//         world->len_entities_bytype[i] = TNECS_INIT_ENTITY_LEN;
//         world->num_entities_bytype[i] = 0;
//     }
//     return(1);
// }

// b32 tnecs_growArray_phase(tnecs_world *world) {
//     size_t olen = world->len_phases;
//     size_t nlen = olen * TNECS_ARRAY_GROWTH_FACTOR;
//     world->len_phases = nlen;
//     if (nlen >= TNECS_PHASES_CAP) {
//         printf("tnecs: phases cap reached\n");
//         return(TNECS_NULL);
//     }

//     world->phases               = tnecs_realloc(world->phases,              olen, nlen, sizeof(*world->phases));
//     TNECS_CHECK_ALLOC(world->phases);
//     world->systems_byphase      = tnecs_realloc(world->systems_byphase,     olen, nlen, sizeof(*world->systems_byphase));
//     TNECS_CHECK_ALLOC(world->systems_byphase);
//     world->systems_idbyphase    = tnecs_realloc(world->systems_idbyphase,   olen, nlen, sizeof(*world->systems_idbyphase));
//     TNECS_CHECK_ALLOC(world->systems_idbyphase);
//     world->len_systems_byphase  = tnecs_realloc(world->len_systems_byphase, olen, nlen, sizeof(*world->len_systems_byphase));
//     TNECS_CHECK_ALLOC(world->len_systems_byphase);
//     world->num_systems_byphase  = tnecs_realloc(world->num_systems_byphase, olen, nlen, sizeof(*world->num_systems_byphase));
//     TNECS_CHECK_ALLOC(world->num_systems_byphase);

//     for (size_t i = olen; i < world->len_phases; i++) {
//         size_t bysize1 = sizeof(**world->systems_byphase);
//         size_t bysize2 = sizeof(**world->systems_idbyphase);

//         world->systems_byphase[i] =   calloc(TNECS_INIT_PHASE_LEN, bysize1);
//         TNECS_CHECK_ALLOC(world->systems_byphase[i]);
//         world->systems_idbyphase[i] = calloc(TNECS_INIT_PHASE_LEN, bysize2);
//         TNECS_CHECK_ALLOC(world->systems_idbyphase[i]);

//         world->len_systems_byphase[i] = TNECS_INIT_PHASE_LEN;
//         world->num_systems_byphase[i] = 0;
//     }
//     return(1);
// }

// b32 tnecs_growArray_bytype(tnecs_world *world, size_t tID) {
//     size_t olen = world->len_entities_bytype[tID];
//     size_t nlen = olen * TNECS_ARRAY_GROWTH_FACTOR;
//     TNECS_DEBUG_ASSERT(olen > 0);
//     world->len_entities_bytype[tID] = nlen;

//     size_t bytesize             = sizeof(*world->entities_bytype[tID]);
//     tnecs_entity *ptr           = world->entities_bytype[tID];
//     world->entities_bytype[tID] = tnecs_realloc(ptr, olen, nlen, bytesize);
//     TNECS_CHECK_ALLOC(world->entities_bytype[tID]);

//     return(1);
// }

// /****************************** STRING HASHING *******************************/
// uint64_t tnecs_hash_djb2(const char *str) {
//     /* djb2 hashing algorithm by Dan Bernstein.
//     * Description: This algorithm (k=33) was first reported by dan bernstein many
//     * years ago in comp.lang.c. Another version of this algorithm (now favored by bernstein)
//     * uses xor: hash(i) = hash(i - 1) * 33 ^ str[i]; the magic of number 33
//     * (why it works better than many other constants, prime or not) has never been adequately explained.
//     * [1] https://stackoverflow.com/questions/7666509/hash-function-for-string
//     * [2] http://www.cse.yorku.ca/~oz/hash.html */
//     uint64_t hash = 5381;
//     int32_t str_char;
//     while ((str_char = *str++))
//         hash = ((hash << 5) + hash) + str_char; /* hash * 33 + c */
//     return (hash);
// }

// uint64_t tnecs_hash_sdbm(const char *str) {
//     /* sdbm hashing algorithm by Dan Bernstein.
//     * Description: This algorithm was created for sdbm (a public-domain
//     * reimplementation of ndbm) database library. It was found to do
//     * well in scrambling bits, causing better distribution of the
//     * keys and fewer splits. It also happens to be a good general hashing
//     * function with good distribution. The actual function is
//     *hash(i) = hash(i - 1) * 65599 + str[i]; what is included below
//     * is the faster version used in gawk. [* there is even a faster,
//     * duff-device version] the magic constant 65599 was picked out of
//     * thin air while experimenting with different constants, and turns
//     * out to be a prime. this is one of the algorithms used in
//     * berkeley db (see sleepycat) and elsewhere.
//     * [1] https://stackoverflow.com/questions/7666509/hash-function-for-string
//     * [2] http://www.cse.yorku.ca/~oz/hash.html */

//     uint64_t hash = 0;
//     uint32_t str_char;
//     while ((str_char = *str++)) {
//         hash = str_char + (hash << 6) + (hash << 16) - hash;
//     }
//     return (hash);
// }

// uint64_t tnecs_hash_combine(uint64_t h1, uint64_t h2) {
//     /* SotA: need to combine couple hashes into 1. Max 4-5? */
//     /* -> Order of combination should not matter -> + or XOR  */
//     /* -> Should be simple and fast -> + or XOR */
//     return (h1 ^ h2); // XOR ignores order
// }

// /* ArchetypeChunk */
// tnecs_chunk tnecs_chunk_Init(const tnecs_world *world, const tnecs_component archetype) {
//     // Chunk init
//     tnecs_chunk chunk = {0};
//     chunk.archetype     = archetype;
//     size_t *mem_header  = tnecs_chunk_BytesizeArr(&chunk);
//     // Adding all component bytesizes in archetype to chunk
//     tnecs_component type            = 0;
//     tnecs_component component_id    = 0;
//     size_t cumul_bytesize       = 0;

//     for (int i = 1; i < world->num_components; ++i) {
//         // Checking if type in archetype
//         type = 1ULL << (i - 1);

//         // Skip if type not in archetype
//         if ((archetype & type) == 0) {
//             continue;
//         }

//         component_id = TNECS_COMPONENT_TYPE2ID(type);
        
//         // Adding component bytesize to chunk header
//         cumul_bytesize += world->component_bytesizes[component_id];
//         mem_header[chunk.components_num] = cumul_bytesize; 
//         chunk.components_num++;
//     }

//     chunk.entities_len = (TNECS_CHUNK_COMPONENTS_BYTESIZE) / cumul_bytesize;
//     return(chunk);
// }
// // Order of entity in entities_bytype -> index of chunk components are stored in
// size_t tnecs_EntityOrder_to_ArchetypeChunk(const tnecs_chunk *chunk, const size_t entity_order) {
//     return(entity_order / chunk->entities_len);
// }

// // Order of entity in entities_bytype -> order of components in current ArchetypeChunk
// size_t tnecs_EntityOrder_to_ChunkOrder(const tnecs_chunk *chunk, const size_t entity_order) {
//     return(entity_order % chunk->entities_len);
// }

// size_t *tnecs_chunk_BytesizeArr(const tnecs_chunk *chunk) {
//     return((size_t*)chunk->mem);
// }
// size_t  tnecs_chunk_TotalBytesize(const tnecs_chunk *chunk) {
//     size_t * header = tnecs_chunk_BytesizeArr(chunk);
//     TNECS_DEBUG_ASSERT(chunk->components_num > 0);
//     return(header[chunk->components_num - 1]);
// }

// void *tnecs_chunk_ComponentArr(tnecs_chunk *chunk, const size_t corder) {
//     size_t *header              = tnecs_chunk_BytesizeArr(chunk);
//     size_t cumul_bytesize       = (corder == 0) ? 0 : header[corder - 1];
//     size_t header_offset        = chunk->components_num * sizeof(size_t);
//     size_t components_offset    = corder * cumul_bytesize * chunk->entities_len;

//     tnecs_byte *bytemem = chunk->mem;
//     return(bytemem + header_offset + components_offset);
// }


// /****************************** SET BIT COUNTING *****************************/
// size_t setBits_KnR_uint64_t(uint64_t in_flags) {
//     // Credits to Kernighan and Ritchie in the C Programming Language
//     size_t count = 0;
//     while (in_flags) {
//         in_flags &= (in_flags - 1);
//         count++;
//     }
//     return (count);
// }

// /*** tnecs_arens ***/

// b32 tnecs_arena_grow(tnecs_arena *arena) {
//     return(0);
// }

b32 tnecs_arena_valid(tnecs_arena *arena) {
    if (arena == NULL)
      return(0);
    if (arena->size <= 0LL)
      return(0);
    if (arena->fill > arena->size)
      return(0);
    
    return(1);
}

// i64 tnecs_arena_push(tnecs_arena *arena, i64 size) {
//     if (!tnecs_arena_valid(arena))
//         return(0);
    
//     /* Checking if out of memory */
//     i64 new_size = size + arena->fill;
//     if (new_size > arena->size)
//          tnecs_arena_grow(arena);

//     i64 oldfill = arena->fill;
//     arena->fill += size;

//     return(oldfill);
// }

// // RISKY: USER MUST BE UPDATED WITH _Arena_Realloc_Handle.
// i64 tnecs_arena_realloc(tnecs_arena *arena, i64 handle, i64 old_len, i64 new_len) {
//     // Move over memory after handle 
//     size_t newfill = arena->fill - old_len + new_len;
    
//     if (newfill > arena->size)
//          tnecs_arena_grow(arena);
         
//     memmove(
//         arena->mem + handle + old_len,
//         arena->mem + handle + new_len,
//         arena->fill - old_len
//     );
//     return(1);
// }

// i64 tnecs_arena_realloc_handle(i64 handle, i64 new_len) {
//     return(handle + new_len);
// }

void *tnecs_arena_ptr(tnecs_arena *arena, i64 handle) {
    if (!tnecs_arena_valid(arena))
        return(NULL);
    
    if ((handle <= 0ull) || (handle >= arena->fill))
        return(NULL);
        
    return(arena->mem + handle);
}

// i64 tnecs_arena_push_zero(tnecs_arena *arena, i64 size) {
//     if (!tnecs_arena_valid(arena))
//         return(0);

//     /* _Arena_Push does the error checking */
//     i64 handle = tnecs_arena_push(arena, size);
//     if (handle < 0)
//         return(0);

//     memset(tnecs_arena_ptr(arena, handle), 0, (size_t)size);
//     return(handle);
// }


#include "tnecs.h"

/****************** PRIVATE DECLARATIONS *******************/
/* --- WORLD FUNCTIONS --- */
static b32 _tnecs_world_breath_phases(      tnecs_world *w);
static b32 _tnecs_world_breath_systems(     tnecs_world *w);
static b32 _tnecs_world_breath_entities(    tnecs_world *w);
static b32 _tnecs_world_breath_components(  tnecs_world *w);
static b32 _tnecs_world_breath_archetypes(  tnecs_world *w);

/* --- REGISTRATION  --- */
static size_t _tnecs_register_archetype(tnecs_world *w, size_t num_components,
                                        tnecs_component archetype);

/********************* WORLD FUNCTIONS ***********************/
b32 tnecs_world_genesis(tnecs_world **world) {
    /* Allocate world itself */
    if (*world != NULL) {
        TNECS_CHECK_CALL(tnecs_world_destroy(world));
    }
    *world = calloc(1, sizeof(tnecs_world));
    TNECS_CHECK_ALLOC(*world);

    /* Allocate world members */
    TNECS_CHECK_CALL(_tnecs_world_breath_phases(    *world));
    TNECS_CHECK_CALL(_tnecs_world_breath_entities(  *world));
    TNECS_CHECK_CALL(_tnecs_world_breath_systems(   *world));
    TNECS_CHECK_CALL(_tnecs_world_breath_archetypes(*world));
    TNECS_CHECK_CALL(_tnecs_world_breath_components(*world));

    return (1);
}

b32 tnecs_world_destroy(tnecs_world **world) {
    for (size_t i = 0; i < (*world)->byphase.len; i++) {
        if ((*world)->byphase.systems != NULL)
            free((*world)->byphase.systems[i]);
        if ((*world)->byphase.systems_id != NULL)
            free((*world)->byphase.systems_id[i]);
    }
    for (size_t i = 0; i < (*world)->bytype.len; i++) {
        if ((*world)->bytype.entities != NULL)
            free((*world)->bytype.entities[i]);
        if ((*world)->bytype.components_id != NULL)
            free((*world)->bytype.components_id[i]);
        if ((*world)->bytype.components_order != NULL)
            free((*world)->bytype.components_order[i]);
        if ((*world)->bytype.archetype_id != NULL)
            free((*world)->bytype.archetype_id[i]);
#ifdef TNECS_CHUNK
        if ((*world)->bytype.chunks != NULL)
            free((*world)->bytype.chunks[i]);
#else
        if ((*world)->bytype.components != NULL) {
            for (size_t j = 0; j < (*world)->bytype.num_components[i]; j++) {
                free((*world)->bytype.components[i][j].components);
            }
            free((*world)->bytype.components[i]);
        }
#endif /* TNECS_CHUNK */
    }

#ifdef TNECS_CHUNK
    free((*world)->bytype.chunks);
    free((*world)->bytype.len_chunks);
#else
    free((*world)->bytype.components);
#endif /* TNECS_CHUNK */

    free((*world)->bytype.components_id);
    free((*world)->bytype.components_order);
    free((*world)->bytype.entities);
    free((*world)->entities.orders);
    free((*world)->entities.id);
    free((*world)->entities_open.arr);
    free((*world)->entities.archetypes);
    free((*world)->bytype.len_entities);
    free((*world)->byphase.len_systems);
    free((*world)->bytype.num_entities);
    free((*world)->byphase.num_systems);
    free((*world)->bytype.num_archetype_ids);
    free((*world)->bytype.num_components);
    free((*world)->byphase.id);
    free((*world)->bytype.archetype_id);
    free((*world)->byphase.systems);
    free((*world)->systems.orders);
    free((*world)->systems.exclusive);
    free((*world)->systems_torun.arr);
    free((*world)->byphase.systems_id);
    free((*world)->systems.archetypes);
    free((*world)->systems.phases);
    free((*world)->bytype.id);
    free(*world);

    *world = NULL;
    return (1);
}

b32 tnecs_world_step(tnecs_world *world, tnecs_ns deltat, void *data) {
    world->systems_torun.num = 0;
    for (size_t phase = 0; phase < world->byphase.num; phase++) {
        if (!TNECS_PHASE_VALID(world, phase))
            continue;

        TNECS_CHECK_CALL(tnecs_world_step_phase(world, phase, deltat, data));
    }
    return (1);
}

b32 tnecs_world_step_phase(tnecs_world *world,  tnecs_phase  phase,
                           tnecs_ns     deltat, void        *data) {
    if (phase != world->byphase.id[phase]) {
        printf("tnecs: Invalid phase '%lu' \n", phase);
        return (0);
    }

    for (size_t sorder = 0; sorder < world->byphase.num_systems[phase]; sorder++) {
        size_t system_id = world->byphase.systems_id[phase][sorder];
        TNECS_CHECK_CALL(tnecs_system_run(world, system_id, deltat, data));
    }
    return (1);
}

b32 _tnecs_world_breath_components(tnecs_world *world) {
    world->components.num                   = TNECS_NULLSHIFT;
    world->components.bytesizes[TNECS_NULL] = TNECS_NULL;
    return (1);
}

b32 _tnecs_world_breath_entities(tnecs_world *world) {
    /* Variables */
    world->entities.num         = TNECS_NULLSHIFT;
    world->entities.len         = TNECS_INIT_ENTITY_LEN;
    world->entities_open.len    = TNECS_INIT_ENTITY_LEN;
    world->entities_open.num    = 0;

    /* Allocs */
    world->entities.id          = calloc(world->entities.len, sizeof(*world->entities.id));
    world->entities_open.arr    = calloc(world->entities.len, sizeof(tnecs_entity));
    world->entities.orders      = calloc(world->entities.len, sizeof(*world->entities.orders));
    world->entities.archetypes  = calloc(world->entities.len, sizeof(*world->entities.archetypes));
    TNECS_CHECK_ALLOC(world->entities.id);
    TNECS_CHECK_ALLOC(world->entities_open.arr);
    TNECS_CHECK_ALLOC(world->entities.orders);
    TNECS_CHECK_ALLOC(world->entities.archetypes);

    return (1);
}

b32 _tnecs_world_breath_phases(tnecs_world *world) {
    world->byphase.len          = TNECS_INIT_PHASE_LEN;
    world->byphase.num          = TNECS_NULLSHIFT;

    world->byphase.id                   = calloc(world->byphase.len,        sizeof(*world->byphase.id));
    world->byphase.systems              = calloc(world->byphase.len,
                                                 sizeof(*world->byphase.systems));
    world->byphase.systems_id           = calloc(world->byphase.len,
                                                 sizeof(*world->byphase.systems_id));
    world->byphase.num_systems          = calloc(world->byphase.len,
                                                 sizeof(*world->byphase.num_systems));
    world->byphase.len_systems          = calloc(world->byphase.len,
                                                 sizeof(*world->byphase.len_systems));

    TNECS_CHECK_ALLOC(world->byphase.id);
    TNECS_CHECK_ALLOC(world->byphase.systems);
    TNECS_CHECK_ALLOC(world->byphase.systems_id);
    TNECS_CHECK_ALLOC(world->byphase.num_systems);
    TNECS_CHECK_ALLOC(world->byphase.len_systems);

    /* Alloc & check for entities_byphase elements */
    for (size_t i = 0; i < world->byphase.len; i++) {
        world->byphase.systems[i]   = calloc(world->byphase.len, sizeof(**world->byphase.systems));
        TNECS_CHECK_ALLOC(world->byphase.systems[i]);
        world->byphase.systems_id[i] = calloc(world->byphase.len, sizeof(**world->byphase.systems_id));
        TNECS_CHECK_ALLOC(world->byphase.systems_id[i]);

        world->byphase.num_systems[i] = 0;
        world->byphase.len_systems[i] = world->byphase.len;
    }
    return (1);
}

b32 _tnecs_world_breath_systems(tnecs_world *world) {
    /* Variables */
    world->systems.len          = TNECS_INIT_SYSTEM_LEN;
    world->systems_torun.len    = TNECS_INIT_SYSTEM_LEN;
    world->systems.num          = TNECS_NULLSHIFT;

    /* Allocs */
    world->systems.phases               = calloc(world->systems.len,
                                                 sizeof(*world->systems.phases));
    world->systems.orders               = calloc(world->systems.len,
                                                 sizeof(*world->systems.orders));
    world->systems.archetypes           = calloc(world->systems.len,
                                                 sizeof(*world->systems.archetypes));
    world->systems.exclusive            = calloc(world->systems.len,
                                                 sizeof(*world->systems.exclusive));
    world->systems_torun.arr            = calloc(world->systems_torun.len,  sizeof(tnecs_system_ptr));

    TNECS_CHECK_ALLOC(world->systems_torun.arr);
    TNECS_CHECK_ALLOC(world->systems.phases);
    TNECS_CHECK_ALLOC(world->systems.orders);
    TNECS_CHECK_ALLOC(world->systems.archetypes);
    TNECS_CHECK_ALLOC(world->systems.exclusive);

    return (1);
}

b32 _tnecs_world_breath_archetypes(tnecs_world *world) {
    /* Variables */
    world->bytype.num = TNECS_NULLSHIFT;
    world->bytype.len = TNECS_INIT_SYSTEM_LEN;

    /* Allocs */
    world->bytype.id                    = calloc(world->bytype.len, sizeof(*world->bytype.id));
    world->bytype.entities              = calloc(world->bytype.len, sizeof(*world->bytype.entities));
    world->bytype.len_entities          = calloc(world->bytype.len,
                                                 sizeof(*world->bytype.len_entities));
    world->bytype.num_entities          = calloc(world->bytype.len,
                                                 sizeof(*world->bytype.num_entities));
    world->bytype.archetype_id          = calloc(world->bytype.len,
                                                 sizeof(*world->bytype.archetype_id));
    world->bytype.components_id         = calloc(world->bytype.len,
                                                 sizeof(*world->bytype.components_id));
    world->bytype.num_components        = calloc(world->bytype.len,
                                                 sizeof(*world->bytype.num_components));
    world->bytype.components_order      = calloc(world->bytype.len,
                                                 sizeof(*world->bytype.components_order));
    world->bytype.num_archetype_ids     = calloc(world->bytype.len,
                                                 sizeof(*world->bytype.num_archetype_ids));

    TNECS_CHECK_ALLOC(world->bytype.id);
    TNECS_CHECK_ALLOC(world->bytype.entities);
    TNECS_CHECK_ALLOC(world->bytype.archetype_id);
    TNECS_CHECK_ALLOC(world->bytype.len_entities);
    TNECS_CHECK_ALLOC(world->bytype.num_entities);
    TNECS_CHECK_ALLOC(world->bytype.components_id);
    TNECS_CHECK_ALLOC(world->bytype.num_components);
    TNECS_CHECK_ALLOC(world->bytype.components_order);
    TNECS_CHECK_ALLOC(world->bytype.num_archetype_ids);


#ifdef TNECS_CHUNK
    world->bytype.len_chunks            = calloc(world->bytype.len,
                                                 sizeof(*world->bytype.len_chunks));
    TNECS_CHECK_ALLOC(world->bytype.len_chunks);
    world->bytype.chunks                = calloc(world->bytype.len, sizeof(*world->bytype.chunks));
    TNECS_CHECK_ALLOC(world->bytype.chunks);
#else
    world->bytype.components            = calloc(world->bytype.len, sizeof(*world->bytype.components));
    TNECS_CHECK_ALLOC(world->bytype.components);
#endif /* TNECS_CHUNK */

    /* Alloc & check for id_bytype elements */
    for (size_t i = 0; i < world->bytype.len; i++) {
        world->bytype.archetype_id[i] = calloc(TNECS_COMPONENT_CAP, sizeof(**world->bytype.archetype_id));
        TNECS_CHECK_ALLOC(world->bytype.archetype_id[i]);
        world->bytype.entities[i] =     calloc(TNECS_INIT_ENTITY_LEN, sizeof(**world->bytype.entities));
        TNECS_CHECK_ALLOC(world->bytype.entities[i]);

        world->bytype.num_entities[i] = 0;
        world->bytype.len_entities[i] = TNECS_INIT_ENTITY_LEN;
    }
    return (1);
}

/**************************** SYSTEM FUNCTIONS ********************************/
b32 tnecs_custom_system_run(tnecs_world *world, tnecs_system_ptr custom_system,
                            tnecs_component archetype, tnecs_ns deltat, void *data) {
    /* Building the systems input */
    tnecs_system_input input = {.world = world, .deltat = deltat, .data = data};
    size_t tID = tnecs_archetypeid(world, archetype);
    if (tID == TNECS_NULL) {
        printf("tnecs: Input archetype is unknown.\n");
        return (0);
    }

    /* Running the exclusive custom system */
    input.entity_archetype_id    = tID;
    input.num_entities          = world->bytype.num_entities[input.entity_archetype_id];
    custom_system(&input);

    /* Running the non-exclusive/inclusive custom system */
    for (size_t tsub = 0; tsub < world->bytype.num_archetype_ids[tID]; tsub++) {
        input.entity_archetype_id    = world->bytype.archetype_id[tID][tsub];
        input.num_entities          = world->bytype.num_entities[input.entity_archetype_id];
        custom_system(&input);
    }
    return (1);
}

b32 tnecs_system_run(tnecs_world *world, size_t in_system_id,
                     tnecs_ns     deltat, void *data) {
    /* Building the systems input */
    tnecs_system_input input = {.world = world, .deltat = deltat, .data = data};
    size_t sorder               = world->systems.orders[in_system_id];
    tnecs_phase phase           = world->systems.phases[in_system_id];
    size_t system_archetype_id   = tnecs_archetypeid(world, world->systems.archetypes[in_system_id]);

    input.entity_archetype_id    = system_archetype_id;
    input.num_entities          = world->bytype.num_entities[input.entity_archetype_id];

    /* Running the exclusive systems in current phase */
    while (world->systems_torun.num >= (world->systems_torun.len - 1)) {
        TNECS_CHECK_CALL(tnecs_grow_torun(world));
    }

    tnecs_system_ptr system                 = world->byphase.systems[phase][sorder];
    size_t system_num                       = world->systems_torun.num++;
    tnecs_system_ptr *system_ptr            = world->systems_torun.arr;
    system_ptr[system_num]                  = world->byphase.systems[phase][sorder];
    system(&input);

    if (world->systems.exclusive[in_system_id])
        return (1);

    /* Running the inclusive systems in current phase */
    for (size_t tsub = 0; tsub < world->bytype.num_archetype_ids[system_archetype_id]; tsub++) {
        input.entity_archetype_id    = world->bytype.archetype_id[system_archetype_id][tsub];
        input.num_entities          = world->bytype.num_entities[input.entity_archetype_id];
        while (world->systems_torun.num >= (world->systems_torun.len - 1)) {
            TNECS_CHECK_CALL(tnecs_grow_torun(world));
        }
        tnecs_system_ptr system                 = world->byphase.systems[phase][sorder];
        size_t system_num                       = world->systems_torun.num++;
        tnecs_system_ptr *system_ptr            = world->systems_torun.arr;
        system_ptr[system_num]                  = system;
        system(&input);
    }
    return (1);
}

/***************************** REGISTRATION **********************************/
size_t tnecs_register_system(tnecs_world *world,
                             tnecs_system_ptr in_system, tnecs_phase phase,
                             b32 isExclusive, size_t num_components, tnecs_component components_archetype) {
    /* Compute new id */
    size_t system_id = world->systems.num++;

    /* Realloc systems if too many */
    if (world->systems.num >= world->systems.len)
        TNECS_CHECK_CALL(tnecs_grow_system(world));

    /* Realloc systems_byphase if too many */
    if (world->byphase.num_systems[phase] >= world->byphase.len_systems[phase])
        TNECS_CHECK_CALL(tnecs_grow_system_byphase(world, phase));

    /* -- Actual registration -- */
    /* Register new phase if didn't exist */
    if (!world->byphase.id[phase])
        TNECS_CHECK_CALL(tnecs_register_phase(world, phase));

    world->systems.exclusive[system_id]     = isExclusive;
    world->systems.phases[system_id]        = phase;
    world->systems.archetypes[system_id]    = components_archetype;

    /* System order */
    size_t system_order                             = world->byphase.num_systems[phase]++;
    world->systems.orders[system_id]                = system_order;
    world->byphase.systems[phase][system_order]     = in_system;
    world->byphase.systems_id[phase][system_order]  = system_id;
    TNECS_CHECK_CALL(_tnecs_register_archetype(world, num_components, components_archetype));
    return (system_id);
}

tnecs_component tnecs_register_component(tnecs_world    *world,
                                         size_t    bytesize) {
    /* Checks */
    if (bytesize <= 0) {
        printf("tnecs: Component should have >0 bytesize.\n");
        return (TNECS_NULL);
    }
    if (world->components.num >= TNECS_COMPONENT_CAP) {
        printf("tnecs: Component capacity reached.\n");
        return (TNECS_NULL);
    }

    /* Registering */
    tnecs_component new_component_id                = world->components.num++;
    tnecs_component new_component_flag              = TNECS_COMPONENT_ID2TYPE(new_component_id);
    world->components.bytesizes[new_component_id]   = bytesize;
    _tnecs_register_archetype(world, 1, new_component_flag);
    return (new_component_id);
}

size_t _tnecs_register_archetype(tnecs_world *world, size_t num_components,
                                 tnecs_component archetype_new) {
    // 0- Check if archetype exists, return
    for (size_t i = 0 ; i < world->bytype.num; i++) {
        if (archetype_new == world->bytype.id[i]) {
            return(i);
            break;
        }
    }

    // 1- Add new bytype.components at [tID]
    if ((world->bytype.num + 1) >= world->bytype.len)
        tnecs_grow_archetype(world);
    world->bytype.id[world->bytype.num++] = archetype_new;
    size_t tID = tnecs_archetypeid(world, archetype_new);
    assert(tID == (world->bytype.num - 1));
    world->bytype.num_components[tID] = num_components;

    // 2- Add arrays to bytype.components[tID] for each component
#ifdef TNECS_CHUNK
    TNECS_CHECK_CALL(tnecs_chunk_new(world, archetype_new));
#else
    TNECS_CHECK_CALL(tnecs_carr_new(world, num_components, archetype_new));
#endif /* TNECS_CHUNK */

    // 3- Add all components to bytype.components_id
    tnecs_component component_id_toadd, component_type_toadd;
    tnecs_component archetype_reduced = archetype_new, archetype_added = 0;
    size_t bytesize1 = sizeof(**world->bytype.components_id);
    size_t bytesize2 = sizeof(**world->bytype.components_order);
    world->bytype.components_id[tID]     = calloc(num_components,      bytesize1);
    TNECS_CHECK_ALLOC(world->bytype.components_id[tID]);
    world->bytype.components_order[tID]  = calloc(TNECS_COMPONENT_CAP, bytesize2);
    TNECS_CHECK_ALLOC(world->bytype.components_order[tID]);

    size_t i = 0;
    while (archetype_reduced) {
        archetype_reduced &= (archetype_reduced - 1);

        component_type_toadd = (archetype_reduced + archetype_added) ^ archetype_new;
        archetype_added      += component_type_toadd;
        assert(component_type_toadd > 0);
        component_id_toadd   = TNECS_COMPONENT_TYPE2ID(component_type_toadd);

        world->bytype.components_id[tID][i]      = component_id_toadd;

        world->bytype.components_order[tID][component_id_toadd] = i++;
    }

    // 4- Check archetypes.
    for (size_t i = 1 ; i < world->bytype.num; i++) {
        world->bytype.num_archetype_ids[i] = 0;
        for (size_t j = 1 ; j < (world->bytype.num); j++) {
            if (i == j)
                continue;

            if (!TNECS_ARCHETYPE_IS_SUBTYPE(world->bytype.id[i], world->bytype.id[j]))
                continue;

            // j is an archetype of i
            world->bytype.archetype_id[i][world->bytype.num_archetype_ids[i]++] = j;
        }
    }

    return (tID);
}

size_t tnecs_register_phase(tnecs_world *world, tnecs_phase phase) {
    if (phase <= 0)
        return (1);

    while (phase >= world->byphase.len) {
        TNECS_CHECK_CALL(tnecs_grow_phase(world));
    }
    world->byphase.id[phase]    = phase;
    world->byphase.num          = (phase >= world->byphase.num) ? (phase + 1) : world->byphase.num;
    return (phase);
}

/***************************** ENTITY MANIPULATION ***************************/
tnecs_entity tnecs_entity_create(tnecs_world *world) {
    tnecs_entity out = TNECS_NULL;

    /* Check if an open entity exists */
    tnecs_entity *arr = world->entities_open.arr;
    while ((out == TNECS_NULL) && (world->entities_open.num > 0)
           && (world->entities_open.num < TNECS_ENTITIES_CAP)) {
        out = arr[--world->entities_open.num];
        arr[world->entities_open.num] = TNECS_NULL;
    }

    /* If no open entity existed, create one */
    if (out == TNECS_NULL) {
        do {
            if (world->entities.num >= world->entities.len) {
                if (!tnecs_grow_entity(world)) {
                    printf("tnecs: Could not allocate more memory for entities.\n");
                    return (TNECS_NULL);
                }
            }
        } while (world->entities.id[out = world->entities.num++] != TNECS_NULL);
    }
    assert(out != TNECS_NULL);

    /* Set entity and checks  */
    world->entities.id[out] = out;
    tnecs_entitiesbytype_add(world, out, TNECS_NULL);
    assert(world->entities.id[out]                                          == out);
    assert(world->bytype.entities[TNECS_NULL][world->entities.orders[out]]  == out);
    return (out);
}

tnecs_entity tnecs_entities_create(tnecs_world *world, size_t num) {
    for (int i = 0; i < num; i++) {
        if (tnecs_entity_create(world) <= TNECS_NULL) {
            printf("tnecs: Could not create another entity.\n");
            return (TNECS_NULL);
        }
    }
    return (num);
}

tnecs_entity tnecs_entity_create_wcomponents(tnecs_world *world, size_t argnum, ...) {
    /* Get archetype of all vararg components ids */
    va_list ap;
    va_start(ap, argnum);
    tnecs_component archetype = 0;
    for (size_t i = 0; i < argnum; i++) {
        tnecs_component component_id = va_arg(ap, tnecs_component);
        archetype += TNECS_COMPONENT_ID2TYPE(component_id);
    }
    va_end(ap);

    /* Create entity with all components */
    tnecs_entity new_entity = tnecs_entity_create(world);
    if (new_entity == TNECS_NULL) {
        printf("tnecs: could not create new entity\n");
        return (TNECS_NULL);
    }
    TNECS_CHECK_CALL(tnecs_entity_add_components(world, new_entity, archetype, 1));

    #ifndef NDEBUG 
    size_t tID      = tnecs_archetypeid(world, archetype);
    size_t order    = world->entities.orders[new_entity];
    assert(world->bytype.entities[tID][order]   == new_entity);
    assert(world->entities.id[new_entity]       == new_entity);
    #endif /* NDEBUG */ 

    return (new_entity);
}

b32 tnecs_entities_open_reuse(tnecs_world *world) {
    // Check for open entities. If not in entities_open, add them.

    for (tnecs_entity i = TNECS_NULLSHIFT; i < world->entities.num; i++) {
        if ((world->entities.id[i] == TNECS_NULL) && !tnecs_entity_isOpen(world, i)) {
            TNECS_CHECK_CALL(tnecs_grow_entities_open(world));
            tnecs_entity *arr = world->entities_open.arr;
            arr[world->entities_open.num++] = i;
        }
    }
    return (1);
};

b32 tnecs_entities_open_flush(tnecs_world *world) {
    // Get rid of all entities in entities_open.
    world->entities_open.num = 0;
    return (1);
}

b32 tnecs_entity_isOpen(tnecs_world *world, tnecs_entity entity) {
    if (entity <= TNECS_NULL) {
        return (0);
    }

    tnecs_entity *open_arr = world->entities_open.arr;

    for (tnecs_entity i = TNECS_NULLSHIFT; i < world->entities_open.num; i++) {
        if (open_arr[i] == entity) {
            return (1);
        }
    }
    return (0);
}


b32 tnecs_entity_destroy(tnecs_world *world, tnecs_entity entity) {
    if (entity <= TNECS_NULL) {
        return (1);
    }

    if (world->entities.id[entity] <= TNECS_NULL) {
        world->entities.id[entity]         = TNECS_NULL;
        world->entities.orders[entity]     = TNECS_NULL;
        world->entities.archetypes[entity] = TNECS_NULL;
        return (1);
    }

    /* Preliminaries */
    tnecs_component archetype   = world->entities.archetypes[entity];

    /* Delete components */
    TNECS_CHECK_CALL(tnecs_component_del(world, entity, archetype));

    #ifndef NDEBUG     
    size_t entity_order         = world->entities.orders[entity];
    size_t tID                  = tnecs_archetypeid(world, archetype);
    assert(world->bytype.num_entities[tID] > TNECS_NULL);
    assert(world->bytype.len_entities[tID] >= entity_order);
    assert(world->bytype.num_entities[tID] > TNECS_NULL);
    #endif /* NDEBUG */ 

    /* Delete entitiesbytype */
    TNECS_CHECK_CALL(tnecs_entitiesbytype_del(world, entity, archetype));

    /* Delete entity */
    world->entities.id[entity]         = TNECS_NULL;

    // Note: reuse_entities used to add to entities_open, so that
    // user can call tnecs_entities_open_reuse to reuse entities manually.
    if (world->reuse_entities) {
        /* Add deleted entity to open entities */
        TNECS_CHECK_CALL(tnecs_grow_entities_open(world));
        tnecs_entity *arr = world->entities_open.arr;
        arr[world->entities_open.num++] = entity;
    }
    assert(world->entities.id[entity]           == TNECS_NULL);
    assert(world->entities.orders[entity]       == TNECS_NULL);
    assert(world->entities.archetypes[entity]   == TNECS_NULL);
    assert(world->entities.orders[entity_order] != entity);
    return (1);
}

/*****************************************************************************/
/***************************** TNECS INTERNALS *******************************/
/*****************************************************************************/
tnecs_entity tnecs_entity_add_components(tnecs_world *world, tnecs_entity entity, tnecs_component archetype_toadd, b32 isNew) {
    if (archetype_toadd <= 0) {
        return (TNECS_NULL);
    }

    if (!TNECS_ENTITY_EXISTS(world, entity)) {
        return (TNECS_NULL);
    }

    tnecs_component archetype_old = world->entities.archetypes[entity];
    
    if (TNECS_ARCHETYPE_HAS_TYPE(archetype_toadd, archetype_old)) {
        return (entity);
    }

    tnecs_component archetype_new = archetype_toadd + archetype_old;
    assert(archetype_new != archetype_old);
    if (isNew)
        TNECS_CHECK_CALL(_tnecs_register_archetype(world, setBits_KnR_u64(archetype_new),
                                                   archetype_new));

    TNECS_CHECK_CALL(tnecs_component_migrate(world,      entity, archetype_old, archetype_new));
    TNECS_CHECK_CALL(tnecs_entitiesbytype_migrate(world, entity, archetype_old, archetype_new));

    #ifndef NDEBUG 
    size_t tID_new = tnecs_archetypeid(world, archetype_new);
    size_t new_order = world->bytype.num_entities[tID_new] - 1;
    assert(world->entities.archetypes[entity]           == archetype_new);
    assert(world->bytype.entities[tID_new][new_order]   == entity);
    assert(world->entities.orders[entity]               == new_order);
    #endif /* NDEBUG */ 
    return (world->entities.id[entity]);
}

b32 tnecs_entity_remove_components(tnecs_world *world, tnecs_entity entity, tnecs_component archetype) {
    /* Get new archetype. Since it is a archetype, just need to substract. */
    tnecs_component archetype_old = world->entities.archetypes[entity];
    tnecs_component archetype_new = archetype_old - archetype;

    if (archetype_new != TNECS_NULL) {
        /* Migrate remaining components to new archetype array. */
        TNECS_CHECK_CALL(_tnecs_register_archetype(world, setBits_KnR_u64(archetype_new),
                                                   archetype_new));
        TNECS_CHECK_CALL(tnecs_component_migrate(world, entity, archetype_old, archetype_new));
    } else {
        /* No remaining component, delete everything. */
        TNECS_CHECK_CALL(tnecs_component_del(world, entity, archetype_old));
    }
    /* Migrate entity to new bytype array. */
    TNECS_CHECK_CALL(tnecs_entitiesbytype_migrate(world, entity, archetype_old, archetype_new));
    assert(archetype_new == world->entities.archetypes[entity]);
    return (1);
}

void *tnecs_get_component(tnecs_world *world, tnecs_entity eID, tnecs_component cID) {
    if (!TNECS_ENTITY_EXISTS(world, eID))
        return(NULL);

    tnecs_component component_flag      = TNECS_COMPONENT_ID2TYPE(cID);
    tnecs_component entity_archetype    = TNECS_ENTITY_ARCHETYPE(world, eID);
    // If entity has component, get output it. If not output NULL.
    if (!TNECS_ARCHETYPE_HAS_TYPE(component_flag, entity_archetype))
        return (NULL);

    size_t tID = tnecs_archetypeid(world, entity_archetype);
    assert(tID > 0);
    size_t component_order = tnecs_component_order_bytype(world, cID, entity_archetype);
    assert(component_order <= world->bytype.num_components[tID]);
    size_t entity_order = world->entities.orders[eID];
    size_t bytesize     = world->components.bytesizes[cID];

    void *out = NULL;
#ifdef TNECS_CHUNK
    tnecs_chunk *chunk  = tnecs_chunk_top(world, entity_order, tID);
    void *comp_array    = tnecs_chunk_component_array(chunk, component_order);
    assert(comp_array != NULL);
    tnecs_byte *temp_component_bytesptr = (tnecs_byte *)(comp_array);
    size_t chunk_component_order = tnecs_chunk_component_order(chunk, entity_order);
    out = temp_component_bytesptr + (bytesize * chunk_component_order);
#else
    tnecs_carr *comp_array = &world->bytype.components[tID][component_order];
    assert(comp_array != NULL);
    tnecs_byte *temp_component_bytesptr = (tnecs_byte *)(comp_array->components);
    out = temp_component_bytesptr + (bytesize * entity_order);
#endif /* TNECS_CHUNK */

    return (out);
}

b32 tnecs_entitiesbytype_add(tnecs_world *world, tnecs_entity entity,
                             tnecs_component archetype_new) {
    size_t tID_new = tnecs_archetypeid(world, archetype_new);
    if ((world->bytype.num_entities[tID_new] + 1) >= world->bytype.len_entities[tID_new]) {
        TNECS_CHECK_CALL(tnecs_grow_bytype(world, tID_new));
    }
    size_t new_order                            = world->bytype.num_entities[tID_new]++;
    world->entities.orders[entity]              = new_order;
    world->entities.archetypes[entity]          = archetype_new;
    world->bytype.entities[tID_new][new_order]  = entity;
    return (1);
}

b32 tnecs_entitiesbytype_del(tnecs_world *world, tnecs_entity entity,
                             tnecs_component archetype_old) {

    if (!TNECS_ENTITY_EXISTS(world, entity))
        return (1);

    if (entity >= world->entities.len)
        return (1);

    size_t archetype_old_id = tnecs_archetypeid(world, archetype_old);
    size_t old_num          = world->bytype.num_entities[archetype_old_id];
    if (old_num <= 0)
        return (1);

    size_t entity_order_old = world->entities.orders[entity];
    assert(archetype_old == world->entities.archetypes[entity]);

    assert(entity_order_old < world->bytype.len_entities[archetype_old_id]);
    assert(world->bytype.entities[archetype_old_id][entity_order_old] == entity);

    tnecs_entity top_entity = world->bytype.entities[archetype_old_id][old_num - 1];

    /* components scrambles -> entitiesbytype too */
    tnecs_arrdel(world->bytype.entities[archetype_old_id], entity_order_old, old_num,
                 sizeof(**world->bytype.entities));

    if (top_entity != entity) {
        world->entities.orders[top_entity] = entity_order_old;
        assert(world->bytype.entities[archetype_old_id][entity_order_old] == top_entity);
    }

    world->entities.orders[entity]      = TNECS_NULL;
    world->entities.archetypes[entity]  = TNECS_NULL;

    --world->bytype.num_entities[archetype_old_id];
    return (1);
}

b32 tnecs_entitiesbytype_migrate(tnecs_world *world, tnecs_entity entity,
                                 tnecs_component archetype_old, tnecs_component archetype_new) {
    /* Migrate entities into correct bytype array */
    TNECS_CHECK_CALL(tnecs_entitiesbytype_del(world, entity, archetype_old));
    assert(world->entities.archetypes[entity]   == TNECS_NULL);
    assert(world->entities.orders[entity]       == TNECS_NULL);
    TNECS_CHECK_CALL(tnecs_entitiesbytype_add(world, entity, archetype_new));

    #ifndef NDEBUG 
    size_t tID_new      = tnecs_archetypeid(world, archetype_new);
    size_t order_new    = world->entities.orders[entity];
    assert(world->entities.archetypes[entity]         == archetype_new);
    assert(world->bytype.num_entities[tID_new] - 1    == order_new);
    assert(world->bytype.entities[tID_new][order_new] == entity);
    #endif /* NDEBUG */ 
    return (1);
}

b32 tnecs_component_add(tnecs_world *world, tnecs_component archetype) {
    /* Check if need to grow component array after adding new component */
    size_t tID          = tnecs_archetypeid(world, archetype);
    size_t new_comp_num = world->bytype.num_components[tID];
    size_t entity_order = world->bytype.num_entities[tID];

    for (size_t corder = 0; corder < new_comp_num; corder++) {
    #ifdef NDEBUG 
        size_t new_order    = world->bytype.num_entities[tID];
    #endif /* NDEBUG */ 

    #ifdef TNECS_CHUNK
        // Take component array of current archetype_id
        tnecs_chunk *chunks = world->bytype.chunks[tID];
        size_t chunk_order  = tnecs_chunk_order(chunks, entity_order);

        if (chunk_order >= world->bytype.len_chunks[tID]) {
            TNECS_CHECK_CALL(tnecs_grow_chunks(world, tID, corder));
        }
    #else
        // Take component array of current archetype_id
        tnecs_carr *comp_arr = &world->bytype.components[tID][corder];
        // check if it need to grow after adding new component
        assert(entity_order == comp_arr->num);

        if (++comp_arr->num >= comp_arr->len)
            tnecs_grow_component_array(world, comp_arr, tID, corder);

    #endif /* TNECS_CHUNK */

    }

    return (1);
}


b32 tnecs_component_chunk_copy(tnecs_world *world, const tnecs_entity entity,
                        const tnecs_component old_archetype, const tnecs_component new_archetype) {
    /* Copy components from old order unto top of new type chunk */
    if (old_archetype == new_archetype) {
        return(1);
    }

    size_t old_tID          = tnecs_archetypeid(world, old_archetype);
    size_t new_tID          = tnecs_archetypeid(world, new_archetype);
    size_t old_entity_order = world->entities.orders[entity];
    size_t new_entity_order = world->bytype.num_entities[new_tID];
    size_t num_comp_new     = world->bytype.num_components[new_tID];
    size_t num_comp_old     = world->bytype.num_components[old_tID];

    size_t old_chunk_component_order, new_chunk_component_order;
    size_t old_component_id, new_component_id, component_bytesize;
    tnecs_chunk             *old_top_chunk,             *new_top_chunk;
    void                    *old_array,                 *new_array;
    tnecs_byte              *old_component_ptr,         *new_component_ptr;
    tnecs_byte              *old_component_bytesptr,    *new_component_bytesptr;
    
    for (size_t old_corder = 0; old_corder < num_comp_old; old_corder++) {
        old_component_id = world->bytype.components_id[old_tID][old_corder];
        for (size_t new_corder = 0; new_corder < num_comp_new; new_corder++) {
            new_component_id = world->bytype.components_id[new_tID][new_corder];
            if (old_component_id != new_component_id)
                continue;

            new_top_chunk = tnecs_chunk_top(world, new_entity_order, new_tID);
            old_top_chunk = tnecs_chunk_top(world, old_entity_order, old_tID);
            assert(new_top_chunk->num_components == num_comp_new);
            assert(old_top_chunk->num_components == num_comp_old);

            new_chunk_component_order = tnecs_chunk_component_order(new_top_chunk, new_entity_order);
            old_chunk_component_order = tnecs_chunk_component_order(old_top_chunk, old_entity_order);

            assert(old_corder < old_top_chunk->num_components);
            assert(new_corder < new_top_chunk->num_components);

            new_array = tnecs_chunk_component_array(new_top_chunk, new_corder);
            old_array = tnecs_chunk_component_array(old_top_chunk, old_corder);

            assert(new_array);
            assert(old_array);

            component_bytesize = world->components.bytesizes[old_component_id];
            assert(component_bytesize > 0);

            old_component_bytesptr = (tnecs_byte *)(old_array);
            assert(old_component_bytesptr != NULL);
            
            old_component_ptr = (old_component_bytesptr + (component_bytesize * old_chunk_component_order));
            assert(old_component_ptr != NULL);
            
            new_component_bytesptr = (tnecs_byte *)(new_array);
            assert(new_component_bytesptr != NULL);
            
            new_component_ptr = (new_component_bytesptr + (component_bytesize * new_chunk_component_order));
            assert(new_component_ptr != NULL);
            assert(new_component_ptr != old_component_ptr);
            
            void *out = memcpy(new_component_ptr, old_component_ptr, component_bytesize);
            assert(out == new_component_ptr);
            break;
        }
    }
    return(1);
}

b32 tnecs_component_chunk_del(tnecs_world *world, tnecs_entity entity, tnecs_component old_archetype) {
    /* Delete ALL components from componentsbytype at old entity order */
    size_t old_tID          = tnecs_archetypeid(world, old_archetype);
    if (old_tID == TNECS_NULL) 
        return(0);

    size_t entity_order_old = world->entities.orders[entity];
    size_t old_comp_num     = world->bytype.num_components[old_tID];
    tnecs_chunk *chunks     = world->bytype.chunks[old_tID];
    assert(old_comp_num == chunks->num_components);
    size_t chunk_order  = tnecs_chunk_order(chunks, entity_order_old);
    assert(chunk_order < tnecs_chunk_len(chunks, world, old_tID));

    for (size_t corder = 0; corder < old_comp_num; corder++) {
        size_t current_component_id = world->bytype.components_id[old_tID][corder];

        assert(chunks->num_components == chunks[chunk_order].num_components);

        tnecs_byte  *comp_ptr           = tnecs_chunk_component_array(&chunks[chunk_order], corder);
        assert(comp_ptr != NULL);


        /* Scramble components too */
        size_t bytesize         = world->components.bytesizes[current_component_id];
        size_t new_comp_num     = chunks->len_entities;
        assert(chunks->len_entities > 0);
        // printf("%zu %zu %zu \n", entity_order_old, chunks->len_entities, chunk_order);
        assert(entity_order_old <= (chunks->len_entities * (chunk_order + 1)));
        assert(new_comp_num * bytesize < TNECS_CHUNK_COMPONENTS_BYTESIZE);
        
        tnecs_byte *scramble    = tnecs_arrdel(comp_ptr, entity_order_old, new_comp_num, bytesize);
        TNECS_CHECK_ALLOC(scramble);
        // Somehow this arrdel makes num_components 0? 
        assert(chunks->num_components == chunks[chunk_order].num_components);
    }
    return(1);
}

b32 tnecs_component_chunk_migrate(tnecs_world *world, tnecs_entity entity,
                             tnecs_component old_archetype, tnecs_component new_archetype) {
    if (old_archetype != world->entities.archetypes[entity]) {
        return(0);
    }
    TNECS_CHECK_CALL(tnecs_component_add(world, new_archetype));
    if (old_archetype > TNECS_NULL) {
        TNECS_CHECK_CALL(tnecs_component_copy(world, entity, old_archetype, new_archetype));
        TNECS_CHECK_CALL(tnecs_component_del( world, entity, old_archetype));
    }
    return(1);
}


b32 tnecs_component_copy(tnecs_world *world, tnecs_entity entity,
                         tnecs_component old_archetype, tnecs_component new_archetype) {
#ifdef TNECS_CHUNK
    return(tnecs_component_chunk_copy(world, entity, old_archetype, new_archetype));
#else
    return(tnecs_component_carr_copy(world, entity, old_archetype, new_archetype));
#endif /* TNECS_CHUNK */
}
b32 tnecs_component_del(tnecs_world *world, tnecs_entity entity,
                        tnecs_component old_archetype) {
#ifdef TNECS_CHUNK
    return(tnecs_component_chunk_del(world, entity, old_archetype));
#else
    return(tnecs_component_carr_del(world, entity, old_archetype));
#endif /* TNECS_CHUNK */
}

b32 tnecs_component_migrate(tnecs_world *world, tnecs_entity entity,
                            tnecs_component old_archetype, tnecs_component new_archetype) {
#ifdef TNECS_CHUNK
    return(tnecs_component_chunk_migrate(world, entity, old_archetype, new_archetype));
#else
    return(tnecs_component_carr_migrate(world, entity, old_archetype, new_archetype));
#endif /* TNECS_CHUNK */
}

b32 tnecs_component_carr_copy(tnecs_world *world, tnecs_entity entity,
                         tnecs_component old_archetype, tnecs_component new_archetype) {
    /* Copy components from old order unto top of new type component array */
    if (old_archetype == new_archetype) {
        return (1);
    }

    size_t old_tID          = tnecs_archetypeid(world, old_archetype);
    size_t new_tID          = tnecs_archetypeid(world, new_archetype);
    size_t old_entity_order = world->entities.orders[entity];
    size_t new_entity_order = world->bytype.num_entities[new_tID];
    size_t num_comp_new     = world->bytype.num_components[new_tID];
    size_t num_comp_old     = world->bytype.num_components[old_tID];

#ifndef NDEBUG
    // Sanity check: entity order is the same in new components array
    for (int i = 0; i < num_comp_new; ++i) {
        size_t num = world->bytype.components[new_tID][i].num;
        assert((num - 1) == new_entity_order);
    }
#endif /* NDEBUG */

    size_t old_component_id, new_component_id, component_bytesize;
    tnecs_carr   *old_array,                 *new_array;
    tnecs_byte              *old_component_ptr,         *new_component_ptr;
    tnecs_byte              *old_component_bytesptr,    *new_component_bytesptr;

    for (size_t old_corder = 0; old_corder < num_comp_old; old_corder++) {
        old_component_id = world->bytype.components_id[old_tID][old_corder];
        for (size_t new_corder = 0; new_corder < num_comp_new; new_corder++) {
            new_component_id = world->bytype.components_id[new_tID][new_corder];
            if (old_component_id != new_component_id)
                continue;

            new_array = &world->bytype.components[new_tID][new_corder];
            old_array = &world->bytype.components[old_tID][old_corder];
            assert(old_array->type == new_array->type);
            assert(old_array != new_array);

            component_bytesize = world->components.bytesizes[old_component_id];
            assert(component_bytesize > 0);

            old_component_bytesptr = (tnecs_byte *)(old_array->components);
            assert(old_component_bytesptr != NULL);

            old_component_ptr = (old_component_bytesptr + (component_bytesize * old_entity_order));
            assert(old_component_ptr != NULL);

            new_component_bytesptr = (tnecs_byte *)(new_array->components);
            assert(new_component_bytesptr != NULL);

            new_component_ptr = (new_component_bytesptr + (component_bytesize * new_entity_order));
            assert(new_component_ptr != NULL);
            assert(new_component_ptr != old_component_ptr);

            #ifndef NDEBUG 
            void *out = memcpy(new_component_ptr, old_component_ptr, component_bytesize);
            assert(out == new_component_ptr);
            #else
            memcpy(new_component_ptr, old_component_ptr, component_bytesize);
            #endif /* NDEBUG */ 
            break;
        }
    }
    return (1);
}

b32 tnecs_component_carr_del(tnecs_world *world, tnecs_entity entity,
                        tnecs_component old_archetype) {
    /* Delete ALL components from componentsbytype at old entity order */
    size_t old_tID      = tnecs_archetypeid(world, old_archetype);
    size_t order_old    = world->entities.orders[entity];
    size_t old_comp_num = world->bytype.num_components[old_tID];
    for (size_t corder = 0; corder < old_comp_num; corder++) {
        size_t current_component_id = world->bytype.components_id[old_tID][corder];
        tnecs_carr   *old_array  = &world->bytype.components[old_tID][corder];
        tnecs_byte              *comp_ptr   = old_array->components;
        assert(comp_ptr != NULL);

        /* Scramble components too */
        size_t comp_by       = world->components.bytesizes[current_component_id];
        size_t new_comp_num  = world->bytype.num_entities[old_tID];
        tnecs_byte *scramble = tnecs_arrdel(comp_ptr, order_old, new_comp_num, comp_by);
        TNECS_CHECK_ALLOC(scramble);

        old_array->num--;
    }
    return (1);
}

b32 tnecs_component_carr_migrate(tnecs_world *world, tnecs_entity entity,
                            tnecs_component old_archetype, tnecs_component new_archetype) {
    if (old_archetype != world->entities.archetypes[entity]) {
        return (0);
    }
    TNECS_CHECK_CALL(tnecs_component_add(world,  new_archetype));
    if (old_archetype > TNECS_NULL) {
        TNECS_CHECK_CALL(tnecs_component_copy(world, entity, old_archetype, new_archetype));
        TNECS_CHECK_CALL(tnecs_component_del( world, entity, old_archetype));
    }
    return (1);
}

b32 tnecs_carr_new(tnecs_world *world, size_t num_components,
                              tnecs_component archetype) {
    tnecs_carr *comp_arr = calloc(num_components, sizeof(tnecs_chunk));
    TNECS_CHECK_ALLOC(comp_arr);

    tnecs_component archetype_reduced = archetype, archetype_added = 0, type_toadd;
    tnecs_component tID = tnecs_archetypeid(world, archetype);
    size_t id_toadd, num_flags = 0;

    while (archetype_reduced) {
        archetype_reduced &= (archetype_reduced - 1);
        type_toadd = (archetype_reduced + archetype_added) ^ archetype;
        assert(type_toadd > 0);
        id_toadd = TNECS_COMPONENT_TYPE2ID(type_toadd);
        assert(id_toadd > 0);
        assert(id_toadd < world->components.num);
        tnecs_carr_init(world, &comp_arr[num_flags], id_toadd);
        num_flags++;
        archetype_added += type_toadd;
    }
    world->bytype.components[tID] = comp_arr;
    assert(id_toadd < world->components.num);
    return ((archetype_added == archetype) && (num_flags == num_components));
}

b32 tnecs_carr_init(tnecs_world *world, tnecs_carr *comp_arr, size_t cID) {
    assert(cID > 0);
    assert(cID < world->components.num);
    tnecs_component in_type = TNECS_COMPONENT_ID2TYPE(cID);
    assert(in_type <= (1 << world->components.num));
    size_t bytesize = world->components.bytesizes[cID];
    assert(bytesize > 0);

    comp_arr->type          = in_type;
    comp_arr->num           = 0;
    comp_arr->len           = TNECS_INIT_COMPONENT_LEN;
    comp_arr->components    = calloc(TNECS_INIT_COMPONENT_LEN, bytesize);
    TNECS_CHECK_ALLOC(comp_arr->components);
    return (1);
}

b32 tnecs_system_order_switch(tnecs_world *world, tnecs_phase phase,
                              size_t order1, size_t order2) {
    if (!world->byphase.id[phase]) {
        return (0);
    }
    if (!world->byphase.systems[phase][order1]) {
        return (0);
    }
    if (!world->byphase.systems[phase][order1]) {
        return (0);
    }

    assert(world->byphase.num > phase);
    assert(world->byphase.num_systems[phase] > order1);
    assert(world->byphase.num_systems[phase] > order2);

    tnecs_system_ptr systems_temp           = world->byphase.systems[phase][order1];
    world->byphase.systems[phase][order1]   = world->byphase.systems[phase][order2];
    world->byphase.systems[phase][order2]   = systems_temp;
    b32 out1 = (world->byphase.systems[phase][order1] != NULL);
    b32 out2 = (world->byphase.systems[phase][order2] != NULL);
    return (out1 && out2);
}

/************************ UTILITY FUNCTIONS/MACROS ***************************/

size_t tnecs_component_order_bytype(tnecs_world *world, size_t cID, tnecs_component flag) {
    tnecs_component tID = tnecs_archetypeid(world, flag);
    return (tnecs_component_order_bytypeid(world, cID, tID));
}

size_t tnecs_component_order_bytypeid(tnecs_world *world, size_t cID, size_t tID) {
    size_t order = TNECS_COMPONENT_CAP;
    for (size_t i = 0; i < world->bytype.num_components[tID]; i++) {
        if (world->bytype.components_id[tID][i] == cID) {
            order = i;
            break;
        }
    }
    return (order);
}

tnecs_component tnecs_component_ids2archetype(size_t argnum, ...) {
    tnecs_component out = 0;
    va_list ap;
    va_start(ap, argnum);
    for (size_t i = 0; i < argnum; i++)
        out += TNECS_COMPONENT_ID2TYPE(va_arg(ap, size_t));
    va_end(ap);
    return (out);
}

size_t tnecs_archetypeid(tnecs_world *world, tnecs_component archetype) {
    size_t id = 0;
    for (size_t i = 0; i < world->bytype.num; i++) {
        if (archetype == world->bytype.id[i]) {
            id = i;
            break;
        }
    }
    return (id);
}

/******************* "DYNAMIC" ARRAYS ********************/
void *tnecs_realloc(void *ptr, size_t old_len, size_t new_len, size_t elem_bytesize) {
    if (!ptr)
        return (0);
    void *realloced = calloc(new_len, elem_bytesize);
    TNECS_CHECK_ALLOC(realloced);
    memcpy(realloced, ptr, (new_len > old_len ? old_len : new_len) * elem_bytesize);
    free(ptr);
    return (realloced);
}

void *tnecs_arrdel(void *arr, size_t elem, size_t len, size_t bytesize) {
    /* Scrambles by copying top element at [len - 1] to [elem] */
    tnecs_byte *bytes = arr;
    if (elem != (len - 1))
        memmove(bytes + (elem * bytesize), bytes + ((len - 1) * bytesize), bytesize);

    memset(bytes + ((len - 1) * bytesize), TNECS_NULL, bytesize);
    return (arr);
}

b32 tnecs_grow_torun(tnecs_world *world) {
    /* Realloc systems_torun if too many */
    size_t old_len              = world->systems_torun.len;
    size_t new_len              = old_len * TNECS_ARRAY_GROWTH_FACTOR;
    world->systems_torun.len    = new_len;
    size_t bytesize             = sizeof(tnecs_system_ptr);

    world->systems_torun.arr    = tnecs_realloc(world->systems_torun.arr, old_len, new_len, bytesize);
    TNECS_CHECK_ALLOC(world->systems_torun.arr);
    return (1);
}

b32 tnecs_grow_entities_open(tnecs_world *world) {
    /* Realloc entities_open if too many */
    if ((world->entities_open.num + 1) >= world->entities_open.len) {
        size_t old_len              = world->entities_open.len;
        size_t new_len              = old_len * TNECS_ARRAY_GROWTH_FACTOR;
        size_t bytesize             = sizeof(tnecs_entity);
        world->entities_open.len    = new_len;

        world->entities_open.arr = tnecs_realloc(world->entities_open.arr, old_len, new_len, bytesize);
        TNECS_CHECK_ALLOC(world->entities_open.arr);
    }
    return (1);
}

b32 tnecs_grow_component_array(tnecs_world *world, tnecs_carr *comp_arr,
                               size_t tID, size_t corder) {
    size_t old_len      = comp_arr->len;
    size_t new_len      = old_len * TNECS_ARRAY_GROWTH_FACTOR;
    comp_arr->len       = new_len;

    size_t cID = world->bytype.components_id[tID][corder];

    size_t bytesize         = world->components.bytesizes[cID];
    comp_arr->components    = tnecs_realloc(comp_arr->components, old_len, new_len, bytesize);
    TNECS_CHECK_ALLOC(comp_arr->components);
    return (1);
}


b32 tnecs_grow_entity(tnecs_world *world) {
    size_t olen = world->entities.len;
    size_t nlen = world->entities.len * TNECS_ARRAY_GROWTH_FACTOR;
    world->entities.len = nlen;
    if (nlen >= TNECS_ENTITIES_CAP) {
        printf("tnecs: entities cap reached\n");
        return (TNECS_NULL);
    }

    world->entities.id          = tnecs_realloc(world->entities.id,         olen, nlen,
                                                sizeof(*world->entities.id));
    TNECS_CHECK_ALLOC(world->entities.id);
    world->entities.orders      = tnecs_realloc(world->entities.orders,    olen, nlen,
                                                sizeof(*world->entities.orders));
    TNECS_CHECK_ALLOC(world->entities.orders);
    world->entities.archetypes  = tnecs_realloc(world->entities.archetypes, olen, nlen,
                                                sizeof(*world->entities.archetypes));
    TNECS_CHECK_ALLOC(world->entities.archetypes);

    return (1);
}

b32 tnecs_grow_system(tnecs_world *world) {
    size_t olen = world->systems.len;
    size_t nlen = olen * TNECS_ARRAY_GROWTH_FACTOR;
    assert(olen > 0);
    world->systems.len          = nlen;

    world->systems.phases       = tnecs_realloc(world->systems.phases,    olen, nlen,
                                                sizeof(*world->systems.phases));
    TNECS_CHECK_ALLOC(world->systems.phases);
    world->systems.orders       = tnecs_realloc(world->systems.orders,    olen, nlen,
                                                sizeof(*world->systems.orders));
    TNECS_CHECK_ALLOC(world->systems.orders);
    world->systems.exclusive    = tnecs_realloc(world->systems.exclusive, olen, nlen,
                                                sizeof(*world->systems.exclusive));
    TNECS_CHECK_ALLOC(world->systems.exclusive);
    world->systems.archetypes   = tnecs_realloc(world->systems.archetypes, olen, nlen,
                                                sizeof(*world->systems.archetypes));
    TNECS_CHECK_ALLOC(world->systems.archetypes);

    return (1);
}

b32 tnecs_grow_archetype(tnecs_world *world) {
    size_t olen = world->bytype.len;
    size_t nlen = olen * TNECS_ARRAY_GROWTH_FACTOR;
    world->bytype.len = nlen;

    world->bytype.id                = tnecs_realloc(world->bytype.id,                   olen, nlen,
                                                    sizeof(*world->bytype.id));
    world->bytype.entities          = tnecs_realloc(world->bytype.entities,             olen, nlen,
                                                    sizeof(*world->bytype.entities));
    world->bytype.num_entities      = tnecs_realloc(world->bytype.num_entities,         olen, nlen,
                                                    sizeof(*world->bytype.num_entities));
    world->bytype.len_entities      = tnecs_realloc(world->bytype.len_entities,         olen, nlen,
                                                    sizeof(*world->bytype.len_entities));
    world->bytype.archetype_id      = tnecs_realloc(world->bytype.archetype_id,         olen, nlen,
                                                    sizeof(*world->bytype.archetype_id));
    world->bytype.components_id     = tnecs_realloc(world->bytype.components_id,        olen, nlen,
                                                    sizeof(*world->bytype.components_id));
    world->bytype.num_components    = tnecs_realloc(world->bytype.num_components,       olen, nlen,
                                                    sizeof(*world->bytype.num_components));
    world->bytype.components_order  = tnecs_realloc(world->bytype.components_order,     olen, nlen,
                                                    sizeof(*world->bytype.components_order));
    world->bytype.num_archetype_ids = tnecs_realloc(world->bytype.num_archetype_ids,    olen, nlen,
                                                    sizeof(*world->bytype.num_archetype_ids));

    TNECS_CHECK_ALLOC(world->bytype.id);
    TNECS_CHECK_ALLOC(world->bytype.entities);
    TNECS_CHECK_ALLOC(world->bytype.num_entities);
    TNECS_CHECK_ALLOC(world->bytype.archetype_id);
    TNECS_CHECK_ALLOC(world->bytype.len_entities);
    TNECS_CHECK_ALLOC(world->bytype.components_id);
    TNECS_CHECK_ALLOC(world->bytype.num_components);
    TNECS_CHECK_ALLOC(world->bytype.components_order);
    TNECS_CHECK_ALLOC(world->bytype.num_archetype_ids);


#ifdef TNECS_CHUNK
    world->bytype.chunks            = tnecs_realloc(world->bytype.chunks,               olen, nlen,
                                                    sizeof(*world->bytype.chunks));
    TNECS_CHECK_ALLOC(world->bytype.chunks);
    world->bytype.len_chunks        = tnecs_realloc(world->bytype.len_chunks,           olen, nlen,
                                                    sizeof(*world->bytype.len_chunks));
    TNECS_CHECK_ALLOC(world->bytype.len_chunks);
#else
    world->bytype.components        = tnecs_realloc(world->bytype.components,           olen, nlen,
                                                    sizeof(*world->bytype.components));
    TNECS_CHECK_ALLOC(world->bytype.components);
#endif /* TNECS_CHUNK */

    for (size_t i = olen; i < world->bytype.len; i++) {
        world->bytype.entities[i]       = calloc(TNECS_INIT_ENTITY_LEN, sizeof(**world->bytype.entities));
        TNECS_CHECK_ALLOC(world->bytype.entities[i]);
        world->bytype.archetype_id[i]   = calloc(TNECS_COMPONENT_CAP, sizeof(**world->bytype.archetype_id));
        TNECS_CHECK_ALLOC(world->bytype.archetype_id[i]);

        world->bytype.len_entities[i] = TNECS_INIT_ENTITY_LEN;
        world->bytype.num_entities[i] = 0;
    }
    return (1);
}

b32 tnecs_grow_phase(tnecs_world *world) {
    size_t olen = world->byphase.len;
    size_t nlen = olen * TNECS_ARRAY_GROWTH_FACTOR;
    world->byphase.len = nlen;
    if (nlen >= TNECS_PHASES_CAP) {
        printf("tnecs: phases cap reached\n");
        return (TNECS_NULL);
    }

    world->byphase.id           = tnecs_realloc(world->byphase.id,          olen, nlen,
                                                sizeof(*world->byphase.id));
    TNECS_CHECK_ALLOC(world->byphase.id);
    world->byphase.systems      = tnecs_realloc(world->byphase.systems,     olen, nlen,
                                                sizeof(*world->byphase.systems));
    TNECS_CHECK_ALLOC(world->byphase.systems);
    world->byphase.systems_id   = tnecs_realloc(world->byphase.systems_id,  olen, nlen,
                                                sizeof(*world->byphase.systems_id));
    TNECS_CHECK_ALLOC(world->byphase.systems_id);
    world->byphase.len_systems  = tnecs_realloc(world->byphase.len_systems, olen, nlen,
                                                sizeof(*world->byphase.len_systems));
    TNECS_CHECK_ALLOC(world->byphase.len_systems);
    world->byphase.num_systems  = tnecs_realloc(world->byphase.num_systems, olen, nlen,
                                                sizeof(*world->byphase.num_systems));
    TNECS_CHECK_ALLOC(world->byphase.num_systems);

    for (size_t i = olen; i < world->byphase.len; i++) {
        size_t bytesize1 = sizeof(**world->byphase.systems);
        size_t bytesize2 = sizeof(**world->byphase.systems_id);

        world->byphase.systems[i]       = calloc(TNECS_INIT_PHASE_LEN, bytesize1);
        TNECS_CHECK_ALLOC(world->byphase.systems[i]);
        world->byphase.systems_id[i]    = calloc(TNECS_INIT_PHASE_LEN, bytesize2);
        TNECS_CHECK_ALLOC(world->byphase.systems_id[i]);

        world->byphase.len_systems[i] = TNECS_INIT_PHASE_LEN;
        world->byphase.num_systems[i] = 0;
    }
    return (1);
}

b32 tnecs_grow_system_byphase(tnecs_world *world, tnecs_phase phase) {
    size_t olen                         = world->byphase.len_systems[phase];
    size_t nlen                         = olen * TNECS_ARRAY_GROWTH_FACTOR;
    world->byphase.len_systems[phase]   = nlen;
    size_t bs                           = sizeof(**world->byphase.systems);
    size_t bsid                         = sizeof(**world->byphase.systems_id);

    tnecs_system_ptr *systems       = world->byphase.systems[phase];
    size_t *system_id               = world->byphase.systems_id[phase];
    world->byphase.systems[phase]   = tnecs_realloc(systems, olen, nlen, bs);
    TNECS_CHECK_ALLOC(world->byphase.systems[phase]);
    world->byphase.systems_id[phase] = tnecs_realloc(system_id, olen, nlen, bsid);
    TNECS_CHECK_ALLOC(world->byphase.systems_id[phase]);
    return (1);
}

b32 tnecs_grow_bytype(tnecs_world *world, size_t tID) {
    size_t olen = world->bytype.len_entities[tID];
    size_t nlen = olen * TNECS_ARRAY_GROWTH_FACTOR;

    assert(olen > 0);
    world->bytype.len_entities[tID] = nlen;

    size_t bytesize             = sizeof(*world->bytype.entities[tID]);
    tnecs_entity *ptr           = world->bytype.entities[tID];
    world->bytype.entities[tID] = tnecs_realloc(ptr, olen, nlen, bytesize);
    TNECS_CHECK_ALLOC(world->bytype.entities[tID]);

    return (1);
}

b32 tnecs_grow_chunks(tnecs_world *world, const size_t tID, const size_t corder) {
    printf("tnecs_grow_chunks \n");
    assert(world != NULL);
    size_t old_len                  = world->bytype.len_chunks[tID];
    size_t new_len                  = old_len * TNECS_ARRAY_GROWTH_FACTOR;
    world->bytype.len_chunks[tID]   = new_len;
    size_t bytesize                 = sizeof(tnecs_chunk);
    
    world->bytype.chunks[tID]       = tnecs_realloc(world->bytype.chunks[tID], old_len, new_len, bytesize);
    TNECS_CHECK_ALLOC(world->bytype.chunks[tID]);
    for (size_t corder = old_len; corder < new_len; corder++) {
        // if (tID == 3)
            // printf("%zu \n", corder);
        assert(world->bytype.chunks[tID][corder].len_entities   == 0);
        assert(world->bytype.chunks[tID][corder].num_components == 0);
        TNECS_CHECK_CALL(tnecs_chunk_init(&world->bytype.chunks[tID][corder], world, world->bytype.id[tID]));
        assert(world->bytype.chunks[tID][corder].len_entities == world->bytype.chunks[tID]->len_entities);
        assert(world->bytype.chunks[tID][corder].len_entities > 0);
        assert(world->bytype.chunks[tID][corder].num_components == world->bytype.chunks[tID]->num_components);
        assert(world->bytype.chunks[tID][corder].num_components > 0);
    }
    return(1);
}

/*************** SET BIT COUNTING *******************/
size_t setBits_KnR_u64(u64 in_flags) {
    // Credits to Kernighan and Ritchie in 'C Programming Language'
    size_t count = 0;
    while (in_flags) {
        in_flags &= (in_flags - 1);
        count++;
    }
    return (count);
}

/********************** CHUNKS *********************/
b32 tnecs_chunk_new(tnecs_world *world, tnecs_component archetype) {
    size_t start_len = 1;
    tnecs_chunk *new_chunk = calloc(start_len, sizeof(*new_chunk));
    TNECS_CHECK_ALLOC(new_chunk);
    TNECS_CHECK_CALL(tnecs_chunk_init(new_chunk, world, archetype));

    tnecs_component tID             = tnecs_archetypeid(world, archetype);
    assert(tID > TNECS_NULL);
    assert(world->bytype.chunks);
    assert(world->bytype.len_chunks);
    assert(tID < world->bytype.len);
    world->bytype.chunks[tID]       = new_chunk;
    world->bytype.len_chunks[tID]   = start_len;
    return(1);
}

b32 tnecs_chunk_init(tnecs_chunk *chunk, tnecs_world *world, const tnecs_component archetype) {
    // Chunk init
    memset(chunk, 0, TNECS_CHUNK_BYTESIZE);
    size_t *mem_header  = tnecs_chunk_mem(chunk);
    size_t tID = tnecs_archetypeid(world, archetype);

    // Adding all component bytesizes in archetype to chunk
    tnecs_component component_id    = 0;
    size_t cumul_bytesize           = 0;

    // Compute component order the same ways _tnecs_register_archetype
    //  component order is -> tnecs_component_order()
    tnecs_component component_type_toadd = 0, archetype_reduced = archetype, archetype_added = 0;
    while (archetype_reduced) {
        archetype_reduced &= (archetype_reduced - 1);

        component_type_toadd    = (archetype_reduced + archetype_added) ^ archetype;
        archetype_added        += component_type_toadd;
        component_id            = TNECS_COMPONENT_TYPE2ID(component_type_toadd);
        
        // Adding component bytesize to chunk header
        cumul_bytesize += world->components.bytesizes[component_id];
        mem_header[chunk->num_components++] = cumul_bytesize; 
    }

    if (tID == TNECS_NULL) {
        TNECS_CHECK_CALL(_tnecs_register_archetype(world, chunk->num_components, archetype));
        tID = tnecs_archetypeid(world, archetype);
    }

    assert(tID > TNECS_NULL);
    assert(cumul_bytesize > 0);
    assert(cumul_bytesize < TNECS_CHUNK_COMPONENTS_BYTESIZE);
    assert(chunk->num_components == world->bytype.num_components[tID]);

    size_t bytesize_header  = chunk->num_components * sizeof(size_t);
    chunk->len_entities     = (TNECS_CHUNK_COMPONENTS_BYTESIZE - bytesize_header) / cumul_bytesize;
    if (chunk->len_entities < 1) {
        printf("Could not fit any entity in chunk. Components too big.");
        return(0);
    }

    assert((chunk->len_entities * cumul_bytesize) <= (TNECS_CHUNK_COMPONENTS_BYTESIZE - bytesize_header));
    return(1);
}

size_t tnecs_chunk_len(tnecs_chunk *chunk, tnecs_world *world, const size_t tID) {
    assert(chunk != NULL);
    assert(world != NULL);
    return((world->bytype.num_entities[tID] / chunk->len_entities) + 1);
}

// Order of entity in entities_bytype -> index of chunk the components are stored in
size_t tnecs_chunk_order(tnecs_chunk *chunk, const size_t entity_order) {
    assert(chunk != NULL);
    return(entity_order / chunk->len_entities);
}
#ifdef TNECS_CHUNK
size_t tnecs_chunk_modulo(tnecs_world *world, const size_t tID) {
    assert(world != NULL);
    return(world->bytype.chunks[tID]->len_entities);
}
#endif /* TNECS_CHUNK */

tnecs_chunk *tnecs_chunk_arr(tnecs_world *world, const size_t tID) {
    assert(world != NULL);
    return(world->bytype.chunks[tID]);
}

// Order of entity in entities_bytype -> order of components in current ArchetypeChunk
size_t tnecs_chunk_component_order(tnecs_chunk *chunk, const size_t entity_order) {
    assert(chunk != NULL);
    assert(chunk->len_entities > 0);
    return(entity_order % chunk->len_entities);
}

// Get component from entity_order, corder
void *tnecs_chunk_component(tnecs_chunk *chunks, const size_t entity_order, const size_t component_order) {
    // Note: chunks is an array, all chunks have the same entities_len 
    // -> can all use the chunk_order functions
    size_t chunk_order              = tnecs_chunk_order(chunks, entity_order);
    size_t chunk_component_order    = tnecs_chunk_component_order(chunks, entity_order);
    tnecs_byte  *byte_arr           = tnecs_chunk_component_array(&chunks[chunk_order], component_order);
    
    size_t      *cumul_bytesize     = tnecs_chunk_mem(chunks);

    size_t component_bytesize = component_order == 0 ? cumul_bytesize[0] : cumul_bytesize[component_order] - cumul_bytesize[component_order - 1]; 

    return(byte_arr + (component_bytesize * chunk_component_order));
}


size_t *tnecs_chunk_mem(tnecs_chunk *chunk) {
    return((size_t*)chunk->mem);
}

size_t  tnecs_chunk_cumul_bytesize(tnecs_chunk *chunk) {
    size_t *header = tnecs_chunk_mem(chunk);
    if (chunk->num_components <= 0) {
        return(0);
    }
    return(header[chunk->num_components - 1]);
}

tnecs_chunk *tnecs_chunk_top(tnecs_world *world, size_t entity_order, size_t tID) {
    size_t chunk_order = tnecs_chunk_order(world->bytype.chunks[tID], entity_order);
    if (chunk_order >= world->bytype.len_chunks[tID]) {
        TNECS_CHECK_CALL(tnecs_grow_chunks(world, tID, chunk_order));
    }

    return(&world->bytype.chunks[tID][chunk_order]);
}

void *tnecs_carr_component_array(tnecs_world *world, const size_t cID, const size_t tID) {
    if ((cID == TNECS_NULL) || (tID == TNECS_NULL)) 
        return(NULL);

    if (cID >= world->components.num) 
        return(NULL);

    tnecs_carr *carr    = world->bytype.components[tID];
    size_t      corder  = world->bytype.components_order[tID][cID];

    return(carr[corder].components);
}

void *tnecs_world_component_array(tnecs_world *world, const size_t cID, const size_t tID, const size_t chunkOrder) {
    // Note: Array is valid from entity_order =
    // [entities_len * chunk_order, (entities_len + 1) * chunk_order,]
    // Array index is tnecs_chunk_component_order(entity_order)

    if ((cID == TNECS_NULL) || (tID == TNECS_NULL)) 
        return(NULL);

    if (cID >= world->components.num) 
        return(NULL);
    
    tnecs_chunk *chunks     = world->bytype.chunks[tID];
    
    if (chunkOrder > tnecs_chunk_len(chunks, world, tID)) 
        return(NULL);

    tnecs_chunk *chunk      = &chunks[chunkOrder];
    size_t       compOrder  = world->bytype.components_order[tID][cID];
    return(tnecs_chunk_component_array(chunk, compOrder));

}

void *tnecs_chunk_component_array(tnecs_chunk *chunk, const size_t compOrder) {
    if (chunk == NULL) {
        printf("NULL CHUNK \n");
        return(NULL);
    } 

    // There is not component array at corder
    if (compOrder >= chunk->num_components) {
        printf("COMP ORDER \n");
        return(NULL);
    }

    size_t *header              = tnecs_chunk_mem(chunk);
    assert(header != NULL);
    size_t cumul_bytesize       = (compOrder == 0) ? 0 : header[compOrder - 1];
    size_t header_offset        = chunk->num_components * sizeof(size_t);
    size_t components_offset    = cumul_bytesize * chunk->len_entities;
    assert((header_offset + components_offset) < TNECS_CHUNK_COMPONENTS_BYTESIZE);

    tnecs_byte *bytemem = chunk->mem;
    return(bytemem + header_offset + components_offset);
}

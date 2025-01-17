
#include "tnecs.h"

/************************* PRIVATE DECLARATIONS ******************************/
/* --- WORLD FUNCTIONS --- */
static b32 _tnecs_world_breath_systems(     tnecs_world *w);
static b32 _tnecs_world_breath_entities(    tnecs_world *w);
static b32 _tnecs_world_breath_components(  tnecs_world *w);
static b32 _tnecs_world_breath_archetypes(  tnecs_world *w);

/* --- REGISTRATION  --- */
static size_t _tnecs_register_archetype(tnecs_world *w, size_t num_components,
                                       tnecs_component archetype);

/**************************** WORLD FUNCTIONS ********************************/
b32 tnecs_world_genesis(tnecs_world **world) {
    /* Allocate world itself */
    if (*world != NULL) {
        TNECS_CHECK_CALL(tnecs_world_destroy(world));   
    }
    *world = calloc(1, sizeof(tnecs_world));
    TNECS_CHECK_ALLOC(*world);

    /* Allocate world members */
    TNECS_CHECK_CALL(_tnecs_world_breath_entities(  *world));
    TNECS_CHECK_CALL(_tnecs_world_breath_archetypes(*world));
    TNECS_CHECK_CALL(_tnecs_world_breath_systems(   *world));
    TNECS_CHECK_CALL(_tnecs_world_breath_components(*world));

    return(1);
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
        if ((*world)->bytype.components != NULL) {
            for (size_t j = 0; j < (*world)->bytype.num_components[i]; j++) {
                free((*world)->bytype.components[i][j].components);
            }
            free((*world)->bytype.components[i]);
        }
    }
    for (size_t i = 0; i < (*world)->components.num; i++) {
        if ((*world)->components.names[i] != NULL) {
            free((*world)->components.names[i]);
            (*world)->components.names[i] = NULL;
        }
    }
    for (size_t i = 0; i < (*world)->systems.num; i++) {
        if ((*world)->systems.names != NULL)
            free((*world)->systems.names[i]);
    }
    free((*world)->bytype.components);
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
    free((*world)->systems.hashes);
    free((*world)->systems.names);
    free((*world)->bytype.id);
    free(*world);

    *world = NULL;
    return(1);
}

b32 tnecs_world_step(tnecs_world *world, tnecs_ns deltat, void *data) {
    world->systems_torun.num = 0;
    for (size_t phase = 0; phase < world->byphase.num; phase++) {
        if (!tnecs_world_step_phase(world, phase, deltat, data)) {
            printf("tnecs: Could not run phase %zu \n", phase);
        }
    }
    return(1);
}

b32 tnecs_world_step_phase(tnecs_world *world,  tnecs_phase  phase,
                           tnecs_ns     deltat, void        *data) {
    if (phase != world->byphase.id[phase]) {
        printf("tnecs: Invalid phase '%d' \n", phase);
        return(0);
    }

    for (size_t sorder = 0; sorder < world->byphase.num_systems[phase]; sorder++) {
        size_t system_id = world->byphase.systems_id[phase][sorder];
        TNECS_CHECK_CALL(tnecs_system_run(world, system_id, deltat, data));
    }
    return(1);
}

b32 _tnecs_world_breath_entities(tnecs_world *world) {
    /* Variables */
    world->entities.num         = TNECS_NULLSHIFT;
    world->entities.len         = TNECS_INIT_ENTITY_LEN;
    world->entities_open.len    = TNECS_INIT_ENTITY_LEN;
    world->entities_open.num    = 0;

    /* Allocs */
    world->entities.id          = calloc(TNECS_INIT_ENTITY_LEN, sizeof(*world->entities.id));
    world->entities_open.arr    = calloc(TNECS_INIT_ENTITY_LEN, sizeof(tnecs_entity));
    world->entities.orders      = calloc(TNECS_INIT_ENTITY_LEN, sizeof(*world->entities.orders));
    world->bytype.entities      = calloc(TNECS_INIT_SYSTEM_LEN, sizeof(*world->bytype.entities));
    world->entities.archetypes  = calloc(TNECS_INIT_ENTITY_LEN, sizeof(*world->entities.archetypes));
    world->bytype.len_entities  = calloc(TNECS_INIT_SYSTEM_LEN, sizeof(*world->bytype.len_entities));
    world->bytype.num_entities  = calloc(TNECS_INIT_SYSTEM_LEN, sizeof(*world->bytype.num_entities));
    TNECS_CHECK_ALLOC(world->entities.id);
    TNECS_CHECK_ALLOC(world->entities_open.arr);
    TNECS_CHECK_ALLOC(world->entities.orders);
    TNECS_CHECK_ALLOC(world->bytype.entities);
    TNECS_CHECK_ALLOC(world->entities.archetypes);
    TNECS_CHECK_ALLOC(world->bytype.len_entities);
    TNECS_CHECK_ALLOC(world->bytype.num_entities);

    /* Alloc & check for ->bytype.entities elements */
    for (size_t i = 0; i < TNECS_INIT_SYSTEM_LEN; i++) {
        world->bytype.entities[i] =     calloc(TNECS_INIT_ENTITY_LEN, sizeof(**world->bytype.entities));
        TNECS_CHECK_ALLOC(world->bytype.entities[i]);

        world->bytype.num_entities[i] = 0;
        world->bytype.len_entities[i] = TNECS_INIT_ENTITY_LEN;
    }
    return(1);
}

b32 _tnecs_world_breath_components(tnecs_world *world) {
    /* NULL component always exists! */
    world->components.num                   = TNECS_NULLSHIFT;
    world->components.hashes[TNECS_NULL]    = TNECS_NULL;

    /* Allocs */
    size_t syslen = TNECS_INIT_SYSTEM_LEN, namelen = 5;
    world->bytype.components            = calloc(syslen, sizeof(*world->bytype.components));
    world->bytype.num_components        = calloc(syslen, sizeof(*world->bytype.num_components));
    world->bytype.components_id         = calloc(syslen, sizeof(*world->bytype.components_id));
    world->bytype.components_order      = calloc(syslen, sizeof(*world->bytype.components_order));
    world->components.names[TNECS_NULL] = malloc(namelen);
    TNECS_CHECK_ALLOC(world->bytype.components);
    TNECS_CHECK_ALLOC(world->bytype.num_components);
    TNECS_CHECK_ALLOC(world->bytype.components_id);
    TNECS_CHECK_ALLOC(world->bytype.components_order);
    TNECS_CHECK_ALLOC(world->components.names[TNECS_NULL]);

    /* Set name of first component */
    strncpy(world->components.names[TNECS_NULL], "NULL\0", namelen);
    return(1);
}

b32 _tnecs_world_breath_systems(tnecs_world *world) {
    /* Variables */
    world->systems.len          = TNECS_INIT_SYSTEM_LEN;
    world->systems_torun.len    = TNECS_INIT_SYSTEM_LEN;
    world->systems.num          = TNECS_NULLSHIFT;
    world->byphase.len           = TNECS_INIT_PHASE_LEN;
    world->byphase.num           = TNECS_NULLSHIFT;

    /* Allocs */
    size_t namelen = 5;
    world->byphase.id                   = calloc(world->byphase.len,        sizeof(*world->byphase.id));
    world->systems.names                = calloc(world->systems.len,        sizeof(*world->systems.names));
    world->systems_torun.arr            = calloc(world->systems_torun.len,  sizeof(tnecs_system_ptr));
    world->systems.hashes               = calloc(world->systems.len,        sizeof(*world->systems.hashes));
    world->systems.phases               = calloc(world->systems.len,        sizeof(*world->systems.phases));
    world->systems.orders               = calloc(world->systems.len,        sizeof(*world->systems.orders));
    world->byphase.systems              = calloc(world->byphase.len,        sizeof(*world->byphase.systems));
    world->systems.archetypes           = calloc(world->systems.len,        sizeof(*world->systems.archetypes));
    world->systems.exclusive            = calloc(world->systems.len,        sizeof(*world->systems.exclusive));
    world->byphase.systems_id           = calloc(world->byphase.len,        sizeof(*world->byphase.systems_id));
    world->byphase.num_systems          = calloc(world->byphase.len,        sizeof(*world->byphase.num_systems));
    world->byphase.len_systems          = calloc(world->byphase.len,        sizeof(*world->byphase.len_systems));
    world->systems.names[TNECS_NULL]    = malloc(namelen);
    TNECS_CHECK_ALLOC(world->byphase.id);
    TNECS_CHECK_ALLOC(world->systems.names);
    TNECS_CHECK_ALLOC(world->systems_torun.arr);
    TNECS_CHECK_ALLOC(world->systems.hashes);
    TNECS_CHECK_ALLOC(world->systems.phases);
    TNECS_CHECK_ALLOC(world->systems.orders);
    TNECS_CHECK_ALLOC(world->byphase.systems);
    TNECS_CHECK_ALLOC(world->systems.archetypes);
    TNECS_CHECK_ALLOC(world->systems.exclusive);
    TNECS_CHECK_ALLOC(world->byphase.systems_id);
    TNECS_CHECK_ALLOC(world->byphase.num_systems);
    TNECS_CHECK_ALLOC(world->byphase.len_systems);
    TNECS_CHECK_ALLOC(world->systems.names[TNECS_NULL]);

    /* Alloc & check for entities_byphase elements */
    for (size_t i = 0; i < world->byphase.len; i++) {
        world->byphase.systems[i]   = calloc(world->byphase.len, sizeof(**world->byphase.systems));
        TNECS_CHECK_ALLOC(world->byphase.systems[i]);
        world->byphase.systems_id[i] = calloc(world->byphase.len, sizeof(**world->byphase.systems_id));
        TNECS_CHECK_ALLOC(world->byphase.systems_id[i]);

        world->byphase.num_systems[i] = 0;
        world->byphase.len_systems[i] = world->byphase.len;
    }

    /* Set name of first system */
    strncpy(world->systems.names[TNECS_NULL], "NULL\0", namelen);
    return(1);
}

b32 _tnecs_world_breath_archetypes(tnecs_world *world) {

    /* Variables */
    world->bytype.num = TNECS_NULLSHIFT;
    world->bytype.len = TNECS_INIT_SYSTEM_LEN;

    /* Allocs */
    world->bytype.id                    = calloc(TNECS_INIT_SYSTEM_LEN, sizeof(*world->bytype.id));
    world->bytype.archetype_id          = calloc(TNECS_INIT_SYSTEM_LEN, sizeof(*world->bytype.archetype_id));
    world->bytype.num_archetype_ids     = calloc(TNECS_INIT_SYSTEM_LEN, sizeof(*world->bytype.num_archetype_ids));
    TNECS_CHECK_ALLOC(world->bytype.id);
    TNECS_CHECK_ALLOC(world->bytype.archetype_id);
    TNECS_CHECK_ALLOC(world->bytype.num_archetype_ids);

    /* Alloc & check for id_bytype elements */
    for (size_t i = 0; i < TNECS_INIT_SYSTEM_LEN; i++) {
        world->bytype.archetype_id[i] = calloc(TNECS_COMPONENT_CAP, sizeof(**world->bytype.archetype_id));
        TNECS_CHECK_ALLOC(world->bytype.archetype_id[i]);
    }
    return(1);
}

/**************************** SYSTEM FUNCTIONS ********************************/
b32 tnecs_custom_system_run(tnecs_world *world, tnecs_system_ptr custom_system,
                             tnecs_component archetype, tnecs_ns deltat, void *data) {
    /* Building the systems input */
    tnecs_system_input input = {.world = world, .deltat = deltat, .data = data};
    size_t tID = tnecs_archetypeid(world, archetype);
    if (tID == TNECS_NULL) {
        printf("tnecs: Input archetype is unknown.\n");
        return(0);
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
    return(1);
}

b32 tnecs_grow_torun(tnecs_world *world) {
    /* Realloc systems_torun if too many */
        size_t old_len              = world->systems_torun.len;
        size_t new_len              = old_len * TNECS_ARRAY_GROWTH_FACTOR;
        world->systems_torun.len    = new_len;
        size_t bytesize             = sizeof(tnecs_system_ptr);

        world->systems_torun.arr    = tnecs_realloc(world->systems_torun.arr, old_len, new_len, bytesize);
        TNECS_CHECK_ALLOC(world->systems_torun.arr);
    return(1);
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
        return(1);

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
    return(1);
}

/***************************** REGISTRATION **********************************/
size_t tnecs_register_system(tnecs_world *world, const char *name,
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
    /* Saving name and hash */
    world->systems.names[system_id] = malloc(strlen(name) + 1);
    TNECS_CHECK_ALLOC(world->systems.names[system_id]);

    tnecs_hash hash                 = tnecs_hash_djb2(name);
    strncpy(world->systems.names[system_id], name, strlen(name) + 1);

    /* Register new phase if didn't exist */
    if (!world->byphase.id[phase])
        TNECS_CHECK_CALL(tnecs_register_phase(world, phase));

    world->systems.exclusive[system_id]     = isExclusive;
    world->systems.phases[system_id]        = phase;
    world->systems.hashes[system_id]        = hash;
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
                                         const char     *name,
                                         const size_t    bytesize) {
    /* Checks */
    if (bytesize <= 0) {
        printf("tnecs: Component should have >0 bytesize.\n");
        return(TNECS_NULL);
    }
    if (world->components.num >= TNECS_COMPONENT_CAP) {
        printf("tnecs: Component capacity reached.\n");
        return(TNECS_NULL);
    }


    /* Registering */
    tnecs_component new_component_id                = world->components.num++;
    world->components.hashes[new_component_id]      = tnecs_hash_djb2(name);
    tnecs_component new_component_flag              = TNECS_COMPONENT_ID2TYPE(new_component_id);
    world->components.bytesizes[new_component_id] = bytesize;

    /* Setting component name */
    world->components.names[new_component_id] = malloc(strlen(name) + 1);
    TNECS_CHECK_ALLOC(world->components.names[new_component_id]);

    strncpy(world->components.names[new_component_id], name, strlen(name) + 1);
    TNECS_CHECK_CALL(_tnecs_register_archetype(world, 1, new_component_flag));
    return (new_component_id);
}

size_t _tnecs_register_archetype(tnecs_world *world, size_t num_components,
                                tnecs_component archetype_new) {
    // 0- Check if archetype exists, return
    size_t tID = 0;
    for (size_t i = 0 ; i < world->bytype.num; i++) {
        if (archetype_new == world->bytype.id[i]) {
            tID = i;
            break;
        }
    }
    if (tID)
        return (tID);

    // 1- Add new bytype.components at [tID]
    if ((world->bytype.num + 1) >= world->bytype.len)
        tnecs_grow_archetype(world);
    world->bytype.id[world->bytype.num++] = archetype_new;
    tID = tnecs_archetypeid(world, archetype_new);
    TNECS_DEBUG_ASSERT(tID == (world->bytype.num - 1));
    world->bytype.num_components[tID] = num_components;

    // 2- Add arrays to bytype.components[tID] for each component
    tnecs_component_array_new(world, num_components, archetype_new);

    // 3- Add all components to bytype.components_id
    tnecs_component component_id_toadd, component_type_toadd;
    tnecs_component archetype_reduced = archetype_new, archetype_added = 0;
    size_t bytesize1 =  sizeof(**world->bytype.components_id);
    size_t bytesize2 =  sizeof(**world->bytype.components_order);
    world->bytype.components_id[tID]     = calloc(num_components,      bytesize1);
    TNECS_CHECK_ALLOC(world->bytype.components_id[tID]);
    world->bytype.components_order[tID]  = calloc(TNECS_COMPONENT_CAP, bytesize2);
    TNECS_CHECK_ALLOC(world->bytype.components_order[tID]);

    size_t i = 0;
    while (archetype_reduced) {
        archetype_reduced &= (archetype_reduced - 1);

        component_type_toadd = (archetype_reduced + archetype_added) ^ archetype_new;
        archetype_added      += component_type_toadd;
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
        return(1);

    while (phase >= world->byphase.len) {
        TNECS_CHECK_CALL(tnecs_grow_phase(world));
    }
    world->byphase.id[phase]    = phase;
    world->byphase.num      = (phase >= world->byphase.num) ? (phase + 1) : world->byphase.num;
    return (phase);
}

/***************************** ENTITY MANIPULATION ***************************/
tnecs_entity tnecs_entity_create(tnecs_world *world) {
    tnecs_entity out = TNECS_NULL;
    
    /* Check if an open entity exists */
    tnecs_entity *arr = world->entities_open.arr;
    while ((out == TNECS_NULL) && (world->entities_open.num > 0) && (world->entities_open.num < TNECS_ENTITIES_CAP)) {
        out = arr[--world->entities_open.num];
        arr[world->entities_open.num] = TNECS_NULL;
    }

    /* If no open entity existed, create one */
    if (out == TNECS_NULL) {
        do {
            if (world->entities.num >= world->entities.len) {
                if (!tnecs_grow_entity(world)) {
                    printf("tnecs: Could not allocate more memory for entities.\n");
                    return(TNECS_NULL);
                }
            }
        } while (world->entities.id[out = world->entities.num++] != TNECS_NULL);
    }
    TNECS_DEBUG_ASSERT(out != TNECS_NULL);

    /* Set entity and checks  */
    world->entities.id[out] = out;
    tnecs_entitiesbytype_add(world, out, TNECS_NULL);
    TNECS_DEBUG_ASSERT(world->entities.id[out]                                          == out);
    TNECS_DEBUG_ASSERT(world->bytype.entities[TNECS_NULL][world->entities.orders[out]]  == out);
    return (out);
}

tnecs_entity tnecs_entities_create(tnecs_world *world, size_t num) {
    for (int i = 0; i < num; i++) {
        if (tnecs_entity_create(world) <= TNECS_NULL) {
            printf("tnecs: Could not create another entity.\n");
            return(TNECS_NULL);            
        }
    }
    return (num);
}

tnecs_entity tnecs_entity_create_wcomponents(tnecs_world *world, size_t argnum, ...) {
    /* Get archetype of all vararg components */
    va_list ap;
    va_start(ap, argnum);
    tnecs_component archetype = 0;
    for (size_t i = 0; i < argnum; i++) {
        tnecs_hash hash = va_arg(ap, tnecs_hash);
        archetype += tnecs_component_hash2type(world, hash);
    }
    va_end(ap);

    /* Create entity with all components */
    tnecs_entity new_entity = tnecs_entity_create(world);
    if (new_entity == TNECS_NULL) {
        printf("tnecs: could not create new entity\n");
        return(TNECS_NULL);
    }
    TNECS_CHECK_CALL(tnecs_entity_add_components(world, new_entity, argnum, archetype, 1));

    /* Check */
    size_t tID      = TNECS_ARCHETYPEID(world, archetype);
    size_t order    = world->entities.orders[new_entity];
    TNECS_DEBUG_ASSERT(world->bytype.entities[tID][order]   == new_entity);
    TNECS_DEBUG_ASSERT(world->entities.id[new_entity]       == new_entity);
    return (new_entity);
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
    return(1);        
}

b32 tnecs_entities_open_reuse(tnecs_world *world) {
    // Check for open entities. If not in entities_open, add them.

    for (tnecs_entity i = TNECS_NULLSHIFT; i < world->entities.num; i++) {
        if ((world->entities.id[i] == TNECS_NULL) && !tnecs_entity_isOpen(world, i)) {
            tnecs_grow_entities_open(world);
            tnecs_entity *arr = world->entities_open.arr; 
            arr[world->entities_open.num++] = i;
        }
    }
    return(1);
};

b32 tnecs_entities_open_flush(tnecs_world *world) {
    // Get rid of all entities in entities_open.
    world->entities_open.num = 0;
    return(1);
}

b32 tnecs_entity_isOpen(tnecs_world *world, tnecs_entity entity) {
    if (entity <= TNECS_NULL) {
        return(0);
    }

    tnecs_entity *open_arr = world->entities_open.arr; 
   
    for (tnecs_entity i = TNECS_NULLSHIFT; i < world->entities_open.num; i++) {
        if (open_arr[i] == entity) {
            return(1);
        }
    }
    return(0);
}


b32 tnecs_entity_destroy(tnecs_world *world, tnecs_entity entity) {
    if (entity <= TNECS_NULL) {
        return(1);
    }

    if (world->entities.id[entity] <= TNECS_NULL) {
        world->entities.id[entity]         = TNECS_NULL;
        world->entities.orders[entity]     = TNECS_NULL;
        world->entities.archetypes[entity] = TNECS_NULL;
        return(1);
    }

    /* Preliminaries */
    tnecs_component archetype   = world->entities.archetypes[entity];
    size_t tID                  = TNECS_ARCHETYPEID(world, archetype);
    size_t entity_order         = world->entities.orders[entity];
    TNECS_DEBUG_ASSERT(world->bytype.num_entities[tID] > TNECS_NULL);
    /* Delete components */
    tnecs_component_del(world, entity, archetype);
    TNECS_DEBUG_ASSERT(world->bytype.len_entities[tID] >= entity_order);
    TNECS_DEBUG_ASSERT(world->bytype.num_entities[tID] > TNECS_NULL);
    /* Delete entitiesbytype */
    tnecs_entitiesbytype_del(world, entity, archetype);

    /* Delete entity */
    world->entities.id[entity]         = TNECS_NULL;
    world->entities.orders[entity]     = TNECS_NULL;
    world->entities.archetypes[entity] = TNECS_NULL;

    // Note: reuse_entities used to add to entities_open, so that
    // user can call tnecs_entities_open_reuse to reuse entities manually.
    if (world->reuse_entities) {
        /* Add deleted entity to open entities */
        tnecs_grow_entities_open(world);
        tnecs_entity *arr = world->entities_open.arr; 
        arr[world->entities_open.num++] = entity;
    }
    TNECS_DEBUG_ASSERT(world->entities.id[entity]           == TNECS_NULL);
    TNECS_DEBUG_ASSERT(world->entities.archetypes[entity]   == TNECS_NULL);
    TNECS_DEBUG_ASSERT(world->entities.orders[entity]       == TNECS_NULL);
    TNECS_DEBUG_ASSERT(world->entities.orders[entity_order] != entity);
    return (1);
}

/*****************************************************************************/
/***************************** TNECS INTERNALS *******************************/
/*****************************************************************************/
tnecs_entity tnecs_entity_add_components(tnecs_world *world, tnecs_entity entity,
                                         size_t num_components_toadd, tnecs_component archetype_toadd, b32 isNew) {
    if (num_components_toadd <= 0) {
        return(TNECS_NULL);
    }
    if (archetype_toadd <= 0) {
        return(TNECS_NULL);
    }
    tnecs_component archetype_old = world->entities.archetypes[entity];
    TNECS_DEBUG_ASSERT(!(archetype_toadd & archetype_old));
    tnecs_component archetype_new = archetype_toadd + archetype_old;
    TNECS_DEBUG_ASSERT(archetype_new != archetype_old);
    if (isNew)
        TNECS_CHECK_CALL(_tnecs_register_archetype(world, setBits_KnR_uint64_t(archetype_new), archetype_new));

    size_t tID_new = tnecs_archetypeid(world, archetype_new);

    TNECS_CHECK_CALL(tnecs_component_migrate(world,      entity, archetype_old, archetype_new));
    TNECS_CHECK_CALL(tnecs_entitiesbytype_migrate(world, entity, archetype_old, archetype_new));

    size_t new_order = world->bytype.num_entities[tID_new] - 1;
    TNECS_DEBUG_ASSERT(world->entities.archetypes[entity]           == archetype_new);
    TNECS_DEBUG_ASSERT(world->bytype.entities[tID_new][new_order]   == entity);
    TNECS_DEBUG_ASSERT(world->entities.orders[entity]               == new_order);
    return (world->entities.id[entity]);
}

b32 tnecs_entity_remove_components(tnecs_world *world, tnecs_entity entity,
                                    size_t num_components, tnecs_component archetype) {
    /* Get new archetype. Since it is a archetype, just need to substract. */
    tnecs_component archetype_old = world->entities.archetypes[entity];
    tnecs_component archetype_new = archetype_old - archetype;

    if (archetype_new != TNECS_NULL) {
        /* Migrate remaining components to new archetype array. */
        TNECS_CHECK_CALL(_tnecs_register_archetype(world, setBits_KnR_uint64_t(archetype_new), archetype_new));
        TNECS_CHECK_CALL(tnecs_component_migrate(world, entity, archetype_old, archetype_new));
    } else {
        /* No remaining component, delete everything. */
        TNECS_CHECK_CALL(tnecs_component_del(world, entity, archetype_old));
    }
    /* Migrate entity to new bytype array. */
    TNECS_CHECK_CALL(tnecs_entitiesbytype_migrate(world, entity, archetype_old, archetype_new));
    TNECS_DEBUG_ASSERT(archetype_new == world->entities.archetypes[entity]);
    return(1);
}


void *tnecs_entity_get_component(tnecs_world *world, tnecs_entity eID,
                                 tnecs_component cID) {

    tnecs_component component_flag      = TNECS_COMPONENT_ID2TYPE(cID);
    tnecs_component entity_archetype    = TNECS_ENTITY_ARCHETYPE(world, eID);
    void *out = NULL;
    // If entity has component, get output it. If not output NULL.
    if ((component_flag & entity_archetype) == 0)
        return (out);

    size_t tID = tnecs_archetypeid(world, entity_archetype);
    size_t component_order = tnecs_component_order_bytype(world, cID, entity_archetype);
    TNECS_DEBUG_ASSERT(component_order <= world->bytype.num_components[tID]);
    size_t entity_order = world->entities.orders[eID];
    size_t bytesize = world->components.bytesizes[cID];
    tnecs_component_array *comp_array;
    comp_array = &world->bytype.components[tID][component_order];
    tnecs_byte *temp_component_bytesptr = (tnecs_byte *)(comp_array->components);
    out = (temp_component_bytesptr + (bytesize * entity_order));
    return (out);
}

b32 tnecs_entitiesbytype_add(tnecs_world *world, tnecs_entity entity,
                                tnecs_component archetype_new) {
    size_t tID_new = tnecs_archetypeid(world, archetype_new);
    if ((world->bytype.num_entities[tID_new] + 1) >= world->bytype.len_entities[tID_new]) {
        TNECS_CHECK_CALL(tnecs_grow_bytype(world, tID_new));
    }
    size_t new_order =                               world->bytype.num_entities[tID_new]++;
    world->entities.orders[entity] =                new_order;
    world->entities.archetypes[entity] =             archetype_new;
    world->bytype.entities[tID_new][new_order] =  entity;
    return(1);
}

b32 tnecs_entitiesbytype_del(tnecs_world *world, tnecs_entity entity,
                             tnecs_component archetype_old) {

    if (entity <= TNECS_NULL) {
        return(1);
    }

    if (world->entities.id[entity] != entity) {
        return(1);
    }

    if (entity >= world->entities.len) {
        return(1);
    }

    size_t archetype_old_id = tnecs_archetypeid(world, archetype_old);
    size_t old_num          = world->bytype.num_entities[archetype_old_id];
    if (old_num <= 0) {
        return(1);
    }

    size_t entity_order_old = world->entities.orders[entity];

    TNECS_DEBUG_ASSERT(entity_order_old < world->bytype.len_entities[archetype_old_id]);
    TNECS_DEBUG_ASSERT(world->bytype.entities[archetype_old_id][entity_order_old] == entity);

    tnecs_entity top_entity = world->bytype.entities[archetype_old_id][old_num - 1];

    /* components scrambles -> entitiesbytype too */
    tnecs_arrdel_scramble(world->bytype.entities[archetype_old_id], entity_order_old, old_num,
                          sizeof(**world->bytype.entities));

    if (top_entity != entity) {
        world->entities.orders[top_entity] = entity_order_old;
        TNECS_DEBUG_ASSERT(world->bytype.entities[archetype_old_id][entity_order_old] == top_entity);
    }

    world->entities.orders[entity]      = TNECS_NULL;
    world->entities.archetypes[entity]  = TNECS_NULL;

    --world->bytype.num_entities[archetype_old_id];
    return(1);
}

b32 tnecs_entitiesbytype_migrate(tnecs_world *world, tnecs_entity entity,
                                    tnecs_component archetype_old, tnecs_component archetype_new) {
    /* Migrate entities into correct bytype array */
    TNECS_CHECK_CALL(tnecs_entitiesbytype_del(world, entity, archetype_old));
    TNECS_DEBUG_ASSERT(world->entities.archetypes[entity]   == TNECS_NULL);
    TNECS_DEBUG_ASSERT(world->entities.orders[entity]       == TNECS_NULL);
    TNECS_CHECK_CALL(tnecs_entitiesbytype_add(world, entity, archetype_new));

    /* Checks */
    size_t tID_new      = tnecs_archetypeid(world, archetype_new);
    size_t order_new    = world->entities.orders[entity];
    TNECS_DEBUG_ASSERT(world->entities.archetypes[entity]         == archetype_new);
    TNECS_DEBUG_ASSERT(world->bytype.num_entities[tID_new] - 1    == order_new);
    TNECS_DEBUG_ASSERT(world->bytype.entities[tID_new][order_new] == entity);
    return (1);
}

b32 tnecs_component_add(tnecs_world *world, tnecs_component archetype) {
    /* Check if need to grow component array after adding new component */
    size_t tID          = tnecs_archetypeid(world, archetype);
    size_t new_comp_num = world->bytype.num_components[tID];
    size_t new_order    = world->bytype.num_entities[tID];

    for (size_t corder = 0; corder < new_comp_num; corder++) {
        // Take component array of current archetype_id
        tnecs_component_array *comp_arr = &world->bytype.components[tID][corder];
        // check if it need to grow after adding new component
        TNECS_DEBUG_ASSERT(new_order == comp_arr->num_components);

        if (++comp_arr->num_components >= comp_arr->len_components)
            tnecs_grow_component_array(world, comp_arr, tID, corder);
    }

    return(1);
}

b32 tnecs_grow_component_array(tnecs_world *world, tnecs_component_array *comp_arr, const size_t tID, const size_t corder) {
    size_t old_len      = comp_arr->len_components;
    size_t new_len      = old_len * TNECS_ARRAY_GROWTH_FACTOR;
    size_t new_comp_num = world->bytype.num_components[tID];
    comp_arr->len_components = new_len;

    size_t cID = world->bytype.components_id[tID][corder];

    size_t bytesize         = world->components.bytesizes[cID];
    comp_arr->components    = tnecs_realloc(comp_arr->components, old_len, new_len, bytesize);
    TNECS_CHECK_ALLOC(comp_arr->components);
    return(1);
}


b32 tnecs_component_copy(tnecs_world *world, const tnecs_entity entity,
                        const tnecs_component old_archetype, const tnecs_component new_archetype) {
    /* Copy components from old order unto top of new type component array */
    if (old_archetype == new_archetype) {
        return(1);
    }

    size_t old_tID          = tnecs_archetypeid(world, old_archetype);
    size_t new_tID          = tnecs_archetypeid(world, new_archetype);
    size_t old_entity_order = world->entities.orders[entity];
    size_t new_entity_order = world->bytype.num_entities[new_tID];
    size_t num_comp_new     = world->bytype.num_components[new_tID];
    size_t num_comp_old     = world->bytype.num_components[old_tID];

#ifdef TNECS_DEBUG_A
    // Sanity check: entity order is the same in new components array
    for (int i = 0; i < num_comp_new; ++i) {
        size_t num = world->bytype.components[new_tID][i].num_components;
        TNECS_DEBUG_ASSERT((num - 1) == new_entity_order);
    }
#endif /* TNECS_DEBUG_A */

    size_t old_component_id, new_component_id, component_bytesize;
    tnecs_component_array   *old_array,                 *new_array;
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
            TNECS_DEBUG_ASSERT(old_array->type == new_array->type);
            TNECS_DEBUG_ASSERT(old_array != new_array);

            component_bytesize = world->components.bytesizes[old_component_id];
            TNECS_DEBUG_ASSERT(component_bytesize > 0);

            old_component_bytesptr = (tnecs_byte *)(old_array->components);
            TNECS_DEBUG_ASSERT(old_component_bytesptr != NULL);
            
            old_component_ptr = (old_component_bytesptr + (component_bytesize * old_entity_order));
            TNECS_DEBUG_ASSERT(old_component_ptr != NULL);
            
            new_component_bytesptr = (tnecs_byte *)(new_array->components);
            TNECS_DEBUG_ASSERT(new_component_bytesptr != NULL);
            
            new_component_ptr = (new_component_bytesptr + (component_bytesize * new_entity_order));
            TNECS_DEBUG_ASSERT(new_component_ptr != NULL);
            TNECS_DEBUG_ASSERT(new_component_ptr != old_component_ptr);
            
            void *out =  memcpy(new_component_ptr, old_component_ptr, component_bytesize);
            TNECS_DEBUG_ASSERT(out == new_component_ptr);
            break;
        }
    }
    return(1);
}

b32 tnecs_component_del(tnecs_world *world, tnecs_entity entity,
                         tnecs_component old_archetype) {
    /* Delete ALL components from componentsbytype at old entity order */
    size_t old_tID      = tnecs_archetypeid(world, old_archetype);
    size_t order_old    = world->entities.orders[entity];
    size_t old_comp_num = world->bytype.num_components[old_tID];
    for (size_t corder = 0; corder < old_comp_num; corder++) {
        size_t current_component_id = world->bytype.components_id[old_tID][corder];
        tnecs_component_array   *old_array  = &world->bytype.components[old_tID][corder];
        tnecs_byte              *comp_ptr   = old_array->components;
        TNECS_DEBUG_ASSERT(comp_ptr != NULL);

        /* Scramble components too */
        size_t comp_by       = world->components.bytesizes[current_component_id];
        size_t new_comp_num  = world->bytype.num_entities[old_tID];
        tnecs_byte *scramble = tnecs_arrdel_scramble(comp_ptr, order_old, new_comp_num, comp_by);
        TNECS_CHECK_ALLOC(scramble);

        old_array->num_components--;
    }
    return(1);
}

b32 tnecs_component_migrate(tnecs_world *world, tnecs_entity entity,
                             tnecs_component old_archetype, tnecs_component new_archetype) {
    if (old_archetype != world->entities.archetypes[entity]) {
        return(0);
    }
    TNECS_CHECK_CALL(tnecs_component_add(world,  new_archetype));
    if (old_archetype > TNECS_NULL) {
        TNECS_CHECK_CALL(tnecs_component_copy(world, entity, old_archetype, new_archetype));
        TNECS_CHECK_CALL(tnecs_component_del( world, entity, old_archetype));
    }
    return(1);
}

b32 tnecs_component_array_new(tnecs_world *world, size_t num_components,
                               tnecs_component archetype) {
    tnecs_component_array *temp_comparray;
    temp_comparray = calloc(num_components, sizeof(tnecs_component_array));
    TNECS_CHECK_ALLOC(temp_comparray);

    tnecs_component archetype_reduced = archetype, archetype_added = 0, type_toadd;
    tnecs_component tID = tnecs_archetypeid(world, archetype);
    size_t id_toadd, num_flags = 0;

    while (archetype_reduced) {
        archetype_reduced &= (archetype_reduced - 1);
        type_toadd = (archetype_reduced + archetype_added) ^ archetype;
        id_toadd = TNECS_COMPONENT_TYPE2ID(type_toadd);
        TNECS_DEBUG_ASSERT(id_toadd > 0);
        TNECS_DEBUG_ASSERT(id_toadd < world->components.num);
        tnecs_component_array_init(world, &temp_comparray[num_flags], id_toadd);
        num_flags++;
        archetype_added += type_toadd;
    }
    world->bytype.components[tID] = temp_comparray;
    TNECS_DEBUG_ASSERT(id_toadd < world->components.num);
    return ((archetype_added == archetype) && (num_flags == num_components));
}

b32 tnecs_component_array_init(tnecs_world *world, tnecs_component_array *in_array,
                                size_t cID) {
    TNECS_DEBUG_ASSERT(cID > 0);
    TNECS_DEBUG_ASSERT(cID < world->components.num);
    tnecs_component in_type = TNECS_COMPONENT_ID2TYPE(cID);
    TNECS_DEBUG_ASSERT(in_type <= (1 << world->components.num));
    size_t bytesize = world->components.bytesizes[cID];
    TNECS_DEBUG_ASSERT(bytesize > 0);

    in_array->type = in_type;
    in_array->num_components    = 0;
    in_array->len_components    = TNECS_INIT_COMPONENT_LEN;
    in_array->components        = calloc(TNECS_INIT_COMPONENT_LEN, bytesize);
    TNECS_CHECK_ALLOC(in_array->components);
    return(1);
}

b32 tnecs_system_order_switch(tnecs_world *world, tnecs_phase phase,
                               size_t order1, size_t order2) {
    if (!world->byphase.id[phase]) {
        return(0);
    }
    if (!world->byphase.systems[phase][order1]) {
        return(0);
    }
    if (!world->byphase.systems[phase][order1]) {
        return(0);
    }

    TNECS_DEBUG_ASSERT(world->byphase.num > phase);
    TNECS_DEBUG_ASSERT(world->byphase.num_systems[phase] > order1);
    TNECS_DEBUG_ASSERT(world->byphase.num_systems[phase] > order2);

    tnecs_system_ptr systems_temp           = world->byphase.systems[phase][order1];
    world->byphase.systems[phase][order1]   = world->byphase.systems[phase][order2];
    world->byphase.systems[phase][order2]   = systems_temp;
    b32 out1 = (world->byphase.systems[phase][order1] != NULL);
    b32 out2 = (world->byphase.systems[phase][order2] != NULL);
    return (out1 && out2);
}

/************************ UTILITY FUNCTIONS/MACROS ***************************/
size_t tnecs_component_name2id(tnecs_world *world,
                               const char *name) {
    return (tnecs_component_hash2id(world, tnecs_hash_djb2(name)));
}

size_t tnecs_component_hash2id(tnecs_world *world, tnecs_hash hash) {
    size_t out;
    for (size_t i = 0; i < world->components.num; i++) {
        if (world->components.hashes[i] == hash) {
            out = i;
            break;
        }
    }
    return (out);
}

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

tnecs_component tnecs_component_names2archetype(tnecs_world *world, size_t argnum, ...) {
    va_list ap;
    tnecs_component archetype = 0;
    va_start(ap, argnum);
    for (size_t i = 0; i < argnum; i++) {
        archetype += world->bytype.id[tnecs_component_name2id(world, va_arg(ap, const char *))];
    }
    va_end(ap);
    return (archetype);
}

void tnecs_components_names_print(tnecs_world *world, tnecs_entity entity) {
    tnecs_component archetype = world->entities.archetypes[entity];
    size_t tID               = tnecs_archetypeid(world, archetype);
    size_t comp_num          = world->bytype.num_components[tID];
    printf("Entity %llu: ", entity);
    for (size_t corder = 0; corder < comp_num; corder++) {
        size_t component_id = world->bytype.components_id[tID][corder];
        printf("%s, ", world->components.names[component_id]);
    }
    printf("\n");
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

tnecs_component tnecs_component_hash2type(tnecs_world *world, tnecs_hash hash) {
    return (TNECS_COMPONENT_ID2TYPE(tnecs_component_hash2id(world, hash)));
}

size_t tnecs_system_name2id(tnecs_world *world, const char *name) {
    tnecs_hash hash = tnecs_hash_djb2(name);
    size_t found = 0;
    for (size_t i = 0; i < world->systems.num; i++) {
        if (world->systems.hashes[i] == hash) {
            found = i;
            break;
        }
    }
    return (found);
}

tnecs_component tnecs_system_name2archetype(tnecs_world *world,
                                           const char *name) {
    size_t id = tnecs_system_name2id(world, name);
    return (world->systems.archetypes[id]);
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

/***************************** "DYNAMIC" ARRAYS ******************************/
void *tnecs_realloc(void *ptr, size_t old_len, size_t new_len, size_t elem_bytesize) {
    if (!ptr)
        return(0);
    void *realloced = (void *)calloc(new_len, elem_bytesize);
    TNECS_CHECK_ALLOC(realloced);
    memcpy(realloced, ptr, (new_len > old_len ? old_len : new_len) * elem_bytesize);
    free(ptr);
    return (realloced);
}

void *tnecs_arrdel(void *arr, size_t elem, size_t len, size_t bytesize) {
    void *out;
    tnecs_byte *bytes = arr;
    if (elem < (len - 1)) {
        tnecs_byte *dst = bytes + (elem * bytesize);
        tnecs_byte *src = bytes + ((elem + 1) * bytesize);
        out = memmove(dst, src, bytesize * (len - elem - 1));
    } else
        out = memset(bytes + (elem * bytesize), TNECS_NULL, bytesize);
    return (out);
}

void *tnecs_arrdel_scramble(void *arr, size_t elem, size_t len, size_t bytesize) {
    tnecs_byte *bytes = arr;
    if (elem != (len - 1))
        memmove(bytes + (elem * bytesize), bytes + ((len - 1) * bytesize), bytesize);

    memset(bytes + ((len - 1) * bytesize), TNECS_NULL, bytesize);
    return (arr);
}

b32 tnecs_grow_entity(tnecs_world *world) {
    size_t olen = world->entities.len;
    size_t nlen = world->entities.len * TNECS_ARRAY_GROWTH_FACTOR;
    world->entities.len = nlen;
    if (nlen >= TNECS_ENTITIES_CAP) {
        printf("tnecs: entities cap reached\n");
        return(TNECS_NULL);
    }

    world->entities.id          = tnecs_realloc(world->entities.id,         olen, nlen, sizeof(*world->entities.id));
    TNECS_CHECK_ALLOC(world->entities.id);
    world->entities.orders      = tnecs_realloc(world->entities.orders,    olen, nlen, sizeof(*world->entities.orders));
    TNECS_CHECK_ALLOC(world->entities.orders);
    world->entities.archetypes  = tnecs_realloc(world->entities.archetypes, olen, nlen, sizeof(*world->entities.archetypes));
    TNECS_CHECK_ALLOC(world->entities.archetypes);

    return(1);
}

b32 tnecs_grow_system(tnecs_world *world) {
    size_t olen = world->systems.len;
    size_t nlen = olen * TNECS_ARRAY_GROWTH_FACTOR;
    TNECS_DEBUG_ASSERT(olen > 0);
    world->systems.len          = nlen;

    world->systems.names        = tnecs_realloc(world->systems.names,     olen, nlen,   sizeof(*world->systems.names));
    TNECS_CHECK_ALLOC(world->systems.names);
    world->systems.phases       = tnecs_realloc(world->systems.phases,    olen, nlen,   sizeof(*world->systems.phases));
    TNECS_CHECK_ALLOC(world->systems.phases);
    world->systems.orders       = tnecs_realloc(world->systems.orders,    olen, nlen,   sizeof(*world->systems.orders));
    TNECS_CHECK_ALLOC(world->systems.orders);
    world->systems.hashes       = tnecs_realloc(world->systems.hashes,    olen, nlen,   sizeof(*world->systems.hashes));
    TNECS_CHECK_ALLOC(world->systems.hashes);
    world->systems.exclusive    = tnecs_realloc(world->systems.exclusive, olen, nlen,   sizeof(*world->systems.exclusive));
    TNECS_CHECK_ALLOC(world->systems.exclusive);
    world->systems.archetypes   = tnecs_realloc(world->systems.archetypes, olen,nlen,   sizeof(*world->systems.archetypes));
    TNECS_CHECK_ALLOC(world->systems.archetypes);

    return(1);
}

b32 tnecs_grow_archetype(tnecs_world *world) {
    size_t olen = world->bytype.len;
    size_t nlen = olen * TNECS_ARRAY_GROWTH_FACTOR;
    world->bytype.len = nlen;

    world->bytype.id                = tnecs_realloc(world->bytype.id,             olen, nlen, sizeof(*world->bytype.id));
    TNECS_CHECK_ALLOC(world->bytype.id);
    world->bytype.entities          = tnecs_realloc(world->bytype.entities,        olen, nlen, sizeof(*world->bytype.entities));
    TNECS_CHECK_ALLOC(world->bytype.entities);
    world->bytype.components        = tnecs_realloc(world->bytype.components,      olen, nlen, sizeof(*world->bytype.components));
    TNECS_CHECK_ALLOC(world->bytype.components);
    world->bytype.num_archetype_ids = tnecs_realloc(world->bytype.num_archetype_ids,      olen, nlen, sizeof(*world->bytype.num_archetype_ids));
    TNECS_CHECK_ALLOC(world->bytype.num_archetype_ids);
    world->bytype.num_entities      = tnecs_realloc(world->bytype.num_entities,    olen, nlen, sizeof(*world->bytype.num_entities));
    TNECS_CHECK_ALLOC(world->bytype.num_entities);
    world->bytype.len_entities      = tnecs_realloc(world->bytype.len_entities,    olen, nlen, sizeof(*world->bytype.len_entities));
    TNECS_CHECK_ALLOC(world->bytype.len_entities);
    world->bytype.components_id      = tnecs_realloc(world->bytype.components_id,    olen, nlen, sizeof(*world->bytype.components_id));
    TNECS_CHECK_ALLOC(world->bytype.components_id);
    world->bytype.archetype_id      = tnecs_realloc(world->bytype.archetype_id,    olen, nlen, sizeof(*world->bytype.archetype_id));
    TNECS_CHECK_ALLOC(world->bytype.archetype_id);
    world->bytype.num_components    = tnecs_realloc(world->bytype.num_components,  olen, nlen, sizeof(*world->bytype.num_components));
    TNECS_CHECK_ALLOC(world->bytype.num_components);
    world->bytype.components_order  = tnecs_realloc(world->bytype.components_order, olen, nlen, sizeof(*world->bytype.components_order));
    TNECS_CHECK_ALLOC(world->bytype.components_order);

    for (size_t i = olen; i < world->bytype.len; i++) {
        world->bytype.entities[i]       = calloc(TNECS_INIT_ENTITY_LEN, sizeof(**world->bytype.entities));
        TNECS_CHECK_ALLOC(world->bytype.entities[i]);
        world->bytype.archetype_id[i]   = calloc(TNECS_COMPONENT_CAP, sizeof(**world->bytype.archetype_id));
        TNECS_CHECK_ALLOC(world->bytype.archetype_id[i]);

        world->bytype.len_entities[i] = TNECS_INIT_ENTITY_LEN;
        world->bytype.num_entities[i] = 0;
    }
    return(1);
}

b32 tnecs_grow_phase(tnecs_world *world) {
    size_t olen = world->byphase.len;
    size_t nlen = olen * TNECS_ARRAY_GROWTH_FACTOR;
    world->byphase.len = nlen;
    if (nlen >= TNECS_PHASES_CAP) {
        printf("tnecs: phases cap reached\n");
        return(TNECS_NULL);
    }

    world->byphase.id           = tnecs_realloc(world->byphase.id,          olen, nlen, sizeof(*world->byphase.id));
    TNECS_CHECK_ALLOC(world->byphase.id);
    world->byphase.systems      = tnecs_realloc(world->byphase.systems,     olen, nlen, sizeof(*world->byphase.systems));
    TNECS_CHECK_ALLOC(world->byphase.systems);
    world->byphase.systems_id   = tnecs_realloc(world->byphase.systems_id,  olen, nlen, sizeof(*world->byphase.systems_id));
    TNECS_CHECK_ALLOC(world->byphase.systems_id);
    world->byphase.len_systems  = tnecs_realloc(world->byphase.len_systems, olen, nlen, sizeof(*world->byphase.len_systems));
    TNECS_CHECK_ALLOC(world->byphase.len_systems);
    world->byphase.num_systems  = tnecs_realloc(world->byphase.num_systems, olen, nlen, sizeof(*world->byphase.num_systems));
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
    return(1);
}

b32 tnecs_grow_system_byphase(tnecs_world *world, const tnecs_phase phase) {
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
    return(1);
}

b32 tnecs_grow_bytype(tnecs_world *world, size_t tID) {
    size_t olen = world->bytype.len_entities[tID];
    size_t nlen = olen * TNECS_ARRAY_GROWTH_FACTOR;
    TNECS_DEBUG_ASSERT(olen > 0);
    world->bytype.len_entities[tID] = nlen;

    size_t bytesize             = sizeof(*world->bytype.entities[tID]);
    tnecs_entity *ptr           = world->bytype.entities[tID];
    world->bytype.entities[tID] = tnecs_realloc(ptr, olen, nlen, bytesize);
    TNECS_CHECK_ALLOC(world->bytype.entities[tID]);

    return(1);
}

/****************************** STRING HASHING *******************************/
uint64_t tnecs_hash_djb2(const char *str) {
    /* djb2 hashing algorithm by Dan Bernstein.
    * Description: This algorithm (k=33) was first reported by dan bernstein many
    * years ago in comp.lang.c. Another version of this algorithm (now favored by bernstein)
    * uses xor: hash(i) = hash(i - 1) * 33 ^ str[i]; the magic of number 33
    * (why it works better than many other constants, prime or not) has never been adequately explained.
    * [1] https://stackoverflow.com/questions/7666509/hash-function-for-string
    * [2] http://www.cse.yorku.ca/~oz/hash.html */
    uint64_t hash = 5381;
    int32_t str_char;
    while ((str_char = *str++))
        hash = ((hash << 5) + hash) + str_char; /* hash * 33 + c */
    return (hash);
}

uint64_t tnecs_hash_sdbm(const char *str) {
    /* sdbm hashing algorithm by Dan Bernstein.
    * Description: This algorithm was created for sdbm (a public-domain
    * reimplementation of ndbm) database library. It was found to do
    * well in scrambling bits, causing better distribution of the
    * keys and fewer splits. It also happens to be a good general hashing
    * function with good distribution. The actual function is
    *hash(i) = hash(i - 1) * 65599 + str[i]; what is included below
    * is the faster version used in gawk. [* there is even a faster,
    * duff-device version] the magic constant 65599 was picked out of
    * thin air while experimenting with different constants, and turns
    * out to be a prime. this is one of the algorithms used in
    * berkeley db (see sleepycat) and elsewhere.
    * [1] https://stackoverflow.com/questions/7666509/hash-function-for-string
    * [2] http://www.cse.yorku.ca/~oz/hash.html */

    uint64_t hash = 0;
    uint32_t str_char;
    while ((str_char = *str++)) {
        hash = str_char + (hash << 6) + (hash << 16) - hash;
    }
    return (hash);
}

uint64_t tnecs_hash_combine(uint64_t h1, uint64_t h2) {
    /* SotA: need to combine couple hashes into 1. Max 4-5? */
    /* -> Order of combination should not matter -> + or XOR  */
    /* -> Should be simple and fast -> + or XOR */
    return (h1 ^ h2); // XOR ignores order
}

/*************** SET BIT COUNTING *******************/
size_t setBits_KnR_uint64_t(uint64_t in_flags) {
    // Credits to Kernighan and Ritchie in the C Programming Language
    size_t count = 0;
    while (in_flags) {
        in_flags &= (in_flags - 1);
        count++;
    }
    return (count);
}

/********************** CHUNKs *********************/
tnecs_chunk tnecs_chunk_Init(const tnecs_world *world, const tnecs_component archetype) {
    // Chunk init
    tnecs_chunk chunk = {0};
    size_t *mem_header  = tnecs_chunk_BytesizeArr(&chunk);

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
        mem_header[chunk.num_components++] = cumul_bytesize; 
    }

    TNECS_DEBUG_ASSERT(cumul_bytesize > 0);

    chunk.len_entities = (TNECS_CHUNK_COMPONENTS_BYTESIZE) / cumul_bytesize;
    return(chunk);
}

// Order of entity in entities_bytype -> index of chunk components are stored in
size_t tnecs_EntityOrder_to_ArchetypeChunk(const tnecs_chunk *chunk, const size_t entity_order) {
    return(entity_order / chunk->len_entities);
}

// Order of entity in entities_bytype -> order of components in current ArchetypeChunk
size_t tnecs_EntityOrder_to_ChunkOrder(const tnecs_chunk *chunk, const size_t entity_order) {
    return(entity_order % chunk->len_entities);
}

size_t *tnecs_chunk_BytesizeArr(const tnecs_chunk *chunk) {
    return((size_t*)chunk->mem);
}
size_t  tnecs_chunk_TotalBytesize(const tnecs_chunk *chunk) {
    size_t *header = tnecs_chunk_BytesizeArr(chunk);
    if (chunk->num_components <= 0) {
        return(0);
    }
    return(header[chunk->num_components - 1]);
}

void *tnecs_chunk_ComponentArr(tnecs_chunk *chunk, const size_t corder) {
    size_t *header              = tnecs_chunk_BytesizeArr(chunk);
    size_t cumul_bytesize       = (corder == 0) ? 0 : header[corder - 1];
    size_t header_offset        = chunk->num_components * sizeof(size_t);
    size_t components_offset    = corder * cumul_bytesize * chunk->len_entities;

    tnecs_byte *bytemem = chunk->mem;
    return(bytemem + header_offset + components_offset);
}

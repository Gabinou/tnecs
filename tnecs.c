
#include "tnecs.h"

uint64_t get_ns() {
    static uint64_t is_init = 0;
#if defined(__APPLE__)
    static mach_timebase_info_data_t info;
    if (0 == is_init) {
        mach_timebase_info(&info);
        is_init = 1;
    }
    uint64_t now;
    now = mach_absolute_time();
    now *= info.numer;
    now /= info.denom;
    return now;
#elif defined(__linux)
    static struct timespec linux_rate;
    if (0 == is_init) {
        clock_getres(CLOCKID, &linux_rate);
        is_init = 1;
    }
    uint64_t now;
    struct timespec spec;
    clock_gettime(CLOCKID, &spec);
    now = spec.tv_sec * 1.0e9 + spec.tv_nsec;
    return now;
#elif defined(_WIN32)
    static LARGE_INTEGER win_frequency;
    if (0 == is_init) {
        QueryPerformanceFrequency(&win_frequency);
        is_init = 1;
    }
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (uint64_t)((1e9 * now.QuadPart) / win_frequency.QuadPart);
#endif
}

#ifdef MICROSECOND_CLOCK
extern double get_us();
#else
#  define FAILSAFE_CLOCK
#  define get_us() (((double)clock())/CLOCKS_PER_SEC*1e6) // [us]
#  define get_ns() (((double)clock())/CLOCKS_PER_SEC*1e9) // [ns]
#endif

struct tnecs_World * tnecs_world_genesis() {
    TNECS_DEBUG_PRINTF("tnecs_world_genesis\n");

    struct tnecs_World * tnecs_world = (struct tnecs_World *)calloc(sizeof(struct tnecs_World), 1);

    tnecs_world->entities = calloc(TNECS_INITIAL_ENTITY_LEN, sizeof(*tnecs_world->entities));
    tnecs_world->entity_typeflags = calloc(TNECS_INITIAL_ENTITY_LEN, sizeof(*tnecs_world->entity_typeflags));
    tnecs_world->entity_orders = calloc(TNECS_INITIAL_ENTITY_LEN, sizeof(*tnecs_world->entity_orders));

    tnecs_world->len_entities = TNECS_INITIAL_ENTITY_LEN;

    tnecs_world->typeflags = calloc(TNECS_INITIAL_ENTITY_LEN, sizeof(*tnecs_world->typeflags));
    tnecs_world->num_typeflags = 1;

    tnecs_world->system_typeflags = calloc(TNECS_INITIAL_SYSTEM_LEN, sizeof(*tnecs_world->system_typeflags));
    tnecs_world->system_hashes = calloc(TNECS_INITIAL_SYSTEM_LEN, sizeof(*tnecs_world->system_hashes));
    tnecs_world->system_phases = calloc(TNECS_INITIAL_SYSTEM_LEN, sizeof(*tnecs_world->system_phases));
    tnecs_world->system_orders = calloc(TNECS_INITIAL_SYSTEM_LEN, sizeof(*tnecs_world->system_orders));

    tnecs_world->len_phases = TNECS_INITIAL_PHASE_LEN;
    tnecs_world->num_phases = 1;
    tnecs_world->phases = calloc(TNECS_INITIAL_PHASE_LEN, sizeof(*tnecs_world->phases));
    tnecs_world->systems_idbyphase = calloc(TNECS_INITIAL_PHASE_LEN, sizeof(*tnecs_world->phases));
    tnecs_world->len_systems_byphase = calloc(TNECS_INITIAL_PHASE_LEN, sizeof(*tnecs_world->len_systems_byphase));
    tnecs_world->num_systems_byphase = calloc(TNECS_INITIAL_PHASE_LEN, sizeof(*tnecs_world->num_systems_byphase));
    tnecs_world->systems_byphase = calloc(TNECS_INITIAL_PHASE_LEN, sizeof(*tnecs_world->systems_byphase));
    for (size_t i = 0; i < TNECS_INITIAL_PHASE_LEN; i++) {
        tnecs_world->systems_byphase[i] = calloc(TNECS_INITIAL_PHASE_LEN, sizeof(*tnecs_world->systems_byphase[i]));
        tnecs_world->systems_idbyphase[i] = calloc(TNECS_INITIAL_PHASE_LEN, sizeof(*tnecs_world->systems_byphase[i]));
        tnecs_world->num_systems_byphase[i] = 0;
        tnecs_world->len_systems_byphase[i] = TNECS_INITIAL_PHASE_LEN;
    }

    tnecs_world->component_hashes[TNECS_NULL] = TNECS_NULL;

    tnecs_world->entities_bytype = calloc(TNECS_INITIAL_SYSTEM_LEN, sizeof(*tnecs_world->entities_bytype));
    tnecs_world->len_entities_bytype = calloc(TNECS_INITIAL_SYSTEM_LEN, sizeof(*tnecs_world->len_entities_bytype));
    tnecs_world->num_entities_bytype = calloc(TNECS_INITIAL_SYSTEM_LEN, sizeof(*tnecs_world->num_entities_bytype));

    tnecs_world->components_bytype = calloc(TNECS_INITIAL_SYSTEM_LEN, sizeof(*tnecs_world->components_bytype));
    tnecs_world->component_names = calloc(TNECS_COMPONENT_CAP, sizeof(*tnecs_world->component_names));
    tnecs_world->num_components_bytype = calloc(TNECS_INITIAL_SYSTEM_LEN, sizeof(*tnecs_world->num_components_bytype));
    tnecs_world->components_idbytype = calloc(TNECS_INITIAL_SYSTEM_LEN, sizeof(*tnecs_world->components_idbytype));
    tnecs_world->components_flagbytype = calloc(TNECS_INITIAL_SYSTEM_LEN, sizeof(*tnecs_world->components_flagbytype));
    tnecs_world->components_orderbytype = calloc(TNECS_INITIAL_SYSTEM_LEN, sizeof(*tnecs_world->components_orderbytype));
    for (size_t i = 0; i < TNECS_INITIAL_SYSTEM_LEN; i++) {
        tnecs_world->entities_bytype[i] = calloc(TNECS_INITIAL_ENTITY_LEN, sizeof(**tnecs_world->entities_bytype));
        tnecs_world->num_entities_bytype[i] = 0;
        tnecs_world->len_entities_bytype[i] = TNECS_INITIAL_ENTITY_LEN;

        tnecs_world->num_components_bytype[i] = 0;
    }
    tnecs_world->len_typeflags = TNECS_INITIAL_SYSTEM_LEN;

    tnecs_world->num_components = TNECS_NULLSHIFT;
    tnecs_world->num_systems = TNECS_NULLSHIFT;
    tnecs_world->num_typeflags = TNECS_NULLSHIFT;

    tnecs_world->systems = calloc(TNECS_INITIAL_SYSTEM_LEN, sizeof(*tnecs_world->systems));
    tnecs_world->len_systems = TNECS_INITIAL_SYSTEM_LEN;
    tnecs_world->num_systems = TNECS_NULLSHIFT;

    tnecs_world->next_entity_id = TNECS_NULLSHIFT;

    return (tnecs_world);
}

void tnecs_world_destroy(struct tnecs_World * in_world) {
    TNECS_DEBUG_PRINTF("tnecs_world_destroy\n");
    for (size_t i = 0; i < in_world->len_typeflags; i++) {

    }


    for (size_t i = 0; i < in_world->len_typeflags; i++) {
        free(in_world->entities_bytype[i]);
        free(in_world->components_idbytype[i]);
        free(in_world->components_flagbytype[i]);
        free(in_world->components_orderbytype[i]);
        for (size_t j = 0; j < in_world->num_components_bytype[i]; j++) {
            free(in_world->components_bytype[i][j].components);
        }
        free(in_world->components_bytype[i]);
    }
    for (size_t i = 0; i < in_world->num_components; i++) {
        free(in_world->component_names[i]);
    }
    free(in_world->entities_bytype);
    free(in_world->components_bytype);
    free(in_world->components_idbytype);
    free(in_world->components_flagbytype);
    free(in_world->components_orderbytype);
    free(in_world->component_names);
    free(in_world->entities);
    free(in_world->entity_typeflags);

    free(in_world->typeflags);

    free(in_world->systems);
    free(in_world->systems_byphase);
    free(in_world->system_typeflags);
    free(in_world->system_phases);
    free(in_world->system_hashes);

    free(in_world->num_components_bytype);
    free(in_world->len_entities_bytype);
    free(in_world->num_entities_bytype);

    free(in_world);
}

void tnecs_world_progress(struct tnecs_World * in_world, tnecs_time_ns_t in_deltat) {
    TNECS_DEBUG_PRINTF("tnecs_world_progress\n");
    // NEED have variable that tracks if systems were changed

    // 0- Compute current time.
    // 1- Make list of all systems (get from previous iteration)
    // 2- Make System_inputs for all systems (get from previous iteration)
    // 3- Run all systems on their respective inputs
    // 4- compute
    struct tnecs_System_Input current_input;
    current_input.world = in_world;
    tnecs_time_ns_t progress_time = get_ns();
    size_t system_id;
    for (size_t phase_id = 0; phase_id < in_world->num_phases; phase_id++) {
        for (size_t sorder = 0; sorder < in_world->num_systems_byphase[phase_id]; sorder++) {
            system_id = in_world->systems_idbyphase[phase_id][sorder];
            current_input.typeflag_id = tnecs_typeflagid(in_world, in_world->system_typeflags[system_id]) ;
            current_input.num_entities = in_world->num_entities_bytype[current_input.typeflag_id];
            in_world->systems_byphase[phase_id][system_id](&current_input);
        }
    }
    progress_time = get_ns() - progress_time;
}

tnecs_entity_t tnecs_new_entity(struct tnecs_World * in_world) {
    TNECS_DEBUG_PRINTF("tnecs_new_entity\n");

    tnecs_entity_t out = TNECS_NULL;
    tnecs_component_t component_flag;
    while ((out == TNECS_NULL) && (in_world->num_opened_entity_ids > 0)) {
        out = in_world->opened_entity_ids[--in_world->num_opened_entity_ids];
        in_world->opened_entity_ids[in_world->num_opened_entity_ids] = TNECS_NULL;
    }
    if (out == TNECS_NULL) {
        out = in_world->next_entity_id++;
    }
    TNECS_DEBUG_ASSERT(out != TNECS_NULL);
    if (in_world->next_entity_id >= in_world->len_entities) {
        tnecs_growArray_entity(in_world);
    }
    in_world->entities[out] =  out;
    in_world->entity_orders[out] = tnecs_entitiesbytype_add(in_world, out, TNECS_NULL);

    return (out);
}

void * tnecs_entity_get_component(struct tnecs_World * in_world, tnecs_entity_t in_entity_id, tnecs_component_t in_component_id) {
    TNECS_DEBUG_PRINTF("tnecs_entity_get_component\n");

    tnecs_component_t component_flag = TNECS_COMPONENT_ID2TYPEFLAG(in_component_id);
    tnecs_component_t entity_typeflag = TNECS_ENTITY_TYPEFLAG(in_world, in_entity_id);

    if ((component_flag & entity_typeflag) > 0) {
        size_t typeflag_id = tnecs_typeflagid(in_world, entity_typeflag);
        size_t component_order = tnecs_componentid_order_bytype(in_world, in_component_id, entity_typeflag);
        TNECS_DEBUG_ASSERT(component_order <= in_world->num_entities_bytype[typeflag_id]);
        size_t entity_order = in_world->entity_orders[in_entity_id];
        size_t bytesize = in_world->component_bytesizes[in_component_id];
        struct tnecs_Components_Array * comp_array = &in_world->components_bytype[typeflag_id][component_order];
        tnecs_byte_t * temp_component_bytesptr = (tnecs_byte_t *)(comp_array->components);
        return (temp_component_bytesptr + (bytesize * entity_order));
    } else {
        return (NULL);
    }
}

void tnecs_component_array_init(struct tnecs_World * in_world, struct tnecs_Components_Array * in_array, size_t in_component_id) {
    TNECS_DEBUG_PRINTF("tnecs_component_array_init\n");

    TNECS_DEBUG_ASSERT(in_component_id > 0);
    tnecs_component_t in_type = TNECS_COMPONENT_ID2TYPEFLAG(in_component_id);
    TNECS_DEBUG_ASSERT(in_type < (1 << in_world->num_components));
    size_t bytesize = in_world->component_bytesizes[in_component_id];
    TNECS_DEBUG_ASSERT(bytesize > 0);
    in_array->type = in_type;
    in_array->num_components = 0;
    in_array->len_components = TNECS_INITIAL_ENTITY_LEN;
    in_array->components = calloc(TNECS_INITIAL_ENTITY_LEN, bytesize);
}

void * tnecs_realloc(void * ptr, size_t old_len, size_t new_len, size_t elem_bytesize) {
    TNECS_DEBUG_PRINTF("tnecs_realloc\n");

    void * temp = (void *)calloc(new_len, elem_bytesize);
    memcpy(temp, ptr, old_len * elem_bytesize);
    free(ptr);
    return (temp);
}

void * tnecs_arrdel_scramble(void * arr, size_t elem, size_t len, size_t bytesize) {
    TNECS_DEBUG_PRINTF("tnecs_arrdel_scramble\n");

    return (memcpy(arr + (elem * bytesize), arr + ((len - 1) * bytesize), bytesize));
}


void * tnecs_arrdel(void * arr, size_t elem, size_t len, size_t bytesize) {
    TNECS_DEBUG_PRINTF("tnecs_arrdel\n");

    return (memcpy(arr + (elem * bytesize), arr + ((elem + 1) * bytesize), bytesize * (len - elem - 1)));
}

size_t tnecs_entitiesbytype_add(struct tnecs_World * in_world, tnecs_entity_t in_entity, tnecs_component_t typeflag_new) {
    TNECS_DEBUG_PRINTF("tnecs_entitiesbytype_add\n");

    size_t typeflag_id_new = tnecs_typeflagid(in_world, typeflag_new);
    if ((in_world->num_entities_bytype[typeflag_id_new] + 1) >= in_world->len_entities_bytype[typeflag_id_new]) {
        size_t old_len = in_world->len_entities_bytype[typeflag_id_new];
        in_world->len_entities_bytype[typeflag_id_new] *= TNECS_ARRAY_GROWTH_FACTOR;
        in_world->entities_bytype[typeflag_id_new] = tnecs_realloc(in_world->entities_bytype[typeflag_id_new], old_len, in_world->len_entities_bytype[typeflag_id_new], sizeof(*in_world->entities_bytype[typeflag_id_new]));
    }
    in_world->entities_bytype[typeflag_id_new][in_world->num_entities_bytype[typeflag_id_new]] = in_entity;
    in_world->entity_typeflags[in_entity] = typeflag_new;
    in_world->entity_orders[in_entity] = in_world->num_entities_bytype[typeflag_id_new];
    return (in_world->num_entities_bytype[typeflag_id_new]++);
}

void tnecs_entitiesbytype_del(struct tnecs_World * in_world, tnecs_entity_t in_entity, tnecs_component_t typeflag_old) {
    TNECS_DEBUG_PRINTF("tnecs_entitiesbytype_del\n");

    TNECS_DEBUG_ASSERT(in_entity);
    size_t typeflag_id_old = tnecs_typeflagid(in_world, typeflag_old);
    size_t entity_order_old = in_world->entity_orders[in_entity];
    if (entity_order_old < in_world->len_entities) {
        TNECS_DEBUG_ASSERT(in_world->entities_bytype[typeflag_id_old][entity_order_old] == in_entity);
        tnecs_entity_t top_entity = in_world->entities_bytype[typeflag_id_old][--in_world->num_entities_bytype[typeflag_id_old]];
        in_world->entities_bytype[typeflag_id_old][entity_order_old] = top_entity;
    }
}

size_t tnecs_entitiesbytype_migrate(struct tnecs_World * in_world, tnecs_entity_t in_entity, tnecs_component_t typeflag_old, tnecs_component_t typeflag_new) {
    TNECS_DEBUG_PRINTF("tnecs_entitiesbytype_migrate\n");

    tnecs_entitiesbytype_del(in_world, in_entity, typeflag_old);
    return (tnecs_entitiesbytype_add(in_world, in_entity, typeflag_new));
}

void tnecs_entity_add_components(struct tnecs_World * in_world, tnecs_entity_t in_entity, size_t num_components_toadd, tnecs_component_t typeflag_toadd, bool isNew) {
    TNECS_DEBUG_PRINTF("tnecs_entity_add_components\n");

    tnecs_component_t typeflag_old = in_world->entity_typeflags[in_entity];
    TNECS_DEBUG_ASSERT((typeflag_toadd != typeflag_old));
    tnecs_component_t typeflag_new = typeflag_toadd + typeflag_old;

    // 1- Checks if the new entity_typeflag exists, if not create empty component array
    if (isNew) {
        tnecs_new_typeflag(in_world, setBits_KnR_uint64_t(typeflag_new), typeflag_new);
    }
    tnecs_component_migrate(in_world, in_entity, typeflag_old, typeflag_new);

    tnecs_entitiesbytype_migrate(in_world, in_entity, typeflag_old, typeflag_new);
}

void tnecs_growArray_phase(struct tnecs_World * in_world) {
    TNECS_DEBUG_PRINTF("tnecs_growArray_phase\n");

    size_t old_len = in_world->len_phases;
    in_world->len_phases *= TNECS_ARRAY_GROWTH_FACTOR;

    // void * temp;
    // temp = calloc(in_world->len_phases, sizeof(*in_world->systems_byphase));
    // memcpy(temp, in_world->system_phases, old_len * sizeof(*in_world->system_phases));
    // free(in_world->system_phases);
    // in_world->system_phases = temp;

    in_world->systems_byphase = tnecs_realloc(in_world->systems_byphase, old_len, in_world->len_phases, sizeof(*in_world->systems_byphase));
    in_world->phases = tnecs_realloc(in_world->phases, old_len, in_world->len_phases, sizeof(*in_world->phases));
    in_world->len_systems_byphase = tnecs_realloc(in_world->len_systems_byphase, old_len, in_world->len_phases, sizeof(*in_world->len_systems_byphase));
    in_world->num_systems_byphase = tnecs_realloc(in_world->num_systems_byphase, old_len, in_world->len_phases, sizeof(*in_world->num_systems_byphase));

}

void tnecs_growArray_system(struct tnecs_World * in_world) {
    TNECS_DEBUG_PRINTF("tnecs_growArray_system\n");

    size_t old_len = in_world->len_systems;
    in_world->len_systems *= TNECS_ARRAY_GROWTH_FACTOR;

    // void * temp;
    // temp = calloc(in_world->len_systems, sizeof(*in_world->systems));
    // memcpy(temp, in_world->systems, old_len * sizeof(*in_world->systems));
    // free(in_world->systems);
    // in_world->entities = temp;

    // temp = calloc(in_world->len_phases, sizeof(*in_world->system_phases));
    // memcpy(temp, in_world->system_phases, old_len * sizeof(*in_world->system_phases));
    // free(in_world->system_phases);
    // in_world->system_phases = temp;

    // temp = calloc(in_world->system_typeflags, sizeof(*in_world->system_typeflags));
    // memcpy(temp, in_world->system_typeflags, old_len * sizeof(*in_world->system_typeflags));
    // free(in_world->system_typeflags);
    // in_world->system_typeflags = temp;

    // temp = calloc(in_world->len_systems, sizeof(*in_world->system_hashes));
    // memcpy(temp, in_world->system_hashes, old_len * sizeof(*in_world->system_hashes));
    // free(in_world->system_hashes);
    // in_world->system_hashes = temp;

    in_world->systems = tnecs_realloc(in_world->systems, old_len, in_world->len_systems, sizeof(*in_world->systems));
    in_world->system_phases = tnecs_realloc(in_world->system_phases, old_len, in_world->len_systems, sizeof(*in_world->system_phases));
    in_world->system_orders = tnecs_realloc(in_world->system_orders, old_len, in_world->len_systems, sizeof(*in_world->system_orders));
    in_world->systems_idbyphase = tnecs_realloc(in_world->systems_idbyphase, old_len, in_world->len_systems, sizeof(*in_world->systems_idbyphase));
    in_world->system_typeflags = tnecs_realloc(in_world->system_typeflags, old_len, in_world->len_systems, sizeof(*in_world->system_typeflags));
    in_world->system_hashes = tnecs_realloc(in_world->system_hashes, old_len, in_world->len_systems, sizeof(*in_world->system_hashes));
}

void tnecs_growArray_entity(struct tnecs_World * in_world) {
    TNECS_DEBUG_PRINTF("tnecs_growArray_entity\n");

    size_t old_len = in_world->len_entities;
    in_world->len_entities *= TNECS_ARRAY_GROWTH_FACTOR;

    // void * temp;
    // temp = calloc(in_world->len_entities, sizeof(*in_world->entities));
    // memcpy(temp, in_world->entities, old_len * sizeof(*in_world->entities));
    // free(in_world->entities);
    // in_world->entities = temp;

    // temp = calloc(in_world->len_entities, sizeof(*in_world->entity_typeflags));
    // memcpy(temp, in_world->entity_typeflags, old_len * sizeof(*in_world->entity_typeflags));
    // free(in_world->entity_typeflags);
    // in_world->entity_typeflags = temp;

    in_world->entity_typeflags = tnecs_realloc(in_world->entity_typeflags, old_len, in_world->len_entities, sizeof(*in_world->entity_typeflags));
    in_world->entity_orders = tnecs_realloc(in_world->entity_orders, old_len, in_world->len_entities, sizeof(*in_world->entity_orders));
    in_world->entities = tnecs_realloc(in_world->entities, old_len, in_world->len_entities, sizeof(*in_world->entities));
}

void tnecs_growArray_typeflag(struct tnecs_World * in_world) {
    TNECS_DEBUG_PRINTF("tnecs_growArray_typeflag\n");

    size_t old_len = in_world->len_typeflags;
    in_world->len_typeflags *= TNECS_ARRAY_GROWTH_FACTOR;

    // FOR BENCH TESTING:
    // CALL FUNCTION BENCHES.
    // REWRITE CODE BENCHES.
    // void * temp;
    // temp = calloc(in_world->len_typeflags, sizeof(*in_world->typeflags));
    // memcpy(temp, in_world->typeflags, old_len * sizeof(*in_world->typeflags) );
    // free(in_world->typeflags);
    // in_world->typeflags = temp;

    // temp = calloc(in_world->len_typeflags, sizeof(*in_world->components_bytype ));
    // memcpy(temp, in_world->components_bytype, old_len * sizeof(*in_world->components_bytype) );
    // free(in_world->components_bytype);
    // in_world->components_bytype = temp;

    // temp = calloc(in_world->len_typeflags, sizeof(*in_world->num_components_bytype ));
    // memcpy(temp, in_world->num_components_bytype, old_len * sizeof(*in_world->num_components_bytype) );
    // free(in_world->num_components_bytype);
    // in_world->num_components_bytype = temp;

    // temp = calloc(in_world->len_typeflags, sizeof(*in_world->entities_bytype));
    // memcpy(temp, in_world->entities_bytype , old_len * sizeof(*in_world->entities_bytype) );
    // free(in_world->entities_bytype);
    // in_world->entities_bytype = temp;

    // temp = calloc(in_world->len_typeflags, sizeof(*in_world->num_entities_bytype ));
    // memcpy(temp, in_world->num_entities_bytype, old_len * sizeof(*in_world->num_entities_bytype));
    // free(in_world->num_entities_bytype );
    // in_world->num_entities_bytype = temp;

    // temp = calloc(in_world->len_typeflags, sizeof(*in_world->len_entities_bytype ));
    // memcpy(temp, in_world->len_entities_bytype, old_len * sizeof(*in_world->len_entities_bytype));
    // free(in_world->len_entities_bytype );
    // in_world->len_entities_bytype = temp;

    // temp = calloc(in_world->len_typeflags, sizeof(*in_world->components_idbytype ));
    // memcpy(temp, in_world->components_idbytype, old_len * sizeof(*in_world->components_idbytype) );
    // free(in_world->components_idbytype );
    // in_world->components_idbytype = temp;

    // temp = calloc(in_world->len_typeflags, sizeof(*in_world->num_components_idbytype ));
    // memcpy(temp, in_world->num_components_idbytype, old_len * sizeof(*in_world->num_components_idbytype) );
    // free(in_world->num_components_idbytype);
    // in_world->components_idbytype = temp;

    // temp = calloc(in_world->len_typeflags, sizeof(*in_world->len_components_idbytype));
    // memcpy(temp, in_world->len_components_idbytype, old_len * sizeof(*in_world->len_components_idbytype));
    // free(in_world->len_components_idbytype);
    // in_world->components_idbytype = temp;

    // temp = alloc(in_world->len_typeflags, sizeof(*in_world->components_flagbytype));
    // memcpy(temp, in_world->components_flagbytype, old_len * sizeof(*in_world->components_flagbytype) );
    // free(in_world->components_flagbytype );
    // in_world->components_flagbytype = temp;

    in_world->typeflags = tnecs_realloc(in_world->typeflags, old_len, in_world->len_typeflags, sizeof(*in_world->typeflags));
    in_world->components_bytype = tnecs_realloc(in_world->components_bytype, old_len, in_world->len_typeflags, sizeof(*in_world->components_bytype));
    in_world->num_components_bytype = tnecs_realloc(in_world->num_components_bytype, old_len, in_world->len_typeflags, sizeof(*in_world->num_components_bytype));
    in_world->entities_bytype = tnecs_realloc(in_world->entities_bytype, old_len, in_world->len_typeflags, sizeof(*in_world->entities_bytype));
    in_world->num_entities_bytype = tnecs_realloc(in_world->num_entities_bytype, old_len, in_world->len_typeflags, sizeof(*in_world->num_entities_bytype));
    in_world->len_entities_bytype = tnecs_realloc(in_world->len_entities_bytype, old_len, in_world->len_typeflags, sizeof(*in_world->len_entities_bytype));
    in_world->components_idbytype = tnecs_realloc(in_world->components_idbytype, old_len, in_world->len_typeflags, sizeof(*in_world->components_idbytype));
    in_world->components_flagbytype = tnecs_realloc(in_world->components_flagbytype, old_len, in_world->len_typeflags, sizeof(*in_world->components_flagbytype));
    in_world->components_orderbytype = tnecs_realloc(in_world->components_orderbytype, old_len, in_world->len_typeflags, sizeof(*in_world->components_orderbytype));
    for (size_t i = (old_len - 1); i < in_world->len_typeflags; i++) {
        in_world->entities_bytype[i] = calloc(TNECS_INITIAL_ENTITY_LEN, sizeof(**in_world->entities_bytype));
        // in_world->components_orderbytype[i] = calloc(TNECS_INITIAL_COMPONENT_LEN, sizeof(size_t));
    }

}

void tnecs_component_array_new(struct tnecs_World * in_world, size_t num_components, tnecs_component_t in_typeflag) {
    TNECS_DEBUG_PRINTF("tnecs_component_array_new\n");
    // assumes new typeflag was added on top of world->typeflags

    struct tnecs_Components_Array * temp_comparray = (struct tnecs_Components_Array *)calloc(num_components, sizeof(struct tnecs_Components_Array));
    tnecs_component_t typeflag_reduced = in_typeflag;
    tnecs_component_t typeflag_added = 0;
    tnecs_component_t type_toadd;
    tnecs_component_t typeflag_id = tnecs_typeflagid(in_world, in_typeflag);
    size_t id_toadd, num_flags = 0;
    while (typeflag_reduced) {
        typeflag_reduced &= (typeflag_reduced - 1);
        type_toadd = (typeflag_reduced + typeflag_added) ^ in_typeflag;

        id_toadd = TNECS_COMPONENT_TYPE2ID(type_toadd);
        TNECS_DEBUG_ASSERT(id_toadd > 0);
        tnecs_component_array_init(in_world, &temp_comparray[num_flags], id_toadd);
        num_flags++;
        typeflag_added += type_toadd;
    }
    in_world->components_bytype[typeflag_id] = temp_comparray;


    TNECS_DEBUG_ASSERT(typeflag_added == in_typeflag);
    TNECS_DEBUG_ASSERT(num_flags == num_components);
}

size_t tnecs_new_typeflag(struct tnecs_World * in_world, size_t num_components, tnecs_component_t typeflag_new) {
    TNECS_DEBUG_PRINTF("tnecs_new_typeflag\n");

    size_t typeflag_id = 0;
    for (size_t i = 0 ; i < in_world->num_typeflags; i++) {
        if (typeflag_new == in_world->typeflags[i]) {
            typeflag_id = i;
            break;
        }
    }
    if (!typeflag_id) {
        // 1- Add new components_bytype at [typeflag_id]
        if ((in_world->num_typeflags + 1) >= in_world->len_typeflags) {
            tnecs_growArray_typeflag(in_world);
        }
        in_world->typeflags[in_world->num_typeflags++] = typeflag_new;
        size_t typeflag_id_new = tnecs_typeflagid(in_world, typeflag_new);
        TNECS_DEBUG_ASSERT(typeflag_id_new == (in_world->num_typeflags - 1));
        in_world->num_components_bytype[typeflag_id_new] = num_components;

        // 2- Add arrays to components_bytype[typeflag_id] for each component
        tnecs_component_array_new(in_world, num_components, typeflag_new);

        // 3- Add all components to components_idbytype and components_flagbytype
        tnecs_component_t component_id_toadd, component_type_toadd;
        tnecs_component_t typeflag_reduced = typeflag_new;
        tnecs_component_t typeflag_added = 0;
        in_world->components_idbytype[typeflag_id_new] =  calloc(num_components, sizeof(**in_world->components_idbytype));
        in_world->components_flagbytype[typeflag_id_new] =  calloc(num_components, sizeof(**in_world->components_flagbytype));
        in_world->components_orderbytype[typeflag_id_new] =  calloc(num_components, sizeof(**in_world->components_flagbytype));

        size_t i = 0;
        while (typeflag_reduced) {
            typeflag_reduced &= (typeflag_reduced - 1);
            component_type_toadd = (typeflag_reduced + typeflag_added) ^ typeflag_new;
            component_id_toadd = TNECS_COMPONENT_TYPE2ID(component_type_toadd);
            in_world->components_idbytype[typeflag_id_new][i] = component_id_toadd;
            in_world->components_flagbytype[typeflag_id_new][i] = component_type_toadd;
            in_world->components_orderbytype[typeflag_id_new][i] = component_id_toadd;

            typeflag_added += component_type_toadd;
            i++;
        }
    }
    return (typeflag_id);
}

size_t tnecs_component_hash2id(struct tnecs_World * in_world, uint64_t in_hash) {
    TNECS_DEBUG_PRINTF("tnecs_component_hash2id\n");

    size_t out;
    for (size_t i = 0; i < in_world->num_components; i++) {
        if (in_world->component_hashes[i] == in_hash) {
            out = i;
            break;
        }
    }
    return (out);
}

size_t tnecs_component_name2id(struct tnecs_World * in_world, const unsigned char * in_name) {
    TNECS_DEBUG_PRINTF("tnecs_component_name2id\n");

    return (tnecs_component_hash2id(in_world, tnecs_hash_djb2(in_name)));
}

tnecs_component_t tnecs_names2typeflag(struct tnecs_World * in_world, size_t argnum, ...) {
    TNECS_DEBUG_PRINTF("tnecs_names2typeflag\n");

    tnecs_component_t out = 0;
    va_list ap;
    va_start(ap, argnum);
    uint64_t temp_hash;
    for (size_t i = 0; i < argnum; i++) {
        temp_hash = tnecs_hash_djb2(va_arg(ap, const unsigned char *));
        for (size_t j = 0; j < in_world->num_components; j++) {
            if (in_world->component_hashes[j] == temp_hash) {
                out += TNECS_COMPONENT_ID2TYPEFLAG(j);
                break;
            }
        }
    }
    va_end(ap);
    return (out);
}

tnecs_component_t tnecs_component_ids2typeflag(size_t argnum, ...) {
    TNECS_DEBUG_PRINTF("tnecs_component_ids2typeflag\n");

    tnecs_component_t out = 0;
    va_list ap;
    va_start(ap, argnum);
    for (size_t i = 0; i < argnum; i++) {
        out += TNECS_COMPONENT_ID2TYPEFLAG(va_arg(ap, size_t));
    }
    va_end(ap);
    return (out);
}

tnecs_component_t tnecs_component_hash2typeflag(struct tnecs_World * in_world, uint64_t in_hash) {
    TNECS_DEBUG_PRINTF("tnecs_component_hash2typeflag \n");

    tnecs_component_t out = TNECS_NULL;
    for (size_t i = 0; i < in_world->num_components; i++) {
        if (in_world->component_hashes[i] == in_hash) {
            out = TNECS_COMPONENT_ID2TYPE(i);
        }
    }
    return (out);
}

tnecs_entity_t tnecs_new_entity_wcomponents(struct tnecs_World * in_world, size_t argnum, ...) {
    TNECS_DEBUG_PRINTF("tnecs_new_entity_wcomponents \n");

    va_list ap;
    va_start(ap, argnum);
    tnecs_component_t typeflag = 0;
    uint64_t current_hash;
    for (size_t i = 0; i < argnum; i++) {
        current_hash = va_arg(ap, uint64_t);
        typeflag += tnecs_component_hash2typeflag(in_world, current_hash);
    }
    va_end(ap);
    tnecs_entity_t new_entity = tnecs_new_entity(in_world);
    size_t typeflag_id = tnecs_new_typeflag(in_world, argnum, typeflag);

    tnecs_component_migrate(in_world, new_entity, TNECS_NULL, typeflag);
    TNECS_DEBUG_ASSERT(in_world->components_bytype[typeflag_id][0].components != NULL);
    tnecs_entitiesbytype_migrate(in_world, new_entity, TNECS_NULL, typeflag);
    TNECS_DEBUG_ASSERT(in_world->entity_typeflags[new_entity] == typeflag);
    return (new_entity);
}

void tnecs_entity_destroy(struct tnecs_World * in_world, tnecs_entity_t in_entity) {
    TNECS_DEBUG_PRINTF("tnecs_entity_destroy \n");

    TNECS_DEBUG_ASSERT(in_entity);
    tnecs_component_t entity_typeflag = in_world->entity_typeflags[in_entity];
    size_t entity_typeflag_id = TNECS_COMPONENT_TYPE2ID(entity_typeflag);
    size_t entity_order = in_world->entity_orders[in_entity];

    size_t component_id, component_bytesize;
    void * temp_component_bytesptr;

    // Deletes associated components
    for (size_t corder = 0; corder < in_world->num_components_bytype[entity_typeflag_id]; corder++) {
        component_id = in_world->components_idbytype[entity_typeflag_id][corder];
        component_bytesize = in_world->component_bytesizes[component_id];
        temp_component_bytesptr = (tnecs_byte_t *)(in_world->components_bytype[entity_typeflag_id][corder].components);
        in_world->components_bytype[entity_typeflag_id][corder].components = tnecs_arrdel(temp_component_bytesptr, entity_order, in_world->num_entities_bytype[entity_typeflag_id], component_bytesize);
    }

    // Entity removed from entities_bytype, entity_typeflags, entities...
    in_world->entities_bytype[entity_typeflag_id] = tnecs_arrdel(in_world->entities_bytype[entity_typeflag_id], entity_order, in_world->num_entities_bytype[entity_typeflag_id], sizeof(**in_world->entities_bytype));
    in_world->num_entities_bytype[entity_typeflag_id]--;
    in_world->entities[in_entity] = TNECS_NULL;
    in_world->entity_typeflags[in_entity] = TNECS_NULL;
}

void tnecs_register_component(struct tnecs_World * in_world, const char * in_name, size_t in_bytesize) {
    TNECS_DEBUG_PRINTF("tnecs_register_component\n");

    tnecs_hash_t in_hash = tnecs_hash_djb2(in_name);

    if (in_world->num_components < TNECS_COMPONENT_CAP) {
        in_world->component_hashes[in_world->num_components] = in_hash;
        tnecs_component_t new_component_flag = TNECS_COMPONENT_ID2TYPEFLAG(in_world->num_components);
        in_world->component_bytesizes[in_world->num_components] = in_bytesize;
        TNECS_DEBUG_ASSERT(in_bytesize > 0);
        char * temp_str = malloc(strlen(in_name));
        strncpy(temp_str, in_name, strlen(in_name));
        in_world->component_names[in_world->num_components] = temp_str;
        size_t typeflag_id = tnecs_new_typeflag(in_world, 1, new_component_flag);
        in_world->num_components++;
    } else {
        printf("TNECS ERROR: Cannot register more than 63 components");
    }
}
size_t tnecs_new_phase(struct tnecs_World * in_world, uint8_t in_phase) {
    TNECS_DEBUG_PRINTF("tnecs_new_phase\n");

    if ((in_world->num_phases + 1) >= in_world->len_phases) {
        tnecs_growArray_phase(in_world);
    }
    in_world->phases[in_world->num_phases] = in_phase;
    return (in_world->num_phases++);
}

void tnecs_register_system(struct tnecs_World * in_world, uint64_t in_hash, void (* in_system)(struct tnecs_System_Input *), uint8_t in_phase, size_t num_components, tnecs_component_t components_typeflag) {
    TNECS_DEBUG_PRINTF("tnecs_register_system\n");

    size_t system_id = in_world->num_systems++;
    if (in_world->num_systems >= in_world->len_systems) {
        tnecs_growArray_system(in_world);
    }

    size_t phase_id = tnecs_phaseid(in_world, in_phase);
    if (in_phase >= in_world->len_phases) {
        phase_id = tnecs_new_phase(in_world, in_phase);
    }

    in_world->system_phases[system_id] = in_phase;
    in_world->system_hashes[system_id] = in_hash;
    in_world->system_typeflags[system_id] = components_typeflag;
    in_world->systems[system_id] = in_system;

    size_t system_order = in_world->num_systems_byphase[phase_id]++;
    if (in_world->num_systems_byphase[phase_id] >= in_world->len_systems_byphase[phase_id]) {
        size_t old_len = in_world->len_systems_byphase[phase_id];
        in_world->len_systems_byphase[phase_id] *= TNECS_ARRAY_GROWTH_FACTOR;
        tnecs_realloc(in_world->systems_byphase[phase_id], old_len, in_world->len_systems_byphase[phase_id], sizeof(**in_world->systems_byphase));
        tnecs_realloc(in_world->systems_idbyphase[phase_id], old_len, in_world->len_systems_byphase[phase_id], sizeof(**in_world->systems_idbyphase));
    }
    in_world->system_orders[system_id] = system_order;
    in_world->systems_byphase[phase_id][system_order] = in_system;
    in_world->systems_idbyphase[phase_id][system_order] = system_id;

    size_t typeflag_id = tnecs_new_typeflag(in_world, num_components, components_typeflag);
}

void tnecs_component_remove(struct tnecs_World * in_world, tnecs_component_t in_typeflag) {
    size_t in_typeflag_id = tnecs_typeflagid(in_world, in_typeflag);
    size_t new_component_num = in_world->num_components_bytype[in_typeflag_id];
    struct tnecs_Components_Array * current_array;
    size_t current_component_id;
}

void tnecs_component_add(struct tnecs_World * in_world, tnecs_component_t in_typeflag) {
    TNECS_DEBUG_PRINTF("tnecs_component_add \n");

    // 1- Check if component array has enough room
    size_t in_typeflag_id = tnecs_typeflagid(in_world, in_typeflag);
    size_t new_component_num = in_world->num_components_bytype[in_typeflag_id];
    struct tnecs_Components_Array * current_array;
    size_t current_component_id;

    for (size_t corder = 0; corder < new_component_num; corder++) {
        current_array = &in_world->components_bytype[in_typeflag_id][corder];
        current_component_id = in_world->components_idbytype[in_typeflag_id][corder];

        TNECS_DEBUG_ASSERT(current_array != NULL);
        if (++current_array->num_components >= current_array->len_components) {
            size_t old_len = current_array->len_components;
            size_t bytesize = in_world->component_bytesizes[current_component_id];
            current_array->len_components *= TNECS_ARRAY_GROWTH_FACTOR;
            current_array->components = tnecs_realloc(current_array->components, old_len, current_array->len_components, bytesize);
        }
    }
}

void tnecs_component_copy(struct tnecs_World * in_world, tnecs_entity_t in_entity, tnecs_component_t old_typeflag, tnecs_component_t new_typeflag) {
    TNECS_DEBUG_PRINTF("tnecs_component_copy \n");
    TNECS_DEBUG_ASSERT(old_typeflag != NULL);

    size_t old_typeflag_id = tnecs_typeflagid(in_world, old_typeflag);
    size_t new_typeflag_id = tnecs_typeflagid(in_world, new_typeflag);
    size_t old_entity_order = in_world->entity_orders[in_entity];
    size_t new_entity_order = in_world->num_entities_bytype[new_typeflag_id]++;

    size_t old_component_id, new_component_id;
    size_t component_bytesize;
    tnecs_byte_t * old_component_ptr, * new_component_ptr;
    tnecs_byte_t * temp_component_bytesptr;
    for (size_t old_corder = 0; old_corder < in_world->num_components_bytype[old_typeflag_id]; old_corder++) {
        old_component_id = in_world->components_idbytype[old_typeflag_id][old_corder];
        for (size_t new_corder = 0; new_corder < in_world->num_components_bytype[new_typeflag_id]; new_corder++) {
            new_component_id = in_world->components_idbytype[new_typeflag_id][new_corder];
            if (old_component_id == new_component_id) {
                component_bytesize = in_world->component_bytesizes[old_component_id];
                temp_component_bytesptr = (tnecs_byte_t *)(in_world->components_bytype[old_typeflag_id][old_corder].components);
                TNECS_DEBUG_ASSERT(temp_component_bytesptr != NULL);

                old_component_ptr = (tnecs_byte_t *)(temp_component_bytesptr + (component_bytesize * old_entity_order));

                TNECS_DEBUG_ASSERT(old_component_ptr != NULL);
                temp_component_bytesptr = (tnecs_byte_t *)(in_world->components_bytype[new_typeflag_id][new_corder].components);
                new_component_ptr = (tnecs_byte_t *)(temp_component_bytesptr + (component_bytesize * new_entity_order));
                TNECS_DEBUG_ASSERT(new_component_ptr != NULL);
                memcpy(new_component_ptr, old_component_ptr, component_bytesize);
                break;
            }
        }
    }

}

void tnecs_component_del(struct tnecs_World * in_world, tnecs_entity_t in_entity, tnecs_component_t old_typeflag) {
    TNECS_DEBUG_PRINTF("tnecs_component_del \n");

    // for input entity, delete ALL components from componentsbytype
    size_t old_typeflag_id = tnecs_typeflagid(in_world, old_typeflag);
    size_t old_component_num = in_world->num_components_bytype[old_typeflag_id];
    size_t entity_order_old = in_world->entity_orders[in_entity];
    size_t component_order_current;
    size_t current_component_id;
    size_t component_bytesize;
    tnecs_byte_t * current_component_ptr, * next_component_ptr;
    tnecs_byte_t * temp_component_ptr;
    for (size_t corder = 0; corder < old_component_num; corder++) {
        current_component_id = in_world->components_idbytype[old_typeflag_id][corder];
        temp_component_ptr = (tnecs_byte_t *)in_world->components_bytype[old_typeflag_id][corder].components;
        TNECS_DEBUG_ASSERT(temp_component_ptr != NULL);

        component_bytesize = in_world->component_bytesizes[current_component_id];
        for (size_t eorder = entity_order_old; eorder < (in_world->num_entities_bytype[old_typeflag_id] - 1); eorder++) {
            current_component_ptr = temp_component_ptr + (component_bytesize * eorder);
            TNECS_DEBUG_ASSERT(current_component_ptr != NULL);
            next_component_ptr = temp_component_ptr + (component_bytesize * (eorder + 1));
            TNECS_DEBUG_ASSERT(next_component_ptr != NULL);

            memcpy(current_component_ptr, next_component_ptr, component_bytesize);
            memset(next_component_ptr, 0, component_bytesize);
        }
    }
}

bool tnecs_component_migrate(struct tnecs_World * in_world, tnecs_entity_t in_entity, size_t entity_order_new, tnecs_component_t new_typeflag) {
    TNECS_DEBUG_PRINTF("tnecs_component_migrate \n");

    tnecs_component_t old_typeflag = in_world->entity_typeflags[in_entity];
    if (old_typeflag > TNECS_NULL) {
        // tnecs_component_copy(in_world, in_entity, old_typeflag, new_typeflag);
        // tnecs_component_del(in_world, in_entity, old_typeflag);
    } else {
        // tnecs_component_add(in_world, new_typeflag);
    }
}

size_t tnecs_typeflagid(struct tnecs_World * in_world, tnecs_component_t in_typeflag) {
    TNECS_DEBUG_PRINTF("tnecs_typeflagid \n");

    size_t id = 0;
    for (size_t i = 0; i < in_world->num_typeflags; i++) {
        if (in_typeflag == in_world->typeflags[i]) {
            id = i;
            break;
        }
    }
    return (id);
}

size_t tnecs_system_hash2id(struct tnecs_World * in_world, uint64_t in_hash) {
    TNECS_DEBUG_PRINTF("tnecs_system_hash2id\n");

    size_t found = 0;
    for (size_t i = 0; i < in_world->num_systems; i++) {
        if (in_world->system_hashes[i] == in_hash) {
            found = i;
            break;
        }
    }
    return (found);
}

tnecs_component_t tnecs_component_hash2type(struct tnecs_World * in_world, uint64_t in_hash) {
    TNECS_DEBUG_PRINTF("tnecs_component_hash2type \n");
    return (TNECS_COMPONENT_ID2TYPEFLAG(tnecs_system_hash2id(in_world, in_hash)));
}

size_t tnecs_system_name2id(struct tnecs_World * in_world, const unsigned char * in_name) {
    TNECS_DEBUG_PRINTF("tnecs_system_name2id\n");
    return (tnecs_system_hash2id(in_world, tnecs_hash_djb2(in_name)));
}

tnecs_component_t tnecs_system_name2typeflag(struct tnecs_World * in_world, const unsigned char * in_name) {
    TNECS_DEBUG_PRINTF("tnecs_system_name2typeflag\n");
    size_t id = tnecs_system_hash2id(in_world, tnecs_hash_djb2(in_name));
    return (in_world->system_typeflags[id]);
}

size_t tnecs_phaseid(struct tnecs_World * in_world, uint8_t in_phase) {
    TNECS_DEBUG_PRINTF("tnecs_phaseid\n");

    size_t out = in_world->len_phases;
    for (size_t i = 0; i < in_world->num_phases; i++) {
        if (in_world->phases[i] == in_phase) {
            out = i;
            break;
        }
    }
    return (out);
}


tnecs_component_t tnecs_component_names2typeflag(struct tnecs_World * in_world, size_t argnum, ...) {
    TNECS_DEBUG_PRINTF("tnecs_component_names2typeflag\n");

    va_list ap;
    tnecs_component_t typeflag = 0;
    va_start(ap, argnum);
    for (size_t i = 0; i < argnum; i++) {
        typeflag += in_world->typeflags[tnecs_component_name2id(in_world, va_arg(ap, const unsigned char *))];
    }
    va_end(ap);
    return (typeflag);
}

size_t tnecs_componentflag_order_bytype(struct tnecs_World * in_world, tnecs_component_t in_component_flag, tnecs_component_t in_typeflag) {
    TNECS_DEBUG_PRINTF("tnecs_componentflag_order_bytype\n");

    size_t order = TNECS_COMPONENT_CAP;
    // tnecs_component_t in_typeflag_id = tnecs_component_flag2id(in_world, in_com);
    // for (size_t i = 0; i < in_world->num_components_bytype[in_typeflag_id]; i++) {
    //     if (in_world->components_flagbytype[in_typeflag_id][i] == in_component_flag) {
    //         order = i;
    //         break;
    //     }
    // }
    return (order);
}

size_t tnecs_componentid_order_bytypeid(struct tnecs_World * in_world, size_t in_component_id, size_t in_typeflag_id) {
    TNECS_DEBUG_PRINTF("tnecs_componentid_order_bytypeid\n");

    size_t order = TNECS_COMPONENT_CAP;
    for (size_t i = 0; i < in_world->num_components_bytype[in_typeflag_id]; i++) {
        if (in_world->components_idbytype[in_typeflag_id][i] == in_component_id) {
            order = i;
            break;
        }
    }
    return (order);
}

size_t tnecs_componentid_order_bytype(struct tnecs_World * in_world, size_t in_component_id, tnecs_component_t in_typeflag) {
    TNECS_DEBUG_PRINTF("tnecs_componentid_order_bytype\n");

    tnecs_component_t in_typeflag_id = tnecs_typeflagid(in_world, in_typeflag);
    return (tnecs_componentid_order_bytypeid(in_world, in_component_id, in_typeflag_id));
}

size_t tnecs_system_order_byphase(struct tnecs_World * in_world, size_t in_system_id, uint8_t in_phase) {
    TNECS_DEBUG_PRINTF("tnecs_system_order_byphase\n");

    // SYStems need to be stored BY PHASE also.
    // size_t order = in_world->num_systems;
    // for (size_t i = 0; i < in_world->num_systems_byphase[in_phase]; i++) {
    //     if (in_world->systems_idbyphase[in_phase][i] == in_system_id) {
    //         order = i;
    //         break;
    //     }
    // }
    // return (order);
}


// ******************* STRING HASHING *************************
uint64_t tnecs_hash_djb2(const unsigned char * str) {
    /* djb2 hashing algorithm by Dan Bernstein.
    * Description: This algorithm (k=33) was first reported by dan bernstein many
    * years ago in comp.lang.c. Another version of this algorithm (now favored by bernstein)
    * uses xor: hash(i) = hash(i - 1) * 33 ^ str[i]; the magic of number 33
    * (why it works better than many other constants, prime or not) has never been adequately explained.
    * [1] https://stackoverflow.com/questions/7666509/hash-function-for-string
    * [2] http://www.cse.yorku.ca/~oz/hash.html */
    TNECS_DEBUG_PRINTF("tnecs_hash_djb2\n");

    uint64_t hash = 5381;
    int32_t str_char;
    while (str_char = *str++) {
        hash = ((hash << 5) + hash) + str_char; /* hash * 33 + c */
    }
    return (hash);
}

uint64_t tnecs_hash_sdbm(const unsigned char * str) {
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
    TNECS_DEBUG_PRINTF("tnecs_hash_djb2\n");

    uint64_t hash = 0;
    uint32_t str_char;
    while (str_char = *str++) {
        hash = str_char + (hash << 6) + (hash << 16) - hash;
    }
    return (hash);
}

// *************************** SET BIT COUNTING *******************************
size_t setBits_KnR_uint64_t(uint64_t in_flags) {
    // Credits to Kernighan and Ritchie in the C Programming Language
    size_t count = 0;
    while (in_flags) {
        in_flags &= (in_flags - 1);
        count++;
    }
    return (count);
}


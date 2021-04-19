
#include "tnecs.h"

struct tnecs_World * tnecs_init() {
    TNECS_DEBUG_PRINTF("tnecs_init\n");

    struct tnecs_World * tnecs_world = (struct tnecs_World *)calloc(sizeof(struct tnecs_World), 1);
    tnecs_world->entities = NULL;
    arrsetcap(tnecs_world->entities, DEFAULT_ENTITY_CAP);
    arrput(tnecs_world->entities, TNECS_NULL);

    tnecs_world->typeflags = NULL;
    arrsetcap(tnecs_world->typeflags, DEFAULT_ENTITY_CAP);
    arrput(tnecs_world->typeflags, TNECS_NULL);

    tnecs_world->entity_typeflags = NULL;
    arrsetcap(tnecs_world->entity_typeflags, DEFAULT_ENTITY_CAP);
    arrput(tnecs_world->entity_typeflags, TNECS_NULL);

    tnecs_world->system_typeflags = NULL;
    arrsetcap(tnecs_world->system_typeflags, DEFAULT_SYSTEM_CAP);
    arrput(tnecs_world->system_typeflags, TNECS_NULL);

    tnecs_world->system_isExclusive = NULL;
    arrsetcap(tnecs_world->system_isExclusive, DEFAULT_SYSTEM_CAP);
    arrput(tnecs_world->system_isExclusive, TNECS_NULL);

    tnecs_world->component_hashes = NULL;
    arrsetcap(tnecs_world->component_hashes, MAX_COMPONENT);
    arrput(tnecs_world->component_hashes, TNECS_NULL);

    tnecs_world->system_hashes = NULL;
    arrsetcap(tnecs_world->system_hashes, DEFAULT_SYSTEM_CAP);
    arrput(tnecs_world->system_hashes, TNECS_NULL);

    tnecs_world->entities_bytype = NULL;
    arrsetcap(tnecs_world->entities_bytype, DEFAULT_SYSTEM_CAP);
    arrsetlen(tnecs_world->num_entitiesbytype, DEFAULT_SYSTEM_CAP);
    tnecs_world->num_componentsbytype = NULL;
    arrsetcap(tnecs_world->num_componentsbytype, DEFAULT_SYSTEM_CAP);
    for (size_t i = 0 ; i < DEFAULT_SYSTEM_CAP; i++) {
        tnecs_world->num_entitiesbytype[i] = 0;
        tnecs_world->num_componentsbytype[i] = 0;
    }
    arrput(tnecs_world->entities_bytype, NULL);
    arrput(tnecs_world->entities_bytype[TNECS_NULL], TNECS_NULL);
    tnecs_world->num_entitiesbytype[TNECS_NULL]++;

    tnecs_world->component_idbytype = NULL;
    arrsetcap(tnecs_world->component_idbytype, DEFAULT_SYSTEM_CAP);
    arrput(tnecs_world->component_idbytype, NULL);

    tnecs_world->component_flagbytype = NULL;
    arrsetcap(tnecs_world->component_flagbytype, DEFAULT_SYSTEM_CAP);
    arrput(tnecs_world->component_flagbytype, NULL);

    tnecs_world->num_entitiesbytype = NULL;
    arrsetcap(tnecs_world->num_entitiesbytype, DEFAULT_SYSTEM_CAP);

    tnecs_world->num_components = TNECS_ID_START;
    tnecs_world->num_systems = TNECS_ID_START;
    tnecs_world->num_typeflags = TNECS_ID_START;

    tnecs_world->components_bytype = NULL;
    arrsetcap(tnecs_world->components_bytype, DEFAULT_SYSTEM_CAP);

    tnecs_world->systems = NULL;
    arrsetcap(tnecs_world->systems, DEFAULT_SYSTEM_CAP);
    arrput(tnecs_world->systems, NULL);

    tnecs_world->next_entity_id = TNECS_ID_START;

    return (tnecs_world);
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
    arrput(in_world->entities, out);
    arrput(in_world->entities_bytype[TNECS_NOCOMPONENT_TYPEFLAG], out);
    in_world->num_entitiesbytype[TNECS_NOCOMPONENT_TYPEFLAG]++;
    return (out);
}

void tnecs_entity_add_components(struct tnecs_World * in_world, tnecs_entity_t in_entity, size_t num_components, tnecs_components_t typeflag_toadd, bool isNew) {
    TNECS_DEBUG_PRINTF("tnecs_entity_add_components\n");

    tnecs_component_t total_typeflag = typeflag_toadd + in_world->entity_typeflags[in_entity];
    if (isNew) {
        tnecs_new_typeflag(in_world, num_components, total_typeflag);
        tnecs_entity_typeflag_change(in_world, in_entity, total_typeflag);
    }
}


size_t tnecs_new_typeflag(struct tnecs_World * in_world, size_t num_components, tnecs_components_t new_typeflag) {
    TNECS_DEBUG_PRINTF("tnecs_new_typeflag\n");
    // outputs 0 is typeflag is new, its index if not
    size_t typeflag_id = 0;
    for (size_t i = 0 ; i < in_world->num_typeflags; i++) {
        if (new_typeflag == in_world->typeflags[i]) {
            typeflag_id = i;
            break;
        }
    }
    if (!typeflag_id) {
        in_world->num_typeflags++;
        arrput(in_world->typeflags, new_typeflag);
        struct tnecs_Components_Array * temp_comparray = NULL;
        arrsetlen(temp_comparray, num_components);
        arrput(in_world->components_bytype, temp_comparray);
        arrput(in_world->entities_bytype, NULL);
        arrput(in_world->num_componentsbytype, num_components);
        arrput(in_world->num_entitiesbytype, 0);
        // } else {
        // TNECS_DEBUG_PRINTF("tnecs_new_typeflag: new_typeflag already exists!");
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

    return (tnecs_component_hash2id(in_world, hash_djb2(in_name)));
}

tnecs_component_t tnecs_names2typeflag(struct tnecs_World * in_world, size_t argnum, ...) {
    TNECS_DEBUG_PRINTF("tnecs_names2typeflag\n");

    tnecs_component_t out = 0;
    va_list ap;
    va_start(ap, argnum);
    uint64_t temp_hash;
    for (size_t i = 0; i < argnum; i++) {
        temp_hash = hash_djb2(va_arg(ap, const unsigned char *));
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

tnecs_entity_t tnecs_new_entity_wcomponents(struct tnecs_World * in_world, size_t argnum, ...) {
    TNECS_DEBUG_PRINTF("tnecs_new_entity_wcomponents \n");

    va_list ap;
    va_start(ap, argnum);
    tnecs_component_t typeflag = 0;
    for (size_t i = 0; i < argnum; i++) {
        TNECS_DEBUG_PRINTF("Current hash %llu\n", va_arg(ap, uint64_t));
        typeflag += tnecs_component_hash2id(in_world, va_arg(ap, uint64_t));
    }
    va_end(ap);
    tnecs_entity_t new_entity = tnecs_new_entity(in_world);
    size_t typeflag_id = tnecs_new_typeflag(in_world, argnum, typeflag);
    arrput(in_world->entities_bytype[typeflag_id], new_entity);
    in_world->num_entitiesbytype[typeflag_id]++;
    return (new_entity);
}

void tnecs_entity_destroy(struct tnecs_World * in_world, tnecs_entity_t in_entity) {
    TNECS_DEBUG_PRINTF("tnecs_entity_destroy \n");

    tnecs_component_t previous_flag = in_world->entity_typeflags[in_entity];
    for (size_t i = 0 ; i < in_world->num_systems; i++) {
        if (previous_flag == in_world->system_typeflags[i]) {
            for (size_t j = 0 ; j < in_world->num_entitiesbytype[i]; j++) {
                if (in_world->entities_bytype[i][j] == in_entity) {
                    arrdel(in_world->entities_bytype[i], j);
                    break;
                }
            }
        }

        in_world->entity_typeflags[in_entity] = 0;
        if (in_world->num_opened_entity_ids < OPEN_IDS_BUFFER) {
            in_world->opened_entity_ids[in_world->num_opened_entity_ids++] = in_entity;
        }
    }
}

void tnecs_register_component(struct tnecs_World * in_world, uint64_t in_hash) {
    TNECS_DEBUG_PRINTF("tnecs_register_component\n");
    arrput(in_world->component_hashes, in_hash);
    arrput(in_world->typeflags, (1ULL << (in_world->num_components - 1)));
    in_world->num_components++;
}

void tnecs_register_system(struct tnecs_World * in_world, uint64_t in_hash, void (* in_system)(struct tnecs_System_Input), uint8_t in_run_phase, bool isExclusive, size_t num_components, tnecs_components_t components_typeflag) {
    TNECS_DEBUG_PRINTF("tnecs_register_system\n");

    arrput(in_world->system_isExclusive, isExclusive);
    arrput(in_world->system_phase, in_run_phase);
    arrput(in_world->system_hashes, in_hash);
    arrput(in_world->system_typeflags, components_typeflag);
    arrput(in_world->systems, in_system);
    arrput(in_world->num_componentsbytype, num_components);

    in_world->num_systems++;
    size_t typeflag_id = tnecs_new_typeflag(in_world, num_components, components_typeflag);

}

void tnecs_new_component(struct tnecs_World * in_world, tnecs_entity_t in_entity, tnecs_components_t typeflag, tnecs_components_t type_toadd) {
    TNECS_DEBUG_PRINTF("tnecs_new_component\n");

    bool found = 0;
    for (size_t i = 0; i < in_world->num_componentsbytype[typeflag]; i++) {
        if (in_world->component_flagbytype[typeflag][i] == type_toadd) {
            found = true;
            break;
        }
    }
    if (!found) {
        struct tnecs_Components_Array temp;
        temp.components = NULL;
        temp.type = type_toadd;
        arrput(in_world->components_bytype[typeflag], temp);
        arrput(in_world->component_flagbytype[typeflag], type_toadd);
    } else {
        TNECS_DEBUG_PRINTF("tnecs_componentsbytype_add: component already in component_flagbytype");
    }

}

void tnecs_entity_typeflag_change(struct tnecs_World * in_world, tnecs_entity_t in_entity, tnecs_components_t new_type) {
    TNECS_DEBUG_PRINTF("tnecs_entity_typeflag_change\n");

    tnecs_components_t previous_flag = in_world->entity_typeflags[in_entity];
    in_world->entity_typeflags[in_entity] = in_world->entity_typeflags[in_entity] | new_type;

    for (size_t i = 0; i < in_world->num_typeflags; i++) {
        if (previous_flag == in_world->entity_typeflags[i]) { //      EXCLUSIVE
            for (size_t j = 0; j < in_world->num_entitiesbytype[i]; j++) {
                if (in_entity == in_world->entities_bytype[i][j]) {
                    arrdel(in_world->entities_bytype[i], j);
                    break;
                }
            }
        }
        if (in_world->entity_typeflags[in_entity] == in_world->entity_typeflags[i]) { //      EXCLUSIVE
            arrput(in_world->entities_bytype[i], in_entity);
        }
        // if (previous_flag & in_world->system_typeflags[i] > 0) { //   INCLUSIVE
        // if (previous_flag & in_world->system_typeflags[i] > 0) { //   INCLUSIVE
    }
}

bool tnecs_componentsbytype_migrate(struct tnecs_World * in_world, tnecs_entity_t in_entity, tnecs_components_t old_flag, tnecs_components_t new_flag) {
    TNECS_DEBUG_PRINTF("tnecs_componentsbytype_migrate \n");

    // Migrates components associated with in_entity
    // -components_bytype: previous_flag -> new_flag
    // DOES NOT CHECK in_entity's TYPE.
    size_t new_type_id = tnecs_type_id(in_world->system_typeflags, in_world->num_typeflags, new_flag);
    size_t old_type_id = tnecs_type_id(in_world->system_typeflags, in_world->num_typeflags, old_flag);

    tnecs_entity_t * new_type_entities = in_world->entities_bytype[new_type_id];
    tnecs_entity_t * old_type_entities = in_world->entities_bytype[old_type_id];

    size_t new_num_entities = in_world->num_entitiesbytype[new_type_id];
    size_t old_num_entities = in_world->num_entitiesbytype[old_type_id];
    size_t new_component_num = in_world->num_componentsbytype[new_type_id];
    size_t old_component_num = in_world->num_componentsbytype[old_type_id];

    struct tnecs_Components_Array * new_type_components_byentity = in_world->components_bytype[new_type_id];
    struct tnecs_Components_Array * old_type_components_byentity = in_world->components_bytype[old_type_id];

    // Deletes in_entity from old_type_entities
    size_t found_old = 0;
    for (size_t i = 0; i < old_num_entities; i++) {
        if (old_type_entities[i] == in_entity) {
            arrdel(old_type_entities, i);
            in_world->num_entitiesbytype[old_type_id]--;
            found_old = i;
        }
    }
    size_t found_new = 0;
    for (size_t i = 0; i < new_num_entities; i++) {
        if (new_type_entities[i] == in_entity) {
            found_new = i;
        }
    }
    if (!found_new) {
        arrput(new_type_entities, in_entity);
        in_world->entities_bytype[new_type_id]++;
    } else {
        TNECS_DEBUG_PRINTF("tnecs_componentsbytype_migrate: entity found in components_bytype for new_flag");
    }

    if (found_old && !found_new) {
        arrput(new_type_components_byentity, old_type_components_byentity[found_old]);
        arrdel(old_type_components_byentity, found_old);
    } else {
        TNECS_DEBUG_PRINTF("tnecs_componentsbytype_migrate: entity found in components_bytype for new_flag");
    }
    return (1);
}

size_t tnecs_type_id(tnecs_components_t * in_typelist, size_t len, tnecs_components_t in_flag) {
    TNECS_DEBUG_PRINTF("tnecs_type_id \n");

    size_t found = TNECS_NULL;
    for (size_t i = 0; i < len; i++) {
        if (in_typelist[i] == in_flag) {
            found = i;
            break;
        }
    }
    return (found);
}

size_t tnecs_component_typeflag2id(struct tnecs_World * in_world, tnecs_component_t in_typeflag) {
    TNECS_DEBUG_PRINTF("tnecs_component_typeflag2id \n");

    size_t id = 0;
    for (size_t i = 0; i < in_world->num_typeflags; i++) {
        if (in_typeflag == in_world->typeflags[i]) {
            id = i;
            break;
        }
    }
    return (id);
}


size_t tnecs_issubtype(tnecs_components_t * in_typelist, size_t len, tnecs_components_t in_flag) {
    // returns position of subtype from in_typelist
    TNECS_DEBUG_PRINTF("tnecs_issubtype\n");

    size_t found = 0;
    for (size_t i = 0; i < len; i++) {
        if ((in_typelist[i] & in_flag) == in_flag) {
            found = i;
            break;
        }
    }
    return (found);
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

size_t tnecs_system_name2id(struct tnecs_World * in_world, const unsigned char * in_name) {
    TNECS_DEBUG_PRINTF("tnecs_system_name2id\n");
    return (tnecs_system_hash2id(in_world, hash_djb2(in_name)));
}

tnecs_component_t tnecs_system_name2typeflag(struct tnecs_World * in_world, const unsigned char * in_name) {
    TNECS_DEBUG_PRINTF("tnecs_system_name2typeflag\n");
    size_t id = tnecs_system_hash2id(in_world, hash_djb2(in_name));
    return (in_world->system_typeflags[id]);
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

// STRING HASHING
uint64_t hash_djb2(const unsigned char * str) {
    /* djb2 hashing algorithm by Dan Bernstein.
    * Description: This algorithm (k=33) was first reported by dan bernstein many
    * years ago in comp.lang.c. Another version of this algorithm (now favored by bernstein)
    * uses xor: hash(i) = hash(i - 1) * 33 ^ str[i]; the magic of number 33
    * (why it works better than many other constants, prime or not) has never been adequately explained.
    * [1] https://stackoverflow.com/questions/7666509/hash-function-for-string
    * [2] http://www.cse.yorku.ca/~oz/hash.html */
    TNECS_DEBUG_PRINTF("hash_djb2\n");

    uint64_t hash = 5381;
    int32_t str_char;
    while (str_char = *str++) {
        hash = ((hash << 5) + hash) + str_char; /* hash * 33 + c */
    }
    return (hash);
}

uint64_t hash_sdbm(const unsigned char * str) {
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
    TNECS_DEBUG_PRINTF("hash_djb2\n");

    uint64_t hash = 0;
    uint32_t str_char;
    while (str_char = *str++) {
        hash = str_char + (hash << 6) + (hash << 16) - hash;
    }
    return (hash);
}

// SET BIT COUNTING
int8_t setBits_KnR_uint64_t(uint64_t in_flags) {
    // Credits to Kernighan and Ritchie in the C Programming Language
    // should ouput -1 on error
    uint64_t count = 0;
    while (in_flags) {
        in_flags &= (in_flags - 1);
        count++;
    }
    return (count);
}

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
    printf("simplecs_init \n");

    struct Simplecs_World * simplecs_world = (struct Simplecs_World *)calloc(sizeof(struct Simplecs_World), 1);
    simplecs_world->entities = NULL;
    arrsetcap(simplecs_world->entities, DEFAULT_ENTITY_CAP);
    arrput(simplecs_world->entities, SIMPLECS_NULL);

    simplecs_world->typeflags = NULL;
    arrsetcap(simplecs_world->typeflags, DEFAULT_ENTITY_CAP);
    arrput(simplecs_world->entity_typeflags, SIMPLECS_NULL);

    simplecs_world->entity_typeflags = NULL;
    arrsetcap(simplecs_world->entity_typeflags, DEFAULT_ENTITY_CAP);
    // arrput(simplecs_world->entity_typeflags, SIMPLECS_NULL);

    simplecs_world->system_typeflags = NULL;
    arrsetcap(simplecs_world->system_typeflags, DEFAULT_SYSTEM_CAP);
    // arrput(simplecs_world->system_typeflags, SIMPLECS_NULL);

    simplecs_world->system_isExclusive = NULL;
    arrsetcap(simplecs_world->system_isExclusive, DEFAULT_SYSTEM_CAP);
    // arrput(simplecs_world->system_isExclusive, SIMPLECS_NULL);

    simplecs_world->component_hashes = NULL;
    arrsetcap(simplecs_world->component_hashes, MAX_COMPONENT);
    arrput(simplecs_world->component_hashes, SIMPLECS_NULL);

    // simplecs_world->component_typehash = NULL;
    // hmdefault(simplecs_world->component_typehash, SIMPLECS_NULL);

    // simplecs_world->component_id = NULL;
    // hmdefault(simplecs_world->component_id, SIMPLECS_NULL);

    simplecs_world->entitiesbytype = NULL;
    arrsetcap(simplecs_world->entitiesbytype, DEFAULT_SYSTEM_CAP);
    arrput(simplecs_world->entitiesbytype, NULL);

    simplecs_world->component_idbytype = NULL;
    arrsetcap(simplecs_world->component_idbytype, DEFAULT_SYSTEM_CAP);
    // arrput(simplecs_world->component_idbytype, NULL);

    simplecs_world->component_flagbytype = NULL;
    arrsetcap(simplecs_world->component_flagbytype, DEFAULT_SYSTEM_CAP);
    // arrput(simplecs_world->component_flagbytype, NULL);

    simplecs_world->num_componentsbytype = NULL;
    arrsetcap(simplecs_world->num_componentsbytype, DEFAULT_SYSTEM_CAP);
    // arrput(simplecs_world->component_flagbytype, NULL);

    simplecs_world->num_entitiesbytype = NULL;
    arrsetcap(simplecs_world->num_entitiesbytype, DEFAULT_SYSTEM_CAP);

    simplecs_world->num_components = 0;
    simplecs_world->num_systems = 0;
    simplecs_world->num_typeflags = 0;

    simplecs_world->components_bytype = NULL;
    arrsetcap(simplecs_world->components_bytype, DEFAULT_SYSTEM_CAP);

    simplecs_world->next_entity_id = ENTITY_ID_START;
    simplecs_world->next_system_id = 0;

    return (simplecs_world);
}

uint64_t hash_djb2(const unsigned char * str) {
    /* djb2 hashing algorithm by Dan Bernstein.
    * Description: This algorithm (k=33) was first reported by dan bernstein many
    * years ago in comp.lang.c. Another version of this algorithm (now favored by bernstein)
    * uses xor: hash(i) = hash(i - 1) * 33 ^ str[i]; the magic of number 33
    * (why it works better than many other constants, prime or not) has never been adequately explained.
    * [1] https://stackoverflow.com/questions/7666509/hash-function-for-string
    * [2] http://www.cse.yorku.ca/~oz/hash.html */
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
    uint64_t hash = 0;
    uint32_t str_char;
    while (str_char = *str++) {
        hash = str_char + (hash << 6) + (hash << 16) - hash;
    }
    return (hash);
}


simplecs_entity_t simplecs_new_entity(struct Simplecs_World * in_world) {
    simplecs_entity_t out = SIMPLECS_NULL;
    simplecs_component_t component_flag;
    while ((out == SIMPLECS_NULL) && (in_world->num_opened_entity_ids > 0)) {
        out = in_world->opened_entity_ids[--in_world->num_opened_entity_ids];
        in_world->opened_entity_ids[in_world->num_opened_entity_ids] = SIMPLECS_NULL;
    }
    if (out == SIMPLECS_NULL) {
        out = in_world->next_entity_id++;
    }
    return (out);
}

void simplecs_new_typeflag(struct Simplecs_World * in_world, simplecs_components_t new_typeflag) {
    // arrput(in_world->num_entitiesbytype[SIMPLECS_NULL])
    bool found = 0;
    for (size_t i = 0 ; i < in_world->num_typeflags; i++) {
        if (new_typeflag == in_world->typeflags[i]) {
            found = true;
            break;
        }
    }
    if (!found) {
        in_world->entitiesbytype;
        in_world->component_idbytype;
        in_world->component_flagbytype;
        in_world->components_bytype;
    } else {
        printf("simplecs_new_typeflag: new_typeflag already exists!");
    }
}



simplecs_component_t simplecs_name2id(struct Simplecs_World * in_world, const char * in_name) {
    simplecs_component_t out = 0;
    uint64_t temp_hash = hash_djb2(in_name);
    for (size_t j = 0; j < in_world->num_components; j++) {
        if (in_world->component_hashes[j] == temp_hash) {
            out = j;
            break;
        }
    }
    return (out);
}

simplecs_component_t simplecs_names2typeflag(struct Simplecs_World * in_world, uint8_t num, ...) {
    simplecs_component_t out = 0;
    va_list ap;
    va_start(ap, num);
    uint64_t temp_hash;
    for (size_t i = 0; i < num; i++) {
        temp_hash = hash_djb2(va_arg(ap, char *));
        for (size_t j = 0; j < in_world->num_components; j++) {
            if (in_world->component_hashes[j] == temp_hash) {
                out += SIMPLECS_ID2TYPEFLAG(j);
                break;
            }
        }
    }
    va_end(ap);
    return (out);
}

simplecs_component_t simplecs_ids2typeflag(uint8_t num, ...) {
    // simplecs_component_t out = 0;
    // va_list ap;
    // va_start(ap, num);
    // char * temp_str;
    // for (size_t i = 0; i < num; i++) {
    //     out += 1 << (va_arg(ap, size_t) - COMPONENT_ID_START);
    // }
    // va_end(ap);
    // return (out);
}

size_t simplecs_component_hash2id(struct Simplecs_World * in_world, uint64_t in_hash) {
    size_t out;
    for (size_t i = 0; i < in_world->num_components; i++) {
        if (in_world->component_hashes[i] == in_hash) {
            out = i;
            break;
        }
    }
    return (out);
}

simplecs_entity_t simplecs_new_entity_wcomponents(struct Simplecs_World * in_world, simplecs_components_t component_typeflag) {
    printf("simplecs_new_entity_wcomponents \n");

}


simplecs_entity_t simplecs_entity_destroy(struct Simplecs_World * in_world, simplecs_entity_t in_entity) {
    printf("simplecs_entity_destroy \n");
    simplecs_component_t previous_flag = in_world->entity_typeflags[in_entity];

    for (size_t i = 0 ; i < in_world->num_systems; i++) {
        if (previous_flag == in_world->system_typeflags[i]) {
            for (size_t j = 0 ; j < in_world->num_entitiesbytype[i]; j++) {
                if (in_world->entitiesbytype[i][j] == in_entity) {
                    arrdel(in_world->entitiesbytype[i], j);
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

void simplecs_register_system(struct Simplecs_World * in_world, simplecs_entity_t * entities_list, uint8_t in_run_phase, bool isexclusive, size_t component_num, simplecs_components_t component_typeflag) {
    printf("simplecs_register_system\n");
    // arrput(in_world->systems_table->systems_list, in_system);
    // arrput(in_world->systems_table->components_num, num_components);
    // simplecs_entity_t * components_list = malloc(num_components * sizeof(simplecs_entity_t));
    // va_list ap;
    // va_start(ap, num_components);
    // for (size_t i = 0; i < num_components; i++) {
    //     components_list[i] = va_arg(ap, simplecs_entity_t);
    // }
    // va_end
    // arrput(in_world->systems_table->components_lists, components_list);
    // in_world->next_system_id++;
}

void simplecs_new_component(struct Simplecs_World * in_world, simplecs_entity_t in_entity, simplecs_components_t typeflag, simplecs_components_t type_toadd) {
    printf("simplecs_new_component\n");
    bool found = 0;
    for (size_t i = 0; i < in_world->num_componentsbytype[typeflag]; i++) {
        if (in_world->component_flagbytype[typeflag][i] == type_toadd) {
            found = true;
            break;
        }
    }
    if (!found) {
        struct Components_Array temp;
        temp.components = NULL;
        temp.type = type_toadd;
        arrput(in_world->components_bytype[typeflag][in_entity], temp);
        arrput(in_world->component_flagbytype[typeflag], type_toadd);
    } else {
        printf("simplecs_componentsbytype_add: component already in component_flagbytype");
    }

}

void simplecs_entity_typeflag_change(struct Simplecs_World * in_world, simplecs_entity_t in_entity, simplecs_components_t new_type) {
    printf("simplecs_entity_typeflag_change\n");
    simplecs_components_t previous_flag = in_world->entity_typeflags[in_entity];
    in_world->entity_typeflags[in_entity] = in_world->entity_typeflags[in_entity] | new_type;

    for (size_t i = 0; i < in_world->num_typeflags; i++) {
        if (previous_flag == in_world->entity_typeflags[i]) { //      EXCLUSIVE
            for (size_t j = 0; j < in_world->num_entitiesbytype[i]; j++) {
                if (in_entity == in_world->entitiesbytype[i][j]) {
                    arrdel(in_world->entitiesbytype[i], j);
                    break;
                }
            }
        }
        if (in_world->entity_typeflags[in_entity] == in_world->entity_typeflags[i]) { //      EXCLUSIVE
            arrput(in_world->entitiesbytype[i], in_entity);
        }
        // if (previous_flag & in_world->system_typeflags[i] > 0) { //   INCLUSIVE
        // if (previous_flag & in_world->system_typeflags[i] > 0) { //   INCLUSIVE
    }
}

bool simplecs_componentsbytype_migrate(struct Simplecs_World * in_world, simplecs_entity_t in_entity, simplecs_components_t old_flag, simplecs_components_t new_flag) {
    printf("simplecs_componentsbytype_migrate \n");

    // Migrates components associated with in_entity
    // -components_bytype: previous_flag -> new_flag
    // DOES NOT CHECK in_entity's TYPE.
    size_t new_type_id = simplecs_type_id(in_world->system_typeflags, in_world->num_typeflags, new_flag);
    size_t old_type_id = simplecs_type_id(in_world->system_typeflags, in_world->num_typeflags, old_flag);

    simplecs_entity_t * new_type_entities = in_world->entitiesbytype[new_type_id];
    simplecs_entity_t * old_type_entities = in_world->entitiesbytype[old_type_id];

    size_t new_num_entities = in_world->num_entitiesbytype[new_type_id];
    size_t old_num_entities = in_world->num_entitiesbytype[old_type_id];
    size_t new_component_num = in_world->num_componentsbytype[new_type_id];
    size_t old_component_num = in_world->num_componentsbytype[old_type_id];

    struct Components_Array ** new_type_components_byentity = in_world->components_bytype[new_type_id];
    struct Components_Array ** old_type_components_byentity = in_world->components_bytype[old_type_id];

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
        in_world->entitiesbytype[new_type_id]++;
    } else {
        printf("simplecs_componentsbytype_migrate: entity found in components_bytype for new_flag");
    }

    if (found_old && !found_new) {
        arrput(new_type_components_byentity, old_type_components_byentity[found_old]);
        arrdel(old_type_components_byentity, found_old);
    } else {
        printf("simplecs_componentsbytype_migrate: entity found in components_bytype for new_flag");
    }

}

size_t simplecs_type_id(simplecs_components_t * in_typelist, size_t len, simplecs_components_t in_flag) {
    size_t found = SIMPLECS_NULL;
    for (size_t i = 0; i < len; i++) {
        if (in_typelist[i] == in_flag) {
            found = i;
            break;
        }
    }
    return (found);
}


size_t simplecs_issubtype(simplecs_components_t * in_typelist, size_t len, simplecs_components_t in_flag) {
    // returns position of subtype from in_typelist
    size_t found = 0;
    for (size_t i = 0; i < len; i++) {
        if ((in_typelist[i] & in_flag) == in_flag) {
            found = i;
            break;
        }
    }
    return (found);
}


## Error Handling
Upon error, functions/macros return 0 or ```NULL```.

## Worlds Creation/Destruction
The world contains everything tnecs needs.
```c
    tnecs_world *world = NULL;
    tnecs_W_genesis(&world);
    ...
    tnecs_W_destroy(&world);
   ```

## Registering Components
A component is a user-defined struct:
```c
typedef struct Position {
    int x;
    int y;
} Position;

// Register Position without init, free function
TNECS_REGISTER_COMPONENT(world, Position, NULL, NULL);
// Keeping track of component IDs is user reponsibility
int Position_ID = 1;

void Position_Init(void *voidpos) {
    struct Position *pos = voidpos;
    // 1. Set variables to non-zero, non-NULL values
    // 2. Alloc member variables
}

void Position_Free(void *voidpos) {
    struct Position *pos = voidpos;
    // Free member variables
}

// All Position Cs initialized on creation,
// freed on destruction
TNECS_REGISTER_COMPONENT(world, Position, Position_Init, Position_Free);

```
The Cs IDs start 1, and increase monotonically, up to a cap of 63.
Tip: Use X macros to create lists of component IDs.

You can get the component type with the macro:
```c
    Position_type   == TNECS_COMPONENT_TYPE2ID(Position_id);
    Position_ID     == TNECS_COMPONENT_ID2TYPE(Position_type);
```
Note: A type only has one set bit. An archetype has multiple set bits, by adding types, OR'ing multiple As.

## Creating/Destroying Entities
```c
    tnecs_entity_t Silou = tnecs_entity_create(world);
    ...
    tnecs_entity_destroy(world, Silou);
```
Entities can be created with any number of Cs directly with this variadic macro: 
```c
    tnecs_entity_t Perignon = TNECS_ENTITY_CREATE_wCOMPONENTS(world, Position_ID, Unit_ID);
```

## Getting Components
```c
    struct Position *pos = tnecs_get_component(world, Silou, Position_ID);
    pos->x = 1;
    pos->y = 2;
```
By default, all component bits are set to zero with ```calloc``` when no init function is provided.

## Adding/Removing Components
```c 
    TNECS_ADD_COMPONENT(world, Silou, Position);
    // TNECS_ADD_COMPONENT is an overloaded macro
    bool isNew = false;
    TNECS_ADD_COMPONENT(world, Silou, Position, isNew);
```
By default, tnecs checks if the entity archetype is new, when the new component is added.
If you know that the archetype isn't new, set isNew to false to skip comparing the entity's new archetype with all other recorded As.

Multiple Cs can also be added at once:
```c
    bool isNew = false;
    TNECS_ADD_COMPONENTS(world, Pirou, isNew, Position, Velocity);
```

## Register System to the world
A system is a user-defined function, with a ```tnecs_Ss_input``` pointer as input and no output:
```c
    void SystemMove(tnecs_Ss_input_t *input) {
        Position *p = TNECS_COMPONENT_ARRAY(input, Position);
        Velocity *v = TNECS_COMPONENT_ARRAY(input, Velocity);

        for (int i = 0; i < input->entity_num; i++) {
            p[i].x += v[i].vx * input->deltat;
            p[i].y += v[i].vy * input->deltat;
        }
    }
    // More about pipeline, phase in next section
    int pipeline        = 0;
    int phase           = 0;
    // Exclusive Ss run only for all Es that have exactly the system's archetype.
    // Otherwise, system is run for every compatible archetype.
    int exclusive       = 0;
    TNECS_REGISTER_S(world, SystemMove, pipeline, phase, exclusive, Position, Unit); 
```

## Updating the world
```c
// Time elapsed by stepping.
tnecs_time_ns_t frame_deltat = 1;
// User-defined data input into system
void *data = NULL; 

// Run all Pis, starting from pipeline 0.
tnecs_W_step(world, frame_deltat, data);

// Run a specific pipeline, starting from phase 0
int pipeline = 1;
tnecs_Pi_step(world, frame_deltat, data, pipeline);

// Run a specific phase, in a specific pipeline
// In each phase, Ss are run first-come first-served 
int phase = 1;
tnecs_Pi_step_Ph(world, frame_deltat, data, pipeline, phase);

```

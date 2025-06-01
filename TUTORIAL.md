
## Error Handling
Upon error, functions/macros return 0 or ```NULL```.

## Worlds Creation/Destruction
The world contains everything tnecs needs.
```c
    struct tnecs_world *world = NULL;
    tnecs_world_genesis(&world);
    ...
    tnecs_world_destroy(&world);
   ```
## Registering Components
A component is a user-defined struct:
```c
typedef struct Position {
    uint32_t x;
    uint32_t y;
} Position;

// Register Position without init, free function
TNECS_REGISTER_COMPONENT(world, Position, NULL, NULL);

void Position_Init(void *voidpos) {
    struct Position *pos = voidpos;
    // 1. Set variables to non-zero, non-NULL values
    // 2. Alloc member variables
}

void Position_Free(void *voidpos) {
    struct Position *pos = voidpos;
    // Free member variables
}

// All Position components initialized on creation,
// freed on destruction
TNECS_REGISTER_COMPONENT(world, Position, Position_Init, Position_Free);

```
The components IDs start 1, and increase monotonically, up to a cap of 63.
Tip: Use X macros to create lists of component IDs.

You can get the component type with the macro:
```c
    Position_ID     == TNECS_COMPONENT_ID2TYPE(Position_type);
    Position_type   == TNECS_COMPONENT_TYPE2ID(Position_id);
```

## Creating/Destroying Entities
```c
    tnecs_entity_t Silou = tnecs_entity_create(world);
    ...
    tnecs_entity_destroy(world, Silou);
```
Entities can be created with any number of components directly with this variadic macro: 
```c
    tnecs_entity_t Perignon = TNECS_ENTITY_CREATE_wCOMPONENTS(world, Position_ID, Unit_ID);
```
Or you can write directly:
```c
    tnecs_entity_t Perignon = tnecs_entity_create_wcomponents(world, 2, Position_ID, Unit_ID);
```

## Getting Components
```c
    int Position_ID = 1;
    struct Position *pos = tnecs_get_component(world, Silou, Position_id);
    pos->x += 1;
    pos->y += 2;
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

Multiple components can also be added at once:
```c
    bool isNew = false;
    TNECS_ADD_COMPONENTS(world, Pirou, isNew, Position, Velocity);
```

## Register System to the world
A system is a user-defined function, with a ```struct *tnecs_system_input``` pointer as input and no output:
```c
    void SystemMove(tnecs_system_input_t * in_input) {
        Position *p = TNECS_COMPONENT_ARRAY(in_input, Position);
        Velocity *v = TNECS_COMPONENT_ARRAY(in_input, Velocity);

        for (int i = 0; i < in_input->entity_num; i++) {
            p[i].x += v[i].vx * in_input->deltat;
            p[i].y += v[i].vy * in_input->deltat;
        }
    }
    int pipeline        = 0;
    int phase           = 0;
    int exclusive       = 0;
    TNECS_REGISTER_SYSTEM(world, SystemMove, pipeline, phase, exclusive, Position, Unit); 
```
System index 0 is reserved for NULL. 

Each pipeline can be run with ```tnecs_pipeline_step```.
Systems are run in phases within each pipeline.
For each phase, the systems are run first come first served.

Phases are greater than zero ```ull``` integers.
Default phase is 0, the NULL phase, which always runs first. 

Inclusive systems, run for entities that have all required components, and more.
Inclusive systems are run once for every compatible archetype to the system archetype, in the order saved in ```systems_torun``` in the ```world``` after each step.
If the system is set to exclusive, it runs only one time for the entities that only have exactly the system's components.

## Updating the world
```c
tnecs_time_ns_t frame_deltat;
tnecs_world_step(world, frame_deltat, NULL);
```
This runs each system in every pipeline, in phases.
Pipelines and phases are always ran in order e.g. pipeline 0 first, pipeline 1 next...
To run only systems registered to a specific pipeline, use ```tnecs_pipeline_step```.
For a specific phase inside a pipeline, use ```tnecs_pipeline_step_phase```.

The frame time is the ```deltat``` member in ```tnecs_system_input_t```, accessible from inside registered systems.


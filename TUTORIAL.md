
## Error Handling
Upon error, functions/macros return 0 or ```NULL```.

## Initializing the world
```c
    struct tnecs_world *world = NULL;
    tnecs_world_genesis(&world);
   ```
The world contains everything tnecs needs.
Use ```tnecs_world_destroy``` to free all memory.

## Entity creation/destruction
```c
    tnecs_entity_t Silou = tnecs_entity_create(world);
```
```tnecs_entity_t``` is a ```ull``` index. 

Entity index 0 is reserved for ```TNECS_NULL```.
Entities can be destroyed with ```tnecs_entity_destroy```, which frees all associated components.

## Register Component to the world
A component is a user-defined struct:
```c
    typedef struct Position {
        uint32_t x;
        uint32_t y;
    } Position;

    TNECS_REGISTER_COMPONENT(world, Position);
```
The components IDs start at 1, and increment for every new component.
Use X macros to create compile-time component IDs. 
```tnecs_component_t``` is an ```ull``` integer, used as a bitflag: each component type only has one bit set, at ```component_id``` location. 
Component index 0 is reserved for the NULL component, so a maximal number of 63 components can be registered.

The component's type can be obtained with:
```c
    tnecs_component_t Position_flag = TNECS_COMPONENT_TYPE(world, Position); 
```
NOTE: A component type has one bit set, an archetype can have any number of bits set.

The relation between component indices and types is:
```c
    Position_type   == (1 << (Position_id - 1));
    Position_id     == ((tnecs_component_t)(log2(Position_type) + 1.1f));  // casting to int truncates to 0
```
which are accessible through the macros:
```c
    Position_id     == TNECS_COMPONENT_ID2TYPE(Position_type);
    Position_type   == TNECS_COMPONENT_TYPE2ID(Position_id);
```

## Add Components to Entities
```c 
    TNECS_ADD_COMPONENT(world, Silou, Position);
```
By default, tnecs checks if the entity archetype is new, when the new component is added.
```TNECS_ADD_COMPONENT``` is an overloaded macro, so you can specify if the archetype is not new and skip a check for better performance:

```c
    bool isNew = false;
    TNECS_ADD_COMPONENT(world, Silou, Position, isNew);
```
Multiple components can also be added at once:
```c
    bool isNew = false;
    TNECS_ADD_COMPONENTS(world, Pirou, isNew, Position, Velocity);
```
```TNECS_ADD_COMPONENTS``` is NOT an overloaded macro.

You can get a pointer to the component:
```c
    int Position_id = 1;
    struct Position * pos_Silou = tnecs_get_component(world, Silou, Position_id);
    pos_Silou->x += 1;
    pos_Silou->y += 2;
```
By default, all component bits are set to zero with ```calloc```.

Entities can be created with any number of components directly with this variadic macro: 
```c
    tnecs_entity_t Perignon = TNECS_ENTITY_CREATE_wCOMPONENTS(world, Position, Unit);
```
```TNECS_ENTITY_CREATE_wCOMPONENTS``` wraps around the variadic function ```tnecs_new_entity_wcomponents``` by counting the number of input components. So you can also write, if you wish:

```c
    tnecs_entity_t Perignon = tnecs_entity_create_wcomponents(world, 2, Position_ID, Unit_ID);
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


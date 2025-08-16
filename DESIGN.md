
# Design

## Intro
- I use the ```tcc``` compiler sometimes, so compiler-specific instructions are out of the question.
- Everything inside the world is a dynamic array that grows exponentially as required.
- Index 0 is always reserved for ```TNECS_NULL```.
- Indices of created entities, registered components, systems, phases, pipelines always start at 1 and increase monotonically.
- Tip: Use X macros to have the indices at compile time.

## Component types
Component ID, archetypes use a ```ull``` as a bitflag.
- ```component_type =  1 << (component_id - 1)```.
The first registered component's type has only its first bit set  ```0b0001```, the next one its second bit set ```0b0010```, etc.
The number of components in a world is limited to 63, with 0 reserved for ```TNECS_NULL```.
Each entity, system has its own archetype.
Simple binary operators are used to check for type match.

## Components
Components are arranged in ```byT``` arrays of pointers.
Each new archetype gets an index, with associated entities and components array pointer at the index.
Then, each component has an order inside the ```byT``` array, on a first come first served basis.
The ```byT``` arrays are exclusive, components are NOT copied for each compatible archetype.
archetypes such, inclusive systems are called once for each compatible archetype.

Every time a new component is added to an entity, its type changes, so each associated components and its index in the ```byT``` arrays is copied over, then deleted from the previous location.
This is somewhat costly, so create entities with all of its components directly with the ```TNECS_ENTITY_CREATE_wCOMPONENTS``` macro.

## Systems
Systems are ran in phases for each independent pipeline, so they are arranged in ```byPh``` arrays of pointers.
Pipelines, phases are always ran in order, starting from the default 0.
System order in each phase is first-come first-served.

## Pipelines
Pipelines let you run systems at different times, e.g. rendering and game logic.
Phases in each pipeline are independent.

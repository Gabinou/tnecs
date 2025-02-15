
# Ｏn tnecs' Design

## Intro
- I use the ```tcc``` compiler sometimes, so compiler-specific instructions are out of the question.
- Everything inside the world is a dynamic array that grows exponentially as required.
- The indices of registered components orsystems, created entities, always start at 1 and increase monotonically.
- Index 0 is always reserved for NULL.
- Use X macros to have the indices at compile time.

## Component types
Component types, archetypes use a ```ull``` as a bitflag.
- ```component_type =  1 << (component_id - 1)```.
The first registered component has only its first bit set  ```0b0001```, the next one its second bit set ```0b0010```, etc.
The number of components is limited to 63, with 0 reserved for the NULL.
Each entity, system has its own typeflag.
Simple binary operators are used to check for type match.

## Components
Components are arranged in "bytype" arrays of pointers.
Each new typeflag gets an index, with associated entities and components array pointer at the index.
Then, each component has an order inside the "bytype" array, on a first come first served basis.
The "bytype" arrays are exclusive, components are NOT copied for each compatible archetype.
As such, inclusive systems are called once for each compatible archetype.

Every time a new component is added to an entity, its type changes, so each associated components and its index in the "bytype" arrays is copied over, then deleted from the previous location.
This is somewhat costly, so create entities with all of its components directly with the ```TNECS_ENTITY_CREATE_wCOMPONENTS``` macro.

## Systems
Systems are ran in phases, so they are arranged in "byphase" arrays of pointers.
By default the NULL phase 0 is ran first.
System order in these phases is first come first served by default, and can be set by the user with ```tnecs_system_order_switch```.

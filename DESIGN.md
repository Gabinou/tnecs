
## Ｏn tnecs' Design

My motivation for tnecs is to make the _simplest possible_ C99 ECS library only with the _minimum necessary features_.
I also use the ```tcc``` compiler sometimes, so compiler-specific instructions are out of the question.

In tnecs:
- an entity is an ```uint64_t``` index, 
- a component is user-defined ```struct```, 
- a system is a user-defined ```function```,
- everything lives inside the world.

For perfomance reasons, everything inside the world is an array.
Whenever entities are created, components or systems are registered, an associated index is created.
The indices always start at 1 and increase monotonically.
Index 0 is always reserved for NULL.
I encourage the use of X macros to have access to the indices at compile time.

The most notable design feature is the use of a ```uint64_t``` as a bitflag to denote the component type.
For example, the first registered component has only its first bit set  ```0b0001```, the next one its second bit set ```0b0010```, etc.
The component id is simply this set bit address, so ```component_type =  1 << (component_id - 1)```.

This greatly simplifies the notion of a typeflag/archetype: it is just an ```uint64_t``` with any number of set bits for each component.
Each entity is associated with a single ```uint64_t``` typeflag, ibid. for systems.
Simple binary operators are used to check if a type is included in a typeflag, if a typeflag is a superset of another, etc.
This effectively limits the number of components to 63, with 0 reserved for the NULL type, but this is more than enough in practice.

Entities and components are arranged in "bytype" arrays of pointers.
Each new typeflag gets an index, with associated entities and components array pointer at the index.
Then, each component/entity has an order inside the "bytype" array, determined on a first come first served basis.
The "bytype" arrays are exclusive, meaning entities and components are NOT copied for each compatible archetype.
This saves some memory, but leads to the inclusive systems being called once for each archetype.
It is unclear to me if the alternative is more performant.

Anyhow, every time a new component is added to an entity, its type changes, so each associated components and its index in the "bytype" arrays is copied over, then deleted from the previous location.
This is somewhat of a costly operation, so I suggest you create entities with all of its components directly with the ```TNECS_ENTITY_CREATE_wCOMPONENTS``` macro.

Systems are ran in phases, so they are arranged in "byphase" arrays of pointers.
By default the NULL phase 0 is ran first.
System order in these phases is first come first served by default, and can be set by the user with ```tnecs_system_order_switch```.

### Miscellaneous
Memory is allocated with exponentially growing arrays.

Macro tricks are used extensively: overloaded macros, macro distribution for variadic macros, input arguments counting for variadic macros...


#include "simplecs.h"

// Simplecs (pronounced simplex) is a very simple implementation of an entity-component-system (ECS)
// ECS is very useful in game programming. 
// OOP: objects and methods, children inheriting from parents, etc.
// ECS: Entities can have any number of independent components, acted upon by systems
// Example: Videogame 
//      -> main character: Physics component, PlayerControlled Component
//      -> enemies: Physics component, AIControlled Component
//      -> Environment tiles: Destroyable Component
// 
// Entities are indices (uint64_t)
// Component are structures
// Systems are functions
// The main loop iterates over systems

simplecs_entity_t simplecs_new_entity(world) {
	simplecs_entity_t out = 0;
	while ((out == 0) && (num_open_entity_ids > 0)) {
		out = open_entity_ids[--num_open_entity_ids];
		open_entity_ids[num_open_entity_ids] = 0;
	}
	if (out == 0) {
		out = next_entity_id++;
	} 
}

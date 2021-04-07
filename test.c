#include "stb_ds.h"
#define STB_DS_IMPLEMENTATION
#include "simplecs.h"
#include <assert.h>

typedef struct Position {
	uint32_t x;
	uint32_t y;
}Position;

typedef struct Unit {
	uint32_t hp;
	uint32_t str;
}Unit;

int main () {
	printf("Hello, World! I am testing Simplecs \n\n");

	printf("simplecs_init tests \n");
	struct Simplecs_World * test_world = simplecs_init();
	printf("component_tables[SIMPLECS_NULLENTITY]: %d\n", component_tables[SIMPLECS_NULLENTITY]);
	printf("\n");

	printf("Component registration tests\n");
	printf("Registering Position Component \n");
	SIMPLECS_REGISTER_COMPONENT(Position);
	assert(Component_Position_id == COMPONENT_ID_START);
	printf("Registering Position Unit \n");
	SIMPLECS_REGISTER_COMPONENT(Unit);
	assert(Component_Unit_id == (COMPONENT_ID_START + 1));
	printf("\n");


	printf("Entity Creation/Destruction tests\n");
	assert(next_entity_id == ENTITY_ID_START);
	printf("Making Silou Entity \n");
	simplecs_entity_t Silou = simplecs_new_entity(test_world);
	assert(next_entity_id == (ENTITY_ID_START + 1));
	printf("Making Pirou Entity \n");
	simplecs_entity_t Pirou = simplecs_new_entity(test_world);
	assert(next_entity_id == (ENTITY_ID_START + 2));
	printf("\n");

	simplecs_entity_t * components_list;
	SIMPLECS_ADD_COMPONENT(test_world, Position, Silou);
	components_list = hmget(test_world, Silou);
	assert(arrlen(components_list) == 1);
	assert(arrlen(components_list) == Component_Position_id);
	// SIMPLECS_ADD_COMPONENT(test_world, Unit, Silou);
	// SIMPLECS_ADD_COMPONENT(test_world, Position, Pirou);
	// SIMPLECS_ADD_COMPONENT(test_world, Unit, Pirou);
    getchar();
	return(0);
}

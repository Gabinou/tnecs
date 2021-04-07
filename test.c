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
	printf("Hello, World! I am testing Simplecs. \n\n");
	simplecs_entity_t * components_list;
	struct Position * temp_position; 
	struct Unit * temp_unit; 

	printf("simplecs_init tests \n");
	struct Simplecs_World * test_world = simplecs_init();
	assert(component_tables[SIMPLECS_NULLENTITY] == NULL);
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
	assert(Silou == ENTITY_ID_START);
	assert(next_entity_id == (ENTITY_ID_START + 1));
	printf("Making Pirou Entity \n");
	simplecs_entity_t Pirou = simplecs_new_entity(test_world);
	assert(Pirou == (ENTITY_ID_START + 1));
	assert(next_entity_id == (ENTITY_ID_START + 2));
	printf("\n");

	printf("Adding Components to Entities\n");
	SIMPLECS_ADD_COMPONENT(test_world, Position, Silou);
	components_list = hmget(test_world, Silou);
	assert(arrlen(components_list) == 1);
	assert(components_list[0] == Component_Position_id);
	SIMPLECS_ADD_COMPONENT(test_world, Unit, Silou);
	components_list = hmget(test_world, Silou);
	assert(arrlen(components_list) == 2);
	assert(components_list[0] == Component_Position_id);
	assert(components_list[1] == Component_Unit_id);

	SIMPLECS_ADD_COMPONENT(test_world, Position, Pirou);
	components_list = hmget(test_world, Pirou);
	assert(arrlen(components_list) == 1);
	assert(components_list[0] == Component_Position_id);
	SIMPLECS_ADD_COMPONENT(test_world, Unit, Pirou);
	components_list = hmget(test_world, Pirou);
	assert(arrlen(components_list) == 2);
	assert(components_list[0] == Component_Position_id);
	assert(components_list[1] == Component_Unit_id);

	printf("\n");
	printf("Getting Components from Entities\n");
	// SIMPLECS_GET_COMPONENT(Position, Silou);
	temp_position = SIMPLECS_GET_COMPONENT(Position, Silou);
	assert(temp_position->x == 0);
	assert(temp_position->y == 0);

	// SIMPLECS_ADD_COMPONENT(test_world, Position, Pirou);
	// SIMPLECS_ADD_COMPONENT(test_world, Unit, Pirou);
	printf("Simplecs Test End");
	return(0);
}

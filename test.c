#include "stb_ds.h"
#define STB_DS_IMPLEMENTATION
#include "simplecs.h"


typedef struct Position {
	uint32_t x;
	uint32_t y;
}Position;

typedef struct Unit {
	uint32_t hp;
	uint32_t str;
}Unit;

int main () {
	printf("Hello, World! I am testing Simplecs \n");
	struct Simplecs_World * test_world = simplecs_init();

	SIMPLECS_REGISTER_COMPONENT(Position);
	printf("%d\n", Component_Position_id);
	SIMPLECS_REGISTER_COMPONENT(Unit);
	printf("%d\n", Component_Unit_id);


	printf("Making Silou Entity \n");
	simplecs_entity_t Silou = simplecs_new_entity(test_world);
	printf("Making Pirou Entity \n");
	simplecs_entity_t Pirou = simplecs_new_entity(test_world);
	
	// SIMPLECS_ADD_COMPONENT(test_world, Position, Silou);
	// SIMPLECS_ADD_COMPONENT(test_world, Unit, Silou);
	// SIMPLECS_ADD_COMPONENT(test_world, Position, Pirou);
	// SIMPLECS_ADD_COMPONENT(test_world, Unit, Pirou);
    getchar();
	return(0);
}

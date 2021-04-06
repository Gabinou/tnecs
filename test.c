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
	struct Simplecs_World * test_wordd = simplecs_init();

	SIMPLECS_REGISTER_COMPONENT(Position);
	printf("%d\n", Component_Position_id);
	SIMPLECS_REGISTER_COMPONENT(Unit);
	printf("%d\n", Component_Unit_id);
    getchar();
	return(0);
}

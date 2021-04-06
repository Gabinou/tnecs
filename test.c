#include <simplecs.h>


typedef struct Position {
	uint32_t x;
	uint32_t y;
}Position;

int main () {
	printf("Hello, World! I am testing Simplecs \n");
	SIMPLECS_REGISTER_COMPONENT(Position);
    getchar();
	return(0);
}

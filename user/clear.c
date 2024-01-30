#include <lib.h>

void clear() {
	printf("\x1b[2J\x1b[H");
}

void usage(void) {
	printf("usage: clear\n");
	exit();
}

void main(int argc, char **argv) {
	clear();
}
#include <lib.h>

void usage(void) {
	printf("usage: pwd\n");
	exit();
}
int main(int argc, char **argv) {

	if (argc > 1) {
		usage();
	} else {
		char buf[128];
		curpath_get(buf);
        printf(GREEN(%s) "\n",buf);
	}
	printf("\n");
	return 0;
}

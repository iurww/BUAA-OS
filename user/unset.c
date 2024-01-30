#include <lib.h>

int flag[256];

void unset(char *name) {
	int r;
	if ((r = syscall_env_var(name, "", 4)) < 0) {
		if(r == -E_ENV_VAR_NOT_FOUND) {
			printf("Environment var " RED([%s]) " Not Exists!\n", name);
		} else if (r == -E_ENV_VAR_READONLY) {
			printf("unset: " RED([%s]) " is readonly\n", name);
		}
		
		return;
	}
}

void usage(void) {
	printf("usage: unset [vars...]\n");
	exit();
}

void main(int argc, char **argv) {
	int i;
	if (argc == 0) {
		return;
	} else {
		for (i = 1; i < argc; i++)
			unset(argv[i]);
	}
}

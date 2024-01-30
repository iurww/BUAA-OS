#include <lib.h>

int main(int argc, char **argv) {
	int i, nflag;

	nflag = 0;
	if (argc > 1 && strcmp(argv[1], "-n") == 0) {
		nflag = 1;
		argc--;
		argv++;
	}
	for (i = 1; i < argc; i++) {
		if (i > 1) {
			printf(" ");
		}
		if(argv[i][0] == '"') {
			printf("%s", argv[i] + 1);
			continue;
		}
		if (argv[i][0] == '$') {
			char value[256] = {0};
			if (syscall_env_var(argv[i] + 1, value, 1) < 0) {
				printf("Environment var " RED([%s]) " Not Exists!\n", argv[i] + 1);
				return 0;
			}

			printf("%s", value);
		} else {
			printf("%s", argv[i]);
		}
	}
	if (!nflag) {
		printf("\n");
	}
	return 0;
}

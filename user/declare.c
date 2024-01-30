#include <lib.h>

int flag[256];

void usage(void) {
	printf("usage: declare [-xr] [NAME [=VALUE]]\n");
	exit();
}

void set(char *name, char *value, int type) {
	int r;
	if ((r = syscall_env_var(name, value, 2)) < 0) {
		if (r == -14)
			syscall_env_var(name, value, 0 | type);
		else if (r == -15)
			printf("set: " RED([%s]) " is readonly\n", name);
		return;
	}
}

void set_readonly(char *name, char *value, int type) {
	int r;
	if ((r = syscall_env_var(name, value, 2)) < 0) {
		if (r == -14)
			syscall_env_var(name, value, 16 | type);
		else if (r == -15)
			printf("set: " RED([%s]) " is readonly\n", name);
		return;
	}
}

char name_table[256 * 64];
char value_table[256 * 256];
void list() {
	// printf("%x %x\n", name_table, value_table);
	memset(name_table, 0, 256 * 64);
	memset(value_table, 0, 256 * 256);
	syscall_env_var(name_table, value_table, 8);
	int i = 0;
	while (name_table[i * 64]) {
		printf("%d : %s = %s\n", i, name_table + i * 64, value_table + i * 256);
		++i;
	}
}

int main(int argc, char **argv) {
	int i;

	ARGBEGIN {
	default:
		usage();
		return;
	case 'x':
	case 'r':
		flag[(u_char)ARGC()]++;
		break;
	}
	ARGEND

	// printf("argc %d : \n", argc);
	// for (int i = 0; i < argc; i++) {
	// 	printf("argv %d : %s\n", i, argv[i]);
	// }

	if (argc > 1) {
		usage();
		return;
	}

	int ronly = flag['r'];
	int pub = flag['x'];

	if (argc == 0) {
		if (ronly || pub) {
			usage();
			return;
		} else {
			list();
		}
	} else if (argc == 1) {
		char name[64] = {0};
		char value[256] = {0};
		char *p;
		if ((p = strchr(argv[0], '=')) != 0) {
			argv[0][p - argv[0]] = 0;
			strcpy(name, argv[0]);
			strcpy(value, p + 1);
			printf("name : %s\nvalue : %s\n", name, value);
		} else {
			strcpy(name, argv[0]);
			printf("name : %s\n", name);
		}
		if (pub) {
			if (!ronly) {
				set(name, value, 0);
			} else {
				set_readonly(name, value, 0);
			}
		} else {
			if (!ronly) {
				set(name, value, 0x0100);
			} else {
				set_readonly(name, value, 0x0100);
			}
		}
	}

	return 0;
}

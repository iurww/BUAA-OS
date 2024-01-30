#include "lib.h"

void usage(void) {
	printf("usage: cd [dir]\n");
	exit();
}

void main(int argc, char **argv) {
	int i, r;
	if (argc == 1) {
		printf("cd: too few args\n");
		return;
	}

	if (strcmp(argv[1], ".") == 0)
		return;

	int path[256];
	if (strcmp(argv[1], "..") == 0) {
		curpath_get_parent(path);
		curpath_set(path);
		printf("cd: %s\n", path);
		return;
	} else {
		if ((r = curpath_get(path)) < 0) {
			printf("cd: cannot get environment var [curpath]\n");
			return;
		}
		char temp[256] = {0};
		curpath_get_absolute(temp, argv[1]);

		struct Stat st;
		r = stat(temp, &st);

		if (r == -E_NOT_FOUND)
			printf("cd: " RED([%s]) " not found\n", temp);
		else if (r < 0)
			printf("cd: cannot cd " RED([%s]) "\n", temp);
		else if (!st.st_isdir)
			printf("cd: %s is not directory\n", temp);
		else {
			if ((r = curpath_set(temp)) < 0)
				printf("Environment var " RED([curpath]) "not found\n");
			printf("curpath: %s\n", temp);
		}
		return;
	}
}

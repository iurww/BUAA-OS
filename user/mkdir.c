#include <lib.h>

int flag[256];

void mkdir(char *path, char *prefix) {
	int r, fd;
	struct Stat st;
	char curpath[MAXPATHLEN] = {'\0'};

	if ((r = curpath_get(curpath)) < 0) {
		printf("mkdir: cannot get environment var [curpath]\n");
	}
	curpath_get_absolute(curpath, path);

	if ((r = create(curpath, FTYPE_DIR)) < 0) {
		printf("Directory %s Already Exists!\n", curpath);
		return;
	}
	printf("Created Directory %s!\n", curpath);
}

void usage(void) {
	printf("usage: mkdir [-dFl] [file...]\n");
	exit();
}

void main(int argc, char **argv) {
	int i;
	ARGBEGIN {
	default:
		usage();
	case 'd':
	case 'F':
	case 'l':
		flag[(u_char)ARGC()]++;
		break;
	}
	ARGEND

	if (argc == 0) {
		return;
	} else {
		for (i = 0; i < argc; i++)
			mkdir(argv[i], argv[i]);
	}
}

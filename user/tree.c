#include <lib.h>

int flag[256];

void tree(char *path, int depth, int finalFile[]) {
	struct Fd *fd;
	struct Filefd *fileFd;
	int r = open(path, O_RDONLY);
	if (r < 0) {
		printf("failed to open : %s\n", path);
		return;
	}

	int i;
	for (i = 0; i < depth; i++) {
		if (i == depth - 1) {
			if (finalFile[i]) {
				printf("`--");
			} else {
				printf("|--");
			}
		} else {
			if (finalFile[i]) {
				printf("    ");
			} else {
				printf("|   ");
			}
		}
	}

	fd = (struct Fd *)num2fd(r);
	fileFd = (struct FileFd *)fd;
	if (fileFd->f_file.f_type == 1) {
		printf("\033[34m%s\033[m\n", fileFd->f_file.f_name);
	} else {
		char *name = fileFd->f_file.f_name;
		int l = strlen(name);
		if (name[l - 1] == 'b' && name[l - 2] == '.')
			printf(GREEN(%s)"\n", name);
		else 
			printf("%s\n", name);
	}
	if (fileFd->f_file.f_type == FTYPE_REG) {
		close(r);
		return;
	}

	u_int size = fileFd->f_file.f_size;
	u_int num = ROUND(size, sizeof(struct File)) / sizeof(struct File);
	struct File *file = (struct File *)fd2data(fd);

	u_int real_num = 0;
	for (i = 0; i < num; i++) {
		if (file[i].f_name[0] != '\0') {
			real_num++;
		}
	}

	u_int j = 0;
	for (i = 0; i < num; i++) {
		if (file[i].f_name[0] == '\0') {
			continue;
		}
		char newpath[MAXPATHLEN] = {'\0'};
		strcpy(newpath, path);
		int len = strlen(newpath);
		if (newpath[len - 1] != '/') {
			newpath[len] = '/';
			len++;
		}
		strcpy(newpath + len, file[i].f_name);
		if (j == real_num - 1) {
			finalFile[depth] = 1;
		}
		tree(newpath, depth + 1, finalFile);
		j++;
	}
	finalFile[depth] = 0;
	close(r);
}

void usage(void) {
	printf("usage: tree [path/to/dir...] \n");
	exit();
}

int main(int argc, char **argv) {

	ARGBEGIN {
	default:
		usage();
	}
	ARGEND

	// printf("tree argc : %d\n", argc);
	// for (int i = 0; i < argc; i++) {
	// 	printf("argv %d : %s\n", i, argv[i]);
	// }

	int finalFile[50] = {0};
	char curpath[128] = {0};
	if (argc == 0) {
		curpath_get(curpath);
		tree(curpath, 0, finalFile);
	} else {
		for(int i=0;i<argc;i++) {
			memset(curpath, 0 , 128);
			curpath_get_absolute(curpath, argv[i]);
			tree(curpath, 0, finalFile);
		}
		
	}
	close_all();
	printf("\n");
	return 0;
}

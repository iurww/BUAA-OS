#include <lib.h>

const char *CURPATH_KEY = "curpath";

void curpath_init(char *path) {
	int r;
	if ((r = syscall_env_var(CURPATH_KEY, path, 0)) < 0)
		user_panic("Init curpath failed: %d.", r);
}

int curpath_get(char *path) {
	int r;
	if ((r = syscall_env_var(CURPATH_KEY, path, 1)) < 0)
		return r;
}

int curpath_set(char *path) {
	int r;
	if ((r = syscall_env_var(CURPATH_KEY, path, 2)) < 0)
		return r;
}

int curpath_get_parent(char *path) {
	int r, i;
	if ((r = curpath_get(path)) < 0)
		return r;
	if (strlen(path) == 1)
		return 0;

	for (i = strlen(path) - 2; path[i - 1] != '/'; i--)
		;
	path[i] = 0;
}

int curpath_extend(char *path) {
}

void curpath_get_absolute(char *temp, char *path) {
	int pos = 0;
	int l = strlen(path);

    char curpath[128] = {0};
    curpath_get(curpath);

	
	if (path[0] != '/') {
		strcpy(temp, curpath);
		int ansl = strlen(temp);
		while (pos < l) {
			if (path[pos] == '.') {
				if (path[pos + 1] == '/' || path[pos + 1] == 0) {
					pos += 2;
					continue;
				} else if (path[pos + 1] == '.' && (path[pos + 2] == '/' || path[pos + 2] == 0)) {
					pos += 3;
                    if(ansl == 1) continue;
					if (temp[ansl - 1] == '/') {
						ansl--;
					}
					while (ansl > 0 && temp[ansl - 1] != '/')
						temp[ansl--] = 0;
					temp[ansl] = 0;
				} else {
                    goto cat;
                }
			} else {
                cat:
				while (path[pos] != '/' && pos < l) {
					temp[ansl] = path[pos];
					ansl++;
					pos++;
				}
				temp[ansl] = '/';
				pos++;
				ansl++;
			}
		}
	} else {
        strcat(temp, path);
        if(path[l-1] != '/') {
            strcat(temp, "/");
        }
    }
}
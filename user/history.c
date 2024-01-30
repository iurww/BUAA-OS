#include <lib.h>

char buf[1024] = {0};

void main(int argc, char **argv) {
	if (argc > 1) {
		printf("usage: history\n");
	} else {
		int f = open("/.history", O_RDONLY);
		struct Fd *fd = (struct Fd *)num2fd(f);

		long n;
        int r;

		while ((n = read(f, buf, (long)sizeof buf)) > 0) {
			if ((r = write(1, buf, n)) != n) {
				printf("write error copying : %d", r);
			}
		}
		if (n < 0) {
			printf("error reading : %d", n);
		}

		// int cnt = 0, p = 0;
		// for (int i = 0; buf[i]; i++) {
		// 	if (buf[i] == '\n') {
		// 		cnt++;
		// 		printf("command %d : ", cnt);
		// 		for (int j = p; j <= i; j++) {
		// 			printf("%c", buf[j]);
		// 		}
		// 		p = i + 1;
		// 	}
		// }
	}
}
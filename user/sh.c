#include <args.h>
#include <lib.h>

#define WHITESPACE " \t\r\n"
#define SYMBOLS "<|>&;()"

char history[8192];
int pos = 0;
int cur = 0, all = 0;
char curpath0[128] = {0};

/* Overview:
 *   Parse the next token from the string at s.
 *
 * Post-Condition:
 *   Set '*p1' to the beginning of the token and '*p2' to just past the token.
 *   Return:
 *     - 0 if the end of string is reached.
 *     - '<' for < (stdin redirection).
 *     - '>' for > (stdout redirection).
 *     - '|' for | (pipe).
 *     - 'w' for a word (command, argument, or file name).
 *
 *   The buffer is modified to turn the spaces after words into zero bytes ('\0'), so that the
 *   returned token is a null-terminated string.
 */
int _gettoken(char *s, char **p1, char **p2) {
	*p1 = 0;
	*p2 = 0;
	if (s == 0) {
		return 0;
	}

	while (strchr(WHITESPACE, *s)) {
		*s++ = 0;
	}
	if (*s == 0) {
		return 0;
	}

	if (strchr(SYMBOLS, *s)) {
		int t = *s;
		*p1 = s;
		*s++ = 0;
		*p2 = s;
		return t;
	}

	if (*s == '"') {
		// *s = 0;
		*p1 = s++;
		while (*s != 0 && *s != '"') {
			s++;
		}
		*s = 0;
		*p2 = s;
		
		return 'y';
	}

	*p1 = s;
	while (*s && !strchr(WHITESPACE SYMBOLS, *s)) {
		s++;
	}
	*p2 = s;
	return 'w';
}

int gettoken(char *s, char **p1) {
	static int c, nc;
	static char *np1, *np2;

	if (s) {
		nc = _gettoken(s, &np1, &np2);
		return 0;
	}
	c = nc;
	*p1 = np1;
	nc = _gettoken(np2, &np1, &np2);
	return c;
}

#define MAXARGS 128

int mark = 2;
int parsecmd(char **argv, int *rightpipe, int *pend) {
again:;
	int argc = 0;
	while (1) {
		char *t;
		int fd, r;
		int temp;
		int c = gettoken(0, &t);
		switch (c) {
		case 0:
			return argc;
		case 'w':
			if (argc >= MAXARGS) {
				debugf("too many arguments\n");
				exit();
			}
			argv[argc++] = t;
			break;
		case 'y':
			if (argc >= MAXARGS) {
				debugf("too  many arguments\n");
				exit();
			}
			argv[argc++] = t;
			break;
		case '<':
			if (gettoken(0, &t) != 'w') {
				debugf("syntax error: < not followed by word\n");
				exit();
			}
			// Open 't' for reading, dup it onto fd 0, and then close the original fd.
			/* Exercise 6.5: Your code here. (1/3) */
			r = open(t, O_RDONLY);
			if (r < 0) {
				user_panic("error happens at < when try to open 't' %d\n", r);
			}
			dup(r, 0);
			close(r);
			mark = 0;
			// user_panic("< redirection not implemented");

			break;
		case '>':
			if (gettoken(0, &t) != 'w') {
				debugf("syntax error: > not followed by word\n");
				exit();
			}
			// Open 't' for writing, dup it onto fd 1, and then close the original fd.
			/* Exercise 6.5: Your code here. (2/3) */
			char abpath[128] = {0};
			curpath_get_absolute(abpath, t);
			r = open(abpath, O_WRONLY | O_CREAT);
			if (r < 0) {
				user_panic("error happens at > when try to open 't' %d\n", r);
			}
			dup(r, 1);
			close(r);
			mark = 1;
			// user_panic("> redirection not implemented");

			break;
		case '|':;
			/*
			 * First, allocate a pipe.
			 * Then fork, set '*rightpipe' to the returned child envid or zero.
			 * The child runs the right side of the pipe:
			 * - dup the read end of the pipe onto 0
			 * - close the read end of the pipe
			 * - close the write end of the pipe
			 * - and 'return parsecmd(argv, rightpipe)' again, to parse the rest of the
			 *   command line.
			 * The parent runs the left side of the pipe:
			 * - dup the write end of the pipe onto 1
			 * - close the write end of the pipe
			 * - close the read end of the pipe
			 * - and 'return argc', to execute the left of the pipeline.
			 */
			int p[2];
			/* Exercise 6.5: Your code here. (3/3) */
			r = pipe(p);
			if (p < 0) {
				user_panic("failed to pipe %d\n", r);
			}
			*rightpipe = fork();
			if (*rightpipe == 0) {
				dup(p[0], 0);
				close(p[0]);
				close(p[1]);
				return parsecmd(argv, rightpipe, pend);
			} else {
				dup(p[1], 1);
				close(p[1]);
				close(p[0]);
				return argc;
			}
			user_panic("| not implemented");

			break;
		case ';':
			temp = fork();
			if (temp) {
				wait(temp);
				if (mark)
					dup(0, 1);
				else 
					dup(1, 0);
				return parsecmd(argv, rightpipe, pend);
			} else {
				return argc;
			}
			break;
		case '&':
			temp = fork();
			if (temp) {
				wait(temp);
				if (mark)
					dup(0, 1);
				else 
					dup(1, 0);
				return parsecmd(argv, rightpipe, pend);
			} else {
				*pend = 1;
				return argc;
			}
			break;
		}
	}

runit:
	return argc;
}

void runcmd(char *s) {
	gettoken(s, 0);

	char *argv[MAXARGS];
	int rightpipe = 0;
	int pend = 0;
	int argc = parsecmd(argv, &rightpipe, &pend);
	if (argc == 0) {
		return;
	}
	argv[argc] = 0;
	// printf("argc : %d\n", argc);
	// for (int i = 0; i < argc; i++) {
	// 	printf("argv %d : %s\n", i, argv[i]);
	// }
	int child = spawn(argv[0], argv, 0);
	if (child < 0) {
		child = spawn(argv[0], argv, 1);
	}
	close_all();

	if (child >= 0) {
		if (!pend) {
			wait(child);
		} else {
			debugf(RED("PEND : %x") "   \n", child);
			int pid = fork();
			if (pid == 0) {
				wait(child);
				debugf(GREEN("DONE : %x") "   \n", child);
				exit();
			}
		}
	} else {
		debugf("spawn %s: %d\n", argv[0], child);
	}
	if (rightpipe) {
		wait(rightpipe);
	}
	exit();
}

char *start, *finish;
char *begin, *end;
char curbuf[1024];
int his;
void last(char *buf, int *type) {
	if (*type == 0) {
		his = open("/.history", O_RDONLY);
		struct Fd *fd = num2fd(his);
		begin = fd2data(fd);
		int size = ((struct Filefd *)fd)->f_file.f_size;
		end = begin + size;
		start = end - 1;
		finish = end - 1;
		memset(curbuf, 0, 1024);
		strcpy(curbuf, buf);
		// printf("\ncopy : %s\n",curbuf);
	} else {
		if (start == begin)
			return;
		finish = start - 1;
		start = finish;
	}

	while (*(start - 1) != '\n' && start > begin)
		start--;
	memset(buf, 0, 1024);
	int cnt = 0;
	for (char *p = start; p < finish; p++) {
		buf[cnt++] = *p;
	}
	buf[cnt] = 0;

	for (int i = 0; i < all - cur; i++)
		printf("\x1b[K");
	for (int i = 0; i < all + 25; i++)
		printf("\b \b");

	printf(GREEN(wwr@mos-shell) ":" BLUE(%s) " $ %s",curpath0, buf);

	all = cnt;
	cur = cnt;
	*type = 1;
}

void next(char *buf, int *type) {
	int cnt = 0;
	if (*type == 0) {
		return;
	} else {
		if (finish == end - 1) {
			start == end;
			strcpy(buf, curbuf);
			cnt = strlen(buf);
			*type = 0;
		} else {
			start = finish + 1;
			finish++;
			while (*finish != '\n' && finish < end - 1)
				finish++;
			for (char *p = start; p < finish; p++)
				buf[cnt++] = *p;
			buf[cnt] = 0;
		}
	}

	for (int i = 0; i < all - cur; i++)
		printf("\x1b[K");
	for (int i = 0; i < all + 25; i++)
		printf("\b \b");

	printf(GREEN(wwr@mos-shell) ":" BLUE(%s) " $ %s",curpath0, buf);

	all = cnt;
	cur = cnt;
	if (*type == 0) {
		close(his);
	}
}

void readline(char *buf, u_int n) {
	int r;
	cur = 0, all = 0; // cursor
	int updown = 0;
	memset(buf, 0, 1024);
	while (all < n) {
		char c;
		if ((r = read(0, &c, 1)) != 1) {
			if (r < 0) {
				debugf("read error: %d\n", r);
			}
			exit();
		}
		if (c == 27) {
			char tmp;
			read(0, &tmp, 1);
			char tmp2;
			read(0, &tmp2, 1);
			if (tmp == 91 && tmp2 == 65) {
				printf("\x1b[B");
				last(buf, &updown);

			} else if (tmp == 91 && tmp2 == 66) {
				// printf("\x1b[A");
				next(buf, &updown);

			} else if (tmp == 91 && tmp2 == 67) {
				if (cur < all)
					cur++;
				else
					printf("\b");
			} else if (tmp == 91 && tmp2 == 68) {
				if (cur > 0)
					cur--;
				else
					printf("\x1b[C");
			}
		} else if (c == '\b' || c == 0x7f) {
			if (updown)
				close(his);
			updown = 0;
			if (all > 0) {
				if (cur == 0)
					continue;

				for (int j = cur; j <= all - 1; j++)
					buf[j - 1] = buf[j];
				buf[all - 1] = 0;
				printf("\b");
				printf("%s", &buf[cur - 1]);
				printf("  ");
				for (int j = cur - 1; j < all; j++)
					printf("\b");
				cur--;
				all--;
				if (c != '\b') {
					printf("\b");
				}
			}

		} else if (c == '\r' || c == '\n') {

			if (updown)
				close(his);
			updown = 0;
			pos += all + 1;
			buf[all] = '\n';
			if (all > 0) {
				int f = open("/.history", O_WAPPEND);
				struct Fd *fd = num2fd(his);
				write(f, buf, all + 1);
				close(f);
			}
			buf[all] = 0;
			return;
		} else {
			if (updown)
				close(his);
			updown = 0;
			cur++;
			all++;
			if (cur != all) {
				char now = c;
				for (int j = all - 1; j >= cur; j--) {
					buf[j] = buf[j - 1];
				}

				buf[cur - 1] = now;
				buf[all] = 0;
				for (int i = 0; i <= all + 35; i++)
					printf("\b");
				printf(GREEN(wwr@mos-shell) ":" BLUE(%s) " $ %s",curpath0, buf);
				for (int j = all - 1; j >= cur; j--) {
					printf("\b");
				}
			} else {
				buf[cur - 1] = c;
			}
		}
	}
	debugf("line too long\n");
	while ((r = read(0, buf, 1)) == 1 && buf[0] != '\r' && buf[0] != '\n') {
		;
	}
	buf[0] = 0;
}

char buf[1024];

void usage(void) {
	debugf("usage: sh [-dix] [command-file]\n");
	exit();
}

int whichshell;

int main(int argc, char **argv) {
	create("/.history", 0);
	curpath_init("/");
	curpath_get(curpath0);
	whichshell = env->env_id;
	printf(RED(SHELL:) " %d\n", whichshell);
	syscall_env_var(0, 0, 65536);
	int r;
	int interactive = iscons(0);
	int echocmds = 0;
	debugf("\n:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	debugf("::                                                         ::\n");
	debugf("::                     MOS Shell 2023                      ::\n");
	debugf("::                                                         ::\n");
	debugf(":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	ARGBEGIN {
	case 'i':
		interactive = 1;
		break;
	case 'x':
		echocmds = 1;
		break;
	default:
		usage();
	}
	ARGEND

	if (argc > 1) {
		usage();
	}
	if (argc == 1) {
		close(0);
		if ((r = open(argv[1], O_RDONLY)) < 0) {
			user_panic("open %s: %d", argv[1], r);
		}
		user_assert(r == 0);
	}
	for (;;) {
		memset(curpath0, 0, 128);
		curpath_get(curpath0);
		if (interactive) {
			printf("\n" GREEN(wwr@mos-shell) ":" BLUE(%s)" $ ", curpath0);
		}
		readline(buf, sizeof buf);

		if (buf[0] == '#') {
			continue;
		}
		if (echocmds) {
			printf("# %s\n", buf);
		}
		if ((r = fork()) < 0) {
			user_panic("fork: %d", r);
		}
		if (r == 0) {
			runcmd(buf);
			exit();
		} else {
			wait(r);
		}
	}
	return 0;
}

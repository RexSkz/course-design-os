#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAXLENGTH 256
#define FROMFILE -1
#define TOFILE 1

void cmd_parse(char *cmd, char *argv[], char *fromfile, char *tofile) {
	int index = 0, curindex = 0, fromindex = 0, toindex = 0;
	int len = strlen(cmd);
	int status = 0;
	argv[0] = (char *)malloc(sizeof(char) * 256);
	for (int i = 0; i < len - 1; ++i) {
		if (cmd[i] == ' ') {
			if (status != 0) {
				continue;
			}
			if (curindex > 0) {
				argv[index][curindex] = 0;
				++index;
				argv[index] = (char *)malloc(sizeof(char) * 256);
				curindex = 0;
			}
		}
		else if (cmd[i] == '<') {
			status = FROMFILE;
		}
		else if (cmd[i] == '>') {
			status = TOFILE;
		}
		else {
			if (status == FROMFILE) {
				fromfile[fromindex++] = cmd[i];
			}
			else if (status == TOFILE) {
				tofile[toindex++] = cmd[i];
			}
			else {
				argv[index][curindex++] = cmd[i];
			}
		}
	}
	argv[index][curindex] = fromfile[fromindex] = tofile[toindex] = 0;
	while (strlen(argv[index]) == 0) {
		free(argv[index]);
		argv[index] = 0;
		--index;
	}
	return;
}

void my_system(char *argv[], char *fromfile, char *tofile) {
	if (fork() == 0) {
		FILE *fromfilefd = NULL, *tofilefd = NULL;
		if (strlen(fromfile) != 0) {
			fromfilefd = fopen(fromfile, "r");
			if (!fromfilefd) {
				printf("%s: No such file or directory.\n", fromfile);
				exit(-1);
			}
			else {
				dup2(fileno(fromfilefd), STDIN_FILENO);
				fclose(fromfilefd);
			}
		}
		else if (strlen(tofile) != 0) {
			tofilefd = fopen(tofile, "w");
			dup2(fileno(tofilefd), STDOUT_FILENO);
			fclose(tofilefd);
		}
		execvp(argv[0], argv);
	}
	else {
		wait(0);
	}
}

int main() {
	char cmd[MAXLENGTH];
	char *argv[20] = {0};
	char fromfile[MAXLENGTH], tofile[MAXLENGTH];
	printf("shell> ");
	while ((fgets(cmd, MAXLENGTH, stdin) != 0)) {
		FILE *fromfilefd = NULL, *tofilefd = NULL;
		for (int i = 0; i < 20; ++i) {
			if (argv[i] != 0) {
				free(argv[i]);
				argv[i] = 0;
			}
		}
		memset(fromfile, 0, sizeof(fromfile));
		memset(tofile, 0, sizeof(tofile));
		cmd_parse(cmd, argv, fromfile, tofile);
		if (strcmp(argv[0], "exit") == 0) {
			return 0;
		}
		else if (strcmp(argv[0], "pwd") == 0) {
			strlen(tofile) && (tofilefd = fopen(tofile, "w"));
			char cwd[64];
			getcwd(cwd, 64);
			fprintf(tofilefd ? tofilefd : stdout, "%s\n", cwd);
			tofilefd && fclose(tofilefd);
		}
		else if (strcmp(argv[0], "cd") == 0) {
			if (chdir(argv[1]) != 0) {
				printf("No such file or directory: %s.\n", argv[1]);
			}
		}
		else {
			my_system(argv, fromfile, tofile);
		}
		printf("shell> ");
	}
	return 0;
}

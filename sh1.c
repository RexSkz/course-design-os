#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void cmd_parse(char *cmd, char *argv[]) {
	int index = 0;
	int curindex = 0;
	int len = strlen(cmd);
	argv[0] = (char *)malloc(sizeof(char) * 256);
	for (int i = 0; i < len - 1; ++i) {
		if (cmd[i] == ' ') {
			if (curindex > 0) {
				argv[index][curindex] = 0;
				++index;
				argv[index] = (char *)malloc(sizeof(char) * 256);
				curindex = 0;
			}
		}
		else {
			argv[index][curindex++] = cmd[i];
		}
	}
	argv[index][curindex] = 0;
	while (strlen(argv[index]) == 0) {
		free(argv[index]);
		argv[index] = 0;
		--index;
	}
	return;
}

void my_system(char *argv[]) {
	if (fork() == 0) {
		execvp(argv[0], argv);
	}
	else {
		wait(NULL);
	}
}

int main() {
	char cmd[256];
	printf("shell> ");
	while ((fgets(cmd, 256, stdin) != 0)) {
		char *argv[20] = {0};
		for (int i = 0; i < 20; ++i) {
			if (argv[i] != 0) {
				free(argv[i]);
				argv[i] = 0;
			}
		}
		cmd_parse(cmd, argv);
		if (strcmp(argv[0], "exit") == 0) {
			return 0;
		}
		else if (strcmp(argv[0], "pwd") == 0) {
			char cwd[64];
			getcwd(cwd, 64);
			printf("%s\n", cwd);
		}
		else if (strcmp(argv[0], "cd") == 0) {
			if (chdir(argv[1]) != 0) {
				printf("No such file or directory: %s.\n", argv[1]);
			}
		}
		else {
			my_system(argv);
		}
		printf("shell> ");
	}
	return 0;
}

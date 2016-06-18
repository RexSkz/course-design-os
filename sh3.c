#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// comment this line below to hide log info
#define _DEBUG_MODE

#define MAXLENGTH 256
#define FROMFILE -1
#define TOFILE 1

void cmd_parse(char *cmd, char *argv[], char *fromfile, char *tofile, char *pipe_cmd) {
	int index = 0, curindex = 0, fromindex = 0, toindex = 0;
	int len = strlen(cmd);
	int status = 0;
	argv[0] = (char *)malloc(sizeof(char) * 256);
	for (int i = 0; i < len; ++i) {
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
		else if (cmd[i] == '|') {
			strcpy(pipe_cmd, cmd + i + 1);
			break;
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
	while (!argv[index] && index > 0) {
		--index;
	}
	argv[index][curindex] = fromfile[fromindex] = tofile[toindex] = 0;
	while (strlen(argv[index]) == 0 && index > 0) {
		free(argv[index]);
		argv[index] = 0;
		--index;
	}
	return;
}

void my_system(char *argv[], char *fromfile, char *tofile, char *pipe_cmd, int *prev_pipe_fd) {
	if (strlen(argv[0]) == 0) {
		return;
	}
	int *pipe_fd = NULL;
	if (strlen(pipe_cmd) != 0) {
		pipe_fd = (int *)malloc(sizeof(int) * 2);
		pipe(pipe_fd);
	}
	if (strcmp(argv[0], "exit") == 0) {
#ifdef _DEBUG_MODE
		fprintf(stderr, "[log %5s] bye\n", argv[0]);
#endif
		exit(0);
	}
	int pid = fork();
	// child process
	if (pid == 0) {
		// if has previus pipe, connect input to pipe's output
		if (prev_pipe_fd) {
			close(STDIN_FILENO);
			dup2(prev_pipe_fd[0], STDIN_FILENO);
			close(prev_pipe_fd[0]);
			close(prev_pipe_fd[1]);
#ifdef _DEBUG_MODE
			fprintf(stderr, "[log %5s] input pipe connected: [%x]\n", argv[0], prev_pipe_fd[0]);
#endif
		}
		// connect output to pipe's input
		if (pipe_fd) {
			close(STDOUT_FILENO);
			dup2(pipe_fd[1], STDOUT_FILENO);
			close(pipe_fd[0]);
			close(pipe_fd[1]);
#ifdef _DEBUG_MODE
			fprintf(stderr, "[log %5s] output pipe connected: [%x]\n", argv[0], pipe_fd[1]);
#endif
		}
		// deal with file redirect
		FILE *fromfilefd = NULL, *tofilefd = NULL;
		if (strlen(fromfile) != 0) {
			fromfilefd = fopen(fromfile, "r");
			if (!fromfilefd) {
				fprintf(stderr, "%s: No such file or directory.\n", fromfile);
				exit(-1);
			}
			else {
				dup2(fileno(fromfilefd), STDIN_FILENO);
				fclose(fromfilefd);
#ifdef _DEBUG_MODE
				fprintf(stderr, "[log %5s] input file connected\n", argv[0]);
#endif
			}
		}
		if (strlen(tofile) != 0) {
			tofilefd = fopen(tofile, "w");
			dup2(fileno(tofilefd), STDOUT_FILENO);
			fclose(tofilefd);
#ifdef _DEBUG_MODE
			fprintf(stderr, "[log %5s] output file connected\n", argv[0]);
#endif
		}
#ifdef _DEBUG_MODE
		fprintf(stderr, "[log %5s] executing %s\n", argv[0], argv[0]);
#endif
		// run built-in and execvp process
		if (strcmp(argv[0], "pwd") == 0) {
			char cwd[64];
			getcwd(cwd, 64);
			printf("%s\n", cwd);
		}
		else if (strcmp(argv[0], "cd") == 0) {
			if (chdir(argv[1]) != 0) {
				fprintf(stderr, "No such file or directory: %s.\n", argv[1]);
			}
		}
		else {
			execvp(argv[0], argv);
		}
		exit(0);
	}
	// parent process
	waitpid(pid, NULL, 0);
#ifdef _DEBUG_MODE
	fprintf(stderr, "[log %5s] child process terminated\n", argv[0]);
#endif
	if (pipe_fd) {
		close(pipe_fd[1]);
#ifdef _DEBUG_MODE
		fprintf(stderr, "[log %5s] pipe [%x] all closed\n", argv[0], pipe_fd[1]);
#endif
	}
	if (strlen(pipe_cmd) != 0) {
#ifdef _DEBUG_MODE
		fprintf(stderr, "[log %5s] has another pipe: %s\n", argv[0], pipe_cmd);
#endif
		char cmd[MAXLENGTH];
		strcpy(cmd, pipe_cmd);
		memset(fromfile, 0, sizeof(fromfile));
		memset(tofile, 0, sizeof(tofile));
		memset(pipe_cmd, 0, sizeof(pipe_cmd));
#ifdef _DEBUG_MODE
		fprintf(stderr, "[log %5s] recursive calling my_system(", argv[0]);
#endif
		cmd_parse(cmd, argv, fromfile, tofile, pipe_cmd);
		fprintf(stderr, "%s)\n", argv[0]);
		my_system(argv, fromfile, tofile, pipe_cmd, pipe_fd);
	}
	if (pipe_fd) {
		free(pipe_fd);
	}
}

int main() {
	char cmd[MAXLENGTH];
	char *argv[20] = {0};
	char fromfile[MAXLENGTH], tofile[MAXLENGTH], pipe_cmd[MAXLENGTH];
	printf("shell> ");
	while ((fgets(cmd, MAXLENGTH, stdin) != 0)) {
		cmd[strlen(cmd) - 1] = 0;
		FILE *fromfilefd = NULL, *tofilefd = NULL;
		for (int i = 0; i < 20; ++i) {
			if (argv[i] != 0) {
				free(argv[i]);
				argv[i] = 0;
			}
		}
		memset(fromfile, 0, sizeof(fromfile));
		memset(tofile, 0, sizeof(tofile));
		memset(pipe_cmd, 0, sizeof(pipe_cmd));
		cmd_parse(cmd, argv, fromfile, tofile, pipe_cmd);
		my_system(argv, fromfile, tofile, pipe_cmd, NULL);
		printf("shell> ");
	}
	return 0;
}

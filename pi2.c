#include <stdio.h>
#include <pthread.h>
#define THREADCOUNT 100

// gcc -Wall pi2.c -o pi2 -lpthread -std=c99

double ans[THREADCOUNT];

void *calc(void *_args) {
	int *args = (int *)_args;
	double result = 0;
	for (int i = args[0]; i <= args[1]; ++i) {
		double delta = 1.0 / (i * 2 - 1);
		result += (i & 1) ? delta : -delta;
	}
	ans[args[2]] = result;
	return NULL;
}

int main() {
	int n = 10000000, args[THREADCOUNT][3];
	pthread_t tid[THREADCOUNT];
	double result = 0;
	int calcs_per_thread = n / THREADCOUNT;
	for (int i = 0; i < THREADCOUNT; ++i) {
		args[i][0] = calcs_per_thread * i + 1;
		args[i][1] = calcs_per_thread * (i + 1);
		args[i][2] = i;
		pthread_create(&tid[i], NULL, calc, (void *)args[i]);
	}
	for (int i = 0; i < THREADCOUNT; ++i) {
		pthread_join(tid[i], NULL);
		result += ans[i];
	}
	printf("The answer is %.7lf.\n", 4 * result);
	return 0;
}

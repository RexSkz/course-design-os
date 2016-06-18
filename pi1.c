#include <stdio.h>
#include <pthread.h>
#define MAXTCOUNT 2

// gcc -Wall pi1.c -o pi1 -lpthread -std=c99

double ans[MAXTCOUNT];

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
	// calculate from 1 to 2 * 99 + 1
	int n = 10000000, arg0[3], arg1[3];
	arg0[0] = 1;
	arg0[1] = n / 2;
	arg0[2] = 0;
	arg1[0] = n / 2 + 1;
	arg1[1] = n;
	arg1[2] = 1;
	pthread_t tid;
	// sub thread
	pthread_create(&tid, NULL, calc, (void *)arg1);
	// main thread
	calc(arg0);
	pthread_join(tid, NULL);
	printf("The answer is %.7lf.\n", 4 * (ans[0] + ans[1]));
	return 0;
}

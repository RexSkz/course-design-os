#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#define MAXN 100

// gcc -Wall ring.c -o ring -lpthread -std=c99

int param[MAXN];
pthread_t tid[MAXN];
pthread_cond_t cond[MAXN];
pthread_mutex_t mutex[MAXN];
int n, integer = 1, wait_count = 0;

void *fun(void *_tid) {
	int tid = ((int *)_tid)[0];
	pthread_mutex_init(&mutex[tid], NULL);
	pthread_mutex_lock(&mutex[tid]);
	++wait_count;
	while (1) {
		pthread_cond_wait(&cond[tid], &mutex[tid]);
		printf("Thread %d received %d\n", tid + 1, integer);
		fflush(stdout);
		++integer;
		pthread_cond_signal(&cond[(tid + 1) % n]);
	}
	return NULL;
}

int main() {
	scanf("%d", &n);
	for (int i = 0; i < n; ++i) {
		param[i] = i;
		pthread_cond_init(&cond[i], NULL);
		pthread_create(&tid[i], NULL, fun, &param[i]);
	}
	while (wait_count < n);
	pthread_cond_signal(&cond[0]);
	for (int i = 0; i < n; ++i) {
		pthread_join(tid[i], NULL);
	}
	return 0;
}

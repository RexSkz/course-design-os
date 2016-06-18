#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define N 2000

// gcc -Wall sort.c -o sort -lpthread -std=c99

int a[N];

void *sort(void *_args) {
	int *args = (int *)_args;
	for (int i = args[0]; i <= args[1]; ++i) {
		int mini = i;
		for (int j = i + 1; j <= args[1]; ++j) {
			if (a[mini] > a[j]) {
				mini = j;
			}
		}
		int t = a[mini];
		a[mini] = a[i];
		a[i] = t;
	}
	return NULL;
}

void merge(int l1, int r1, int l2, int r2) {
	int *b = (int *)malloc(sizeof(int) * (r2 - l1 + 1));
	int p = l1, q = l2, r = 0;
	while (p <= r1 && q <= r2) {
		if (a[p] < a[q]) {
			b[r++] = a[p++];
		}
		else {
			b[r++] = a[q++];
		}
	}
	while (p <= r1) b[r++] = a[p++];
	while (q <= r2) b[r++] = a[q++];
	for (int i = 0; i < r2 - l1 + 1; ++i) {
		a[l1 + i] = b[i];
	}
	free(b);
}

int main() {
	int n;
	scanf("%d", &n);
	for (int i = 0; i < n; ++i) {
		scanf("%d", a + i);
	}
	int args[2][2];
	// sub thread sort the latter
	args[0][0] = n / 2, args[0][1] = n - 1;
	pthread_t tid;
	pthread_create(&tid, NULL, sort, (void *)args[0]);
	// main thread sort the former
	args[1][0] = 0, args[1][1] = n / 2 - 1;
	sort(args[1]);
	pthread_join(tid, NULL);
	merge(0, n / 2 - 1, n / 2, n - 1);
	for (int i = 0; i < n; ++i) {
		printf("%d ", a[i]);
	}
	printf("\n");
	return 0;
}

#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#define CAPACITY 4

// gcc -Wall pc1.c -o pc1 -lpthread -std=c99

struct queue {
	char q[CAPACITY];
	int head, tail;
};

struct queue buffer[2];
pthread_t tid[2];
pthread_mutex_t mutex[2];
pthread_cond_t not_full[2];
pthread_cond_t not_empty[2];

int is_empty(int id) {
	return buffer[id].head == buffer[id].tail;
}

int is_full(int id) {
	return (buffer[id].tail + 1) % CAPACITY == buffer[id].head;
}

void enque(int id, char item) {
	pthread_mutex_lock(&mutex[id]);
	if (is_full(id)) {
		pthread_cond_wait(&not_full[id], &mutex[id]);
	}
	buffer[id].q[buffer[id].tail] = item;
	buffer[id].tail = (buffer[id].tail + 1) % CAPACITY;
	pthread_cond_signal(&not_empty[id]);
	pthread_mutex_unlock(&mutex[id]);
}

int deque(int id) {
	pthread_mutex_lock(&mutex[id]);
	if (is_empty(id)) {
		pthread_cond_wait(&not_empty[id], &mutex[id]);
	}
	int item = buffer[id].q[buffer[id].head];
	buffer[id].head = (buffer[id].head + 1) % CAPACITY;
	pthread_cond_signal(&not_full[id]);
	pthread_mutex_unlock(&mutex[id]);
	return item;
}

void produce() {
	for (char ch = 'a'; ch <= 'h'; ++ch) {
		enque(0, ch);
		printf("Produce '%c'\n", ch);
		fflush(stdout);
	}
	printf("Producer terminated.\n");
	fflush(stdout);
}

void *calculate(void *args) {
	while (1) {
		char item = deque(0);
		char result = toupper(item);
		enque(1, result);
		printf("Calculate '%c' to '%c'\n", item, result);
		fflush(stdout);
	}
	return NULL;
}

void *consume(void *args) {
	while (1) {
		char item = deque(1);
		printf("Consume '%c'\n", item);
		fflush(stdout);
	}
	return NULL;
}

int main() {
	for (int i = 0; i < 2; ++i) {
		pthread_mutex_init(&mutex[i], NULL);
		pthread_cond_init(&not_full[i], NULL);
		pthread_cond_init(&not_empty[i], NULL);
		pthread_cond_signal(&not_full[i]);
	}
	pthread_create(&tid[0], NULL, consume, NULL);
	pthread_create(&tid[1], NULL, calculate, NULL);
	produce();
	for (int i = 0; i < 2; ++i) {
		pthread_join(tid[i], NULL);
	}
	// the program will not terminate because of
	// the infinite loop in calculate and consume
	return 0;
}

#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include <semaphore.h>
#define CAPACITY 4

// gcc -Wall pc2.c -o pc2 -lpthread -std=c99

struct queue {
	char q[CAPACITY];
	int head, tail;
};

struct queue buffer[2];
pthread_t tid[2];
sem_t mutex[2];
sem_t available[2];
sem_t used[2];

void enque(int id, char item) {
	sem_wait(&available[id]);
	sem_wait(&mutex[id]);
	buffer[id].q[buffer[id].tail] = item;
	buffer[id].tail = (buffer[id].tail + 1) % CAPACITY;
	sem_post(&mutex[id]);
	sem_post(&used[id]);
}

int deque(int id) {
	sem_wait(&used[id]);
	sem_wait(&mutex[id]);
	int item = buffer[id].q[buffer[id].head];
	buffer[id].head = (buffer[id].head + 1) % CAPACITY;
	sem_post(&mutex[id]);
	sem_post(&available[id]);
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
		sem_init(&mutex[i], 0, 1);
		sem_init(&available[i], 0, CAPACITY);
		sem_init(&used[i], 0, 0);
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

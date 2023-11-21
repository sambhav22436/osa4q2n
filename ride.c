#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>

#define MAX_PASSENGERS 10

sem_t car_mutex, passenger_mutex, board_mutex, offboard_mutex, all_boarded_mutex, all_unboarded_mutex;
int passenger_count = 0, total_passengers, car_capacity, passengers_boarded = 0, passengers_unboarded = 0;

void* car(void* args);
void* passenger(void* args);
void load();
void unload();
void board(int id);
void offboard(int id);

void load() {
    sem_wait(&car_mutex);
    printf("Car is loading passengers...\n");
    sleep(1); // Simulating loading time
    sem_post(&passenger_mutex);
    sem_wait(&all_boarded_mutex); // Wait for all passengers to board
    sem_post(&car_mutex);
}

void unload() {
    sem_wait(&car_mutex);
    printf("Car is unloading passengers...\n");
    sleep(1); // Simulating unloading time
    sem_post(&all_unboarded_mutex); // Signal that all passengers have unboarded
    sem_post(&car_mutex);
}

void board(int id) {
    sem_wait(&board_mutex);
    sem_wait(&passenger_mutex);
    printf("Passenger %d is boarding the car.\n", id);
    passenger_count++;
    if (passenger_count == car_capacity) {
        sem_post(&all_boarded_mutex); // Signal car to start
    }
    sem_post(&passenger_mutex);
    sem_post(&board_mutex);
    sleep(1); // Introduce a delay between each boarding action
}

void offboard(int id) {
    sem_wait(&offboard_mutex);
    sem_wait(&passenger_mutex);
    printf("Passenger %d is getting off the car.\n", id);
    passenger_count--;
    if (passenger_count == 0) {
        sem_post(&all_unboarded_mutex); // Signal car to unload
    }
    sem_post(&passenger_mutex);
    sem_post(&offboard_mutex);
    sleep(1); // Introduce a delay between each unboarding action
}

void* car(void* args) {
    while (1) {
        load();
        sem_wait(&all_unboarded_mutex); // Wait for all passengers to unboard
        unload();
        passengers_boarded = 0; // Reset for the next ride
        passengers_unboarded = 0; // Reset for the next ride
    }
    return NULL;
}

void* passenger(void* args) {
    int id = *((int*)args);
    while (1) {
        board(id);
        if (++passengers_boarded == car_capacity) {
            sem_post(&all_boarded_mutex); // Signal that all passengers have boarded
            sem_wait(&all_unboarded_mutex); // Wait until all passengers have unboarded
        }
        offboard(id);
        if (++passengers_unboarded == car_capacity) {
            sem_post(&all_unboarded_mutex); // Signal that passenger has unboarded
            sem_wait(&all_boarded_mutex); // Wait until all passengers have boarded
        }
    }
    return NULL;
}

int main() {
    pthread_t car_thread, passenger_threads[MAX_PASSENGERS];
    int passenger_ids[MAX_PASSENGERS];

    sem_init(&car_mutex, 0, 1);
    sem_init(&passenger_mutex, 0, 0);
    sem_init(&board_mutex, 0, 1);
    sem_init(&offboard_mutex, 0, 1);
    sem_init(&all_boarded_mutex, 0, 0);
    sem_init(&all_unboarded_mutex, 0, 0);

    printf("Enter the car capacity: ");
    scanf("%d", &car_capacity);

    total_passengers = MAX_PASSENGERS; // Set the total passengers to the maximum initially

    pthread_create(&car_thread, NULL, car, NULL);

    for (int i = 0; i < total_passengers; i++) {
        passenger_ids[i] = i + 1;
        pthread_create(&passenger_threads[i], NULL, passenger, &passenger_ids[i]);
    }

    // Sleep for a sufficient number of iterations (manually terminate the program)
    sleep(10);

    // Terminate threads
    pthread_cancel(car_thread);
    for (int i = 0; i < total_passengers; i++) {
        pthread_cancel(passenger_threads[i]);
    }

    sem_destroy(&car_mutex);
    sem_destroy(&passenger_mutex);
    sem_destroy(&board_mutex);
    sem_destroy(&offboard_mutex);
    sem_destroy(&all_boarded_mutex);
    sem_destroy(&all_unboarded_mutex);

    return 0;
}







#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

pthread_mutex_t globalMutex = PTHREAD_MUTEX_INITIALIZER;

struct FanThreadArgs {
    sem_t* teamSemaphores;
    int* teamFansCount;
    int team;
};

void requestRide(struct FanThreadArgs* fanArgs, int currentTeam) {
    pthread_mutex_lock(&globalMutex);
    printf("Thread ID: %ld, Team: %c, I am looking for a car\n", pthread_self(), (char)(currentTeam + 'A'));
    fanArgs->teamFansCount[currentTeam]++;
    pthread_mutex_unlock(&globalMutex);
}

void joinCar(struct FanThreadArgs* fanArgs, int currentTeam, int otherTeam) {
    sem_post(&fanArgs->teamSemaphores[currentTeam]);
    for (int i = 0; i < 2; i++) sem_post(&fanArgs->teamSemaphores[otherTeam]);
    fanArgs->teamFansCount[currentTeam] -= 2;
    fanArgs->teamFansCount[otherTeam] -= 2;
}

void driveCar(struct FanThreadArgs* fanArgs, int currentTeam) {
    for (int i = 0; i < 3; i++) sem_post(&fanArgs->teamSemaphores[currentTeam]);
    fanArgs->teamFansCount[currentTeam] -= 4;
}

void* rideFinder(void* arg) {
    struct FanThreadArgs* fanArgs = (struct FanThreadArgs*)arg;
    int currentTeam = fanArgs->team;
    int otherTeam = 1 - currentTeam;
    int isCarFull = 0;

    requestRide(fanArgs, currentTeam);

    if (fanArgs->teamFansCount[currentTeam] > 1 && fanArgs->teamFansCount[otherTeam] == 2) {
        joinCar(fanArgs, currentTeam, otherTeam);
        isCarFull = 1;
    } else if (fanArgs->teamFansCount[currentTeam] == 4) {
        driveCar(fanArgs, currentTeam);
        isCarFull = 1;
    } else {
        sem_wait(&fanArgs->teamSemaphores[currentTeam]);
    }

    printf("Thread ID: %ld, Team: %c, I have found a spot in a car \n", pthread_self(), (char)(currentTeam + 'A'));

    if (isCarFull == 1) {
        pthread_mutex_lock(&globalMutex);
        printf("Thread ID: %ld, Team: %c, I am the captain and driving the car \n", pthread_self(), (char)(currentTeam + 'A'));
        pthread_mutex_unlock(&globalMutex);
    }
}


int main(int argc, char* argv[]) {
    int teamCounts[2] = { atoi(argv[1]), atoi(argv[2]) };

    if ((teamCounts[0] + teamCounts[1]) % 4 != 0 || (teamCounts[0] % 2 != 0) || (teamCounts[1] % 2 != 0)) {
        printf("Invalid team counts. The main terminates.\n");
    } else {
        sem_t teamSemaphores[2];
        for (int i = 0; i < 2; i++) sem_init(&teamSemaphores[i], 0, 0);

        int totalFansCount = teamCounts[0] + teamCounts[1];
        pthread_t threads[totalFansCount];
        int teamFansWaitingCount[2] = {0, 0};

        struct FanThreadArgs fanThreadArgs_A = {&teamSemaphores, &teamFansWaitingCount, 0};
        struct FanThreadArgs fanThreadArgs_B = {&teamSemaphores, &teamFansWaitingCount, 1};

        for (int k = 0; k < totalFansCount; k++) {
            if (k < teamCounts[0]) {
                pthread_create(&threads[k], NULL, rideFinder, &fanThreadArgs_A);
            } else {
                pthread_create(&threads[k], NULL, rideFinder, &fanThreadArgs_B);
            }
        }

        for (int k = 0; k < totalFansCount; k++) {
            pthread_join(threads[k], NULL);
        }

        printf("The main terminates\n");

        for (int i = 0; i < 2; i++) sem_destroy(&teamSemaphores[i]);
    }
    return 0;
}


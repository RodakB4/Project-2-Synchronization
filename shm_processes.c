/*
 * Name: Rodak Tehwodros Beckure
 * Project: Project 2 - Synchronization
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>

void dear_old_dad(int *bankAccount, sem_t *mutex) {
    int localBalance;
    int amount;
    srand(time(NULL) ^ (getpid() << 16));

    while (1) {
        sleep(rand() % 6); // Sleep 0-5 seconds
        printf("Dear Old Dad: Attempting to Check Balance\n");
        
        int r = rand();
        if (r % 2 == 0) { // Even
            sem_wait(mutex);
            localBalance = *bankAccount;
            if (localBalance < 100) {
                amount = rand() % 101; // 0-100
                int r2 = rand();
                if (r2 % 2 == 0) {
                    localBalance += amount;
                    printf("Dear old Dad: Deposits $%d / Balance = $%d\n", amount, localBalance);
                    *bankAccount = localBalance;
                } else {
                    printf("Dear old Dad: Doesn't have any money to give\n");
                }
            } else {
                printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", localBalance);
            }
            sem_post(mutex);
        } else { // Odd
            sem_wait(mutex);
            localBalance = *bankAccount;
            printf("Dear Old Dad: Last Checking Balance = $%d\n", localBalance);
            sem_post(mutex);
        }
    }
}

void lovable_mom(int *bankAccount, sem_t *mutex) {
    int localBalance;
    int amount;
    srand(time(NULL) ^ (getpid() << 16));

    while (1) {
        sleep(rand() % 11); // Sleep 0-10 seconds
        printf("Loveable Mom: Attempting to Check Balance\n");
        
        sem_wait(mutex);
        localBalance = *bankAccount;
        if (localBalance <= 100) {
            amount = rand() % 126; // 0-125
            localBalance += amount;
            printf("Lovable Mom: Deposits $%d / Balance = $%d\n", amount, localBalance);
            *bankAccount = localBalance;
        }
        sem_post(mutex);
    }
}

void poor_student(int *bankAccount, sem_t *mutex, int id) {
    int localBalance;
    int need;
    srand(time(NULL) ^ (getpid() << 16) ^ id);

    while (1) {
        sleep(rand() % 6); // Sleep 0-5 seconds
        printf("Poor Student: Attempting to Check Balance\n");
        
        int r = rand();
        if (r % 2 == 0) { // Even
            sem_wait(mutex);
            localBalance = *bankAccount;
            need = rand() % 51; // 0-50
            printf("Poor Student needs $%d\n", need);
            if (need <= localBalance) {
                localBalance -= need;
                printf("Poor Student: Withdraws $%d / Balance = $%d\n", need, localBalance);
                *bankAccount = localBalance;
            } else {
                printf("Poor Student: Not Enough Cash ($%d)\n", localBalance);
            }
            sem_post(mutex);
        } else { // Odd
            sem_wait(mutex);
            localBalance = *bankAccount;
            printf("Poor Student: Last Checking Balance = $%d\n", localBalance);
            sem_post(mutex);
        }
    }
}

int main(int argc, char *argv[]) {
    int num_parents = 1;
    int num_students = 1;

    if (argc == 3) {
        num_parents = atoi(argv[1]);
        num_students = atoi(argv[2]);
    }

    // Setup Shared Memory for BankAccount
    int *bankAccount = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *bankAccount = 0; // Initialize account with 0

    // Setup Semaphore
    sem_t *mutex = sem_open("/bank_semaphore", O_CREAT, 0666, 1);
    sem_unlink("/bank_semaphore"); // Ensure it gets cleaned up after execution

    // Fork Parents
    if (num_parents >= 1) {
        if (fork() == 0) {
            dear_old_dad(bankAccount, mutex);
            exit(0);
        }
    }
    if (num_parents >= 2) {
        if (fork() == 0) {
            lovable_mom(bankAccount, mutex);
            exit(0);
        }
    }

    // Fork Children (Poor Students)
    for (int i = 0; i < num_students; i++) {
        if (fork() == 0) {
            poor_student(bankAccount, mutex, i);
            exit(0);
        }
    }

    // Parent process waits for children to finish (they loop indefinitely, so we rely on manual kill)
    for (int i = 0; i < num_parents + num_students; i++) {
        wait(NULL);
    }

    return 0;
}
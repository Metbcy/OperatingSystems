#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>

#define SHMKEY_BANK_ACCOUNT 1234
#define SHMKEY_TURN 5678

typedef struct {
    int BankAccount;
    int Turn;
} shared_mem;

void depositMoney(shared_mem *shared, int balance) {
    if (balance % 2 == 0) {
        shared->BankAccount += balance;
        printf("Dear old Dad: Deposits $%d / Balance = $%d\n", balance, shared->BankAccount);
    } 
    else {
        printf("Dear old Dad: Doesn't have any money to give\n");
    }
    shared->Turn = 1;
}

void withdrawMoney(shared_mem *shared, int balance) {
    if (balance <= shared->BankAccount) {
        shared->BankAccount -= balance;
        printf("Poor Student: Withdraws $%d / Balance = $%d\n", balance, shared->BankAccount);
    }
    else {
        printf("Poor Student: Not Enough Cash ($%d)\n", shared->BankAccount);
    }
    shared->Turn = 0;
}

int main() {
    int shmid_bank, shmid_turn;
    shared_mem *shared;

    key_t key_bank = ftok("bank_account", SHMKEY_BANK_ACCOUNT);
    key_t key_turn = ftok("turn", SHMKEY_TURN);

    if ((shmid_bank = shmget(key_bank, sizeof(int), IPC_CREAT | 0666)) < 0) {
        perror("shmget (bank account)");
        exit(1);
    }

    if ((shmid_turn = shmget(key_turn, sizeof(int), IPC_CREAT | 0666)) < 0) {
        perror("shmget (turn)");
        exit(1);
    }

    shared = (shared_mem *) shmat(shmid_bank, NULL, 0);
    shared->BankAccount = 0;
    shared->Turn = 0;

    pid_t pid;
    pid = fork();

    if (pid == 0) {
        int i;
        for (i = 0; i < 25; i++) {
            sleep(rand() % 6);
            int balance = rand() % 51;
            printf("Poor Student needs $%d\n", balance);
            while (shared->Turn != 1) {
                // Do nothing
            }
            withdrawMoney(shared, balance);
        }
    }
    else {
        int i;
        for (i = 0; i < 25; i++) {
            sleep(rand() % 6);
            int balance = rand() % 101;
            int account = shared->BankAccount;
            while (shared->Turn != 0) {
                // Do nothing
            }
            if (shared->BankAccount <= 100) {
                depositMoney(shared, balance);
            }
            else {
                printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account);
            }
        }
        wait(NULL);
        shmdt(shared);
        shmctl(shmid_bank, IPC_RMID, NULL);
        shmctl(shmid_turn, IPC_RMID, NULL);
    }
    return 0;
}

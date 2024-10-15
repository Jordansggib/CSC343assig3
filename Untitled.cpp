#include <iostream>
#include <sys/ipc.h>  // For IPC keys
#include <sys/msg.h>  // For message queues
#include <unistd.h>   // For fork()
#include <cstdlib>    // For rand() and srand()
#include <ctime>      // For time()
#include <climits>    // For INT_MAX

#define SIZE 20

// Structure for message queue
struct message_buffer {
    long message_type;
    int min_value;
};

// Function to find the minimum in a given part of the array
int findMin(int arr[], int start, int end) {
    int minVal = INT_MAX;
    for (int i = start; i <= end; ++i) {
        if (arr[i] < minVal) {
            minVal = arr[i];
        }
    }
    return minVal;
}

int main() {
    int arr[SIZE];
    srand(time(0));

    // Filling the array with random numbers
    for (int i = 0; i < SIZE; i++) {
        arr[i] = rand() % 100;  // Random numbers between 0 and 99
    }

    // Create a message queue
    key_t key = ftok("min_queue", 65);  // Unique key for the message queue
    int msgid = msgget(key, 0666 | IPC_CREAT);  // Create message queue and return id

    pid_t pid = fork();  // Create a child process

    if (pid < 0) {
        std::cerr << "Fork failed!" << std::endl;
        return 1;
    } 
    else if (pid == 0) {  // Child process
        // Find the minimum in the second half of the array
        int childMin = findMin(arr, SIZE / 2, SIZE - 1);
        std::cout << "Child Process (PID: " << getpid() << ") found min: " << childMin << std::endl;

        // Send the result to the parent process using message queue
        message_buffer message;
        message.message_type = 1;
        message.min_value = childMin;
        msgsnd(msgid, &message, sizeof(message.min_value), 0);

        exit(0);  // Exit the child process
    } 
    else {  // Parent process
        // Find the minimum in the first half of the array
        int parentMin = findMin(arr, 0, (SIZE / 2) - 1);
        std::cout << "Parent Process (PID: " << getpid() << ") found min: " << parentMin << std::endl;

        // Wait for the child to send its message
        message_buffer message;
        msgrcv(msgid, &message, sizeof(message.min_value), 1, 0);

        // Calculate the overall minimum
        int overallMin = (parentMin < message.min_value) ? parentMin : message.min_value;
        std::cout << "Overall minimum number in the array: " << overallMin << std::endl;

        // Destroy the message queue
        msgctl(msgid, IPC_RMID, nullptr);
    }

    return 0;
}

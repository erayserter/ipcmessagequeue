#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/types.h>

void sendAndGetMessagesFromId(char* identity);
void parceAndExecuteCommand(char* command);
void createMailBox(char* identity);
void readMessage();
void sendMessage();
void programExit();
char* listen(char* pathname, int projectId);
void send(char* message, char* pathname, int projectId, int messageType);
bool isStringsEqual(char string1[], char string2[]);

struct MessageStruct {
    long messageType;
    char messageText[1024];
    char messageTo[512];
} MessageStruct;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        puts("You should specisify an identity.");
        return 0;
    }

    char* identity = argv[1];

    sendAndGetMessagesFromId(identity);

    return 0;
}

void sendAndGetMessagesFromId(char* identity) {
    createMailBox(identity);
    
    while (true)
    {
        char command[64] = "";
        puts("Commands:\n\t1)\"read\" - Reads received messages\n\t2)\"send\" - Sends entered message to entered client\n\t3)\"exit\" - Terminates the process");
        scanf("%s", command);
        getchar();

        printf("-------------------------------------------------\n\n");

        parceAndExecuteCommand(command);

        printf("\n-------------------------------------------------\n");
    }
}

void parceAndExecuteCommand(char* command) {
    if (isStringsEqual(command, "read")) {
        readMessage();
    } else if (isStringsEqual(command, "send")) {
        sendMessage();
    } else if (isStringsEqual(command, "exit")) {
        programExit();
    } else {
        puts("Unknown Command Exception.");
        exit(-1);
    }
}

void createMailBox(char* identity) {
    char message[1024];
    snprintf(message, sizeof(message), "%d:%s", getpid(), identity);
    send(message, "./server.c", 65, 1);
}

void readMessage() {
    char* message = listen("./client.c", getpid());
    printf("%s\n", message);
}

void sendMessage() {
    char identifier[16];
    char message[1024];
    char factoriedMessage[1024];
    printf("Identifier to send message: ");
    fgets(identifier, 16, stdin);
    identifier[strcspn(identifier, "\n")] = 0;

    strcpy(MessageStruct.messageTo, identifier);

    printf("Message to send: ");
    fgets(message, 1024, stdin);
    message[strcspn(message, "\n")] = 0;

    send(message, "./client.c", getpid(), 1);
}

void programExit() {
    puts("Process successfully terminated.");
    exit(0);
}

char* listen(char* pathname, int projectId) {
    key_t key = ftok(pathname, projectId);
    int messageID = msgget(key, 0666 | IPC_CREAT);


    uint hasMessage = 1;
    static char message[4096];

    while(hasMessage) {
        struct msqid_ds buf;
        int rc = msgctl(messageID, IPC_STAT, &buf);

        msgrcv(messageID, &MessageStruct, sizeof(MessageStruct), 2, 0);
        strcat(message, MessageStruct.messageText);
        strcat(message, "\n");
        hasMessage = (uint)(buf.msg_qnum);
    }

    return message;
}

void send(char* message, char* pathname, int projectId, int messageType) {
    key_t key = ftok(pathname, projectId);
    int messageID = msgget(key, 0666 | IPC_CREAT);

    strcpy(MessageStruct.messageText, message);
    MessageStruct.messageType = messageType;

    msgsnd(messageID, &MessageStruct, sizeof(MessageStruct), 0);
}

bool isStringsEqual(char string1[], char string2[]) {
    return strcmp(string1, string2) == 0;
}

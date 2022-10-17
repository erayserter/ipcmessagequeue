#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <pthread.h>

void listenMainMailBox();
void createCommunicationThreadFor(char* pID);
void* communicateWith(void* pID);
char* listen(char* pathname, int projectId);
void send(char* message, char* pathname, int projectId, int messageType);
char* findLinkedList(char* identifier);
void putLinkedList(char* pID, char* identifier);
bool isStringsEqual(char string1[], char string2[]);

struct MessageStruct {
    long messageType;
    char messageText[1024];
    char messageTo[512];
} MessageStruct;

struct Node {
    struct Node *next;
    char* pID;
    char* identifier;
};

static struct Node* identifierLinkedList;

int main () {
    while (true)
        listenMainMailBox();

    return 0;
}

void listenMainMailBox() {
    char* message = listen("./server.c", 65);

    char* pID = strtok(message, ":");
    char* identifier = strtok(NULL, ":");

    if (findLinkedList(identifier) == NULL) {
        putLinkedList(pID, identifier);
        createCommunicationThreadFor(pID);
    }   
}

void createCommunicationThreadFor(char* pID) {
    pthread_t tid;
    pthread_create(&tid, NULL, communicateWith, (void *)pID);
}

void* communicateWith(void* pID) {
    int id = atoi((char*)pID);
    while(true) {
        char* message = listen("./client.c", id);

        char* processID = findLinkedList(MessageStruct.messageTo);

        if (processID == NULL)
            continue;

        int destinationPID = atoi(processID);

        key_t key = ftok("./client.c", destinationPID);
        int messageID = msgget(key, 0666 | IPC_CREAT);
        MessageStruct.messageType = 2;
        msgsnd(messageID, &MessageStruct, sizeof(MessageStruct), 0);
    }
    return pID;
}

char* listen(char* pathname, int projectId) {
    key_t key = ftok(pathname, projectId);
    int messageID = msgget(key, 0666 | IPC_CREAT);

    msgrcv(messageID, &MessageStruct, sizeof(MessageStruct), 1, 0);

    return MessageStruct.messageText;
}

void send(char* message, char* pathname, int projectId, int messageType) {
    key_t key = ftok("./client.c", projectId);
    int messageID = msgget(key, 0666 | IPC_CREAT);

    strcpy(MessageStruct.messageText, message);
    MessageStruct.messageType = messageType;

    msgsnd(messageID, &MessageStruct, sizeof(MessageStruct), 0);
}

char* findLinkedList(char* identifier) {
    struct Node* head = identifierLinkedList;
    while(head != NULL) {
        if (isStringsEqual(identifier, head->identifier))
            return head->pID;
        head = head->next;
    }
    return NULL;
}

void putLinkedList(char* pID, char* identifier) {
    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    
    newNode->pID = (char*)malloc(sizeof(pID));
    newNode->identifier = (char*)malloc(sizeof(identifier));

    strcpy(newNode->pID, pID);
    strcpy(newNode->identifier, identifier);

    struct Node** headReferance = &identifierLinkedList;

    newNode->next = *headReferance;

    *headReferance = newNode;
}

bool isStringsEqual(char string1[], char string2[]) {
    return strcmp(string1, string2) == 0;
}
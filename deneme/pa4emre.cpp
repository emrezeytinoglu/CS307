
#ifndef ALLOCATOR_CPP
#define ALLOCATOR_CPP
#include <iostream>
#include <pthread.h>
#include <unistd.h>
using namespace std;


pthread_mutex_t lockMalloc = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lockFree = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lockPrint = PTHREAD_MUTEX_INITIALIZER;

struct HeapNode{
    int id, size, index;
    HeapNode *next;
    HeapNode() : next(NULL) {}
    HeapNode(int idParam, int sizeParam, int indexParam) : id(idParam), size(sizeParam), index(indexParam), next(NULL) {}
};

class HeapManager{
    HeapNode *head; 

public:
    HeapManager() : head(NULL) {}
    ~HeapManager();
    int initHeap(int size);
    int myMalloc(int ID, int size);
    int myFree(int ID, int index);
    void print();
};

int HeapManager::initHeap(int size){
    head = new HeapNode(-1, size, 0);
    print();
    return 1;
}

int HeapManager::myMalloc(int ID, int size){
    pthread_mutex_lock(&lockMalloc); 
    HeapNode *current = head;
    while (current != NULL) {
        if (current->id == -1 && current->size >= size) {
            HeapNode *lastCreatedNode = new node(ID, size, current->index);
            lastCreatedNode->next = current->next;
            current->next = lastCreatedNode;
            current->size -= size;
            current->index += size;
            cout << "Allocated for thread " << ID << endl;
            print();
            pthread_mutex_unlock(&lockMalloc); 
            return lastCreatedNode->index;
        }
        current = current->next;
    }

    cout << "Can not allocate, requested size " << size << " for thread " << ID << " is bigger than remaining size" << endl;
    pthread_mutex_unlock(&lock); 
    return -1;
}

int HeapManager::myFree(int ID, int index){
    pthread_mutex_lock(&lockFree); 
    bool isNodeFound = false;
    HeapNode *current = head;
    while (current != NULL) {
        if (current->id == ID && current->index == index) {
            current->id = -1;
            isNodeFound = true;
            break;
        }
        current = current->next;
    }
    current = head;
    while (current != NULL && current->next != NULL) {
        if (current->id == -1 && current->next->id == -1) {
            HeapNode *temp = current->next;
            current->size += temp->size;
            current->next = temp->next;
            delete temp;
        } else {
            current = current->next;
        }
    }

    cout << "Freed for thread " << ID << endl;
    print();
    pthread_mutex_unlock(&lockFree); 
    return isNodeFound ? 1 : -1;
}


void HeapManager::print(){
    pthread_mutex_lock(&lockPrint); 
    HeapNode *temp = head;
    while (temp != NULL) {
        cout << "[" << temp->id << "][" << temp->size << "][" << temp->index << "]";
        if (temp->next != NULL)
            cout << "---";
        else
            cout << endl;
        temp = temp->next;
    }
    pthread_mutex_unlock(&lockPrint); 
}

HeapManager::~HeapManager() {
    while (head != NULL) {
        HeapNode *temp = head;
        head = head->next;
        delete temp;
    }
}

#endif

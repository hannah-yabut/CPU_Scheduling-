/*
 * Student Name: Hannah Yabut and Ismael Robleh 
 * Student ID:   3131432 
 * Date:         ____________________________________________
 * File:         ____________________________________________
 * Description:  ____________________________________________
 */
#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct Node {
    int pid;
    int run_time;
    struct Node* next;
} Node;

typedef struct {
    Node* front;
    Node* rear;
    int size;
} Queue;

void initialize_queue(Queue* q) {
    q->front = NULL;
    q->rear = NULL;
    q->size = 0;
}

bool is_empty(Queue* q) {
    return q->front == NULL;
}

void enqueue(Queue* q, int pid) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (new_node == NULL) {
        printf("Memory allocation failed!\n");
        return;
    }
    new_node->pid = pid;
    new_node->next = NULL;

    if (is_empty(q)) {
        q->front = new_node;
        q->rear = new_node;
    }
    else {
        q->rear->next = new_node;
        q->rear = new_node;
    }
    q->size++;
}

int dequeue(Queue* q) {
    if (is_empty(q)) {
        printf("Cannot dequeue in empty queue!");
        return -1;
    }
    Node* temp = q->front;
    int value = temp->pid;
    q->front = q->front->next;
    if (q->front == NULL) {
        q->rear = NULL;
    }
    free(temp);
    q->size--;
    return value;
}

int get_front(Queue* q) {
    if (is_empty(q)) {
        printf("Queue is empty!\n");
        return -1;
    }
    return q->front->pid;
}

void free_queue(Queue* q) {
    while (!is_empty(q)) {
        dequeue(q);
    }
}

#endif


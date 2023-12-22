#pragma once

#include "queue_node.hpp"
#include <pthread.h>

class SCSPQueue {
  public:
    SCSPQueue() {
        head = nullptr;

        pthread_mutex_init(&read_mutex, NULL);
        pthread_mutex_init(&write_mutex, NULL);
    }

    ~SCSPQueue() {
        QueueNode *node = head;
        while (node) {
            QueueNode *next = node->next;
            delete node;
            node = next;
        }

        pthread_mutex_destroy(&read_mutex);
        pthread_mutex_destroy(&write_mutex);
    }

    void enqueue(int val) {
        pthread_mutex_lock(&write_mutex);

        ++size;

        if (head == nullptr) {
            head = new QueueNode{val, nullptr};
        } else {
            QueueNode *node = head;
            while (node->next) {
                node = node->next;
            }
            node->next = new QueueNode{val, nullptr};
        }

        pthread_mutex_unlock(&write_mutex);
    }

    int dequeue() {
        pthread_mutex_lock(&read_mutex);

        int ret_val = 0;

        --size;

        if (head != nullptr) {
            QueueNode *node = head;
            head = head->next;

            ret_val = node->val;
            delete node;
        }

        pthread_mutex_unlock(&read_mutex);

        return ret_val;
    }

    bool empty() { return size == 0; }

  private:
    pthread_mutex_t read_mutex;
    pthread_mutex_t write_mutex;

    QueueNode *head;

    size_t size = 0;
};
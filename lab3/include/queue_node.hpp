#pragma once
#include <pthread.h>

class QueueNode {
  public:
    QueueNode(int val, QueueNode *next) : val(val), next(next) { pthread_mutex_init(&mutex, NULL); }
    ~QueueNode() {
        pthread_mutex_lock(&mutex);
        pthread_mutex_destroy(&mutex);
    }

    int get_val() {
        pthread_mutex_lock(&mutex);
        int ret_val = val;
        pthread_mutex_unlock(&mutex);

        return ret_val;
    }

    QueueNode *get_next() {
        pthread_mutex_lock(&mutex);
        QueueNode *ret_val = next;
        pthread_mutex_unlock(&mutex);

        return ret_val;
    }

    void set_next(QueueNode *next) {
        pthread_mutex_lock(&mutex);
        this->next = next;
        pthread_mutex_unlock(&mutex);
    }

  private:
    int val;
    QueueNode *next;
    pthread_mutex_t mutex;
};

//
// Created by samuele on 02/12/17.
//

#ifndef BIGRAMMI_PARQUEUE_H
#define BIGRAMMI_PARQUEUE_H



#include <iostream>
#include <cstring>
#include <mutex>


#include "SeqQueue.h"



template <typename T>
class ParQueue : public SeqQueue<T>{
private:
    std::mutex tailLock;
    std::mutex headLock;

public:
    void enqueue(std::string words[NGRAM_LENGTH]) {
        NodeTemplate<T>* tmp = new NodeTemplate<T>();
        for(int i = 0;i < NGRAM_LENGTH; i++)
            tmp->value[i] = words[i];
        tmp->counter = 1;
        tmp->prev = nullptr;

        tailLock.lock();
        if(SeqQueue<T>::tail == nullptr){ //devo gestire anche head dato che head == tail == nullptr
            tmp->next  = nullptr;
            SeqQueue<T>::tail = tmp;
            headLock.lock();
            SeqQueue<T>::head = tmp;
            headLock.unlock();
        }
        else{
            tmp->next  = SeqQueue<T>::tail;
            SeqQueue<T>::tail->prev = tmp;
            SeqQueue<T>::tail = tmp;

        }
        SeqQueue<T>::enqueued++;
        tailLock.unlock();
    }
    void enqueue(std::string line){
        NodeTemplate<T>* tmp = new NodeTemplate<T>();
        tmp->value = line;
        tmp->prev = nullptr;

        tailLock.lock();
        tmp->next = SeqQueue<T>::tail;

        if(SeqQueue<T>::tail != nullptr)
            SeqQueue<T>::tail->prev = tmp;

        SeqQueue<T>::tail = tmp;

        if(SeqQueue<T>::head == nullptr)
            SeqQueue<T>::head = SeqQueue<T>::tail;

        SeqQueue<T>::enqueued++;

        tailLock.unlock();

    }
    bool dequeue(NodeTemplate<T>& result) {
        headLock.lock();
        if(SeqQueue<T>::head == SeqQueue<T>::tail)
            tailLock.lock();
        NodeTemplate<T>* tmp = SeqQueue<T>::head;
        if (tmp == 0){
            headLock.unlock();
            tailLock.unlock();
            return false; // queue was empty
        }

        if(SeqQueue<T>::head == SeqQueue<T>::tail)
            SeqQueue<T>::tail = nullptr;
        SeqQueue<T>::head = SeqQueue<T>::head -> prev;
        result = *tmp;

        SeqQueue<T>::dequeued++;
        headLock.unlock();
        if(SeqQueue<T>::head == SeqQueue<T>::tail)
            tailLock.unlock();
        return true;
    }
    void updateQueue(std::string words[NGRAM_LENGTH]){
        headLock.lock();
        NodeTemplate<T>* tmp = SeqQueue<T>::head;

        if(tmp == 0) {
            /* devo controllare "atomicamente" che effettivamente ancora nessun elemento
             * è stato inserito, per evitare di fare enqueue senza aver tenuto di conto
             * gli elementi che potrebbero essere stati aggiunti da thread più veloci */
            SeqQueue<T>::enqueue(words);
            headLock.unlock();
            return;
        }
        headLock.unlock();
        while(true){
            if(compareArraysOfStrings(tmp->value,words,NGRAM_LENGTH)){
                tmp -> counter++; //andrebbe gestito atomicamente?
                return;
            }
            else if(tmp == SeqQueue<T>::tail){
                /*devo controllare di essere effettivamente in coda e che nessuno
                 * abbia aggiunto altri elementi, eseguendo quindi una enqueue
                 * senza aver controllato i nuovi elementi aggiunti */
                enqueue(words);
                return;
            }
            else
                tmp = tmp ->prev;
        }

    }

};



#endif //BIGRAMMI_PARQUEUE_H
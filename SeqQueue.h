//
// Created by samuele on 12/11/17.
//

#ifndef BIGRAMMI_CLASSES_H
#define BIGRAMMI_CLASSES_H



#include <iostream>
#include <cstring>
#include <mutex>
#include <ctime>

#include "NodeTemplate.h"


template <typename T>
class SeqQueue{
protected:
    NodeTemplate<T>* head;
    NodeTemplate<T>* tail;
    int enqueued;
    int dequeued;

public:
    SeqQueue(){
        head = tail = nullptr;
        enqueued = dequeued = 0;
    }
    ~SeqQueue(){
        NodeTemplate<T>* curr = head;
        NodeTemplate<T>* tmp;

        while (curr != 0) {
            tmp = curr;
            curr = curr->next;
            delete(tmp);
        }
        enqueued = dequeued = 0;
        head = tail = 0;
    }
    void enqueue(std::string words[NGRAM_LENGTH]) {
        NodeTemplate<T>* tmp = new NodeTemplate<T>();
        for(int i = 0;i < NGRAM_LENGTH; i++)
            tmp->value[i] = words[i];
        tmp->counter = 1;
        tmp->next  = tail;
        tmp->prev = nullptr;
        if(tail != nullptr)
            tail->prev = tmp;
        tail = tmp;
        if(head == nullptr)
            head = tail;
        enqueued++;
    }
    bool dequeue(NodeTemplate<T>& result) {
        NodeTemplate<T>* tmp = head;
        if (tmp == 0){
            return false; // queue was empty
        }

        if(head == tail)
            tail = nullptr;
        head = head -> prev;
        result = *tmp;

        dequeued++;

        return true;
    }
    void updateQueue(std::string words[NGRAM_LENGTH]){
        NodeTemplate<T>* tmp = head;

        if(tmp == 0) {
            enqueue(words);
            return;
        }

        while(true){
            if(compareArraysOfStrings(tmp->value,words,NGRAM_LENGTH)){
                tmp -> counter++;
                return;
            }
            else if(tmp == tail){
                enqueue(words);
                return;
            }
            else
                tmp = tmp ->prev;
        }
    }
    void printQueue(std::ofstream& output){
        std::time_t curtime = std::time(nullptr);
        NodeTemplate<T>* tmp = tail;
        while(tmp != 0) {
            for(int i = 0;i < NGRAM_LENGTH; i++) {
                //std::cout << tmp->value[i] << " ";
                output << tmp->value[i] << " ";
            }
            //std::cout << ": " << tmp -> counter << std::endl;
            output << ": " << tmp -> counter << std::endl;
            tmp = tmp->next;
        }
        //std::cout << enqueued << " gruppi trovati."<< std::endl;
        output << enqueued << " elementi trovati."<< std::endl;
        //output << std::asctime(std::localtime(&curtime)) << std::endl;
        delete(tmp);
    }
    void printLines(){
        NodeTemplate<T>* tmp = tail;
        while(tmp != 0) {
            std::cout << ": " << tmp -> counter << std::endl;
            tmp = tmp->next;
        }
        std::cout << enqueued << " gruppi trovati."<< std::endl;
        delete(tmp);
    }
    bool empty(){ return (head == nullptr); }
    int getEnqueued(){ return enqueued; }
    int getDequeued(){ return dequeued; }

};


#endif //BIGRAMMI_CLASSES_H
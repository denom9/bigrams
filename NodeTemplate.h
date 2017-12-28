//
// Created by samuele on 02/12/17.
//

#ifndef BIGRAMMI_NODETEMPLATE_H
#define BIGRAMMI_NODETEMPLATE_H




template <typename T>
class NodeTemplate{
public:
    T value;
    int counter;
    NodeTemplate<T>* next;
    NodeTemplate<T>* prev;
};


#endif //BIGRAMMI_NODETEMPLATE_H
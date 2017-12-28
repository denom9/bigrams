//
// Created by samuele on 15/11/17.
//

#ifndef BIGRAMMI_THREADS_H
#define BIGRAMMI_THREADS_H

#endif //BIGRAMMI_THREADS_H


#include <fstream>
#include <atomic>
#include <condition_variable>
#include <chrono>
#include <thread>
#include <cmath>


std::mutex streamAccess;
std::mutex cond;
std::mutex hashLettersAccess,hashWordsAccess;
std::condition_variable waitLine;

std::atomic_int numProducers(0);


bool computeWords(char a, std::string *group, std::string &tmpString, int &index){

    if (isalpha(a)) {
        if (isupper(a))
            a = tolower(a);
        tmpString += a;
    }
    else{
        if(a == '\'')
            tmpString += a;
        if(!(isgroupterminator(a) && (index + 1) < NGRAM_LENGTH ) && !tmpString.empty()) {
            group[index] = tmpString;
            index++;
        }
        else
            index = 0;

        tmpString = "";
    }
    return (index == NGRAM_LENGTH); //group found
}
bool computeLetters(char a,char *group, int &index){

    if (isalpha(a)) {
        if (isupper(a))
            a = tolower(a);
        group[index] = a;
        index++;
    }
    else
        index = 0;

    return (index == NGRAM_LENGTH); //group found
}


void consume(int consumer_id,ParQueue<std::string>& linesQueue,ParQueue<std::string[NGRAM_LENGTH]>& resultQueue) {
    printf("Hello from consumer %i\n",consumer_id);
    NodeTemplate<std::string> tmp;



    std::string ngram[NGRAM_LENGTH];
    std::string tmpString = "";
    int index = 0;

    while(linesQueue.getEnqueued() == 0 || linesQueue.getEnqueued() - linesQueue.getDequeued() > 0 || numProducers > 0){

        std::unique_lock<std::mutex> lk(cond);
        waitLine.wait(lk, [&linesQueue,consumer_id] {
            return !((linesQueue.empty() && numProducers > 0) || linesQueue.getEnqueued() == 0);
        });

        linesQueue.dequeue(tmp);
        lk.unlock();


        //computeLine(tmp.value, resultQueue);

        for(int i = 0; i < tmp.value.length(); i++){

            if (computeWords(tmp.value[i], ngram, tmpString, index)) {
                resultQueue.updateQueue(ngram);
                if (isgroupterminator(tmp.value[i]))
                    index = 0;
                else {
                    index = NGRAM_LENGTH - 1;
                    shiftArrayofStrings(ngram);
                }
            }
        }

    }
    printf("Consumer %i: terminated.\n",consumer_id);
    waitLine.notify_all(); //notifico per risvegliare eventuali consumer in attesa quando tutti i producer sono terminati
}
void produce(int producer_id,ParQueue<std::string>& linesQueue,std::ifstream& reader) {
    numProducers++;
    printf("Hello from producer %i\n",producer_id);

    std::string line;
    char nextChar;
    int foundSpaces = 0;
    bool lineFound = false;

    while(true){
        streamAccess.lock();
        if(reader.peek() == EOF) {
            streamAccess.unlock();
            break;
        }


        /* zona critica: utilizzo del fstream */
        while(reader.get(nextChar)){
            if(isspace(nextChar))
                foundSpaces++;

            if((nextChar=='\n')||(reader.peek() == EOF)){
                if((foundSpaces >= NGRAM_LENGTH - 1)&& (line.length() >= NGRAM_LENGTH + 1))
                    lineFound = true;
                break;
            }
            else
                line += nextChar;
        }
        /* uscita zona critica */
        streamAccess.unlock();

        if(lineFound){
            linesQueue.enqueue(line);
            waitLine.notify_one();
            //printf("Producer %i: enqueued: %s\n",producer_id,line.c_str());
            line = "";
            lineFound = false;
            foundSpaces = 0;
        }



    }
    printf("Producer %i: terminated.\n",producer_id);

    numProducers--;
    waitLine.notify_all();
}





void work(int thread_id, long int file_size,const char* INPUT_PATH,std::unordered_map<std::string,int> &letters_hashtable1,std::unordered_map<std::string,int> &words_hashtable1){
    /*DEBUGGING VARIABLES*/

    //std::ofstream deb;
    //std::string output_path = "../files/parts/part" + std::to_string(thread_id) + ".txt";
    //deb.open(output_path, std::ios::binary);
    /*-------------*/

    std::unordered_map<std::string,int> words_hashtable;
    std::unordered_map<std::string,int> letters_hashtable;


    std::ifstream reader;
    reader.open(INPUT_PATH,std::ios::binary);

    std::string tmpString = "";
    std::string words_key = "";
    std::string words_ngram[NGRAM_LENGTH];
    int index = 0,letters_index = 0, tmpValue = 0;
    char nextChar,letters_ngram[NGRAM_LENGTH + 1] = "";

    /* adjusting start and end positions */
    long int start_position = floor(file_size / NUM_WORKERS) * thread_id;
    if(start_position > 0) {
        reader.seekg(--start_position);
        reader.get(nextChar);
        if(!isgroupterminator(nextChar)) {
            do{
                reader.get(nextChar);
                start_position++;
            }while (!isgroupterminator(nextChar));
        }
        start_position++;
    }
    long int end_position = ceil(file_size / NUM_WORKERS) * (thread_id + 1);
    //printf("Worker %i: initial end_position:%li\n",thread_id,end_position);
    if(end_position < file_size) {
        reader.seekg(--end_position);
        reader.get(nextChar);
        end_position++;
        //printf("Worker %i: nextChar: %c, end_position: %li\n",thread_id,nextChar,end_position);

        if (!isgroupterminator(nextChar) && (end_position < file_size - 1)) {
            if(thread_id == NUM_WORKERS - 1)
                end_position = file_size - 1;
            else {
                do {
                    reader.get(nextChar);
                    end_position++;
                    //if(thread_id == NUM_WORKERS - 1)
                    //  printf("Worker %i: nextChar: %c, end_position: %i\n",thread_id,nextChar,end_position);
                } while (!isgroupterminator(nextChar));
            }
        }

    }

    //printf("Worker %i: start_position: %li, end_position:%li\n",thread_id,start_position,end_position);


    reader.seekg(start_position);


    if ((reader.peek() == EOF) || (int)reader.tellg() >= end_position || (end_position - start_position == 0)){
        //printf("Worker %i: terminated\n",thread_id);
        return;
    }



    /* computing own part */



    for (int i = 0; i < end_position - start_position; i++) {
        reader.get(nextChar);
        //line += nextChar; //DEBUGGING

        /*
        if(computeWords(nextChar, words_ngram, tmpString, index)){
            resultQueue.updateQueue(words_ngram);
            //printf("Worker %i computed: %s %s\n",thread_id,words_ngram[0].c_str(),words_ngram[1].c_str());
            if (isgroupterminator(nextChar))
                index = 0;
            else {
                index = NGRAM_LENGTH - 1;
                shiftArrayofStrings(words_ngram);
            }
        }

*/
        if(computeWords(nextChar, words_ngram, tmpString, index)){
            words_key = words_ngram[0] + " " + words_ngram[1];
            hashWordsAccess.lock();
            auto mapIterator = words_hashtable.find(words_key);
            if(mapIterator == words_hashtable.end())
                words_hashtable.emplace(words_key,1);
            else{
                tmpValue = mapIterator -> second;
                tmpValue++;
                mapIterator->second = tmpValue;
            }
            hashWordsAccess.unlock();

            if (isgroupterminator(nextChar))
                index = 0;
            else {
                index = NGRAM_LENGTH - 1;
                shiftArrayofStrings(words_ngram);
            }
        }

        if(computeLetters(nextChar,letters_ngram,letters_index)){
            hashLettersAccess.lock();
            auto mapIterator = letters_hashtable.find(letters_ngram);
            if(mapIterator == letters_hashtable.end())
                letters_hashtable.emplace(letters_ngram,1);
            else{
                tmpValue = mapIterator -> second;
                tmpValue++;
                mapIterator->second = tmpValue;
            }
            hashLettersAccess.unlock();

            letters_ngram[0] = letters_ngram[1];
            letters_index = 1;
        }
    }

    //printf("Worker %i: terminated\n",thread_id);

/*
    deb << line; //DEBUGGING
    deb.close(); //DEBUGGING
*/
}
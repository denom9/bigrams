#include <iostream>
#include <fstream>
#include <unordered_map>
#include <omp.h>
#include <cmath>
#include <cstring>


#define NGRAM_LENGTH 2

#include "utilities.h"

int main(int argc, char* argv[]) {
    const char* INPUT_PATH = argv[1];
    const char* OUTPUT_PATH = argv[2];
    const int thread_num = *argv[3] - '0';
    omp_set_num_threads(thread_num);
    //const int thread_num = omp_get_max_threads();

    std::ifstream size,input;
    std::ofstream outputWords, outputLetters;

    int tid;
    char nextChar;
    char* buffer;

    long int tmpPosition;
    long int positions[thread_num+1];

    std::unordered_map<std::string,int> lettersHashtable[thread_num],wordsHashtable[thread_num];
    std::unordered_map<std::string,int> lettersReduce,wordsReduce;


    auto start = omp_get_wtime();

    //trovo la dimensione del file
    size.open(INPUT_PATH, std::ios::ate);
    const long int file_size = size.tellg();
    size.close();

    //leggo tutto il file nel buffer
    buffer = new char[file_size];
    input.open(INPUT_PATH,std::ios::binary);
    input.read(buffer,file_size);
    input.close();

    //trovo le porzioni da assegnare ai thread
    positions[0] = 0;
    for(int i = 1; i<thread_num+1;i++){
        if(i == omp_get_max_threads())
            positions[i] = file_size - 1;
        else {
            tmpPosition = floor((file_size - 1) / thread_num) * i;
            nextChar = buffer[tmpPosition-1];
            if (!isgroupterminator(nextChar)) {
                do {
                    nextChar = buffer[tmpPosition];
                    tmpPosition++;
                } while (!isgroupterminator(nextChar));
            }
            tmpPosition++;
            positions[i] = tmpPosition;
        }
    }


    #pragma omp parallel default(none) shared(lettersHashtable,wordsHashtable,positions,buffer)private(tid)
    {
        tid = omp_get_thread_num();


        std::string tmpString = "";
        std::string words_key = "";
        std::string words_ngram[NGRAM_LENGTH];
        int index = 0,letters_index = 0;
        char nextChar,letters_ngram[NGRAM_LENGTH + 1] = "";

        long int start_position = positions[tid];
        long int end_position = positions[tid+1];

        //printf("thread %i, positions: %li-%li\n",tid,start_position,end_position);

            for (int i = start_position; i < end_position; i++) {
                nextChar = buffer[i];
                if (computeWords(nextChar, words_ngram, tmpString, index)) {
                    words_key = words_ngram[0] + " " + words_ngram[1];
                    wordsHashtable[tid][words_key] += 1;

                    if (isgroupterminator(nextChar))
                        index = 0;
                    else {
                        index = NGRAM_LENGTH - 1;
                        shiftArrayofStrings(words_ngram);
                    }
                }

                if (computeLetters(nextChar, letters_ngram, letters_index)) {
                    lettersHashtable[tid][letters_ngram] += 1;

                    letters_ngram[0] = letters_ngram[1];
                    letters_index = 1;
                }
            }
    };



    /* REDUCE STEP */
    for(int i = 0; i < thread_num; i++){
        for(auto r : lettersHashtable[i])
            lettersReduce[r.first] += r.second;
        for(auto r : wordsHashtable[i])
            wordsReduce[r.first] += r.second;
    }

    double elapsedTime = omp_get_wtime() - start;


    /* PRINT RESULTS */

    outputWords.open((std::string)OUTPUT_PATH + "parallel_output_words.txt", std::ios::binary);
    outputLetters.open((std::string)OUTPUT_PATH + "parallel_output_letters.txt", std::ios::binary);

    for ( auto mapIterator =lettersReduce.begin(); mapIterator != lettersReduce.end(); ++mapIterator )
        outputLetters << mapIterator->first << " : " << mapIterator->second<<std::endl;
    outputLetters << lettersReduce.size() << " elementi trovati." << std::endl;

    for ( auto mapIterator =wordsReduce.begin(); mapIterator != wordsReduce.end(); ++mapIterator )
        outputWords << mapIterator->first << " : " << mapIterator->second<<std::endl;
    outputWords << wordsReduce.size() << " elementi trovati." << std::endl;

    outputWords.close();
    outputLetters.close();

    std::cout << elapsedTime;


    return 0;
}
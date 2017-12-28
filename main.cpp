#include <iostream>
#include <fstream>
#include <thread>
#include <iomanip>
#include <locale>
#include <chrono>
#include <unordered_map>
#include <omp.h>
#include "parameters.h"
#include "utilities.h"

#include "ParQueue.h"
#include "threads.h"




int main(int argc, char* argv[]) {
    const char* INPUT_PATH = argv[1];

    std::ifstream sequentialInput,size;
    std::ofstream sequentialOutput_words,sequentialOutput_letters;
    std::ofstream parallelOutput_words, parallelOutput_letters;

    std::string ngram[NGRAM_LENGTH],tmpString = "",line,words_ngram;


    std::thread workThreads[NUM_WORKERS];

    /*
    SeqQueue<std::string[NGRAM_LENGTH]> sequentialResult_words;
    ParQueue<std::string> linesQueue;
    ParQueue<std::string[NGRAM_LENGTH]> parallelResult1;
    ParQueue<std::string[NGRAM_LENGTH]> partialResult2;
    ParQueue<std::string[NGRAM_LENGTH]> parallelResult2;
    */


    int words_index,letters_index,tid;
    char nextChar,letters_ngram[NGRAM_LENGTH + 1] = "";

    std::unordered_map<std::string,int> sequential_letters_hashtable,sequential_words_hashtable;
    std::unordered_map<std::string,int>::const_iterator sequential_mapIterator;
    std::unordered_map<std::string,int> parallel_letters_hashtable,parallel_words_hashtable;
    std::unordered_map<std::string,int>::const_iterator parallel_mapIterator;







    /* SEQUENTIAL SECTION BEGIN */
    //auto start = std::chrono::system_clock::now();
    double start = omp_get_wtime();
    printf("\n---- START SEQUENTIAL SECTION ----");


    sequentialInput.open(INPUT_PATH, std::ios::binary);




    if(sequentialInput.is_open()) {
        while(sequentialInput.get(nextChar)) {
            /*
            //printf("%c\n",nextChar);
            if (computeWords(nextChar, ngram, tmpString, words_index)) {
                sequentialResult_words.updateQueue(ngram);
                if (isgroupterminator(nextChar))
                    words_index = 0;
                else {
                    words_index = NGRAM_LENGTH - 1;
                    shiftArrayofStrings(ngram);
                }
            }
*/
            //NGRAMMI DI PAROLE
            if (computeWords(nextChar, ngram, tmpString, words_index)) {
                words_ngram = ngram[0] + " " + ngram[1];

                sequential_words_hashtable[words_ngram] += 1;
                /*
                auto sequential_mapIterator = sequential_words_hashtable.find(words_ngram);
                if(sequential_mapIterator == sequential_words_hashtable.end())
                    sequential_words_hashtable[words_ngram] = 1;
                else
                    sequential_words_hashtable[words_ngram]++;
                */
                if (isgroupterminator(nextChar))
                    words_index = 0;
                else {
                    words_index = NGRAM_LENGTH - 1;
                    shiftArrayofStrings(ngram);
                }
            }

            //NGRAMMI DI LETTERE
            if(computeLetters(nextChar,letters_ngram,letters_index)){
                sequential_letters_hashtable[letters_ngram] += 1;
                /*
                auto sequential_mapIterator = sequential_letters_hashtable.find(letters_ngram);
                if(sequential_mapIterator == sequential_letters_hashtable.end())
                    sequential_letters_hashtable[letters_ngram] = 1;
                else
                    sequential_letters_hashtable[letters_ngram]++;
                */
                letters_ngram[0] = letters_ngram[1];
                letters_index = 1;
            }
        }
    }



    sequentialInput.close();

    printf("\n---- END SEQUENTIAL SECTION ----\n\n");
    //auto end = std::chrono::system_clock::now();
    //std::chrono::duration<double> sequential_time = end-start;
    double sequential_time = omp_get_wtime() - start;
    /* SEQUENTIAL SECTION END */






    /* PARALLEL SECTION BEGIN - 2nd solution */
    printf("\n---- START PARALLEL SECTION ----");
    //start = std::chrono::system_clock::now();
    start = omp_get_wtime();



    size.open(INPUT_PATH, std::ios::ate);
    const long int file_size = size.tellg();


    #pragma omp parallel default(none) shared(INPUT_PATH,parallel_letters_hashtable,parallel_words_hashtable)private(tid)
    {
        tid = omp_get_thread_num();
        std::ifstream reader;
        reader.open(INPUT_PATH,std::ios::binary);

        std::string tmpString = "";
        std::string words_key = "";
        std::string words_ngram[NGRAM_LENGTH];
        int index = 0,letters_index = 0;
        char nextChar,letters_ngram[NGRAM_LENGTH + 1] = "";


        long int start_position = floor(file_size / NUM_WORKERS) * tid;
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
        long int end_position = ceil(file_size / NUM_WORKERS) * (tid + 1);
        //printf("Worker %i: initial end_position:%li\n",thread_id,end_position);
        if(end_position < file_size) {
            reader.seekg(--end_position);
            reader.get(nextChar);
            end_position++;
            //printf("Worker %i: nextChar: %c, end_position: %li\n",thread_id,nextChar,end_position);

            if (!isgroupterminator(nextChar) && (end_position < file_size - 1)) {
                if(tid == NUM_WORKERS - 1)
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

        //printf("Worker %i: start_position: %li, end_position:%li\n",tid,start_position,end_position);
        reader.seekg(start_position);


        if ((reader.peek() != EOF) && (int)reader.tellg() < end_position && (end_position - start_position > 0)) {

            for (int i = 0; i < end_position - start_position; i++) {
                reader.get(nextChar);

                if (computeWords(nextChar, words_ngram, tmpString, index)) {
                    words_key = words_ngram[0] + " " + words_ngram[1];
                    //printf("%i: %s\n",tid,words_key.c_str());
                    #pragma omp critical(wordsUpdate)
                        parallel_words_hashtable[words_key] += 1;

                    if (isgroupterminator(nextChar))
                        index = 0;
                    else {
                        index = NGRAM_LENGTH - 1;
                        shiftArrayofStrings(words_ngram);
                    }
                }

                if (computeLetters(nextChar, letters_ngram, letters_index)) {
                    #pragma omp critical(lettersUpdate)
                        parallel_letters_hashtable[letters_ngram] += 1;

                    letters_ngram[0] = letters_ngram[1];
                    letters_index = 1;
                }
            }
        }

    };





/*
    for(int i = 0; i < NUM_WORKERS; i++)
        workThreads[i] = std::thread(work, i, file_size, INPUT_PATH, std::ref(parallel_letters_hashtable), std::ref(parallel_words_hashtable));
    for(int i = 0; i < NUM_WORKERS; i++)
        workThreads[i].join();
*/


    double parallel_time = omp_get_wtime() - start;
    //end = std::chrono::system_clock::now();
    //std::chrono::duration<double> parallel_time = end-start;

    printf("\n---- END PARALLEL SECTION ----\n\n");
    /* PARALLEL SECTION END - 2nd solution */







    /* PRINT RESULTS */

    sequentialOutput_words.open("../files/sequential_output_words.txt", std::ios::binary);
    sequentialOutput_letters.open("../files/sequential_output_letters.txt", std::ios::binary);
    parallelOutput_words.open("../files/parallel_output_words.txt", std::ios::binary);
    parallelOutput_letters.open("../files/parallel_output_letters.txt", std::ios::binary);




    for ( auto sequential_mapIterator = sequential_words_hashtable.begin(); sequential_mapIterator != sequential_letters_hashtable.end(); ++sequential_mapIterator )
        sequentialOutput_words << sequential_mapIterator->first << " : " << sequential_mapIterator->second<<std::endl;
    sequentialOutput_words << sequential_words_hashtable.size() << " elementi trovati." << std::endl;

    for ( auto sequential_mapIterator = sequential_letters_hashtable.begin(); sequential_mapIterator != sequential_letters_hashtable.end(); ++sequential_mapIterator )
        sequentialOutput_letters << sequential_mapIterator->first << " : " << sequential_mapIterator->second<<std::endl;
    sequentialOutput_letters << sequential_letters_hashtable.size() << " elementi trovati." << std::endl;



    for ( auto parallel_mapIterator =parallel_letters_hashtable.begin(); parallel_mapIterator != parallel_letters_hashtable.end(); ++parallel_mapIterator )
        parallelOutput_letters << parallel_mapIterator->first << " : " << parallel_mapIterator->second<<std::endl;
    parallelOutput_letters << parallel_letters_hashtable.size() << " elementi trovati." << std::endl;

    for ( auto parallel_mapIterator =parallel_words_hashtable.begin(); parallel_mapIterator != parallel_words_hashtable.end(); ++parallel_mapIterator )
        parallelOutput_words << parallel_mapIterator->first << " : " << parallel_mapIterator->second<<std::endl;
    parallelOutput_words << parallel_words_hashtable.size() << " elementi trovati." << std::endl;


    sequentialOutput_words.close();
    sequentialOutput_letters.close();
    parallelOutput_words.close();
    parallelOutput_letters.close();


    std::cout << std::left << std::setw(20) << "Sequential time:" << sequential_time << std::endl;
    std::cout << std::left << std::setw(20) << "Parallel time:" << parallel_time << std::endl;


    return 0;
}
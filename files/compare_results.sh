#!/bin/bash
sort sequential_output_words.txt > comparing/sequential_output_words_sorted.txt
sort parallel_output_words.txt > comparing/parallel_output_words_sorted.txt

sort sequential_output_letters.txt > comparing/sequential_output_letters_sorted.txt
sort parallel_output_letters.txt > comparing/parallel_output_letters_sorted.txt

diff comparing/sequential_output_words_sorted.txt comparing/parallel_output_words_sorted.txt
diff comparing/sequential_output_letters_sorted.txt comparing/parallel_output_letters_sorted.txt

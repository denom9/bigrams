//
// Created by samuele on 02/12/17.
//


#ifndef BIGRAMMI_UTILITIES_H
#define BIGRAMMI_UTILITIES_H

#endif //BIGRAMMI_UTILITIES_H





bool compareArraysOfStrings(std::string a[], std::string b[],int length){
    for(int i = 0; i < length; i++)
        if(a[i]!=b[i]) return false;
    return true;
}

void shiftArrayofStrings(std::string array[NGRAM_LENGTH]){
    for(int i = 0; i < NGRAM_LENGTH-1; i++){
        array[i] = array[i+1];
    }
}

bool ispunctMod(int a){
    return (ispunct(a) && a != '\'');
}

bool isgroupterminator(int a){
    //return ((a == '\n') || (a == '\t') || ispunctMod(a) || isdigit(a));
    return !(isalpha(a) || isspace(a));
}

bool iswordterminator(int a){
    return (isgroupterminator(a) || isspace(a) || (a == '\''));
}
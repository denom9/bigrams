//
// Created by samuele on 02/12/17.
//


#ifndef BIGRAMMI_UTILITIES_H
#define BIGRAMMI_UTILITIES_H

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
    return !(isalpha(a) || isspace(a));
}

bool iswordterminator(int a){
    return (isgroupterminator(a) || isspace(a) || (a == '\''));
}

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


#endif //BIGRAMMI_UTILITIES_H






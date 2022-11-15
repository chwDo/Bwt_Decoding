#include <iostream>
#include <cstring>
#include <fstream>
#include <set>
#include <cmath>
#include<vector>
#include<algorithm>
#include <bitset>
#include <cstdio>
using namespace std;

# define SIZE_OF_BLOCK  (1024*1024) //1024*1024 1MB
# define GAP_OF_OCC  1024
# define GAP_OF_DECODE_OCC 512
# define SIZE_OF_OCC_BLOCK (1024*1024*3)
// Structure to store data of a rotation
struct rotation {
    int index;
    char* suffix;
};

// Compares the rotations and
// sorts the rotations alphabetically
int cmpfunc(const void* x, const void* y)
{
    struct rotation* rx = (struct rotation*)x;
    struct rotation* ry = (struct rotation*)y;
    return strcmp(rx->suffix, ry->suffix);
}

// Takes text to be transformed and its length as
// arguments and returns the corresponding suffix array
int* computeSuffixArray(char* input_text, int len_text)
{
    // Array of structures to store rotations and
    // their indexes
    struct rotation suff[len_text];

    // Structure is needed to maintain old indexes of
    // rotations after sorting them
    for (int i = 0; i < len_text; i++) {
        suff[i].index = i;
        suff[i].suffix = (input_text + i);
    }

    // Sorts rotations using comparison
    // function defined above
    qsort(suff, len_text, sizeof(struct rotation),
          cmpfunc);

    // Stores the indexes of sorted rotations
    int* suffix_arr
            = (int*)malloc(len_text * sizeof(int));
    for (int i = 0; i < len_text; i++)
        suffix_arr[i] = suff[i].index;

    // Returns the computed suffix array
    return suffix_arr;
}

// Takes suffix array and its size
// as arguments and returns the
// Burrows - Wheeler Transform of given text
char* findLastChar(char* input_text,
                   int* suffix_arr, int n)
{
    // Iterates over the suffix array to find
    // the last char of each cyclic rotation
    char* bwt_arr = (char*)malloc(n * sizeof(char));
    int i;
    for (i = 0; i < n; i++) {
        // Computes the last char which is given by
        // input_text[(suffix_arr[i] + n - 1) % n]
        int j = suffix_arr[i] - 1;
        if (j < 0)
            j = j + n;

        bwt_arr[i] = input_text[j];
    }

    bwt_arr[i] = '\0';

    // Returns the computed Burrows - Wheeler Transform
    return bwt_arr;
}

void search_without_blocks(FILE *bwt_file,size_t bwt_size,string query,int option_flag);
void search_with_blocks(FILE *bwt_file,size_t bwt_size,string query,int option_flag);


void construct_c_table(int *c_table,string bwt_content_str, string &found_chars){
    int index[128] = {};
    for(auto c: bwt_content_str){
        bitset<8> bits(c);
        int ascii_value = bits.to_ulong();
        index[ascii_value]++;
    }
    int flag = 0;
    c_table[10] = index[10];
    for(int i=10;i<128;i++){
        if(index[i] != 0){
            found_chars += char(i);
            index[i] += index[flag];
            if(flag!=0){
                c_table[i] = index[flag];
            }
            flag = i;
        }
    }
}

int occ_without_block(char c, int position,string bwt_content_str){
    int orrcurrence = 0;
    for(int i=0;i< position + 1; i++){
        if(c==bwt_content_str[i]){
            orrcurrence++;
        }
    }
    return orrcurrence;
}


void decode_target_line(int pos,char current_char,string bwt_content_str, int * c_table, string current_string,set<string> &answers,int option_flag,bool finding_last_line = false, bool is_blocked = false) {
    if(finding_last_line){
        int oc = occ_without_block(current_char,pos - 1,bwt_content_str) ;
        pos =  (oc + 1) % c_table[10] ;
    }
    current_char = bwt_content_str[pos];
    while (current_char != '\n') {
        current_string = current_char + current_string;
        pos = c_table[int(current_char)] + occ_without_block(current_char, pos - 1, bwt_content_str);
        current_char = bwt_content_str[pos];
    }
    if (finding_last_line) {
        answers.insert(current_string);
    } else {
        if (current_string.find(' ') == string::npos) {
            decode_target_line(pos, current_char, bwt_content_str, c_table, "", answers, option_flag, true);
        } else {
            if (option_flag == 1) {
                int line_number = 0;
                for (int i = 0; current_string[i] != ' '; i++){
                    line_number = line_number * 10 + int(current_string[i]) - 48;
                }
                answers.insert(to_string(line_number));
            } else {
                //find the next \n
                decode_target_line(pos, current_char, bwt_content_str, c_table, "", answers, option_flag,
                                   true);
            }
        }
    }
}
void backward_search(string bwt_content_str,size_t bwt_size,int *c_table,string found_chars,string query,int option_flag,bool is_blocked=false){
    int position = query.length()-1;
    char c = query[position];
    int first = c_table[int(c)],last;
    if(c == found_chars[found_chars.length() - 1]){
        last =  bwt_size/sizeof(char) - 1;
    }else{
        last =  c_table[int(found_chars[found_chars.find(c)+1])] - 1;
    }
    while(first<=last && position >=1){
        //now c is the next char need
        c = query[position - 1];
        position--;
        first = c_table[int(c)] + occ_without_block(c,first - 1,bwt_content_str);
        last = c_table[int(c)] + occ_without_block(c,last,bwt_content_str) - 1;
    }
    set<string> answers;
    if(last < first) {
        cout << "no row";
    }
    else{
        if(option_flag == 2){
            cout<<last - first + 1<<endl;
        }
        else {
            for(int i = first;i<=last;i++)
                decode_target_line(i,c,bwt_content_str,c_table,query,answers,option_flag,false);

            if(option_flag == 1){
                cout<<answers.size()<<endl;
            }else if(option_flag == 0){
                for(auto i= answers.begin();i!=answers.end();i++ ){
                    cout<<*i<<endl;
                }
            }

        }
    }
}

void search_without_blocks(FILE *bwt_file,size_t bwt_size,string query,int option_flag){
    char bwt_content[bwt_size + 1];
    memset(bwt_content,'\0', sizeof(bwt_content));
    fread(bwt_content,sizeof(char),bwt_size,bwt_file);
    string bwt_content_str(bwt_content);
    int c_table[128] = {};
    string found_chars;
    construct_c_table(c_table,bwt_content_str,found_chars);
    backward_search(bwt_content_str,bwt_size,c_table,found_chars,query,option_flag);
}

char locate_at(int position,FILE *bwt_file){
    int index_of_chunk = position / SIZE_OF_BLOCK;
    int offset = position % SIZE_OF_BLOCK;
    fseek(bwt_file,index_of_chunk * SIZE_OF_BLOCK,SEEK_SET);
    char * bwt_content = new char[SIZE_OF_BLOCK];
    memset(bwt_content,'\0', sizeof(SIZE_OF_BLOCK));
    fread(bwt_content,sizeof(char),SIZE_OF_BLOCK,bwt_file);
    char c =  bwt_content[offset];
    delete [] bwt_content;
    return c;
}


int occ(char c,int position,int **occ_array,FILE *bwt_file){
    int index_of_block = position / GAP_OF_OCC;
    int offset = position % GAP_OF_OCC;
    int index_of_char = int(c);
    //cout<<c<<" ";
    if(index_of_char >=30){
        index_of_char -=29;
    }
    else{
        index_of_char = 0;
    }
    int count = 0;
    if(occ_array[index_of_char][index_of_block+1] - occ_array[index_of_char][index_of_block] == 0){
        return occ_array[index_of_char][index_of_block];
    }
    else{
        fseek(bwt_file,index_of_block * GAP_OF_OCC,SEEK_SET);
        char * bwt_content = new char[GAP_OF_OCC];
        memset(bwt_content,'\0', sizeof(GAP_OF_OCC));
        fread(bwt_content,sizeof(char),GAP_OF_OCC,bwt_file);
        for(int i=0;i< offset + 1;i++){
            if (bwt_content[i] == c){
                count++;
            }
        }
        delete [] bwt_content;
        return (count + occ_array[index_of_char][index_of_block]);
    }
}

void decode_target_line_with_blocks(int pos,char current_char,FILE *bwt_file, int * c_table, string current_string,set<string> &answers,int option_flag, int** occ_array,bool finding_last_line = false){
    if(finding_last_line){
        int oc = occ(current_char,pos - 1,occ_array,bwt_file); ;
        pos =  (oc + 1) % c_table[10] ;
    }
    current_char = locate_at(pos,bwt_file);
    while (current_char != '\n') {
        current_string = current_char + current_string;
        int c = c_table[int(current_char)];
        int oc = occ(current_char,pos - 1,occ_array,bwt_file);
        //pos = c_table[int(current_char)] + occ(current_char,pos - 1,occ_array,bwt_file); ;
        pos = c + oc;
        current_char = locate_at(pos,bwt_file);
    }
    if (finding_last_line) {
        answers.insert(current_string);
    } else {
        if (current_string.find(' ') == string::npos) {
            decode_target_line_with_blocks(pos, current_char, bwt_file, c_table, "", answers, option_flag,occ_array, true);
        } else {
            if (option_flag == 1) {
                int line_number = 0;
                for (int i = 0; current_string[i] != ' '; i++){
                    line_number = line_number * 10 + int(current_string[i]) - 48;
                }
                answers.insert(to_string(line_number));
            } else {
                //find the next \n
                decode_target_line_with_blocks(pos, current_char, bwt_file, c_table, "", answers, option_flag,occ_array,
                                               true);
            }
        }
    }
}

void backward_search_with_block(FILE *bwt_file,size_t bwt_size,int *c_table,string found_chars,string query,int option_flag,int ** occ_array){
    int position = query.length()-1;
    char c = query[position];

    int first = c_table[int(c)],last;
    if(c == found_chars[found_chars.length() - 1]){
        last =  bwt_size/sizeof(char) - 1;
    }else{
        last =  c_table[int(found_chars[found_chars.find(c)+1])] - 1;
    }
    while(first<=last && position >=1){
        //now c is the next char need
        c = query[position - 1];
        position--;
        first = c_table[int(c)] + occ(c,first - 1,occ_array,bwt_file);
        last = c_table[int(c)] + occ(c,last,occ_array,bwt_file) - 1;
    }
    set<string> answers;
    if(last < first) {
        cout << "no row";
    }
    else{
        if(option_flag == 2){
            cout<<last - first + 1<<endl;
        }
        else {
            for(int i = first;i<=last;i++)
                decode_target_line_with_blocks(i,c,bwt_file,c_table,query,answers,option_flag,occ_array,false);

            if(option_flag == 1){
                cout<<answers.size()<<endl;
            }else if(option_flag == 0){
                for(auto i= answers.begin();i!=answers.end();i++ ){
                    cout<<*i<<endl;
                }
            }

        }
    }
}
void construct_occ_array(FILE *bwt_file,size_t bwt_size,int **occ_array,int * c_table,string &found_chars){
    int number_of_blocks = 0;
    char * bwt_content = new char[SIZE_OF_BLOCK];
    int index[128] = {};
    int count_outer = 1;
    int count_inner = 0;
    int num_read = 0;
    int cnt = 0;
    while(true){
        num_read = fread(bwt_content,sizeof(char),SIZE_OF_BLOCK,bwt_file);
        cnt += num_read;
        number_of_blocks++;
        for(int i=0;i<num_read;i++) {
            bitset<8> bits(bwt_content[i]);
            int asc_value = bits.to_ulong();
            index[asc_value]++;
            count_inner++;
            //  cout<<asc_value;
            if (asc_value>=30){
                asc_value -= 29;
            }else{
                asc_value = 0;
            }
            occ_array[asc_value][count_outer]++;
            if(count_inner == GAP_OF_OCC){
                count_inner = 0;
                count_outer ++;
                for(int j=0;j<100;j++){
                    occ_array[j][count_outer] = occ_array[j][count_outer - 1];
                }
            }
        }
        if(num_read < SIZE_OF_BLOCK ){
            break;
        }
    }
    int asi = int('.') - 29;

    int flag = 0;
    c_table[10] = index[10];
    for(int i=10;i<128;i++){
        if(index[i] != 0){
            found_chars += char(i);
            index[i] += index[flag];
            if(flag!=0){
                c_table[i] = index[flag];
            }
            flag = i;
        }
    }
    delete [] bwt_content;
}

void search_with_blocks(FILE *bwt_file,size_t bwt_size,string query,int option_flag){
    int ** occ_array;
    occ_array = new int *[100];
    for(int i=0;i<100;i++){
        occ_array[i] = new int[bwt_size/GAP_OF_OCC+2000];
        memset(occ_array[i],0,sizeof(occ_array)/sizeof(int));
    }
    int c_table[128] = {};
    string found_chars = "";
    construct_occ_array(bwt_file,bwt_size,occ_array,c_table,found_chars);
    backward_search_with_block(bwt_file,bwt_size,c_table,found_chars,query,option_flag,occ_array);
    for(int i=0;i<100;i++){
        delete [] occ_array[i];
    }
}
void no_block_decode(string bwt_content_str,size_t bwt_size,int *c_table, char* decode_file){
    int cnt = 0;
    int pos = 0;
    char current_char= '\n';
    string buffer;
    FILE* output = fopen(decode_file,"w");
    while (cnt < bwt_size) {
        buffer = current_char + buffer;
        current_char = bwt_content_str[pos];
        if(current_char == '\n'){
            pos = occ_without_block(current_char, pos - 1, bwt_content_str);
        }else{
            pos = c_table[int(current_char)] + occ_without_block(current_char, pos - 1, bwt_content_str);
        }
        cnt++;
    }
    char char_buffer[buffer.length() + 1];
    strcpy(char_buffer,buffer.c_str());
    fwrite(char_buffer,sizeof(char),cnt,output);
}
void decoder_without_blocks(FILE *bwt_file,size_t bwt_size, char* decode_file){
    char bwt_content[bwt_size + 1];
    memset(bwt_content,'\0', sizeof(bwt_content));
    fread(bwt_content,sizeof(char),bwt_size,bwt_file);
    string bwt_content_str(bwt_content);
    int c_table[128] = {};
    string found_chars;
    construct_c_table(c_table,bwt_content_str,found_chars);
    no_block_decode(bwt_content_str,bwt_size,c_table, decode_file);
}
int decode_occ(char c,int position,int **occ_array,FILE *bwt_file){
    if (position<0){
        return 0;
    }
    int index_of_block = position / GAP_OF_DECODE_OCC;
    int offset = position % GAP_OF_DECODE_OCC;
    int index_of_char = int(c);
    if(index_of_char >=30){
        index_of_char -=29;
    }
    else{
        index_of_char = 0;
    }
    int count = 0;
    if(occ_array[index_of_char][index_of_block+1] - occ_array[index_of_char][index_of_block] == 0){
        return occ_array[index_of_char][index_of_block];
    }
    else{
        fseek(bwt_file,index_of_block * GAP_OF_DECODE_OCC,SEEK_SET);
        char * bwt_content = new char[GAP_OF_DECODE_OCC];
        memset(bwt_content,'\0', sizeof(GAP_OF_DECODE_OCC));
        fread(bwt_content,sizeof(char),GAP_OF_DECODE_OCC,bwt_file);
        for(int i=0;i< offset + 1;i++){
            if (bwt_content[i] == c){
                count++;
            }
        }
        delete [] bwt_content;
        return (count + occ_array[index_of_char][index_of_block]);
    }
}
int locate_at_decode(int position,FILE* bwt_file,char *bwt_content_buffer){
    if(position<SIZE_OF_OCC_BLOCK){
        return bwt_content_buffer[position];
    }
    else{
        int index_of_chunk = position / SIZE_OF_BLOCK;
        int offset = position % SIZE_OF_BLOCK;
        fseek(bwt_file,index_of_chunk * SIZE_OF_BLOCK,SEEK_SET);
        char * bwt_content = new char[SIZE_OF_BLOCK];
        memset(bwt_content,'\0', sizeof(SIZE_OF_BLOCK));
        fread(bwt_content,sizeof(char),SIZE_OF_BLOCK,bwt_file);
        char c =  bwt_content[offset];
        delete [] bwt_content;
        return c;
    }
}

void block_decode(FILE* bwt_file,size_t bwt_size,int *c_table,int **occ_array,char* decode_file) {
    int cnt = 0,cnt_2 = 0;
    int pos = 0;
    char current_char= '\n';
    string buffer;
    FILE* output = fopen(decode_file,"w");
    char * bwt_content_buffer = new char[SIZE_OF_OCC_BLOCK];
    fseek(bwt_file,0,SEEK_SET);
    fread(bwt_content_buffer,sizeof(char),SIZE_OF_OCC_BLOCK,bwt_file);
    while (cnt < bwt_size) {
        cnt_2 = 0;
        buffer = "";
        while(cnt_2 < SIZE_OF_BLOCK && cnt < bwt_size){
            buffer = current_char + buffer;
            current_char = locate_at_decode(pos,bwt_file,bwt_content_buffer);
            if(current_char == '\n'){
                pos = decode_occ(current_char, pos - 1,occ_array, bwt_file);
            }else{
                pos = c_table[int(current_char)] + decode_occ(current_char, pos - 1, occ_array, bwt_file);
            }
            cnt_2++;
            cnt++;
        }
        char char_buffer[buffer.length() + 1];
        strcpy(char_buffer,buffer.c_str());
        fwrite(char_buffer,sizeof(char),cnt,output);
        fseek(output, 0, SEEK_SET);
    }
    delete [] bwt_content_buffer;
}

void construct_block_occ_array(FILE *bwt_file,size_t bwt_size,int **occ_array,int * c_table,string &found_chars){
    int number_of_blocks = 0;
    char * bwt_content = new char[SIZE_OF_BLOCK];
    int index[128] = {};
    int count_outer = 1;
    int count_inner = 0;
    int num_read = 0;
    int cnt = 0;
    while(true){
        num_read = fread(bwt_content,sizeof(char),SIZE_OF_BLOCK,bwt_file);
        cnt += num_read;
        number_of_blocks++;
        for(int i=0;i<num_read;i++) {
            bitset<8> bits(bwt_content[i]);
            int asc_value = bits.to_ulong();
            index[asc_value]++;
            count_inner++;
            if (asc_value>=30){
                asc_value -= 29;
            }else{
                asc_value = 0;
            }
            occ_array[asc_value][count_outer]++;
            if(count_inner == GAP_OF_DECODE_OCC){
                count_inner = 0;
                count_outer ++;
                for(int j=0;j<100;j++){
                    occ_array[j][count_outer] = occ_array[j][count_outer - 1];
                }
            }
        }
        if(num_read < SIZE_OF_BLOCK ){
            break;
        }
    }
    int asi = int('.') - 29;

    int flag = 0;
    c_table[10] = index[10];
    for(int i=10;i<128;i++){
        if(index[i] != 0){
            found_chars += char(i);
            index[i] += index[flag];
            if(flag!=0){
                c_table[i] = index[flag];
            }
            flag = i;
        }
    }
    delete [] bwt_content;
}

void decoder_with_blocks(FILE *bwt_file,size_t bwt_size, char* decode_file){
    int ** occ_array;
    occ_array = new int *[100];
    for(int i=0;i<100;i++){
        occ_array[i] = new int[bwt_size/GAP_OF_DECODE_OCC+2000];
        memset(occ_array[i],0,sizeof(occ_array[i])/sizeof(int));
    }
    int c_table[128] = {};
    string found_chars = "";
    construct_block_occ_array(bwt_file,bwt_size,occ_array,c_table,found_chars);
    block_decode(bwt_file,bwt_size,c_table,occ_array,decode_file);

}

int main(int argc, char *argv[]) {
    char *file_path, *index_path;
    string query;
    // option_flag 0 for output whole line, 1 for output with duplicates, 2 for no duplicates
    int option_flag;
    char *decode_file;
    if (argc == 5) {
        file_path = argv[2];
        index_path = argv[3];
        query = argv[4];
        if (strcmp(argv[1], "-m") == 0) {
            option_flag = 2;
        } else if (strcmp(argv[1], "-n") == 0) {
            option_flag = 1;
        }else if (strcmp(argv[1], "-o") == 0){
            option_flag = 3;
            decode_file = argv[4];
        }
    } else {
        option_flag = 0;
        file_path = argv[1];
        index_path = argv[2];
        query = argv[3];
    }
    FILE *bwt_file = fopen(file_path, "r");
    FILE *index_file = fopen(index_path, "ab+");
    fseek(bwt_file, 0, SEEK_END);
    //get length of the file
    size_t bwt_size = ftell(bwt_file);
    fseek(bwt_file, 0, SEEK_SET);
    if (bwt_size < SIZE_OF_BLOCK * 6) {
        //search_without_blocks(bwt_file, bwt_size, query, option_flag);
        if(option_flag == 3){
            decoder_without_blocks(bwt_file, bwt_size,decode_file);
        }else{
            search_without_blocks(bwt_file, bwt_size, query, option_flag);
        }
    } else {
        if(option_flag == 3){
            decoder_with_blocks(bwt_file, bwt_size,decode_file);
        }else{
            search_with_blocks(bwt_file, bwt_size, query, option_flag);
        }
    }

    fclose(bwt_file);
}
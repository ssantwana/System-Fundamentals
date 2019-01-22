#include <stdlib.h>

#include "debug.h"
#include "hw1.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

int flag_h = 0;
int flag_u = 0;
int flag_d = 0;
int flag_f = 0;
int flag_p = 0;
int flag_c = 0;
int flag_k = 0;
int factor = -1;
char *key = NULL;
int size_of_argument_string = 0;
int frame_size = 0;
int sample_size = 0;
int channels = 0;



void switch_int_mem(void *addr);
void switch_and_print_an_integer(void *addr);
int get_size_of_argument_string(char **argv);
int fill_input_annotation();
int fill_output_annotation(int pos);
int fill_output_annotation_with_argv(char **argv);
void Copy_data_frame_to_internal_frame(char *, char *, char *, char *);
void Copy_internal_frame_to_data_frame(char *, char *, char *, char *);

/*
 * You may modify this file and/or move the functions contained here
 * to other source files (except for main.c) as you wish.
 *
 * IMPORTANT: You MAY NOT use any array brackets (i.e. [ and ]) and
 * you MAY NOT declare any arrays or allocate any storage with malloc().
 * The purpose of this restriction is to force you to use pointers.
 * Variables to hold the content of three frames of audio data and
 * two annotation fields have been pre-declared for you in const.h.
 * You must use those variables, rather than declaring your own.
 * IF YOU VIOLATE THIS RESTRICTION, YOU WILL GET A ZERO!
 *
 * IMPORTANT: You MAY NOT use floating point arithmetic or declare
 * any "float" or "double" variables.  IF YOU VIOLATE THIS RESTRICTION,
 * YOU WILL GET A ZERO!
 */

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 1 if validation succeeds and 0 if validation fails.
 * Upon successful return, the selected program options will be set in the
 * global variables "global_options", where they will be accessible
 * elsewhere in the program.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 1 if validation succeeds and 0 if validation fails.
 * Refer to the homework document for the effects of this function on
 * global variables.
 * @modifies global variable "global_options" to contain a bitmap representing
 * the selected options.
 */
int getStrlength(char *arg){
    int count = 0;
    while( *( arg + count) != '\0'){
        count++;
        if( count > ANNOTATION_MAX + 5)
            return count;
    }
    return count;
}

void set_flag(int x, int flag){
    if(flag == 0)
        return;
    debug("Set flags : Global args : %lu before major flags",global_options);
    unsigned long test = 1;
    /*
    for(int i = 0 ; i < x ; i++){
        unsigned long temp = 1;
        temp = temp<<i;
        printf("When shift is %d the no is %lx \n",i,temp);
    }
    */
    //printf( " x : %d \n" ,x);
    test = test<<(x);
    global_options = global_options^test;
    debug("Set flags : Global args : %lxu  and test : %lx after major flags, bits moved : %d",global_options,test,(x-1));

}

void set_global_options(int flag_h,int flag_u,int flag_d,int flag_f,int flag_p,int flag_c,int flag_k,int factor,char *key){
    debug("Global args : %lx before major flags",global_options);
    set_flag(63,flag_h);
    set_flag(62,flag_u);
    set_flag(61,flag_d);
    set_flag(60,flag_c);
    set_flag(59,flag_p);
    debug("Global args : %lx after major flags",global_options);
    if(flag_f == 1){
        unsigned long test = factor - 1;
        test = test<<48;
        global_options = global_options^test;
    }
    if(flag_k == 1){
        unsigned long test = 0;
        int len = getStrlength(key);
        for(int i = 0 ; i < len ; i++){
            //int bit_shift = 8*(len - i - 1 );
            char c = *(key + i );
            unsigned long value = 0;
            if(c >= '0' && c <= '9'){
                value = (c - '0');
            }else{
                if(c >= 'a' && c <= 'z')
                    c = c + 'A' - 'a';
                value = 10 + ( c - 'A');
            }
            int dist = len - i - 1;
            int bit_shift = dist*4;
            test = test^(value<<bit_shift);
            //test = test<<bit_shift;
        }
        //debug(" test : %lx \n",test);
        global_options = global_options^test;
    }

}

// Verify if the key is valid or not
// Else return false
int VerifyKey(char *key){
    int len = getStrlength(key);
    if(len == 0 || len > 8){
        debug("Invalid Key");
        return 1;
    }
    for(int i = 0; i < len ; i++){
        char c = *( key + i);
        if(!( ( c >= '0' && c <= '9') || ( c >= 'a' && c <= 'f' ) || ( c >= 'A' && c <= 'F') )){
            debug("Invalid Character : %c ",c);
            return 1;
        }
    }
    return 0;
}

// Convert a String to a no if the String contains character other than 0-9 return -1;
// Tested OK
int convert_String_to_no(char *S){
    int len = getStrlength(S);
    int no = 0;
    if(len == 0 || len > 15 )
        return -1;


    for(int i = 0; i < len ; i++){
        char c = *(S + i );
        if( c < '0' || c > '9')
            return -1;
        no = no * 10 + (int)(c - '0');
    }

    return no;

}



int validargs(int argc, char **argv)
{
    int start = 1;
    int end  = argc - 1;
    int curr = 1;
    /*
    int flag_h = 0;
    int flag_u = 0;
    int flag_d = 0;
    int flag_f = 0;
    int flag_p = 0;
    int flag_c = 0;
    int flag_k = 0;
    int factor = -1;
    char *key = NULL;
    */
    char prev_flag = '-';
    char flag = '-';
    while(curr <= end ){
        char *arg = *(argv + curr);
        int len = getStrlength(arg);
        if(len != 2 || *arg != '-'){
            debug(" len  : %d and string is for %d and string %s",len,curr,arg);
            debug("Must have reached here , either argument length != 2 or '-' is not present");
            return 0;
        }
        prev_flag = flag;
        flag = *(arg + 1);
        switch(flag){
            case 'h' :
            //debug("In h \n");
            flag_h = 1 ;
            if(curr == 1){
                set_global_options(flag_h,flag_u,flag_d,flag_f,flag_p,flag_c,flag_k,factor,key);
                return 1;
            }
            else{
                set_global_options(flag_h,flag_u,flag_d,flag_f,flag_p,flag_c,flag_k,factor,key);
                return 0;
            }
            break;

            case 'u' :
            //debug("In u \n");
            if(flag_u == 1  || curr != 1 ) return 0;
            flag_u = 1 ;
            break;

            case 'd' :
            //debug("In d \n");
            if(flag_d == 1 || curr != 1) return 0;
            flag_d = 1;
            break;

            case 'f' :
            //debug("In f \n");
            if(flag_f == 1 ) return 0;
            if(curr == 1 || curr == end || (flag_d == 0 && flag_u == 0)) return 0;
            if(prev_flag != 'p' && prev_flag != 'u' && prev_flag != 'd' ) return 0;
            flag_f = 1 ;
            curr++;
            char *factor_in_string =  *(argv + curr);
            factor = convert_String_to_no(factor_in_string);
            if( factor <= 0 || factor > 1024 ) {
                debug("Out of range");
                return 0;
            }
            debug("Factor : %d",factor);
            break;

            case 'p' :
            //debug("In p \n");
            if(flag_p == 1 || curr == 1 || (flag_d == 0 && flag_u == 0 && flag_c == 0))
                return 0;
            flag_p = 1 ;
            break;

            case 'c' :
            //debug("In c \n");
            if(flag_c == 1 || curr == end || curr != 1) return 0;
            flag_c = 1 ;break;

            case 'k' :
            //debug("In k \n");
            if(flag_k == 1 || curr == 1 || flag_c == 0 || curr == end) return 0;
            if(prev_flag != 'c' && prev_flag != 'p') return 0;
            flag_k = 1 ;
            curr++;
            debug("Trying to access key ");
            key = *(argv + curr);
            if(VerifyKey(key) != 0) return 0;
            debug(" key  : %s",key);
            break;

            default:
            debug("No Arguments Matched");
            return 0;
        }
        curr++;
    }
    if(flag_u == 0 && flag_d == 0 && flag_c == 0) return 0;
    if(flag_c == 1 && flag_k == 0) return 0;
    if(factor == -1 && (flag_u == 1 || flag_d == 1)) factor = 1;
    set_global_options(flag_h,flag_u,flag_d,flag_f,flag_p,flag_c,flag_k,factor,key);
    //global_options_in_hex(global_options)
    debug("**** Sucessfull Finished****");

    return 1;


}

/**
 * @brief  Recodes a Sun audio (.au) format audio stream, reading the stream
 * from standard input and writing the recoded stream to standard output.
 * @details  This function reads a sequence of bytes from the standard
 * input and interprets it as digital audio according to the Sun audio
 * (.au) format.  A selected transformation (determinedc by the global variable
 * "global_options") is applied to the audio stream and the transformed stream
 * is written to the standard output, again according to Sun audio format.
 *
 * @param  argv  Command-line arguments, for constructing modified annotation.
 * @return 1 if the recoding completed successfully, 0 otherwise.
 */
int recode(char **argv) {
    //long mem ;
    AUDIO_HEADER x = {0,0,0,0,0,0};
    AUDIO_HEADER *hp = &x;
    /*
    printf("size of int : %d\n",(int)sizeof(int));
    printf("size of long : %d\n",(int)sizeof(long));
    printf("size of long long : %d\n",(int)sizeof(long long));
    printf("size of double : %d\n",(int)sizeof(double));
    printf("size of char : %d\n",(int)sizeof(char));
    */
    size_of_argument_string = get_size_of_argument_string(argv);
    debug("Reading read_header\n");
    if(read_header(hp) == 0) return 0;

    // Setting input_offset
    //if(read_annotation(char *ap, unsigned int size) == 0) return 0;

    // Some safety checks
    // test

    if(hp->data_offset < 24 || hp->data_offset - 24 > ANNOTATION_MAX ) return 0;


    if(hp->data_offset == 24)
        *input_annotation = '\0';
    else
        if(read_annotation(input_annotation, hp->data_offset - 24) == 0) return 0;



    debug("Reading Annotation Complete\n");

    int already_filled_output_annotation = 0;
    if(flag_p == 0){
        already_filled_output_annotation = fill_output_annotation_with_argv(argv);
    }
    if(already_filled_output_annotation == -1) return 0;

    debug("Loading Arguments finished in case required\n");

    if(fill_output_annotation(already_filled_output_annotation) == -1) return 0;

    debug("Abbout to write Header\n");

    if(write_header(hp) == 0 ) return 0;

    int output_annotation_length = getStrlength(output_annotation) + 1;
    int remaining = 8 - output_annotation_length % 8;
    if(remaining == 8) remaining = 0;
    for(int k = 0; k < remaining ; k++){
        *(output_annotation + (output_annotation_length + k)) = '\0';
    }
    debug("Writing Annotation\n");
    if(output_annotation_length != 1)
        if(write_annotation(output_annotation, remaining + output_annotation_length) == 0) return 0;


    long data_read = 0;
    long data_to_be_written = hp->data_size;
        debug("data to be written : %ld in hex %lx",data_to_be_written,data_to_be_written);
    sample_size = ((int)(hp->encoding)) - 1;
    unsigned long space = 0;
    unsigned long *data = &space;
    int remainder = 0;

    float factor_internal = 1;
    if(factor != 0 ) factor_internal = (float) factor;

    unsigned int seed = (unsigned int)(global_options&0xffffffff);
    debug("seed : %x",seed);
    mysrand(seed);

    int prev_first = 0;
    int prev_second = 0;
    unsigned int index = 0;
    int first_read_flag = 0;

    while( data_to_be_written == 0xfffffffff || data_read < data_to_be_written ){

        //debug("About to read frame");
        *data = 0;
        int result = read_frame((int *)data,channels,sample_size);
        if(result == 0){
            debug("I am here");
            debug("data_to_be_written : %lx" , data_to_be_written);
            if(data_to_be_written != 0xffffffff ){
                debug("Fail as it is not infinite reading");
                return 0;
            }

            if(*data == 0xffffffffffffffff){
                debug("fail infinite but full frame missing so ok");
                return 1;
            }else{
                debug("failing in between frame");
                return 0;
            }
        }
        data_read += sample_size * hp->channels;
        //debug("reading frame : %d and data_already_read : %lu",index,data_read);
        index++;

        if(flag_u == 1 ){
            if( remainder % (int)factor_internal  == 0 ){
                write_frame((int *)data,channels,sample_size);
                remainder = 0;
            }
        }


        if(flag_c == 1){
            char *data_first = (char *)data;
            char *data_second = data_first + 4;
            int first = 0;
            int second = 0;
            Copy_data_frame_to_internal_frame(data_first, data_second, (char *)(&first), (char *)(&second));
            unsigned long rand  = myrand32();
            first = first^rand;
            if(hp->channels == 2){
                rand = myrand32();
                second = second^rand;
            }
            Copy_internal_frame_to_data_frame(data_first, data_second, (char *)&first,(char *)&second);
            write_frame((int *)data,channels,sample_size);
        }

        if(flag_d == 1){
            if(first_read_flag == 0){
                char *data_first = (char *)data;
                char *data_second = data_first + 4;
                first_read_flag = 1;
                Copy_data_frame_to_internal_frame(data_first, data_second, (char *)(&prev_first), (char *)(&prev_second));
            }else{
                //debug("writing frame");
                char *data_first = (char *)data;
                char *data_second = data_first + 4;
                int first = 0;
                int second = 0;

                Copy_data_frame_to_internal_frame(data_first, data_second, (char *)(&first), (char *)(&second));
                //debug("Data : %lx    first : %x      second : %x",*data,first , second);
                //debug("While reading first : %d second : %d prev_first : %d and prev_second : %d",first,second,prev_first,prev_second);
                for(float i = 0 ; i < factor_internal ; i = i + 1){
                    //debug("")
                    long temp_first_temp = (first - prev_first ) * i;
                    int temp_first = prev_first + (int)(temp_first_temp / factor_internal);
                    //float multiplier = i/factor_internal;
                    //int temp_first = first + (int )((first - prev_first )*multiplier);
                    long temp_second_temp = (second - prev_second )*i;
                    int temp_second = prev_second + (int)(temp_second_temp / factor_internal);
                    //int temp_second = second + (int)((second - prev_second )*multiplier);
                    //debug("While writing for i : %d first : %d second : %d",(int)i,temp_first,temp_second);
                    Copy_internal_frame_to_data_frame(data_first, data_second, (char *)&temp_first,(char *)&temp_second);
                    write_frame((int *)data,channels,sample_size);
                }
                prev_first = first;
                prev_second = second;
            }
            if(data_read == data_to_be_written){
                char *data_first = (char *)data;
                char *data_second = data_first + 4;
                Copy_internal_frame_to_data_frame(data_first, data_second, (char *)&prev_first,(char *)&prev_second);
                write_frame((int *)data,channels,sample_size);
            }

        }


        data_read += frame_size;
        remainder++;
        //debug("data written : %ld : ",data_read);

    }






















    return 1;
}


/**
 * @brief Read the header of a Sun audio file and check it for validity.
 * @details  This function reads 24 bytes of data from the standard input and
 * interprets it as the header of a Sun audio file.  The data is decoded into
 * six unsigned int values, assuming big-endian byte order.   The decoded values
 * are stored into the AUDIO_HEADER structure pointed at by hp.
 * The header is then checked for validity, which means:  no error occurred
 * while reading the header data, the magic number is valid, the data offset
 * is a multiple of 8, the value of encoding field is one of {2, 3, 4, 5},
 * and the value of the channels field is one of {1, 2}.
 *
 * @param hp  A pointer to the AUDIO_HEADER structure that is to receive
 * the data.
 * @return  1 if a valid header was read, otherwise 0.
 */
int read_header(AUDIO_HEADER *hp){

    debug("test");
    debug("size of header : %d",(int)sizeof(AUDIO_HEADER));
    //hp = {0,0,0,0,0,0};
    void *add = hp;
    for(int i = 0 ; i < 6 ;i++){
        for(int j = 0 ; j < 4 ; j++){
            char *curr = add + i * 4 + j;
            int char_in_int = 0;
            char_in_int = getchar();
            *curr = (char)char_in_int;
            if( char_in_int == EOF){
                return 0;
            }
        }
        switch_int_mem(add+i*4);
    }
    if(hp->magic_number != AUDIO_MAGIC)
        return 0;
    if(hp->data_offset % 8 != 0 || hp->data_offset < 24)
        return 0;
    if(hp->data_offset > 24 && hp->data_offset > 24 + ANNOTATION_MAX )
        return 0;
    if( hp->encoding < 2 || hp->encoding > 5)
        return 0;
    if(hp->channels != 1 && hp->channels != 2)
        return 0;
    // Todo and todo Validate the fields of Audio Header
    debug(" read_header : magic no : %d and hex : %x",hp->magic_number,hp->magic_number);
    debug(" read_header :  data_offset : %d and hex : %x" , hp->data_offset,hp->data_offset);
    debug(" read_header :  data_size : %d and hex : %x", hp->data_size,hp->data_size);
    debug(" read_header :  encoding : %d and hex : %x", hp->encoding,hp->encoding);
    debug(" read_header :  sample_rate : %d and hex : %x",hp->sample_rate,hp->sample_rate);
    debug(" read_header :  channels : %d and hex : %x", hp->channels,hp->channels);
    //printf("Reading Header Do")

    return 1;
}

/**
 * @brief  Write the header of a Sun audio file to the standard output.
 * @details  This function takes the pointer to the AUDIO_HEADER structure passed
 * as an argument, encodes this header into 24 bytes of data according to the Sun
 * audio file format specifications, and writes this data to the standard output.
 *
 * @param  hp  A pointer to the AUDIO_HEADER structure that is to be output.
 * @return  1 if the function is successful at writing the data; otherwise 0.
 */
int write_header(AUDIO_HEADER *hp){

    // Writing Magic No
    void *addr = hp;
    debug(" read_header : magic no : %d and hex : %x",hp->magic_number,hp->magic_number);
    switch_and_print_an_integer(addr);
    addr += 4;

    // TO write data_offset
    unsigned int output_annotation_length = getStrlength(output_annotation) + 1;
    debug("output_annotation_length : %d",output_annotation_length);
    if(output_annotation_length == 1) output_annotation_length = 0;
    if(output_annotation_length % 8 != 0){
        output_annotation_length += 8 - output_annotation_length % 8;
    }
    unsigned int new_offset = 24 + output_annotation_length;
    debug("new offset : %d",new_offset);
    debug(" read_header :  data_offset : %d and hex : %x" , new_offset,new_offset);
    switch_and_print_an_integer(&new_offset);
    addr += 4;

    //To write Data Size
    unsigned long current_data_size = hp->data_size;
    unsigned long size_of_frame = hp->channels * (hp->encoding - 1);
    unsigned long new_data_size = 0;
    //debug("size of frame : %d",size_of_frame);
    if(flag_f == 1 && factor != 1){
        if(factor == -1){
            debug("Issue factor is currently set to -1");
        }
        if(flag_u == 1){
            unsigned long old_data_frames = current_data_size / size_of_frame;
            int extra = 0;
            if(old_data_frames % factor != 0)
                extra = 1;

            unsigned long new_data_frame = old_data_frames / factor + extra;
            new_data_size = new_data_frame * size_of_frame;
        }else{
            new_data_size = current_data_size + (current_data_size - size_of_frame ) * (factor - 1);
            //debug("old data size : %d new data size : %d",current_data_size,new_data_size);
            if(new_data_size > 0xffffffff)
                new_data_size = 0xffffffff;
        }
    }else{
        new_data_size = hp->data_size;
    }
    debug("new data size : %ld ",new_data_size);

    if(new_data_size > 0xffffffff || current_data_size  == 0xffffffff )
        new_data_size = 0xffffffff;
    debug(" write_header :  data_size : %ld and hex : %lx", new_data_size,new_data_size);
    unsigned int int_new_data_size = (unsigned int)new_data_size;
    switch_and_print_an_integer(&int_new_data_size);
    // Todo and todo validate the field of data-size also see if channls affect the size on factorization



    //To write encoding
    unsigned int encoding  = hp->encoding;
    switch_and_print_an_integer(&encoding);


    // To write Sample Rate
    // Todo and todo check sample rate with factor.

    unsigned int sample_rate  = hp->sample_rate;
    switch_and_print_an_integer(&sample_rate);


    // To write Channels
    unsigned int channels  = hp->channels;
    switch_and_print_an_integer(&channels);



    debug(" write_header :  encoding : %d and hex : %x", encoding,encoding);
    debug(" write_header :  sample_rate : %d and hex : %x",sample_rate,sample_rate);
    debug(" write_header :  channels : %d and hex : %x", channels,channels);

    return 1;

}

void switch_int_mem(void *addr){
    char a = *(char *)(addr);
    char b = *(char *)(addr + 1);
    char c = *(char *)(addr + 2);
    char d = *(char *)(addr + 3);
    *(char *)(addr + 3) = a;
    *(char *)(addr + 2) = b;
    *(char *)(addr + 1) = c;
    *(char *)(addr) = d;
}

void switch_and_print_an_integer(void *addr){
    switch_int_mem(addr);
    for(int i = 0 ; i < 4 ; i++){
        printf("%c",*((char *)(addr+i)));
    }
}

int get_size_of_argument_string(char **argv){
    char *start = *argv;
    int count = 0;
    int total = 0;
    while(start != NULL){
        total += getStrlength(start);
        count ++;
        start = *(argv+count);
    }
    total += count;
    return total;
}



/**
 * @brief  Read annotation data for a Sun audio file from the standard input,
 * storing the contents in a specified buffer.
 * @details  This function takes a pointer 'ap' to a buffer capable of holding at
 * least 'size' characters, and it reads 'size' characters from the standard input,
 * storing the characters read in the specified buffer.  It is checked that the
 * data read is terminated by at least one null ('\0') byte.
 *
 * @param  ap  A pointer to the buffer that is to receive the annotation data.
 * @param  size  The number of bytes of data to be read.
 * @return  1 if 'size' bytes of valid annotation data were successfully read;
 * otherwise 0.
 */
int read_annotation(char *buffer, unsigned int size){
    unsigned int  count = 0;
    char c = '\0';
    int char_in_int = 0;
    char_in_int = getchar();
    c = (char)char_in_int;
    //c = getchar();
    if(char_in_int == EOF){
        return 0;
    }
    //scanf("%c",&c);
    while(c != '\0'){
        *(buffer + count) = c;
        count++;
        if(count == size ) return 0;
        char_in_int = getchar();
        c = (char)char_in_int;
        if(char_in_int == EOF){
            return 0;
        }
        //scanf("%c",&c);
    }
    *(buffer + count) = '\0';
    while(count < size){
        char_in_int = getchar();
        c = (char)char_in_int;
        if(char_in_int == EOF){
            return 0;
        }
        //scanf("%c",&c);
        count++;
    }
    return 1;
}

/*
int fill_input_annotation(){
    int count = 0;
    char c = '\0';
    scanf("%c",&c);
    //int pos = 0;
    while(c != '\0'){
        *(input_annotation + count) = c;
        scanf("%c",&c);
        count++;
        if(count > ANNOTATION_MAX ) return -1;
    }
    *(input_annotation+count) = '\0';
    return 1;
}
*/

//Perfectly Implemented
// Fills the output annotation arraion with argv and save it in buffer.
int fill_output_annotation_with_argv(char **argv){
    int max_to_be_filled = ANNOTATION_MAX;
    int currently_filled = 0;
    int count = 0;
    char *current_ptr = *(argv);

    //printf("Loading Arguments to output annotation\n");
    while(current_ptr != NULL){
        //printf(" Loading argument : %s with length : %d\n",current_ptr, getStrlength(current_ptr));
        int len = getStrlength(current_ptr);
        if(currently_filled + len +1 > max_to_be_filled)
            return -1;

        for(int i = 0 ; i < len ; i++){
            *(output_annotation + currently_filled) = *(current_ptr + i);
            currently_filled ++;
        }
        *(output_annotation + currently_filled) = ' ';
        currently_filled ++;
        //currently_filled += len + 1;
        count++;
        current_ptr = *(argv + count);
    }
    *(output_annotation + currently_filled-1) = '\n';
    return currently_filled;
}

int fill_output_annotation(int pos){
    int already_filled_output_annotation = pos;
    int to_be_filled = getStrlength(input_annotation);
    //if(pos == 0 && to_be_filled == 0) return 0;
    if(already_filled_output_annotation + to_be_filled >= ANNOTATION_MAX)
        return -1;
    int count = pos;
    //char c = '\0';
    for(int i = 0 ; i <=  to_be_filled ; i++){
        *(output_annotation + count) = *(input_annotation + i);
        count++;
    }
    return count;
}

/**
 * @brief  Write annotation data for a Sun audio file to the standard output.
 * @details  This function takes a pointer 'ap' to a buffer containing 'size'
 * characters, and it writes 'size' characters from that buffer to the standard
 * output.
 *
 * @param  ap  A pointer to the buffer containing the annotation data to be
 * written.
 * @param  size  The number of bytes of data to be written.
 * @return  1 if 'size' bytes of data were successfully written; otherwise 0.
 */
int write_annotation(char *buffer, unsigned int size){
    for(int i = 0; i < size ; i++ ){
        printf("%c",*(buffer + i));
        debug("%x",*(buffer + i));
    }
    return 1;
}


/**
 * @brief Read, from the standard input, a single frame of audio data having
 * a specified number of channels and bytes per sample.
 * @details  This function takes a pointer 'fp' to a buffer having sufficient
 * space to hold 'channels' values of type 'int', it reads
 * 'channels * bytes_per_sample' data bytes from the standard input,
 * interpreting each successive set of 'bytes_per_sample' data bytes as
 * the big-endian representation of a signed integer sample value, and it
 * stores the decoded sample values into the specified buffer.
 *
 * @param  fp  A pointer to the buffer that is to receive the decoded sample
 * values.
 * @param  channels  The number of channels.
 * @param  bytes_per_sample  The number of bytes per sample.
 * @return  1 if a complete frame was read without error; otherwise 0.
 */
int read_frame(int *fp, int channels, int bytes_per_sample){
    char *data_first = (char *)fp;
    char *data_second = data_first + 4;
    char c = 'p';
    int flag = 0;
    int char_in_int = 0;
    //debug("READ FRAME()");
    //debug("data first addr : %p data second addr : %p",data_first,data_second);

    //strange++;
    *fp = 0;
    for(int i = 0 ; i < bytes_per_sample ; i++){
        char_in_int = getchar();
        c = (char)char_in_int;
        if(char_in_int == EOF){
            if( i == 0 ){
                unsigned long *info = (unsigned long *)fp;
                debug("Reached end of file but good exit");
                *info = 0xffffffffffffffff;
                //return 1;
            }
            debug("reached end of file returning");
            return 0;
        }
        //scanf("%c",&c);
        //debug("~~~~ AJA ~~~~~");
        int n = c&0x80;
        //debug("char c : %x",c);
        //debug("char c : %x and n : %x",c,n);
        if( i == 0 && n == 0x80){
            //debug("FIRST FRAME ==== FLAG SET =====");
            flag = 1;
        }
        if(flag == 1){
            *( data_first + (bytes_per_sample - 1 - i)) = c^0xff;
        }else
            *( data_first + (bytes_per_sample - 1 - i) ) = c;
        //debug( " i : %d and char is %c ",i,c);
    }
    if(flag == 1){
        *fp = *fp + 1;
        *fp = -1 * *fp;
    }
    //debug("     Read Frame :     First no is %d",*fp);

    if(channels == 1)
        return 1;
    //debug("SECOND FRAME");
    fp++;
    *fp = 0;
    flag = 0;
    for(int i = 0 ; i < bytes_per_sample ; i++){
        int check = getchar();
        c = (char)check;
        if(check == EOF){
            return 0;
        }
        //scanf("%c",&c);
        //scanf("%c",&c);
        //debug("~~~~ AJA ~~~~~");
        int n = c&0x80;
        //debug("char c : %x and n : %x",c,n);
        if( i == 0 && n == 0x80){
            //debug("SECOND FRAME ==== FLAG SET =====");
            //debug("==== FLAG SET =====");
            flag = 1;
        }
        if(flag == 1){
            *(data_second + (bytes_per_sample - 1 - i ) ) = c^0xff;
        }else
            *(data_second + (bytes_per_sample - 1 - i ) ) = c;
    }
    if(flag == 1){
        *fp = *fp + 1;
        *fp = -1 * *fp;
    }
    //debug("*fp : %d",*fp);
    //debug("     Read Frame :     Second no 1s complement %d  and hex %x flag : %d ",*fp,*fp,flag);
    //debug("     Read Frame :     Second no is %d and hex %x",*fp,*fp);
    //debug("Writing frame complete");
    return 1;
}


/**
 * @brief  Write, to the standard output, a single frame of audio data having
 * a specified number of channels and bytes per sample.
 * @details  This function takes a pointer 'fp' to a buffer that contains
 * 'channels' values of type 'int', and it writes these data values to the
 * standard output using big-endian byte order, resulting in a total of
 * 'channels * bytes_per_sample' data bytes written.
 *
 * @param  fp  A pointer to the buffer that contains the sample values to
 * be written.
 * @param  channels  The number of channels.
 * @param  bytes_per_sample  The number of bytes per sample.
 * @return  1 if the complete frame was written without error; otherwise 0.
 */
int write_frame(int *fp, int channels, int bytes_per_sample){
    char *data_first = (char *)fp;
    char *data_second = data_first + 4;
    int flag = 0;
    //debug("First frame before writing :%d",*fp);
    if(*fp < 0){
        *fp = *fp * -1;
        *fp = *fp - 1;
        flag = 1;
    }
    //debug("First frame first Complement :%d  in hex %x",*fp, *fp);
    //char c = 'p';
    //debug("Writing Frame");
    for(int i = 0 ; i < bytes_per_sample ; i++){
        if(flag == 1){
            printf("%c",( *(data_first + (bytes_per_sample - 1 - i) ) )^0xff);
        }else{
            printf("%c",*(data_first + ( bytes_per_sample - 1 - i ) ) );
        }
    }
    //debug("First frame final writing :%d  in hex %x",*fp,*fp);
    //debug("No is %d",*fp);
    if(channels == 1)
        return 1;
    flag = 0;
    fp++;
    //debug("Second frame before writing :%d",*fp);
    if(*fp < 0){
        *fp = *fp * -1;
        *fp = *fp - 1;
        flag = 1;
    }
    //debug("Second frame first Complement :%d in hex %x",*fp,*fp);
    for(int i = 0 ; i < bytes_per_sample ; i++){
        if(flag == 1)
            printf("%c",( *( data_second + ( bytes_per_sample - 1 - i ) ) )^0xff);
        else
            printf("%c",*(data_second + ( bytes_per_sample - 1 - i) ) );
    }
    //debug("Second frame final writing :%d in hex %x",*fp,*fp);
    //debug("Second No is %d",*fp);
    return 1;
}

void Copy_data_frame_to_internal_frame( char *data_first, char *data_second, char *first, char *second){
    for(int i = 0 ; i < 4 ; i++){
        *(first + i) = *(data_first + i);
        *(second + i) = *(data_second + i);
    }
}

void Copy_internal_frame_to_data_frame(char *data_first, char *data_second, char *first , char *second){
    for(int i = 0 ; i < 4 ; i++){
        *(data_first + i) = *(first + i);
        *(data_second + i) = *(second + i);
    }
}
#include <stdlib.h>
#include <string.h>

#ifndef STRSPLIT 

int count_word_between_delimiter (char * string, const char delimiter) 
{
    int chunks_counter = 0; 
    char * string_iterator = string; 
    char * last_delimiter_position = NULL; 
    while (*string_iterator) {
        
        if (delimiter == *string_iterator) {
            ++ chunks_counter; 
            last_delimiter_position = string_iterator;             
        }
        ++ string_iterator; 
    } 

    // increment chunks if there is another string after the last
    // delimiter detected. 
    char * last_logic_address_position = string + strlen(string) - 1;  
    if (last_delimiter_position < last_logic_address_position)
        ++ chunks_counter; 
    
    return chunks_counter; 
}

char ** strsplit (char * string, const char delimiter)
{
    char ** chunks = 0;
    size_t chunks_counter = count_word_between_delimiter(string, delimiter); 

    // increment the counter to assign to the last element a NULL
    // pointer so the caller can find the end of the strings array 
    ++ chunks_counter; 

    char delimiter_as_string[2]; 
    delimiter_as_string[0] = delimiter; 
    delimiter_as_string[1] = '\0'; 
     
    // allocating the space for each pointer to a chunk 
    chunks = malloc(sizeof(char*) * chunks_counter); 

    size_t chunks_index_pointer = 0; 
    char * token = strtok(string, delimiter_as_string); 
    while (token) {
        *(chunks + chunks_index_pointer ++) = strdup(token);
        token = strtok(0, delimiter_as_string);      
    }
    *(chunks + chunks_index_pointer) = NULL; 
    return chunks; 
}

#endif
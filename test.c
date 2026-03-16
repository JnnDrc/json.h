#include <stdio.h>
#include <stdlib.h>

#include <cjson/cJSON.h>

#define JSONH_IMPL
#include "json.h"

noreturn void panic(char* msg){
    fprintf(stderr,"%s\n",msg);
    exit(1);
}

int main(void){

    FILE* f = fopen("test.json","r");
    if(!f) return -1;

    jsonh_t* json = jsonh_read(f);

    printf("ROOT: %d: %p\n",json->type,json->value.child);
    
    // jsonh_write(stdout,json);

    jsonh_t* strings = jsonh_obj_get(json,"strings");
    jsonh_t* string  = jsonh_obj_get(strings,"simple");

    jsonh_write(stdout,string);

    jsonh_t* array = jsonh_obj_get(json,"arrays");
    if(!array) panic("array does not exist");
    size_t len = jsonh_arr_size(array);
    printf("\narray has %zu elements\n",len);
    jsonh_t* arr3  = jsonh_arr_get(array,3);
    if(!arr3) panic("array doesn't have three elements");

    jsonh_write(stdout,arr3);

    jsonh_delete(json);

    return 0;
}

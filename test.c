#include <stdio.h>
#include <stdlib.h>

#include <cjson/cJSON.h>

#define JH_PREFIX
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

    FILE* out = fopen("out.json","wb");
    if(!out) return -1;

    jsonh_t* obj = jsonh_obj_get(json,"object");
    if(!obj) panic("no object");
    jsonh_write(stdout,obj);
    jsonh_t* Obj = jsonh_obj_iget(json,"ObJeCT");
    if(!Obj) panic("No ObJeCT");
    jsonh_write(stdout,Obj);


    jsonh_obj_add(obj,"anull",jsonh_new_null());
    jsonh_obj_del(obj,"nested");
    
    jsonh_write(out,obj);

    FILE* out2 = fopen("out2.json","wb");
    if(!out2) return -1;

    jsonh_t* floats = jh_obj_get(json,"floats");
    jh_arr_pop(floats);
    jh_arr_push(floats,jh_new_number(6.7f));
    
    jh_arr_pull(floats);
    jh_arr_put(floats,jh_new_number(6.9f));

    jh_arr_remove(floats,2);
    jh_arr_insert(floats,2,jh_new_number(2.4f));

    jh_num_set(jh_arr_get(floats,1),0.1420);

    jh_write(out2,floats);
    
    jsonh_delete(json);

    return 0;
}

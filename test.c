#include <stdio.h>
#include <stdlib.h>

#define JSONH_IMPL
#include "json.h"

int main(void){

    FILE* f = fopen("test.json","r");
    if(!f) return -1;

    fseek(f,0L,SEEK_END);
    size_t siz = ftell(f);
    rewind(f);

    char* buf = calloc(1,siz);
    if(!buf) return -1;

    fread(buf,1,siz,f);

    jsonh_t* json = jsonh_parse(buf,strlen(buf));

    printf("ROOT: %d: %p\n",json->type,json->value.child);
    
    jsonh_print(json);

    jsonh_t* str = jsonh_get(json,"str");

    jsonh_delete(json);

    return 0;
}

# JSON.h

single header json parser/library

## Usage

```c
#define JSONH_IMPL  // include implementation
#include "jsonh.h"  // header, see here for api functions/docs
int main(){
    // example code
    jsonh_t* json = jsonh_read("file.json");

    jsonh_obj_add(json,"number",jsonh_new_number(6.7));

    jsonh_write(stdout, json);

    jsonh_delete(json);
    return 0;
}
```

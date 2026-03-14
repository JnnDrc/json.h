#ifndef _JSON_H
#define _JSON_H

// TODO: Better print/formatting
// TODO: Better error handling
// TODO: Edit/Write API
// TODO: Iterate over array/object API
// TODO: Better type checking

// -----------------------------------------------------------------------------
// JSONH API -------------------------------------------------------------------
// -----------------------------------------------------------------------------

#include <stddef.h>
#include <stdnoreturn.h>
#if __STDC_VERSION__ < 202000L
#include <stdbool.h>
#endif /* < C23 */

// currently, jsonh supports only ASCII, instead of Unicode (ecma-404)
#ifdef JSONH_WIDE
#include <wchar.h>
typedef wchar_t jchar_t;
#error "WIDE CHARS NOT SUPPORTED YET"
#else
typedef char jchar_t;
#endif /* JSONH_WIDE */

typedef enum jsonh_type{
    // value types specified by ecma-404
    JSONH_OBJECT,
    JSONH_ARRAY,
    JSONH_NUMBER,
    JSONH_STRING,
    JSONH_TRUE,
    JSONH_FALSE,
    JSONH_NULL,
}json_type_n;

typedef struct jsonh{
    json_type_n type;           // value type

    // sequetial walk on object/array items, values are ordered by order in file
    struct jsonh* next;
    struct jsonh* prev;

    union{
        jchar_t*  str;          // value as a string
        double    num;          // value as number
        bool      bol;          // value as boolean (true/false)
        struct jsonh* child;    // pointer to first element of object/array
    }value;

    jchar_t* name;              // value name if it's the child of a object
}jsonh_t;

jsonh_t* jsonh_parse(jchar_t* src, size_t len);
void jsonh_delete(jsonh_t* root);

jsonh_t* jsonh_get(jsonh_t* root, char* key);

int      jsonh_print(jsonh_t* root);

int      jsonh_print_object(jsonh_t* obj);
int      jsonh_print_array(jsonh_t* arr);

// -----------------------------------------------------------------------------
// INTERNALS -------------------------------------------------------------------
// -----------------------------------------------------------------------------

// definition of ecma-404 specified Unicode codepoints to match specification (nerd shit)
// structural tokens (ecma-404)
#define _JTOK_LSBRA 0x005B  // [
#define _JTOK_LCBRA 0x007B  // {
#define _JTOK_RSBRA 0x005D  // ]
#define _JTOK_RCBRA 0x007D  // }
#define _JTOK_COLON 0x003A  // :
#define _JTOK_COMMA 0x002C  // ,
// literal name tokens (first char only) (ecma-404)
#define _JTOK_TRUE  0x0074  // t
#define _JTOK_FALSE 0x0066  // f
#define _JTOK_NULL  0x006E  // n
// white space characters (ecma-404) codepoints
#define _JWSP_TB   0x0009   // \t
#define _JWSP_LF   0x000A   // \n
#define _JWSP_CR   0x000D   // \r
#define _JWSP_SP   0x0020   // ' '
// other characters (ecma-404)
#define _JCHR_QUOT 0x0022 // string marker, "
#define _JCHR_PSIG 0x002B // plus,  +
#define _JCHR_MSIG 0x002D // minus, -
#define _JCHR_DCPT 0x002E // decimal point, .
#define _JCHR_SEXP 0x0065 // small exponential marker, e
#define _JCHR_CEXP 0x0045 // capital exponential marker, E
#define _JCHR_DIG0 0x0030 // 0
#define _JCHR_DIG9 0x0039 // 9
#define _JCHR_RSOL 0x005C // reverse solidus, '\'
#define _JCHR_SOLI 0x002F // solidus, /

// definiton of ecma-404 literal name values
#define _JLIT_TRUE  "true"
#define _JLIT_FALSE "false"
#define _JLIT_NULL  "null"

#define _JLEN_TRUE   sizeof(_JLIT_TRUE)  - 1
#define _JLEN_FALSE  sizeof(_JLIT_FALSE) - 1
#define _JLEN_NULL   sizeof(_JLIT_NULL)  - 1

// parser state

struct jsonh_parser{
    jchar_t*    src;
    size_t      size;
    size_t      pos;
    size_t      line, column;
    jchar_t     cur;
};

static jchar_t _jhY_next(struct jsonh_parser* p);
static jchar_t _jhY_peek(struct jsonh_parser* p);
static int _jhY_check(struct jsonh_parser* p, jchar_t c);
static void _jhY_skipws(struct jsonh_parser* p);
static int _jhy_isdigit(struct jsonh_parser* p);

noreturn static void _jhY_panic(char* fmt,...);

// value parsers
static jsonh_t* _jsonhY_parse_object(struct jsonh_parser* p);
static jsonh_t* _jsonhY_parse_array(struct jsonh_parser* p);
static jsonh_t* _jsonhY_parse_string(struct jsonh_parser* p);
static jsonh_t* _jsonhY_parse_true(struct jsonh_parser* p);
static jsonh_t* _jsonhY_parse_false(struct jsonh_parser* p);
static jsonh_t* _jsonhY_parse_null(struct jsonh_parser* p);
static jsonh_t* _jsonhY_parse_number(struct jsonh_parser* p);

static jsonh_t* _jsonhY_parse_value(struct jsonh_parser* p);

#endif /* _JSON_H */

// -----------------------------------------------------------------------------
// IMPLEMENTATION
// -----------------------------------------------------------------------------

#if defined JSONH_IMPL || defined JSONH_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

// PARSER ----------------------------------------------------------------------

static jchar_t _jhY_next(struct jsonh_parser* p){
    if(p->cur == '\n'){
        p->line++;
        p->column = 1;
    }else p->column++;
    p->pos++;
    return p->cur = p->src[p->pos];
}

static jchar_t _jhY_peek(struct jsonh_parser* p){
    return p->src[p->pos+1];
}

static int _jhY_check(struct jsonh_parser* p, jchar_t c){
    return (p->cur == c);
}

static void _jhY_skipws(struct jsonh_parser* p){
    while(  p->cur == _JWSP_TB ||
            p->cur == _JWSP_LF ||
            p->cur == _JWSP_CR ||
            p->cur == _JWSP_SP) _jhY_next(p);
}

static int _jhy_isdigit(struct jsonh_parser* p){
    return (p->cur >= _JCHR_DIG0 && p->cur <= _JCHR_DIG9);
}

noreturn static void _jhY_panic(char* fmt,...){
    
    va_list va;
    va_start(va,fmt);
    vfprintf(stderr,fmt,va);
    va_end(va);
    fflush(stderr);
    exit(1);
}

static void _jhY_skip(struct jsonh_parser* p, jchar_t c){
    if(p->cur != c) _jhY_panic("JSONH: Expected '%c' at %zu:%zu but got '%c'",c,p->line,p->column,p->cur);
    _jhY_next(p);
}


// VALUE PARSING ---------------------------------------------------------------

static void _jhY_llappend(jsonh_t** head, jsonh_t** tail, jsonh_t* node){
    node->next =  NULL;
    node->prev = *tail;

    if(*tail) (*tail)->next = node;
    else *head = node;

    *tail = node;
}

static jchar_t* _jhY_parse_cstr(struct jsonh_parser* p){
    _jhY_skip(p,_JCHR_QUOT);

    if(_jhY_check(p,_JCHR_QUOT)){
        // empty string
        jchar_t* s = malloc(1);
        if(!s) return NULL;
        s[0] = '\0';
        return s;
    }

    size_t start = p->pos;

    while(1){
        if(p->pos >= p->size || p->cur == '\0') _jhY_panic("Unterminated string");

        if(_jhY_check(p,_JCHR_QUOT)){
            size_t len = p->pos - start;
            
            jchar_t* out = calloc(len+1,sizeof(jchar_t));
            if(!out) return NULL;
            memcpy(out,&p->src[start],len*sizeof(jchar_t));
            out[len] = '\0';

            return out;
        }
        if(_jhY_check(p,_JCHR_RSOL)) break;
        _jhY_next(p);
    }
    
    size_t plen = p->pos - start;

    size_t cap = plen + 32;
    size_t len = plen;
    jchar_t* buf = calloc(cap,sizeof(jchar_t));
    if(!buf) return NULL;

    memcpy(buf,&p->src[start],plen);
    
    while(1){
        if(p->pos >= p->size || p->cur == '\0') _jhY_panic("Unterminated string");
        if(_jhY_check(p,_JCHR_QUOT)) break;
        
        jchar_t c = p->src[p->pos];

        if(c == _JCHR_RSOL){
            switch((c = _jhY_next(p))){
                case _JCHR_QUOT: c = '"';  break;
                case _JCHR_RSOL: c = '\\'; break;
                case _JCHR_SOLI: c = '/';  break;
                case 'b':  c = '\b'; break;
                case 'f':  c = '\f'; break;
                case 'n':  c = '\n'; break;
                case 'r':  c = '\r'; break;
                case 't':  c = '\t'; break;
                case 'u':
                    {
                        jchar_t hexbuf[5];
                        memcpy(hexbuf,&p->src[p->pos],4*sizeof(jchar_t));
                        hexbuf[4] = '\0';
                        jchar_t hex = strtol(hexbuf,NULL,16);
                        c = hex;
                        p->pos += 3;
                    }
                    break;
                default:
                    _jhY_panic("JSONH: Invalid scape code at %d:%d",p->line,p->column);
            }
        }
        if(len + 1 >= cap){
            cap *= 2;
            buf = realloc(buf,cap);
        }
        buf[len++] = c;

        _jhY_next(p);
    }

    buf[len] = '\0';
    return buf;
}

static jsonh_t* _jsonhY_parse_object(struct jsonh_parser* p){
    jsonh_t* obj = (jsonh_t*)malloc(sizeof(jsonh_t));
    if(!obj) return NULL;
    obj->type = JSONH_OBJECT;
    obj->next = obj->prev = obj->value.child = NULL;
    obj->name = NULL;
    
    _jhY_next(p);
    _jhY_skipws(p);
    if(_jhY_check(p,_JTOK_RCBRA)){
        // empty object
        _jhY_next(p);
        return obj;
    }
    
    jsonh_t* head = NULL;
    jsonh_t* tail = NULL;

    _jhY_skipws(p);
    while(1){
        char* name = _jhY_parse_cstr(p);

        _jhY_next(p);
        _jhY_skipws(p);
        
        _jhY_skip(p,_JTOK_COLON);
        _jhY_skipws(p);

        jsonh_t* member = _jsonhY_parse_value(p);
        member->name = name;
        
        _jhY_llappend(&head,&tail,member);

        _jhY_skipws(p);

        if(_jhY_check(p,_JTOK_RCBRA)){
            _jhY_next(p);
            break;
        }
        _jhY_skip(p,_JTOK_COMMA);

        _jhY_skipws(p);
    }
    obj->value.child = head;

    return obj;
}
static jsonh_t* _jsonhY_parse_array(struct jsonh_parser* p){
    jsonh_t* arr = (jsonh_t*)malloc(sizeof(jsonh_t));
    if(!arr) return NULL;
    arr->type = JSONH_ARRAY;
    arr->next = arr->prev = arr->value.child = NULL;
    arr->name = NULL;
    
    _jhY_next(p);
    _jhY_skipws(p);

    if(_jhY_check(p,_JTOK_RSBRA)){
        // empty array
        _jhY_next(p);
        return arr;
    }
    _jhY_skipws(p);

    jsonh_t* head = NULL;
    jsonh_t* tail = NULL;

    while(1){
        jsonh_t* item = _jsonhY_parse_value(p);
        _jhY_llappend(&head,&tail,item);
        _jhY_skipws(p);
        if(_jhY_check(p,_JTOK_RSBRA)){
            _jhY_next(p);
            break;
        }
        _jhY_skip(p,_JTOK_COMMA);

        _jhY_skipws(p);
    }

    arr->value.child = head;
    return arr;
}
static jsonh_t* _jsonhY_parse_string(struct jsonh_parser* p){
    jsonh_t* str = (jsonh_t*)malloc(sizeof(jsonh_t));
    if(!str) return NULL;
    str->type = JSONH_STRING;
    str->next = str->prev = str->value.child = NULL;
    str->name = NULL;
    
    str->value.str = _jhY_parse_cstr(p);
    _jhY_next(p);

    return str;
}
static jsonh_t* _jsonhY_parse_true(struct jsonh_parser* p){
    if(strncmp(&p->src[p->pos],_JLIT_TRUE,_JLEN_TRUE)) return NULL;
    p->pos += _JLEN_TRUE;
    p->cur = p->src[p->pos];
    jsonh_t* t = (jsonh_t*)malloc(sizeof(jsonh_t));
    if(!t) return NULL;
    t->type = JSONH_TRUE;
    t->next = t->prev = t->value.child = NULL;
    t->name = NULL;
    
    t->value.bol = true;

    return t;
}
static jsonh_t* _jsonhY_parse_false(struct jsonh_parser* p){
    if(strncmp(&p->src[p->pos],_JLIT_FALSE,_JLEN_FALSE)) return NULL;
    p->pos += _JLEN_FALSE;
    p->cur = p->src[p->pos];

    jsonh_t* f = (jsonh_t*)malloc(sizeof(jsonh_t));
    if(!f) return NULL;
    f->type = JSONH_FALSE;
    f->next = f->prev = f->value.child = NULL;
    f->name = NULL;
    
    f->value.bol = false;

    return f;
}
static jsonh_t* _jsonhY_parse_null(struct jsonh_parser* p){
    if(strncmp(&p->src[p->pos],_JLIT_NULL,_JLEN_NULL)) return NULL;
    p->pos += _JLEN_NULL;
    p->cur = p->src[p->pos];
    jsonh_t* f = (jsonh_t*)malloc(sizeof(jsonh_t));
    if(!f) return NULL;
    f->type = JSONH_NULL;
    f->next = f->prev = f->value.child = NULL;
    f->name = NULL;
    
    f->value.num = 0;

    return f;
}
static jsonh_t* _jsonhY_parse_number(struct jsonh_parser* p){
    jsonh_t* num = (jsonh_t*)malloc(sizeof(jsonh_t));
    if(!num) return NULL;
    num->type = JSONH_NUMBER;
    num->next = num->prev = num->value.child = NULL;
    num->name = NULL;

    int sign = 1;
    size_t digits = 0;
    int frac_digits = 0;
    int exp_sign = 1;
    size_t exp = 0;
    int exp10       = 0;

    if(_jhY_check(p,_JCHR_MSIG)){
        sign = -1;
        _jhY_next(p);
    }
    
    // integer part
    if(_jhY_check(p,_JCHR_DIG0)) _jhY_next(p);
    else{
        if(!_jhy_isdigit(p)) _jhY_panic("JSONH: Expected digit at %d:%d",p->line,p->column);
        while(_jhy_isdigit(p)){
            digits = (digits*10) + (p->cur - _JCHR_DIG0);
            _jhY_next(p);
        }
    }

    
    // decimal part
    if(_jhY_check(p,_JCHR_DCPT)){
        _jhY_next(p);
        if(!_jhy_isdigit(p)) _jhY_panic("JSONH: Expected digit at %d:%d",p->line,p->column);
        while(_jhy_isdigit(p)){
            digits = (digits*10) + (p->cur - _JCHR_DIG0);
            frac_digits++;
            _jhY_next(p);
        }
    }
    
    // exponent
    if(_jhY_check(p,_JCHR_SEXP) || _jhY_check(p,_JCHR_CEXP)){
        _jhY_next(p);

        if(_jhY_check(p,_JCHR_PSIG) || _jhY_check(p,_JCHR_MSIG)){
            if(_jhY_check(p,_JCHR_MSIG)) exp_sign = -1;
            _jhY_next(p);
        }

        if(!_jhy_isdigit(p)) _jhY_panic("JSONH: Expected digit at %d:%d",p->line,p->column);

        while(_jhy_isdigit(p)){
            exp = (exp * 10) + (p->cur - _JCHR_DIG0);
            _jhY_next(p);
        }
    }
    
    exp10 = (exp_sign * exp) - frac_digits;
    double val = (double)digits;
    if(exp10 != 0) val *= pow(10.0,exp10);

    num->value.num = sign * val;
    return num;
}


static jsonh_t* _jsonhY_parse_value(struct jsonh_parser* p){
    _jhY_skipws(p);

    switch(p->cur){
        case _JTOK_LCBRA: return _jsonhY_parse_object(p);
        case _JTOK_LSBRA: return _jsonhY_parse_array(p);
        case _JCHR_QUOT:  return _jsonhY_parse_string(p);
        case _JTOK_TRUE:  return _jsonhY_parse_true(p);
        case _JTOK_FALSE: return _jsonhY_parse_false(p);
        case _JTOK_NULL:  return _jsonhY_parse_null(p);
        default:          return _jsonhY_parse_number(p);
    }

}

jsonh_t* jsonh_parse(char* src, size_t len){
    jsonh_t* json;
    struct jsonh_parser p = {.src = src,.size = len, .line = 1, .column = 1, .cur = src[0]};
    json = _jsonhY_parse_value(&p);
    return json;
}

void jsonh_delete(jsonh_t* root){
    if(!root) return;

        if(root->type == JSONH_OBJECT || root->type == JSONH_ARRAY){
            jsonh_t* cur = root->value.child;

            while(cur){
                jsonh_t* next = cur->next;
                jsonh_delete(cur);
                cur = next;
            }
        }
    else if(root->type == JSONH_STRING){
        free(root->value.str);
    }

    if(root->name)
        free(root->name);

    free(root);
}

jsonh_t* jsonh_get(jsonh_t* root, char* key){
    for(jsonh_t* cur = root->value.child; cur; cur = cur->next){
        if(!strcmp(cur->name,key)) return cur;
    }
    return NULL;
}

int      jsonh_print(jsonh_t* root){
    int r = 0;
    for(jsonh_t* cur = root->value.child; cur; cur = cur->next){
        switch(cur->type){
        case JSONH_OBJECT: 
            r += jsonh_print_object(cur);
            break;
        case JSONH_ARRAY:
            r += jsonh_print_array(cur);
            break;
        case JSONH_NUMBER:
            r += printf("%s: %lf\n",cur->name,cur->value.num);
            break;
        case JSONH_STRING:
            r += printf("%s: %s\n",cur->name,cur->value.str);
            break;
        case JSONH_TRUE:
            r += printf("%s: true\n",cur->name);
            break;
        case JSONH_FALSE:
            r += printf("%s: false\n",cur->name);
            break;
        case JSONH_NULL:
            r += printf("%s: null\n",cur->name);
            break;
        }
    }
    return r;
}
int      jsonh_print_object(jsonh_t* obj){
    if(!obj->value.child) return printf("{}\n");
    int r = 0;
    if(obj->name) r += printf("%s: {\n", obj->name);
    else r += printf("{\n");
    for(jsonh_t* cur = obj->value.child; cur; cur = cur->next){
        switch(cur->type){
        case JSONH_OBJECT: 
            r += jsonh_print_object(cur);
            break;
        case JSONH_ARRAY:
            r += jsonh_print_array(cur);
            break;
        case JSONH_NUMBER:
            r += printf("%s: %lf\n",cur->name,cur->value.num);
            break;
        case JSONH_STRING:
            r += printf("%s: %s\n",cur->name,cur->value.str);
            break;
        case JSONH_TRUE:
            r += printf("%s: true\n",cur->name);
            break;
        case JSONH_FALSE:
            r += printf("%s: false\n",cur->name);
            break;
        case JSONH_NULL:
            r += printf("%s: null\n",cur->name);
            break;
        }
    }
    r += printf("}\n");
    return r;
}
int      jsonh_print_array(jsonh_t* arr){
    if(!arr->value.child) return printf("[]\n");
    int r = 0;
    if(arr->name) r += printf("%s: [", arr->name);
    else r += printf("[");
    for(jsonh_t* cur = arr->value.child; cur; cur = cur->next){
        switch(cur->type){
        case JSONH_OBJECT:
            r += jsonh_print_object(cur);
            break;
        case JSONH_ARRAY: 
            r += jsonh_print_array(cur);
            break;
        case JSONH_NUMBER:
            r += printf("%lf",cur->value.num);
            break;
        case JSONH_STRING:
            r += printf("%s",cur->value.str);
            break;
        case JSONH_TRUE:
            r += printf("true");
            break;
        case JSONH_FALSE:
            r += printf("false");
            break;
        case JSONH_NULL:
            r += printf("null");
            break;
        }
        if(cur->next) printf(", ");
    }
    r += printf("]\n");
    return r;
}

#endif /* JSONH_IMPL */

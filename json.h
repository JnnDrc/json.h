#ifndef _JSON_H
#define _JSON_H

// TODO: Better error handling
// TODO: Edit/Write API
// TODO: Iterate over array/object API
// TODO: Better type checking
// TODO: Unicode/wchar_t support

// -----------------------------------------------------------------------------
// JSONH API -------------------------------------------------------------------
// -----------------------------------------------------------------------------

#include <stdio.h>
#include <stddef.h>
#include <stdnoreturn.h>
#if __STDC_VERSION__ < 202000L
#include <stdbool.h>
#endif /* < C23 */

// currently, jsonh supports only ASCII and WIDE mode (unlike UNICODE of ecma-404, blame C standard for this)
#ifdef JSONH_WIDE
// Because of wide char being the best c can do, wide is is very shit to use,
// you will need to convert output files (may be utf-16le) to utf-8
// i highly recommend to use ASCII mode

#include <wchar.h>
#include <wctype.h>
typedef wchar_t jchar_t;
#define jcslen(s)        wcslen(s)
#define jcscmp(s1,s2)    wcscmp(s1,s2)
#define jcsncmp(s1,s2,n) wcsncmp(s1,s2,n)
#define jcscpy(s1,s2)    wcscpy(s1,s2)
#define jcsdup(s)        wcsdup(s)
#define jcstol(n,e,b)    wcstol(n,e,b)

#define tojlower(c)      towlower((wint_t)(c))

#define fgetjs(d,s,f)   fgetws(d,s,f)
#define fgetjc(f)       fgetwc(f)

#define fjprintf fwprintf

#define JEOF WEOF

#define J(s) L##s
#define JFMT "%ls"

#else

typedef char jchar_t;
#define jcslen(s)        strlen(s)
#define jcscmp(s1,s2)    strcmp(s1,s2)
#define jcsncmp(s1,s2,n) strncmp(s1,s2,n)
#define jcscpy(s1,s2)    strcpy(s1,s2)
#define jcsdup(s)        strdup(s)
#define jcstol(n,e,b)    strtol(n,e,b)

#define tojlower(c)      towlower((unsigned char)(c))

#define fgetjs(d,s,f)   fgets(d,s,f)
#define fgetjc(f)       fgetc(f)

#define fjprintf fprintf

#define JEOF EOF

#define J(s) s
#define JFMT "%s"

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
}jsonh_type_n;

typedef struct jsonh{
    jsonh_type_n type;           // value type

    // sequetial walk on object/array items, values are ordered by order in file
    struct jsonh* next;
    struct jsonh* prev;

    union{
        jchar_t*  str;          // value as string
        double    num;          // value as number
        bool      bol;          // value as boolean (true/false)
        struct jsonh* child;    // pointer to first element of object/array
    }value;

    jchar_t* name;              // value name if it's the child of an object
}jsonh_t;

// parse -------------------------------
// parse jsonh string
jsonh_t* jsonh_parse(jchar_t* src, size_t len);
// open file and parse it as json
jsonh_t* jsonh_read(FILE* stream);

// Object methods ----------------------
// check if object have member #key
bool     jsonh_obj_has(jsonh_t* obj,jchar_t* key);
// return member #key of object or NULL if doesn't exist
jsonh_t* jsonh_obj_get(jsonh_t* obj, jchar_t* key);
// check if object have member #key, case insensitive
bool     jsonh_obj_ihas(jsonh_t* obj, jchar_t* key);
// return member #key of object or NULL if doesn't exist, case insensitive
jsonh_t* jsonh_obj_iget(jsonh_t* obj, jchar_t* key);
// add item to object
int      jsonh_obj_add(jsonh_t* obj, jchar_t* key, jsonh_t* item);
// remove item from object
int      jsonh_obj_del(jsonh_t* obj, jchar_t* key);
// remove item from object, case insensitive
int      jsonh_obj_idel(jsonh_t* obj, jchar_t* key);

// Array methods -----------------------
size_t   jsonh_arr_size(jsonh_t* arr);
jsonh_t* jsonh_arr_get(jsonh_t* arr, size_t index);
int      jsonh_arr_push(jsonh_t* arr, jsonh_t* item);
int      jsonh_arr_pop(jsonh_t* arr);
int      jsonh_arr_put(jsonh_t* arr, jsonh_t* item);
int      jsonh_arr_pull(jsonh_t* arr);
int      jsonh_arr_insert(jsonh_t* arr, size_t index, jsonh_t* item);
int      jsonh_arr_remove(jsonh_t* arr, size_t index);

// Number methods ----------------------
// get number value
double   jsonh_num_get(jsonh_t* num);
// set number value
int      jsonh_num_set(jsonh_t* num, double val);

// String methods ----------------------
// get string value
jchar_t* jsonh_str_get(jsonh_t* str);
// get duplicate of string value (need to free)
jchar_t* jsonh_str_dup(jsonh_t* str);
// set string value
int      jsonh_str_set(jsonh_t* str, jchar_t* val);

// Bool methods ------------------------
// get bool value
bool     jsonh_bol_get(jsonh_t* bol);
// set bool value
int      jsonh_bol_set(jsonh_t* bol, bool val);

// is ----------------------------------
// check type of json object
bool jsonh_is(jsonh_t* json, jsonh_type_n type);
bool jsonh_is_object(jsonh_t* json);
bool jsonh_is_array(jsonh_t* json);
bool jsonh_is_number(jsonh_t* json);
bool jsonh_is_string(jsonh_t* json);
bool jsonh_is_true(jsonh_t* json);
bool jsonh_is_false(jsonh_t* json);
bool jsonh_is_bool(jsonh_t* json);
bool jsonh_is_null(jsonh_t* json);

// new ----------------------------
// create object of type
jsonh_t* jsonh_new(jsonh_type_n type);
jsonh_t* jsonh_new_object(void);
jsonh_t* jsonh_new_array(void);
jsonh_t* jsonh_new_number(double num);
jsonh_t* jsonh_new_string(jchar_t* str);
jsonh_t* jsonh_new_true(void);
jsonh_t* jsonh_new_false(void);
jsonh_t* jsonh_new_bool(bool bol);
jsonh_t* jsonh_new_null(void);

// output ------------------------------
// print json structure to stdout
int jsonh_print(jsonh_t* root);
// print json structure to file stream
int jsonh_write(FILE* stream, jsonh_t* root);
// TODO: render json structure to a string
jchar_t* jsonh_render(jsonh_t* root);

// clean-up ----------------------------
// recursive delete of json value
void     jsonh_delete(jsonh_t* root);

#if defined JSONH_PREFIX || defined JH_PREFIX

#define jh_parse        jsonh_parse
#define jh_read         jsonh_read

#define jh_obj_has      jsonh_obj_has
#define jh_obj_get      jsonh_obj_get
#define jh_obj_ihas     jsonh_obj_ihas
#define jh_obj_iget     jsonh_obj_iget
#define jh_obj_add      jsonh_obj_add
#define jh_obj_del      jsonh_obj_del
#define jh_obj_idel     jsonh_obj_idel

#define jh_arr_size     jsonh_arr_size
#define jh_arr_get      jsonh_arr_get
#define jh_arr_push     jsonh_arr_push
#define jh_arr_pop      jsonh_arr_pop
#define jh_arr_put      jsonh_arr_put
#define jh_arr_pull     jsonh_arr_pull
#define jh_arr_insert   jsonh_arr_insert
#define jh_arr_remove   jsonh_arr_remove

#define jh_num_get   jsonh_num_get
#define jh_num_set   jsonh_num_set

#define jh_str_get   jsonh_str_get
#define jh_str_dup   jsonh_str_dup
#define jh_str_set   jsonh_str_set

#define jh_bol_get   jsonh_bol_get
#define jh_bol_set   jsonh_bol_set

#define jh_is           jsonh_is
#define jh_is_object    jsonh_is_object
#define jh_is_array     jsonh_is_array
#define jh_is_number    jsonh_is_number
#define jh_is_string    jsonh_is_string
#define jh_is_true      jsonh_is_true
#define jh_is_false     jsonh_is_false
#define jh_is_bool      jsonh_is_bool
#define jh_is_null      jsonh_is_null

#define jh_new          jsonh_new
#define jh_new_object   jsonh_new_object
#define jh_new_array    jsonh_new_array
#define jh_new_number   jsonh_new_number
#define jh_new_string   jsonh_new_string
#define jh_new_true     jsonh_new_true
#define jh_new_false    jsonh_new_false
#define jh_new_bool     jsonh_new_bool
#define jh_new_null     jsonh_new_null

#define jh_print        jsonh_print
#define jh_write        jsonh_write
#define jh_sprint       jsonh_sprint

#define jh_delete       jsonh_delete

#endif /* JSONH_PREFIX */

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
static const jchar_t _JLIT_TRUE[]  = {0x0074,0x0072,0x0075,0x0065};
static const jchar_t _JLIT_FALSE[] = {0x0066,0x0061,0x006C,0x0073,0x0065};
static const jchar_t _JLIT_NULL[]  = {0x006E,0x0075,0x006C,0x006C};

#define _JLEN_TRUE   (sizeof(_JLIT_TRUE)  / sizeof(jchar_t))
#define _JLEN_FALSE  (sizeof(_JLIT_FALSE) / sizeof(jchar_t))
#define _JLEN_NULL   (sizeof(_JLIT_NULL)  / sizeof(jchar_t))

// parser state

struct _jhY{
    jchar_t*    src;
    size_t      size;
    size_t      pos;
    size_t      line, column;
    jchar_t     cur;
};

static jchar_t _jhY_next(struct _jhY* p);
static jchar_t _jhY_peek(struct _jhY* p);
static int _jhY_check(struct _jhY* p, jchar_t c);
static void _jhY_skipws(struct _jhY* p);
static int _jhY_isdigit(struct _jhY* p);

noreturn static void _jhY_panic(char* fmt,...);

// value parsers
static jsonh_t* _jhY_parse_object(struct _jhY* p);
static jsonh_t* _jhY_parse_array(struct _jhY* p);
static jsonh_t* _jhY_parse_string(struct _jhY* p);
static jsonh_t* _jhY_parse_true(struct _jhY* p);
static jsonh_t* _jhY_parse_false(struct _jhY* p);
static jsonh_t* _jhY_parse_null(struct _jhY* p);
static jsonh_t* _jhY_parse_number(struct _jhY* p);

static jsonh_t* _jhY_parse_value(struct _jhY* p);

static void _jhY_llappend(jsonh_t** head, jsonh_t** tail, jsonh_t* node);

// printing

static int _jh_indent(FILE* stream, int depth);
static int _jh_print_object(FILE* stream, jsonh_t* node, int depth);
static int _jh_print_array(FILE* stream, jsonh_t* node, int depth);
static int _jh_print_value(FILE* stream, jsonh_t* node, int depth);

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

static jchar_t _jhY_next(struct _jhY* p){
    if(p->cur == '\n'){
        p->line++;
        p->column = 1;
    }else p->column++;
    p->pos++;
    if(p->pos >= p->size) return p->cur = 0;
    return p->cur = p->src[p->pos];
}

static jchar_t _jhY_peek(struct _jhY* p){
    if(p->pos + 1 >= p->size) return 0;
    return p->src[p->pos+1];
}

static int _jhY_check(struct _jhY* p, jchar_t c){
    return (p->cur == c);
}

static void _jhY_skipws(struct _jhY* p){
    while(  p->cur == _JWSP_TB ||
            p->cur == _JWSP_LF ||
            p->cur == _JWSP_CR ||
            p->cur == _JWSP_SP) _jhY_next(p);
}

static int _jhY_isdigit(struct _jhY* p){
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

static void _jhY_skip(struct _jhY* p, jchar_t c){
    if(p->cur != c) _jhY_panic("JSONH: Expected '%c' at %zu:%zu but got '%c'",c,p->line,p->column,p->cur);
    _jhY_next(p);
}

// JSON PARSING ----------------------------------------------------------------

static jchar_t* _jhY_parse_cstr(struct _jhY* p){
    _jhY_skip(p,_JCHR_QUOT);

    if(_jhY_check(p,_JCHR_QUOT)){
        // empty string
        jchar_t* s = (jchar_t*)calloc(1,sizeof(jchar_t));
        if(!s) return NULL;
        s[0] = 0;
        return s;
    }

    size_t start = p->pos;

    while(1){
        if(p->pos >= p->size || p->cur == '\0') _jhY_panic("Unterminated string");

        if(_jhY_check(p,_JCHR_QUOT)){
            size_t len = p->pos - start;
            
            jchar_t* out = (jchar_t*)calloc(len+1,sizeof(jchar_t));
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
    jchar_t* buf = (jchar_t*)calloc(cap,sizeof(jchar_t));
    if(!buf) return NULL;

    memcpy(buf,&p->src[start],plen*sizeof(jchar_t));
    
    while(1){
        if(p->pos >= p->size || p->cur == '\0') _jhY_panic("Unterminated string");
        if(_jhY_check(p,_JCHR_QUOT)) break;
        
        jchar_t c = p->cur;

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
                        for(int i = 0; i < 4; i++) hexbuf[i] = _jhY_next(p);
                        hexbuf[4] = 0;
                        jchar_t hex = jcstol(hexbuf,NULL,16);
                        c = hex;
                    }
                    break;
                default:
                    _jhY_panic("JSONH: Invalid scape code at %d:%d",p->line,p->column);
            }
        }
        if(len + 1 >= cap){
            cap *= 2;
            buf = (jchar_t*)realloc(buf,cap*sizeof(jchar_t));
        }
        buf[len++] = c;

        _jhY_next(p);
    }

    buf[len] = 0;
    return buf;
}

static jsonh_t* _jhY_parse_object(struct _jhY* p){
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
    _jhY_skipws(p);
    
    jsonh_t* head = NULL;
    jsonh_t* tail = NULL;

    while(1){
        jchar_t* name = _jhY_parse_cstr(p);

        _jhY_next(p);
        _jhY_skipws(p);
        
        _jhY_skip(p,_JTOK_COLON);
        _jhY_skipws(p);

        jsonh_t* member = _jhY_parse_value(p);
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
static jsonh_t* _jhY_parse_array(struct _jhY* p){
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
        jsonh_t* item = _jhY_parse_value(p);
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
static jsonh_t* _jhY_parse_string(struct _jhY* p){
    jsonh_t* str = (jsonh_t*)malloc(sizeof(jsonh_t));
    if(!str) return NULL;
    str->type = JSONH_STRING;
    str->next = str->prev = str->value.child = NULL;
    str->name = NULL;
    
    str->value.str = _jhY_parse_cstr(p);
    _jhY_next(p);

    return str;
}
static jsonh_t* _jhY_parse_true(struct _jhY* p){
    if(jcsncmp(&p->src[p->pos],_JLIT_TRUE,_JLEN_TRUE)) return NULL;
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
static jsonh_t* _jhY_parse_false(struct _jhY* p){
    if(jcsncmp(&p->src[p->pos],_JLIT_FALSE,_JLEN_FALSE)) return NULL;
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
static jsonh_t* _jhY_parse_null(struct _jhY* p){
    if(jcsncmp(&p->src[p->pos],_JLIT_NULL,_JLEN_NULL)) return NULL;
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
static jsonh_t* _jhY_parse_number(struct _jhY* p){
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
        if(!_jhY_isdigit(p)) _jhY_panic("JSONH: Expected digit at %d:%d",p->line,p->column);
        for(; _jhY_isdigit(p); _jhY_next(p)) digits = (digits*10) + (p->cur - _JCHR_DIG0);
    }

    // decimal part
    if(_jhY_check(p,_JCHR_DCPT)){
        _jhY_next(p);
        if(!_jhY_isdigit(p)) _jhY_panic("JSONH: Expected digit at %d:%d",p->line,p->column);
        while(_jhY_isdigit(p)){
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

        if(!_jhY_isdigit(p)) _jhY_panic("JSONH: Expected digit at %d:%d",p->line,p->column);
        
        for(; _jhY_isdigit(p); _jhY_next(p)) exp = (exp * 10) + (p->cur - _JCHR_DIG0);
    }

    exp10 = (exp_sign * exp) - frac_digits;
    double val = (double)digits;
    if(exp10 != 0) val *= pow(10.0,exp10);

    num->value.num = sign * val;
    return num;
}


static jsonh_t* _jhY_parse_value(struct _jhY* p){
    _jhY_skipws(p);

    switch(p->cur){
        case _JTOK_LCBRA: return _jhY_parse_object(p);
        case _JTOK_LSBRA: return _jhY_parse_array(p);
        case _JCHR_QUOT:  return _jhY_parse_string(p);
        case _JTOK_TRUE:  return _jhY_parse_true(p);
        case _JTOK_FALSE: return _jhY_parse_false(p);
        case _JTOK_NULL:  return _jhY_parse_null(p);
        default:          return _jhY_parse_number(p);
    }

}

static void _jhY_llappend(jsonh_t** head, jsonh_t** tail, jsonh_t* node){
    node->next =  NULL;
    node->prev = *tail;

    if(*tail) (*tail)->next = node;
    else *head = node;

    *tail = node;
}

static int _jh_indent(FILE* stream, int depth)
{
    int r = 0;
    for(int i = 0; i < depth; i++) r += fjprintf(stream,J("  "));
    return r;
}
static int _jh_print_object(FILE* stream, jsonh_t* node, int depth){
    if(node->value.child == NULL) return fjprintf(stream,J("{}"));
    int r = 0;
    r += fjprintf(stream,J("{\n"));
    for(jsonh_t* cur = node->value.child; cur; cur = cur->next){
        r += _jh_indent(stream,depth+1);
        r += fjprintf(stream,J("\"" JFMT "\": "),cur->name);
        r += _jh_print_value(stream,cur,depth+1);
        
        if(cur->next) r += fjprintf(stream,J(","));
        r += fjprintf(stream,J("\n"));
    }
    r += _jh_indent(stream,depth);
    r += fjprintf(stream,J("}"));
    return r;
}
static int _jh_print_array(FILE* stream, jsonh_t* node, int depth){
    if(node->value.child == NULL) return fjprintf(stream,J("[]"));
    int r = 0;
    r += fjprintf(stream,J("[\n"));
    for(jsonh_t* cur = node->value.child; cur; cur = cur->next){
        r += _jh_indent(stream,depth+1);
        r += _jh_print_value(stream,cur,depth+1);
        
        if(cur->next) r += fjprintf(stream,J(","));
        r += fjprintf(stream,J("\n"));
    }
    r += _jh_indent(stream,depth);
    r += fjprintf(stream,J("]"));
    return r;
}
static int _jh_print_value(FILE* stream, jsonh_t* node, int depth){
    switch(node->type){
        case JSONH_OBJECT:
            return _jh_print_object(stream,node,depth);
        case JSONH_ARRAY:
            return _jh_print_array(stream,node,depth);
        case JSONH_NUMBER:
            return fjprintf(stream,J("\"%g\""),node->value.num);
        case JSONH_STRING:
            return fjprintf(stream,J("\"" JFMT "\""),node->value.str);
        case JSONH_TRUE:
            return fjprintf(stream,J("true"));
        case JSONH_FALSE:
            return fjprintf(stream,J("false"));
        case JSONH_NULL:
            return fjprintf(stream,J("null"));
    }
}

// -----------------------------------------------------------------------------
// API IMPLEMENTATION ----------------------------------------------------------
// -----------------------------------------------------------------------------

jsonh_t* jsonh_parse(jchar_t* src, size_t len){
    if(!src) return NULL;
    jsonh_t* json;
    struct _jhY p = {.src = src,.size = len, .line = 1, .column = 1, .cur = src[0]};
    json = _jhY_parse_value(&p);
    return json;
}

jsonh_t* jsonh_read(FILE* file){
    if(!file) return NULL;

#ifdef JSONH_WIDE
    fwide(file,1);
#endif
    
    fseek(file,0L,SEEK_END);
    long sz = ftell(file);
    if(sz < 0) return NULL;
    rewind(file);
    size_t fsiz = (size_t)sz;

    jchar_t* buf = (jchar_t*)calloc(fsiz,sizeof(jchar_t));
    if(!buf) return NULL;
    
    jchar_t jc;
    size_t  len = 0;
    while((jc = fgetjc(file)) != JEOF){
        buf[len++] = jc;
    }
    buf[len] = 0;

    jsonh_t* json = jsonh_parse(buf,jcslen(buf));
    free(buf);
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

int jcscmpi(const jchar_t* s1, const jchar_t* s2){
    while(*s1 && (tojlower(*s1) == tojlower(*s2))) (s1++,s2++);
    return tojlower(*s1) - tojlower(*s2);
}

bool jsonh_obj_has(jsonh_t* obj, jchar_t* key){
    if(!obj || !key) return false;
    if(obj->type != JSONH_OBJECT) return false;
    for (jsonh_t* cur = obj->value.child; cur; cur = cur->next) {
        if(!jcscmp(cur->name,key)) return true;
    }
    return false;
}

jsonh_t* jsonh_obj_get(jsonh_t* obj, jchar_t* key){
    if(!obj || !key) return NULL;
    if(obj->type != JSONH_OBJECT) return NULL;
    for (jsonh_t* cur = obj->value.child; cur; cur = cur->next) {
        if(!jcscmp(cur->name,key)) return cur;
    }
    return NULL;
}

bool     jsonh_obj_ihas(jsonh_t* obj, jchar_t* key){
    if(!obj || !key) return false;
    if(obj->type != JSONH_OBJECT) return false;
    for (jsonh_t* cur = obj->value.child; cur; cur = cur->next) {
        if(!jcscmpi(cur->name,key)) return true;
    }
    return false;
}
jsonh_t* jsonh_obj_iget(jsonh_t* obj, jchar_t* key){
    if(!obj || !key) return NULL;
    if(obj->type != JSONH_OBJECT) return NULL;
    for (jsonh_t* cur = obj->value.child; cur; cur = cur->next) {
        if(!jcscmpi(cur->name,key)) return cur;
    }
    return NULL;
}

int jsonh_obj_add(jsonh_t* obj,jchar_t* key, jsonh_t* item){
    if(!obj || !item) return -1;
    if(obj->type != JSONH_OBJECT) return -1;
    
    jsonh_t* cur = obj->value.child;
    if(!cur){
        obj->value.child = item;
        item->name = jcsdup(key);
        return 0;
    }
    while(cur->next) cur = cur->next;

    cur->next  = item;
    item->prev = cur;
    item->name = jcsdup(key);

    return 0;

}

int jsonh_obj_del(jsonh_t* obj, jchar_t* key){
    if(!obj || !key) return -1;
    if(obj->type != JSONH_OBJECT) return -1;
    
    for(jsonh_t* cur = obj->value.child; cur; cur = cur->next){
        if(!jcscmp(cur->name,key)){
            jsonh_t* prev = cur->prev;
            jsonh_t* next = cur->next;

            prev->next = next;
            next->prev = prev;

            jsonh_delete(cur);
            return 0;
        }
    }

    return -1;
}

int jsonh_obj_idel(jsonh_t* obj, jchar_t* key){
    if(!obj || !key) return -1;
    if(obj->type != JSONH_OBJECT) return -1;
    
    for(jsonh_t* cur = obj->value.child; cur; cur = cur->next){
        if(!jcscmpi(cur->name,key)){
            jsonh_t* prev = cur->prev;
            jsonh_t* next = cur->next;

            prev->next = next;
            next->prev = prev;

            jsonh_delete(cur);
            return 0;
        }
    }

    return -1;
}

size_t   jsonh_arr_size(jsonh_t* arr){
    if(!arr) return 0;
    if(arr->type != JSONH_ARRAY) return 0;
    size_t s = 0;
    for (jsonh_t* cur = arr->value.child; cur; cur = cur->next) s++;
    return s;
}
jsonh_t* jsonh_arr_get(jsonh_t* arr, size_t index){
    if(!arr || index < 0) return NULL;
    if(arr->type != JSONH_ARRAY) return NULL;
    jsonh_t* cur = arr->value.child;
    for(size_t i = 0; i < index; i++){
        if(cur->next) cur = cur->next;
        else return NULL;
    }
    return cur;
}

int     jsonh_arr_push(jsonh_t* arr, jsonh_t* item){
    if(!arr | !item) return -1;
    if(arr->type != JSONH_ARRAY) return -1;
    jsonh_t* cur = arr->value.child;
    if(!cur) {
        arr->value.child = item;
        return 0;
    }
    while(cur->next) cur = cur->next;
    cur->next = item;
    item->prev = cur;
    return 0;
}
int     jsonh_arr_pop(jsonh_t* arr){
    if(!arr) return -1;
    if(arr->type != JSONH_ARRAY) return -1;
    jsonh_t* cur = arr->value.child;
    while(cur->next) cur = cur->next;
    cur->prev->next = NULL;
    jsonh_delete(cur);
}
int     jsonh_arr_put(jsonh_t* arr, jsonh_t* item){
    if(!arr | !item) return -1;
    if(arr->type != JSONH_ARRAY) return -1;

    item->next = arr->value.child;
    arr->value.child = item;
    return 0;
}
int     jsonh_arr_pull(jsonh_t* arr){
    if(!arr) return -1;
    if(arr->type != JSONH_ARRAY) return -1;

    jsonh_t* cur = arr->value.child;
    arr->value.child = cur->next;
    jsonh_delete(cur);
}
int     jsonh_arr_insert(jsonh_t* arr, size_t index, jsonh_t* item){
    if(!arr | !item || index < 0 ) return -1;
    if(arr->type != JSONH_ARRAY) return -1;

    jsonh_t* cur = arr->value.child;
    if(!cur) {
        arr->value.child = item;
        return 0;
    }
    for(size_t i = 0; i < index; i++){
        if(cur->next) cur = cur->next;
        else return -1;
    }
    
    jsonh_t* prev = cur->prev;
    if(prev) prev->next = item;
    cur->prev  = item;
    item->prev = prev;
    item->next = cur;
    return 0;
}
int     jsonh_arr_remove(jsonh_t* arr, size_t index){
    if(!arr || index < 0 ) return -1;
    if(arr->type != JSONH_ARRAY) return -1;

    jsonh_t* cur = arr->value.child;
    for(size_t i = 0; i < index; i++){
        if(cur->next) cur = cur->next;
        else return -1;
    }

    jsonh_t* prev = cur->prev;
    jsonh_t* next = cur->next;
    
    if(prev) prev->next = next;
    if(next) next->prev = prev;

    jsonh_delete(cur);
    return 0;
}

double   jsonh_num_get(jsonh_t* num){
    if(!num) return 0;
    if(num->type != JSONH_NUMBER) return 0;
    return num->value.num;
}
int     jsonh_num_set(jsonh_t* num, double val){
    if(!num) return -1;
    if(num->type != JSONH_NUMBER) return -1;
    num->value.num = val;
    return 0;
}

jchar_t* jsonh_str_get(jsonh_t* str){
    if(!str) return NULL;
    if(str->type != JSONH_STRING) return NULL;
    return str->value.str;
}
jchar_t* jsonh_str_dup(jsonh_t* str){
    if(!str) return NULL;
    if(str->type != JSONH_STRING) return NULL;
    size_t len = jcslen(str->value.str);
    jchar_t* dup = (jchar_t*)calloc(len,sizeof(jchar_t));
    if(!dup) return NULL;
    jcscpy(dup,str->value.str);
    return dup;
}

int jsonh_str_set(jsonh_t* str, jchar_t* val){
    if(!str || !val) return -1;
    if(str->type != JSONH_STRING) return -1;

    free(str->value.str);
    str->value.str = jcsdup(val);

    return 0;
}

bool     jsonh_bol_get(jsonh_t* bol){
    if(!bol) return false;
    if(bol->type != JSONH_TRUE && bol->type != JSONH_FALSE) return false;
    return bol->value.bol;
}
int     jsonh_bol_set(jsonh_t* bol, bool val){
    if(!bol) return -1;
    if(bol->type != JSONH_TRUE && bol->type != JSONH_FALSE) return -1;
    bol->value.bol = val;
    return 0;
}

bool jsonh_is(jsonh_t* json, jsonh_type_n type){
    return json->type == type;
}

bool jsonh_is_object(jsonh_t* json){
    return json->type == JSONH_OBJECT;
}
bool jsonh_is_array(jsonh_t* json){
    return json->type == JSONH_ARRAY;
}
bool jsonh_is_number(jsonh_t* json){
    return json->type == JSONH_NUMBER;
}
bool jsonh_is_string(jsonh_t* json){
    return json->type == JSONH_STRING;
}
bool jsonh_is_true(jsonh_t* json){
    return json->type == JSONH_TRUE;
}
bool jsonh_is_false(jsonh_t* json){
    return json->type == JSONH_FALSE;
}
bool jsonh_is_bool(jsonh_t* json){
    return json->type == JSONH_TRUE || json->type == JSONH_FALSE;
}
bool jsonh_is_null(jsonh_t* json){
    return json->type == JSONH_NULL;
}

jsonh_t* jsonh_new(jsonh_type_n type){
    jsonh_t* n = (jsonh_t*)calloc(1,sizeof(jsonh_t));
    if(!n) return NULL;
    n->type = type;
    n->next = n->prev;
    return n;
}

jsonh_t* jsonh_new_object(void){
    jsonh_t* o = (jsonh_t*)calloc(1,sizeof(jsonh_t));
    if(!o) return NULL;
    o->type = JSONH_OBJECT;
    o->next = o->prev = NULL;
    o->value.child = NULL;
    return o;
}
jsonh_t* jsonh_new_array(void){
    jsonh_t* a = (jsonh_t*)calloc(1,sizeof(jsonh_t));
    if(!a) return NULL;
    a->type = JSONH_ARRAY;
    a->next = a->prev = NULL;
    a->value.child = NULL;
    return a;
}
jsonh_t* jsonh_new_number(double num){
    jsonh_t* n = (jsonh_t*)calloc(1,sizeof(jsonh_t));
    if(!n) return NULL;
    n->type = JSONH_NUMBER;
    n->next = n->prev = NULL;
    n->value.num = num;
    return n;
}
jsonh_t* jsonh_new_string(jchar_t* str){
    jsonh_t* s = (jsonh_t*)calloc(1,sizeof(jsonh_t));
    if(!s) return NULL;
    s->type = JSONH_STRING;
    s->next = s->prev = NULL;
    s->value.str = (jchar_t*)calloc(jcslen(str),sizeof(jchar_t));
    if(!s->value.str){
        free(s);
        return NULL;
    }
    jcscpy(s->value.str,str);
    return s;
}
jsonh_t* jsonh_new_true(void){
    jsonh_t* t = (jsonh_t*)calloc(1,sizeof(jsonh_t));
    if(!t) return NULL;
    t->type = JSONH_TRUE;
    t->next = t->prev = NULL;
    t->value.bol = true;
    return t;
}
jsonh_t* jsonh_new_false(void){
    jsonh_t* f = (jsonh_t*)calloc(1,sizeof(jsonh_t));
    if(!f) return NULL;
    f->type = JSONH_FALSE;
    f->next = f->prev = NULL;
    f->value.bol = false;
    return f;
}
jsonh_t* jsonh_new_bool(bool bol){
    jsonh_t* b = (jsonh_t*)calloc(1,sizeof(jsonh_t));
    if(!b) return NULL;
    b->type = bol ? JSONH_TRUE : JSONH_FALSE;
    b->next = b->prev = NULL;
    b->value.bol = bol;
    return b;
}
jsonh_t* jsonh_new_null(void){
    jsonh_t* n = (jsonh_t*)calloc(1,sizeof(jsonh_t));
    if(!n) return NULL;
    n->type = JSONH_NULL;
    n->next = n->prev = n->value.child = NULL;
    return n;
}

int jsonh_print(jsonh_t* root){
    if(!root) return 0;
    return _jh_print_value(stdout,root,0);
}

int jsonh_write(FILE* stream, jsonh_t* root){
    if(!root) return 0;
    return _jh_print_value(stream,root,0);
}

#endif /* JSONH_IMPL */

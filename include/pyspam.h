#ifndef PYSPAM_H
#define PYSPAM_H


// Version
#define ODB4PY_VERSION "1.3.3"

#define PY_SSIZE_T_CLEAN
#define ODB_STRLEN 8  
#include <numpy/arrayobject.h>
#include <numpy/ndarraytypes.h>

#include  <stdio.h> 	
#include  <ctype.h>
#include  <Python.h>
#include  "odbdump.h"
#include  "odc/api/odc.h"

#define SMAX 50 


//  The unclude file could be renamed diffently , BUT  it stays  "pyspam.h"
// Contains diffrent functions not specific to a given module or C method 



// This avoid printing the message (ODC initialized twice !) when conversion to odb2 is used in a loop
// Init only one time
static int odc_initialized = 0;
static void ensure_odc_init() {
    if (!odc_initialized) {
        odc_initialise_api();
        odc_integer_behaviour(ODC_INTEGERS_AS_LONGS);
        odc_initialized = 1;
    }
}


// Remove @ from the returned column names and replace by underscore 
void sanitize_name(char *name){
    char *src = name;
    char *dst = name;
    char prev = 0;
    while (*src) {
        char c = *src;
        if (!isalnum((unsigned char)c)) {
            c = '_';
        }
        if (!(c == '_' && prev == '_')) {
            *dst++ = c;
            prev = c;
        }
        src++;
    }
    *dst = '\0';
    if (dst > name && *(dst - 1) == '_') {
        *(dst - 1) = '\0';
    }
}

/*static void make_printable(char *s, int slen)
{ for (int j = 0; j < slen; j++) { unsigned char c = s[j];  if (!isprint(c)) s[j] = ' '; } }*/


//    Destroy np Capsule   
static void free_when_done(PyObject *capsule) {
void *ptr = PyCapsule_GetPointer(capsule, NULL); if (ptr) { free(ptr); } }


// Convert boolean PyObject  (from python True /False to C true/false)
static inline Bool PyObj_ToBool(PyObject *obj, Bool default_value)
{
    if (obj == NULL || obj == Py_None)
        return default_value;

    int truth = PyObject_IsTrue(obj);
    if (truth == -1) {          
        PyErr_Clear();         
        return default_value;
    }

    return (Bool)(truth != 0);
}


// Formatting floats
double  format(double value , char *fmt ) {
static  char str [SMAX ] ;
double    x  ;
sprintf(str, fmt , value );
x = atof(str );
return   x ;
}

// Set the number of decimal digits 
static inline double format_float  (double val, int precision) {
    if (!isfinite(val)) return val;

    // Ensure decimal are between  0 and 15  
    if (precision <= 0) {
        precision = 15;
    }
    if (precision > 15) {
        precision = 15;
    }
    char fmt[16];
    snprintf(fmt, sizeof(fmt), "%%.%df", precision);
    char buffer[64];
    snprintf(buffer, sizeof(buffer), fmt, val);
    return atof(buffer);
}

// String concat
char* concat(const char *s1, const char *s2){
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

// str to num 
void strtonum(char [], int);
void strtonum(char str[], int num)
{
    int i, rem, len = 0, n; 
    n = num;
    while (n != 0) {
        len++;
        n /= 10;
    }
    for (i = 0; i < len; i++) {
        rem = num % 10;
        num = num / 10;
        str[len - (i + 1)] = rem + '0';
    }
    str[len] = '\0';
}

// string len 
int get_strlen (  const char *  string  ) {  int str_len  ;
if ( string  ) { str_len=  strlen( string ); } else  { str_len=0   ;  };
return str_len ;
}

#endif 


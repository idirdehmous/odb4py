#ifndef PYSPAM_H
#define PYSPAM_H


// Version
#define ODB4PY_VERSION "1.3.1"




#define PY_SSIZE_T_CLEAN
#define ODB_STRLEN 8  // 8 chars + '\0' 
#include <numpy/arrayobject.h>
#include <numpy/ndarraytypes.h>

#include  <stdio.h> 	
#include  <Python.h>
#include  "odbdump.h"
#define SMAX 50 

//  The unclude file could be renamed diffently , BUT  it stays  "pyspam.h"
//  since with a module called  pyspam.c  that i learned writing 
//  C extension for py  :-)  ... always remember !
//  Link could be found here :
//  https://docs.python.org/3/extending/extending.html  
//  Valid the : 11/03/2026


 /* For description :
 * In this package these lines have been taken from  :   include/privpub.h  
 * 
#define DATATYPE_UNDEF                0x0           (0 dec) undef 
#define DATATYPE_BIT                  0x100         (256 dec) bit : 1 bits, 0 bytes 
#define DATATYPE_INT1                 0x10f         (271 dec) char : 8 bits, 1 bytes 
#define DATATYPE_INT2                 0x113         (275 dec) short : 16 bits, 2 bytes 
#define DATATYPE_INT4                 0x117         (279 dec) int : 32 bits, 4 bytes 
#define DATATYPE_INT8                 0x11b         (283 dec) long long : 64 bits, 8 bytes 
#define DATATYPE_UINT1                0x10e         (270 dec) uchar : 8 bits, 1 bytes 
#define DATATYPE_UINT2                0x112         (274 dec) ushort : 16 bits, 2 bytes 
#define DATATYPE_UINT4                0x116         (278 dec) uint : 32 bits, 4 bytes 
#define DATATYPE_UINT8                0x11a         (282 dec) ulonglong : 64 bits, 8 bytes 
#define DATATYPE_REAL4                0x216         (534 dec) float : 32 bits, 4 bytes 
#define DATATYPE_REAL8                0x21a         (538 dec) double : 64 bits, 8 bytes 
#define DATATYPE_REAL16               0x21e         (542 dec) long double : 128 bits, 16 bytes 
#define DATATYPE_CMPLX4               0x31a         (794 dec) complex4 : 64 bits, 8 bytes 
#define DATATYPE_CMPLX8               0x31e         (798 dec) complex8 : 128 bits, 16 bytes 
#define DATATYPE_CMPLX16              0x322         (802 dec) complex16 : 256 bits, 32 bytes 
#define DATATYPE_BITFIELD             0x416         (1046 dec) Bitfield : 32 bits, 4 bytes 
#define DATATYPE_STRING               0x818         (2072 dec) string : 64 bits, 8 bytes 
#define DATATYPE_YYYYMMDD             0x1016        (4118 dec) yyyymmdd : 32 bits, 4 bytes 
#define DATATYPE_HHMMSS               0x2016        (8214 dec) hhmmss : 32 bits, 4 bytes 
#define DATATYPE_LINKOFFSET           0x4017        (16407 dec) linkoffset_t : 32 bits, 4 bytes 
#define DATATYPE_LINKLEN              0x8017        (32791 dec) linklen_t : 32 bits, 4 bytes 
#define DATATYPE_BUFR                 0x10016       (65558 dec) bufr : 32 bits, 4 bytes 
#define DATATYPE_GRIB                 0x20016       (131094 dec) grib : 32 bits, 4 bytes 
#define DATATYPE_BLOB                 0x4004c       (262220 dec) blob64kB : 65536 bytes 
#define DATATYPE_LONGBLOB             0x80088       (524424 dec) blob2GB : 2147483648 bytes 
#define DATATYPE_CHAR                 0x10002c      (1048620 dec) char(1:255) : 256 bytes 
#define DATATYPE_VARCHAR              0x20002c      (2097196 dec) varchar(1:255) : 256 bytes 
*/




//  Map ODB1 and sqlite datatypes 
const char* map_type(int odb_type) {
    switch (odb_type) {
        case DATATYPE_UNDEF:
	     return NULL ; 
	case DATATYPE_INT1:
	case DATATYPE_INT2:
	case DATATYPE_INT4:
	case DATATYPE_INT8:
	case DATATYPE_UINT1:
	case DATATYPE_UINT2:
	case DATATYPE_UINT4:
	case DATATYPE_UINT8:
            return "INTEGER";
        case DATATYPE_REAL4:
        case DATATYPE_REAL8:
        case DATATYPE_REAL16:
            return "REAL";
        case DATATYPE_STRING:
	case DATATYPE_CHAR:
	case DATATYPE_VARCHAR:
            return "TEXT";
        case DATATYPE_BITFIELD:
	case DATATYPE_YYYYMMDD:
        case DATATYPE_HHMMSS:
	case DATATYPE_LINKOFFSET:
	case DATATYPE_LINKLEN:
	case DATATYPE_BUFR:
	case DATATYPE_GRIB:
            return "INTEGER";
        default:
            return "REAL";  // fallback 
    }
}



/*    Destroy np Capsule   */
static void free_when_done(PyObject *capsule) {
    void *ptr = PyCapsule_GetPointer(capsule, NULL);
    if (ptr) {
       free(ptr);
    }
}


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


static inline double format_float  (double val, int precision) {
    if (!isfinite(val)) return val;

    // Ensure decimal are between  0 and 15  
    if (precision <= 0) {
        printf("%s\n", "--WARNING : The number of decimal digits must >= 0 . -->  Fallback to default : fmt_float=15" ) ;
        precision = 15;
    }
    if (precision > 15) {
        printf("%s\n", "--WARNING : The number of decimal digits greater than 15. -->  Fallback to default : fmt_float=15" ) ;
        precision = 15;
    }
    char fmt[16];
    snprintf(fmt, sizeof(fmt), "%%.%df", precision);
    char buffer[64];
    snprintf(buffer, sizeof(buffer), fmt, val);
    return atof(buffer);
}


char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}


void strtonum(char [], int);
void strtonum(char str[], int num)
{
    int i, rem, len = 0, n; 
    n = num;
    while (n != 0)
    {
        len++;
        n /= 10;
    }
    for (i = 0; i < len; i++)
    {
        rem = num % 10;
        num = num / 10;
        str[len - (i + 1)] = rem + '0';
    }
    str[len] = '\0';
}

int get_strlen (  const char *  string  ) {  int str_len  ;
if ( string  ) { str_len=  strlen( string ); } else  { str_len=0   ;  };
return str_len ;
}
#endif 


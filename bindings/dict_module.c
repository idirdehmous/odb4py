/*
 * odb4py
 * Copyright (C) 2026 Royal Meteorological Institute of Belgium (RMI)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#define PY_SSIZE_T_CLEAN

//NUMPY API 
#include <numpy/arrayobject.h>
#include <numpy/ndarraytypes.h>
// C 
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <Python.h>

// Custom
#include "pyspam.h"
#include "progress.h" 
#include "rows.h" 

#define ODB_STRLEN 8  // 8 chars + '\0'

// Declare excption  proto 
//static PyObject *PyOdbEmptyResultError = NULL;


// Function  : odb_dict_method
static PyObject *odb_dict_method(PyObject *Py_UNUSED(self),
                                 PyObject *args,
                                 PyObject *kwargs) {

    // Arguments keywords
    const char *database  = NULL;
    const char *sql_query = NULL;
    const char *queryfile = NULL;
    int   fcols     = 0;
    int   fmt_float = 15  ;

    // Objects 
    PyObject *poolmask_obj = Py_None;

    // Options (Boolean args )
    PyObject *pbar  = Py_None;
    PyObject *pverb = Py_None;
    Py_INCREF(pbar  );
    Py_INCREF(pverb );

    Bool lpbar   = false;
    Bool verbose = false;

    // Keyword list 
    static char *kwlist[] = {  "database" , 
	                       "sql_query", 
			       "nfunc"    ,
			       "fmt_float",
                               "queryfile", 
			       "poolmask" , 
			       "pbar"     , 
			       "verbose"  ,  NULL
                             };

    // Parse keyword args 
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "szi|izOOO", kwlist,   // 3 requiered , 5 optional 
                                     &database,
                                     &sql_query,
                                     &fcols    ,
                                     &fmt_float,
                                     &queryfile,
                                     &poolmask_obj,
                                     &pbar,
                                     &pverb)) {
	 return NULL  ;
    }

    // Required  
    if (!sql_query && !queryfile) {
       PyErr_SetString(PyExc_TypeError,
        "--odb4py : Either 'sql_query' or 'queryfile' must be provided");
    return NULL;
    }


    // Convert to string  
    const char *poolmask_str = NULL;
    if (poolmask_obj != Py_None) {
        if (!PyUnicode_Check(poolmask_obj)) {
         PyErr_SetString(PyExc_TypeError, "--odb4py : poolmask must be a string.  ex: '1 2 3 N' or '1:N' N=Number of pools'  \n") ;  
         return NULL;
         }
     poolmask_str  = PyUnicode_AsUTF8(poolmask_obj);
    }
    
    // Conversion to boolean C variable
    lpbar   = PyObj_ToBool ( pbar , lpbar      ) ; 
    verbose = PyObj_ToBool ( pverb , verbose   ) ; 

    if (verbose && poolmask_str ) {
        printf("Fetch data from pool(s) #: %s\n", poolmask_str );
    }

    
    char  *varvalue = NULL;
    int    maxlines = -1;
    void  *h        = NULL;
    int    maxcols  = 0 ;
    int    rc       = 0 ;
    size_t ip       = 0 ;
    size_t prog_max = 0 ;
    int    row_idx  = 0 ;
    int   nci       = 0;
    Bool print_mdi  = true ; 


    // Get maximum number of rows 
    int total_rows = getMaxrows(database, sql_query , queryfile , poolmask_str );

    // Check number of rows --> check the query answer 
    if ( total_rows ==0 ) {
      PyErr_SetString(PyExc_RuntimeError, "--odb4py : The SQL request returned zero rows.");  
      return NULL ;  }

    if (total_rows <= 0) total_rows = 4096;   // Fallback  

    prog_max = (size_t)total_rows;

    if (maxlines == 0) return PyLong_FromLong(rc);
    if (verbose) {
        if (sql_query)
            printf("--odb4py : Executing query from string: %s\n", sql_query);
    }   else if (queryfile) {
            printf("--odb4py : Executing query from file   : %s\n", queryfile);
	    printf("%s\n", "--odb4py : WARNING --> Executing the queries from SQL file is DEPRECATED. Not completly stable");
    }
    //  OPEN ODB
    h = odbdump_open(database, sql_query, queryfile, poolmask_str , varvalue, &maxcols);
    
    if (!h || maxcols <= 0) {
        PyErr_SetString(PyExc_RuntimeError, "--odb4py : Failed to open ODB or invalid number of columns");
        return NULL  ; }

    // Number of columns taking into account the number of functions in the query  (col pure - n columns function)
    int ncols = maxcols - fcols;
    if (verbose)   printf("--odb4py : Number of requested columns : %d\n", ncols);

    // Allocation 
    double *buffer    = (double *)malloc(sizeof(double) * (size_t)total_rows * (size_t)ncols);
    char  **strbufs   = (char**)calloc((size_t)ncols, sizeof(char*));
    // If allocation failed  close !
    if (!buffer) { odbdump_close(h);  PyErr_SetString(PyExc_RuntimeError, "--odb4py : Failed to allocate memory buffer for numeric values ");  return NULL ;  }
    if (!strbufs){ odbdump_close(h);  PyErr_SetString(PyExc_RuntimeError, "--odb4py : Failed to allocate memory buffer for string  values ");  return NULL ;  }
   // Internal ODB vars 
    int new_dataset = 0;
    colinfo_t *ci   = NULL;
    double *d       = NULL;
    ALLOCX(d, maxcols);
    Bool packed = false;
    int (*nextrow)(void *, void *, int, int *) =
        packed ? odbdump_nextrow_packed : odbdump_nextrow;
    int dlen = packed ? maxcols * (int)sizeof(*d) : maxcols;
    ll_t nrtot = 0;
    int nd     = 0;
    double float_val= 0 ; 
   // List to hold the column names  and values 
   PyObject **col_lists        = (PyObject **)malloc(ncols * sizeof(PyObject *));
   if (!col_lists) { 
	PyErr_SetString(PyExc_RuntimeError, "--odb4py : PyObject memory allocation error");  
	return NULL  ;
   }

   // Dict object 
   PyObject  *dict = PyDict_New();
   if (!dict) {
        PyErr_SetString(PyExc_RuntimeError, "--odb4py : PyObject dictionary allocation error");
       return NULL  ; 
    }

   // Init buffer strings 
   for (int i = 0; i < ncols; ++i) {
    strbufs[i] = (char*)calloc((size_t)total_rows, (size_t)ODB_STRLEN +1);
    }

    // Init columns lists 
    for (int i = 0; i < ncols; ++i) {
        col_lists[i] = PyList_New(total_rows);
        if (!col_lists[i]) {
            PyErr_SetString(PyExc_RuntimeError, "--odb4py : PyObject memory allocation error");
             return NULL  ; 
        } }

    // Loop over the rows    
    while ((nd = nextrow(h, d, dlen, &new_dataset)) > 0) {
        if (lpbar) {  ++ip;            print_progress(ip, prog_max); }   // useful for huge ODBs 
        
	// Reallocate buffer if needed 
        if (row_idx >= total_rows) {
            total_rows *= 2;
            double *tmp = (double *)realloc(buffer, sizeof(double) * (size_t)total_rows * (size_t)ncols);
            if (!tmp) {
                free(buffer);
                odbdump_close(h);
                FREEX(d);
                return PyErr_NoMemory();
                     }
            buffer = tmp;
            }
        // New  dataset 
        if (new_dataset) {
            ci = odbdump_destroy_colinfo(ci, nci);
            ci = odbdump_create_colinfo(h, &nci);
            new_dataset = 0;
        }

	// Cols 
        for (int i = 0; i < ncols; ++i) {
            colinfo_t *pci    = &ci[i];
            PyObject  *value  =  NULL ;
           if (print_mdi && pci->dtnum != DATATYPE_STRING && ABS(d[i]) == mdi) {
         	  //value = Py_None  ; 
                  //Py_INCREF(Py_None)  ;   Gives None in pandas ,not suitable 
                  value = PyFloat_FromDouble(NAN);   //  use NAN     
          } else {
              switch (pci->dtnum) {
            	case DATATYPE_STRING: {
                        char *dst = &strbufs[i][(size_t)row_idx * ODB_STRLEN];
                        union { char s[sizeof(double)]; double d; } u;
                        u.d = d[i];
                        memcpy(dst, u.s , ODB_STRLEN);
                        dst[ODB_STRLEN ] = '\0';  
			if ( dst ) 
			{ value = PyUnicode_FromStringAndSize(dst, strnlen(dst, ODB_STRLEN));			      
			}else {
				//value = PyUnicode_FromFormat ("NULL    ")  ; 
                                value = Py_None   ;
			        Py_INCREF(Py_None);	
				}
                break;
	             }
            case DATATYPE_INT1:
	    case DATATYPE_INT2:
	    case DATATYPE_INT4:
		buffer[(size_t)row_idx * (size_t)ncols + (size_t)i] = (double)(int)d[i];
                value = PyLong_FromLong(buffer[(size_t)row_idx * (size_t)ncols + (size_t)i]);
	        break ; 
            case DATATYPE_YYYYMMDD:
	         buffer[(size_t)row_idx * (size_t)ncols + (size_t)i] = (double)(int)d[i];	    
                 value = PyLong_FromLong(buffer[(size_t)row_idx * (size_t)ncols + (size_t)i]);
                break ; 
            case DATATYPE_HHMMSS:
	    buffer[(size_t)row_idx * (size_t)ncols + (size_t)i] = (double)(int)d[i];
               value = PyLong_FromLong(buffer[(size_t)row_idx * (size_t)ncols + (size_t)i]);
                break;
            default:
               float_val   =  (double)d[i];
	       double fval =  format_float ( float_val , fmt_float);
	       buffer[(size_t)row_idx * (size_t)ncols + (size_t)i] = (double) fval  ;
               value = PyFloat_FromDouble(buffer[(size_t)row_idx * (size_t)ncols + (size_t)i]);
               break;
        }   // switch  

	      }  // else of mdi  
       if (maxlines > 0 && nrtot >= maxlines)
            break;

// Fill the lists 
if (row_idx < total_rows  ) {
    PyList_SET_ITEM(col_lists[i], row_idx, value);
       } else {
    PyList_Append(col_lists[i], value);
    Py_DECREF(value);   // "DECREF  the reference object"
         }
      }   // cols  
        ++row_idx;
        ++nrtot;
} // while 

// Get the colnames 
for (int i=0 ; i<ncols ; i++ )   {
    const char *colname = ci[i].nickname ? ci[i].nickname : ci[i].name;
    PyDict_SetItemString(dict , colname, col_lists[i]);
}
//  FREE  struct and PyObject pointers 
if (strbufs) {
    for (int i = 0; i < ncols; ++i) {
        if (strbufs[i]) free(strbufs[i]);
    }
    free(strbufs);
}
if (col_lists) {
    for (int i = 0; i < ncols; ++i) {
        Py_XDECREF(col_lists[i]);
          }
    }
if (col_lists) {    free(col_lists); }
free (buffer ) ;  
ci = odbdump_destroy_colinfo(ci, nci);
odbdump_close(h);
if (d) {  FREEX(d);   }
return dict  ; 
}

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
#include <time.h>
#include <fcntl.h>
#include <Python.h>

// odc 
#include "odc/api/odc.h"

// Custom
#include "pyspam.h"
#include "progress.h"
#include "rows.h"
#include "ncdf.h"

// Constants 
#define ODB_STRLEN 8      // 8 chars + '\0'
#define ODC_STRLEN 16     // 8 is default in ODB but in ODB2 it's extended to 16 byte (such AMDAR wigos identifiers )
#define DEFAULT_CHUNK 10
#define ODB_RMDI     -2147483647
#define ODB_NMDI      2147483647
#define ODC_BITFIELD  0  

// macros 
#define CHECK_RESULT(x) \
    do {                \
        int rc = (x);   \
        if (rc != ODC_SUCCESS) {   \
            fprintf(stderr, "Error calling odc function \"%s\": %s\n", #x, odc_error_string(rc)); \
            exit(1);    \
        }               \
    } while (false);





// Function  : odb_to_odb2 
static PyObject *odb_to_odb2_method(PyObject *Py_UNUSED(self),
                                 PyObject *args,
                                 PyObject *kwargs) {

    // Arguments keywords
    const char *database  = NULL;
    char *sql_query = NULL  ;
    char *queryfile = NULL  ;
    char *outfile   = NULL  ;
    int   fcols     = 0     ;
    int   fmt_float = 15    ;
    int   nrows_chunk=10  ;

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
    static char *kwlist[] = {  "database"  ,
	                       "sql_query" ,
			       "nfunc"     ,
			       "outfile"   ,
			       "rows_per_frame", 
			       "fmt_float",
                               "queryfile",
			       "poolmask" ,
			       "pbar"     ,
			       "verbose"  ,
			        NULL
                             };
    // Parse keyword args
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "szisi|izOOO", kwlist,   // 5 requiered , 5 optional
                                     &database,
                                     &sql_query,
                                     &fcols    ,
				     &outfile    ,
				     &nrows_chunk , 
                                     &fmt_float,
                                     &queryfile,
                                     &poolmask_obj,
                                     &pbar,
                                     &pverb)) {
          return PyLong_FromLong(-1);
    }
    // Required
    if (!sql_query && !queryfile) {
       PyErr_SetString(PyExc_TypeError,
        "--odb4py : Either 'sql_query' or 'queryfile' must be provided");
     return PyLong_FromLong(-1);
    }

    // Convert to string
     char *poolmask_str = NULL;
    if (poolmask_obj != Py_None) {
        if (!PyUnicode_Check(poolmask_obj)) {
	  printf("%s\n","--odb4py : poolmask must be a string.  ex: '1 2 3 N' or '1:N' N=Number of pools'  \n") ;
          return PyLong_FromLong(-1);
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
    int   nci       = 0;
    colinfo_t *ci = NULL;

    // Get maximum number of rows
    if (lpbar)  {
    int total_rows = getMaxrows(database, sql_query , queryfile , poolmask_str );
   // if (total_rows <= 0) total_rows = 4096;   // Fallback
    prog_max = (size_t)total_rows;


    // Check number of rows --> check the query answer
    if ( total_rows ==0 ) {
        printf ("%s\n",  "--odb4py : The SQL request returned zero rows.");
        return PyLong_FromLong(-1) ;
       }
     }
     // if (total_rows <= 0) total_rows = 4096;   // Fallback
     //  prog_max = (size_t)total_rows;




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
	printf("%s\n", "--odb4py : Failed to open ODB or invalid number of columns") ; 
        return PyLong_FromLong(-1)  ;
    }
    // Number of columns taking into account the number of functions in the query  (col pure - n columns function)
    int ncols = maxcols - fcols;
    if (verbose)   printf("--odb4py : Number of requested columns : %d\n", ncols);

// ALLOCATION
// ODB cols container 
double *d = malloc(maxcols * sizeof(double));

// buffers
double   **buffers     = malloc(ncols * sizeof(double*));
double   **int_buffers = malloc(ncols * sizeof(double*));
char     **str_buffers = malloc(ncols * sizeof(char*));

// For ODC col  types 
int *odc_type  = malloc(ncols * sizeof(int));
int *is_string = malloc(ncols * sizeof(int));


// Reset the number of rows per chunk (frame)
int rows_by_chunk=DEFAULT_CHUNK  ; 
if ( nrows_chunk   &&  nrows_chunk !=  DEFAULT_CHUNK ) {
     rows_by_chunk = nrows_chunk ;    // override  if argument is given 
}

for (int i = 0; i < ncols; i++) {
    buffers[i]     = malloc(CHUNK * sizeof(double));
    int_buffers[i] = malloc(CHUNK * sizeof(double));
    str_buffers[i] = malloc(CHUNK * ODC_STRLEN);
    is_string  [i] = 0;
}

// INIT ODC & declare the encoder 
//odc_initialise_api();   NOT INIT here ! it should be initiliazed only once !
ensure_odc_init()     ;
odc_encoder_t* encoder;
odc_new_encoder(&encoder);

// Missing values 
if (odc_set_missing_integer(NMDI) != ODC_SUCCESS ) {  printf( "%s\n", "Failed to set integer missing value"  ) ; PyLong_FromLong(-1) ;  }
if (odc_set_missing_double (RMDI) != ODC_SUCCESS ) {  printf( "%s\n", "Failed to set real missing value"     ) ; PyLong_FromLong(-1) ;  }

// output filename 
int fd = open(outfile , O_CREAT | O_TRUNC | O_WRONLY, 0666);

// Init counters 
int k =0  ; 
int new_dataset = 0;
int first_dataset = 1;

// ODB rows   loop  
int (*nextrow)(void *, void *, int, int *) = odbdump_nextrow;
while (nextrow(h, d, maxcols, &new_dataset) > 0) {
	if (lpbar) {  ++ip;            print_progress(ip, prog_max); }
    if (new_dataset) {
        ci = odbdump_destroy_colinfo(ci, nci);
        ci = odbdump_create_colinfo(h  ,&nci);

        // DEFINE COLUMNS ACCORING TO THE DATATYPES (only once) 
        if (first_dataset) {
            for (int i = 0; i < ncols; i++) {
                const char *name = ci[i].nickname ? ci[i].nickname : ci[i].name;
                int odb_dtype = ci[i].dtnum;
                switch (odb_dtype) {
                    case DATATYPE_STRING:
                        CHECK_RESULT(odc_encoder_add_column(encoder, name, ODC_STRING));
                        odc_type[i] = ODC_STRING;
                        break;
                    case DATATYPE_INT1:
                    case DATATYPE_INT2:
                    case DATATYPE_INT4:
                    case DATATYPE_YYYYMMDD:
                    case DATATYPE_HHMMSS:
                          CHECK_RESULT(odc_encoder_add_column(encoder, name, ODC_INTEGER));
                          odc_type[i] = ODC_INTEGER;
                        break;
                    default:
                          CHECK_RESULT(odc_encoder_add_column(encoder, name,  ODC_REAL));
                          odc_type[i] =  ODC_REAL ;
                        break;
                  }  // switch  
            } //  ncols 
            first_dataset = 0;
        }// first dataset
        new_dataset = 0;
    }

    // fill buffer with data 
    for (int i = 0; i < ncols; i++) {
	// cast to double even it's integer .  INTEGER handling is done inside ODC core
	// Also , the integers and double strides MUST be separated .  
	
        if (odc_type[i] == ODC_INTEGER  &&  ABS(d[i]) == ODB_NMDI ) {	    
	    int_buffers[i][k] =  llround(ODB_NMDI); 
	    }   else {
            int_buffers[i][k] = (double) d[i] ;   
            } 
	if (odc_type[i]    ==  ODC_REAL &&  ABS(d[i] )==  ODB_RMDI ) {
             buffers[i][k] =     ODB_RMDI  ;
	    }  else {
             buffers[i][k] = d[i];
        }
	if (odc_type[i] == ODC_STRING) {
            char *dst = &str_buffers[i][k * ODC_STRLEN];
            memcpy(dst, &d[i], 8);   // default ODB
           // use memset rather than a loop 
            memset(dst + 8, ' ', ODC_STRLEN - 8);    // add if > 8   
        }
    }
    k++;

    // Write the frame into odc array   	
    //if (k == CHUNK) {
    if (k== rows_by_chunk) {
        CHECK_RESULT(odc_encoder_set_row_count     (encoder, rows_by_chunk));
        CHECK_RESULT(odc_encoder_set_rows_per_frame(encoder, rows_by_chunk));
        for (int i = 0; i < ncols; i++) {
           if (odc_type[i] == ODC_INTEGER ) {
                CHECK_RESULT( odc_encoder_column_set_data_array( encoder, i, sizeof(double), sizeof(double), int_buffers[i]) );
            } else if (odc_type[i] == ODC_REAL) {
                CHECK_RESULT( odc_encoder_column_set_data_array( encoder, i, sizeof(double), sizeof(double), buffers[i]) );
            } else if (odc_type[i] == ODC_STRING) {
                CHECK_RESULT( odc_encoder_column_set_data_array( encoder, i, ODC_STRLEN, ODC_STRLEN, str_buffers[i]) );
            }
        }
         CHECK_RESULT(odc_encode_to_file_descriptor(encoder, fd, NULL) );
        k = 0;   // reset counter  
    } // rows by chunk 

}  // while 

// if CHUNK % k  != 0 ---> the remaining rows are not written  ---->>  k is still > 0
if (k > 0) {
        CHECK_RESULT(odc_encoder_set_row_count     (encoder, k ));
        CHECK_RESULT(odc_encoder_set_rows_per_frame(encoder, k ));
        for (int i = 0; i < ncols; i++) {
           if (odc_type[i] == ODC_INTEGER ) {
                CHECK_RESULT( odc_encoder_column_set_data_array( encoder, i, sizeof(double), sizeof(double), int_buffers[i]) );
            } else if (odc_type[i] == ODC_REAL) {
                CHECK_RESULT( odc_encoder_column_set_data_array( encoder, i, sizeof(double), sizeof(double), buffers[i]) );
            } else if (odc_type[i] == ODC_STRING) {
                CHECK_RESULT( odc_encoder_column_set_data_array( encoder, i, ODC_STRLEN, ODC_STRLEN, str_buffers[i]) );
            }
        }	
         CHECK_RESULT(odc_encode_to_file_descriptor(encoder, fd, NULL) );
    } 


// Local date & Time
char datetime[64];
time_t now = time(NULL);
struct tm tm_info;
gmtime_r(&now, &tm_info);

strftime(datetime, sizeof(datetime), "%Y-%m-%d %H:%M:%S UTC", &tm_info);

// Add some attributes 
int tab_size=6; 
char *key  [tab_size];
char *value[tab_size]; 
// attributes 
key [0] ="File_format : ";
key [1] ="Encoded_by  : " ;
key [2] ="Datetime_creation : ";
key [3] ="Meteorological_centre : ";
key [4] ="Native_format :";
key [5] ="ODC_software_version : ";
// values 
value[0]="ODB2"; 
value[1]="ECMWF_ODC_+_odb4py";
value[2]=datetime;
value[3]="Royal_Meteorological_Institute_of_Belgium_(RMI)";
value[4]="ODB1";
value[5]="1.6.3";

// set the propreties 
for (int i=0 ; i < tab_size ; i++) {
CHECK_RESULT(odc_encoder_add_property(encoder, key[i]  ,  value[i] ));
}

// close file descriptor
close(fd);
CHECK_RESULT(odc_free_encoder(encoder));

// Fee buffers 
if (buffers    )  free(buffers) ; 
if (int_buffers)  free(int_buffers) ;
if (str_buffers)  free(str_buffers) ;
if (is_string  )  free(is_string  ) ;
if (odc_type   )  free(odc_type   ) ;

// Free ODB vars 
ci = odbdump_destroy_colinfo(ci, nci);
odbdump_close(h);
if (d) {  FREEX(d);   }
return PyLong_FromLong(0) ; 
}

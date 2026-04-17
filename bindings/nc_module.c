
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
#include <Python.h>

#include "netcdf.h"


// Custom
#include "pyspam.h"
#include "progress.h"
#include "rows.h"
#include "ncdf.h"


#define ODB_STRLEN 8  // 8 chars + '\0'
//#define CHUNK 5000
#define DEFAULT_CHUNK  5000




static int DefineNcfile       (  const char *database,
                                  char *filename     ,
                                  char *sql_query    ,
                                  char *poolmask_str ,
                                  int  *ncid         )

{
// Local date & Time 
char datetime[64];
time_t now = time(NULL);
struct tm tm_info;
gmtime_r(&now, &tm_info);

strftime(datetime, sizeof(datetime), "%Y-%m-%d %H:%M:%S UTC", &tm_info);

// Analysis & creation datetime  (From .dd file )
int  size = 512       ; 
char ddfile [ size ]  ;
build_dd_path (database ,  ddfile  , size  ) ;    // Look for the path 

int vers  =0, majv =0 ;   // ODB version  realease  
int ana_date  =0 ;        // analysis datetime
int ana_time  =0 ; 
int creat_date=0 ;        // creatime datetime 
int creat_time=0 ;
int npools    =0 ;        // npools 
int ntabs     =0 ;        // number of considered tables   (ODB_CONSIDER_TABLES env var  in ODB user guide )
// Read  $dbname.dd file  
scan_dd_file ( database  , &vers , &majv , 
		           &ana_date , 
		           &ana_time , 
			   &creat_date , 
			   &creat_time ,
			   &npools     ,
			   &ntabs    )  ;

// nc check  ID 
int check  ;  
// Prepare datetime for writing  
char ana_str  [32];
char creat_str[32];
format_datetime(ana_date, ana_time, ana_str);
format_datetime(creat_date, creat_time, creat_str);

// The title & global attrib 
const char *conv        = "CF-1.10" ;
const char *title       = "ODB data in NetCDF format";
const char *history     = "Created by: odb4py python package";
const char *institution = "Royal Meteorological Institute of Belgium (RMI)";
const char *feature     = "point" ; 
const char *data_source = "ECMWF ODB";

// Get nc version  
const char *nc_version     = nc_inq_libvers();

// Normalize strings  .. remove blanks 
char sql_stmt [4096] ; 
char version  [4096] ; 
// Operate on local string  
strcpy(sql_stmt , sql_query  );
strcpy(version  ,nc_version  ); 
normalize_spaces ( sql_stmt  );
normalize_spaces ( version   );


check  =nc_put_att_text(  *ncid, NC_GLOBAL, "Title"      ,strlen(title) , title);
   if (check != NC_NOERR) { ERR(check);    return -1;  }

check  =nc_put_att_text(  *ncid, NC_GLOBAL, "NetCDF_filename" ,strlen(filename),filename );
   if (check != NC_NOERR) { ERR(check);    return -1;  }

check = nc_put_att_text(*ncid, NC_GLOBAL, "Conventions", strlen(conv), conv);
    if (check != NC_NOERR) { ERR(check); return -1; }

check = nc_put_att_text(*ncid, NC_GLOBAL, "NetCDF_datetime_creation", strlen(datetime), datetime);
  if (check != NC_NOERR) { ERR(check); return -1; }

if (sql_query) {
    check = nc_put_att_text(*ncid, NC_GLOBAL, "Data_SQL_statement", strlen(sql_stmt),sql_stmt);
    if (check != NC_NOERR) { ERR(check); return -1; }
}    
if (poolmask_str) {
    check = nc_put_att_text(*ncid, NC_GLOBAL, "pool#", strlen(poolmask_str), poolmask_str);
    if (check != NC_NOERR) { ERR(check); return -1; }
}
check  =nc_put_att_text(  *ncid, NC_GLOBAL, "NetCDF_library_version" ,strlen(nc_version),nc_version);
   if (check != NC_NOERR) { ERR(check);    return -1;  }

check  =nc_put_att_text(  *ncid, NC_GLOBAL, "History"    ,strlen(history) , history);
   if (check != NC_NOERR) { ERR(check);    return -1;  }
check  =nc_put_att_text(  *ncid, NC_GLOBAL, "odb4py_version"    ,strlen(ODB4PY_VERSION) , ODB4PY_VERSION);
   if (check != NC_NOERR) { ERR(check);    return -1;  }

check  =nc_put_att_text(  *ncid, NC_GLOBAL, "Institution",strlen(institution) , institution);
   if (check != NC_NOERR) { ERR(check);    return -1;  }
check  =nc_put_att_text(  *ncid, NC_GLOBAL, "Native_fomrat" ,strlen(data_source) , data_source );
   if (check != NC_NOERR) { ERR(check);    return -1;  }

check  =nc_put_att_text(  *ncid, NC_GLOBAL, "featureType",strlen(feature )    , feature );
   if (check != NC_NOERR) { ERR(check);    return -1;  }

// Date and time
check  =nc_put_att_text(  *ncid, NC_GLOBAL, "NetCDF_datetime_creation",strlen(datetime) , datetime );
   if (check != NC_NOERR) { ERR(check);    return -1;  }
check  =nc_put_att_text(  *ncid, NC_GLOBAL, "ODB_analysis_datetime"       , strlen(ana_str)  , ana_str  );
   if (check != NC_NOERR) { ERR(check);    return -1;  }
check  =nc_put_att_text(  *ncid, NC_GLOBAL, "ODB_creation_datetime"       , strlen(creat_str), creat_str);
   if (check != NC_NOERR) { ERR(check);    return -1;  }

// The current odb attributes
check  =nc_put_att_int(  *ncid, NC_GLOBAL, "ODB_software_version"      , NC_INT, 1, &vers);
   if (check != NC_NOERR) { ERR(check);    return -1;  }
check  =nc_put_att_int(  *ncid, NC_GLOBAL, "ODB_major_version", NC_INT, 1, &majv);
   if (check != NC_NOERR) { ERR(check);    return -1;  }
check  =nc_put_att_int(  *ncid, NC_GLOBAL, "Number_of_ODB_pools"       , NC_INT, 1, &npools);
   if (check != NC_NOERR) { ERR(check);    return -1;  }
check  =nc_put_att_int(  *ncid, NC_GLOBAL, "Number_of_considered_ODB_tables"      , NC_INT, 1, &ntabs);
   if (check != NC_NOERR) { ERR(check);    return -1;  }  
   return 0;
}




// Function  : odb_to_nc_method
static PyObject *odb_to_nc_method(PyObject *Py_UNUSED(self),
                                 PyObject *args,
                                 PyObject *kwargs) {

    // Arguments keywords
    const char *database  = NULL;
    char *sql_query = NULL;
    char *queryfile = NULL;
    char *ncfile    = NULL;

    int   fcols     = 0   ;
    int   fmt_float = 15  ;
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
			       "rows_per_chunk", 
			       "fmt_float",
                               "queryfile",
			       "poolmask" ,
			       "pbar"     ,
			       "verbose"  ,
			        NULL
                             };
    // Parse keyword args
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "szis|iizOOO", kwlist,   // 4 requiered , 6 optional
                                     &database,
                                     &sql_query,
                                     &fcols    ,
				     &ncfile   ,
				     &nrows_chunk,
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

    // Convert poolmask to string
    char *poolmask_str = NULL;
    if (poolmask_obj != Py_None) {
        if (!PyUnicode_Check(poolmask_obj)) {
         PyErr_SetString(PyExc_TypeError, "--odb4py : poolmask must be a string.  ex: '1 2 3 N' or '1:N' N=Number of pools'  \n") ;
         return PyLong_FromLong(-1);
         }
     poolmask_str  = PyUnicode_AsUTF8(poolmask_obj);
    }
    // Conversion to boolean C variable
    lpbar   = PyObj_ToBool ( pbar , lpbar      ) ;
    verbose = PyObj_ToBool ( pverb , verbose   ) ;

// Normalize strings  .. remove blanks 
char sql_stmt [4096] ;
char poolmask [4096] ;
// Operate on local strings  

if ( sql_query ) {
strcpy(sql_stmt , sql_query  );	
normalize_spaces ( sql_stmt  );
}

if ( poolmask_str ){
    strcpy(poolmask , poolmask_str  );
    normalize_spaces ( poolmask   );
}

    if (verbose && poolmask_str ) {
        printf("Fetch data from pool(s) #: %s\n", poolmask );
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
    int total_rows = getMaxrows(database, sql_query , queryfile , poolmask_str );

    // Check number of rows --> check the query answer
    if ( total_rows ==0 ) {
      PyErr_SetString(PyExc_RuntimeError, "--odb4py : The SQL request returned zero rows.");
      return PyLong_FromLong(-1) ;
    }
    if (total_rows <= 0) total_rows = 4096;   // Fallback
    prog_max = (size_t)total_rows;
    if (maxlines == 0) return PyLong_FromLong(rc);
    if (verbose) {
        if (sql_query)
            printf("--odb4py : Executing query from string: %s\n", sql_stmt);
    }   else if (queryfile) {
            printf("--odb4py : Executing query from file   : %s\n", queryfile);
	    printf("%s\n", "--odb4py : WARNING --> Executing the queries from SQL file is DEPRECATED. Not completly stable");
    }
    //  OPEN ODB
    h = odbdump_open(database, sql_query, queryfile, poolmask_str , varvalue, &maxcols);
    if (!h || maxcols <= 0) {
        PyErr_SetString(PyExc_RuntimeError, "--odb4py : Failed to open ODB or invalid number of columns");
        return PyLong_FromLong(-1)  ;
    }
    // Number of columns taking into account the number of functions in the query  (col pure - n columns function)
    int ncols = maxcols - fcols;
    if (verbose)   printf("--odb4py : Number of requested columns : %d\n", ncols);



// Reset the number of rows per chunk (frame)
int rows_by_chunk=DEFAULT_CHUNK  ;
if ( nrows_chunk   &&  nrows_chunk !=  DEFAULT_CHUNK ) {
     rows_by_chunk = nrows_chunk ;    // override  if argument is given 
}


// Init nc vars 
int ncid, dimid, strlen_dimid;
int *varid     = malloc(ncols * sizeof(int));
int *is_string = malloc(ncols * sizeof(int));
int check = 0 ; 


// create ncetcdf   file 
check = nc_create(ncfile  , NC_NETCDF4, &ncid);
     if (check != NC_NOERR) { ERR(check); return PyLong_FromLong(-1) ; }


// Encode the metadata  
if ( DefineNcfile  (database  ,
                  ncfile      ,
	          sql_query   , 
		  poolmask_str,  
		  &ncid   ) != 0 ) {
      printf("%s\n" ,   "--odb4py : Failed to write meta data" )   ; 
      return PyLong_FromLong(-1)   ; 
} 

//  Add the ODB data according to sql query and columns/rows 
//  dimensions
check = nc_def_dim(ncid, "obs", NC_UNLIMITED, &dimid);
if (check != NC_NOERR) { ERR(check); return PyLong_FromLong(-1) ; }
check = nc_def_dim(ncid, "strlen", ODB_STRLEN, &strlen_dimid);
if (check != NC_NOERR) { ERR(check); return PyLong_FromLong(-1) ; }

//  ODB data are returned here !
double *d = malloc(maxcols * sizeof(double));
//char *colnames= calloc (  maxcols * ODB_STRLEN  ) ; 
char **colnames = calloc(ncols, sizeof(char*));

// Temporary data buffers 
double **buffers     = malloc(ncols * sizeof(double*));
char   **str_buffers = malloc(ncols * sizeof(char*));

// Exit if not allocated 
if ( !buffers || !str_buffers )  {
      printf("%s\n", "--odb4py : Failed to allocate buffer for NetCDF encoding.") ; 
      return PyLong_FromLong(-1) ;
}

for (int i = 0; i < ncols; i++) {
    buffers[i]     = malloc(rows_by_chunk * sizeof(double));
    str_buffers[i] = malloc(rows_by_chunk * ODB_STRLEN * sizeof(char));
    is_string  [i] = 0;
    if ( !buffers[i] || !str_buffers[i]  ) {
         
	    // Free 
        for (int j = 0; j <= i; j++) {
            if (buffers[j])     free(buffers[j]);
            if (str_buffers[j]) free(str_buffers[j]);
        }
	    if (verbose) {
                 printf(  "--odb4py : Memory allocation failed for column %d\n", i);
                  }
        return PyLong_FromLong(-1) ;
    }
}



// Indices 
size_t k = 0;
size_t global_index = 0;

int first_dataset = 1;
int new_dataset   = 0;

// Loop over ODB  rows 
int (*nextrow)(void *, void *, int, int *) = odbdump_nextrow;
while (nextrow(h, d, maxcols, &new_dataset) > 0) {
    if (lpbar) {  ++ip;            print_progress(ip, prog_max); }   // useful for huge ODBs 
        if (new_dataset) {
            ci = odbdump_destroy_colinfo(ci, nci);
            ci = odbdump_create_colinfo(h, &nci);
            new_dataset = 0;
        }
    // Define nc variables only once 
    if (first_dataset) {
        for (int i = 0; i < ncols; i++) {
            const char *name = ci[i].nickname ? ci[i].nickname : ci[i].name;
            char  cname[128];
            strcpy(cname, name);
            sanitize_name(cname);

	    colnames [i] = strdup(cname);

            int odb_dtype = ci[i].dtnum;
            if (odb_dtype == DATATYPE_STRING) {
                is_string[i] = 1;   // ---> this col  is  string  dtype 
                int dimids[2] = {dimid, strlen_dimid};
           // column name
           check = nc_def_var(ncid, cname, NC_CHAR, 2, dimids, &varid[i]);		
	      if (check != NC_NOERR) { ERR(check); return PyLong_FromLong(-1) ; }
            } else {
                is_string[i] = 0;
                int nc_type;
                switch (odb_dtype) {
                    case DATATYPE_INT1:
                    case DATATYPE_INT2:
                    case DATATYPE_INT4:
                    case DATATYPE_YYYYMMDD:
                    case DATATYPE_HHMMSS:  nc_type = NC_INT64 ; break;
                    default:               nc_type = NC_DOUBLE; break;
                }
		// def vars according to  datatype 
                check = nc_def_var(ncid, cname, nc_type, 1, &dimid, &varid[i]);
                   if (check != NC_NOERR) { ERR(check); return PyLong_FromLong(-1) ; }

        if (strcmp(cname,"degrees_lat")==0 || strcmp(cname,"lat_hdr")==0) {
         check = nc_put_att_text(ncid, varid[i], "units", strlen("degrees_north"),"degrees_north");
            if (check != NC_NOERR) { ERR(check); return PyLong_FromLong(-1) ; }
         check =nc_put_att_text(ncid, varid[i],"standard_name",strlen("latitude"),"latitude");
	    if (check != NC_NOERR) { ERR(check); return PyLong_FromLong(-1) ; }
        } // lat
        if (strcmp(cname,"degrees_lon")==0 || strcmp(cname,"lon_hdr")==0) {
         check=nc_put_att_text(ncid, varid[i], "units", strlen("degrees_east"), "degrees_east");
	    if (check != NC_NOERR) { ERR(check); return PyLong_FromLong(-1) ; }
         check=nc_put_att_text(ncid, varid[i], "standard_name", strlen("longitude"), "longitude");
	    if (check != NC_NOERR) { ERR(check); return PyLong_FromLong(-1) ; }
        } // lon
 
	size_t chunk[1] = {rows_by_chunk};
        check  = nc_def_var_chunking(ncid, varid[i], NC_CHUNKED, chunk);
             if (check != NC_NOERR) { ERR(check); return PyLong_FromLong(-1) ; }
            }
        } // for ncols 1  -->  var definition 
        check = nc_enddef(ncid);
	   if (check != NC_NOERR) { ERR(check); return PyLong_FromLong(-1) ; }
        first_dataset = 0;
    }   // first dataset 

    //  FILL BUFFERS 
    for (int i = 0; i < ncols; i++) {        	    
        ci = odbdump_create_colinfo(h, &nci);
        const char *name = ci[i].nickname ? ci[i].nickname : ci[i].name;
        char  cname[128];
        strcpy(cname, name);
        sanitize_name(cname); 
        if (is_string[i]) {
            char *dst = &str_buffers[i][k * ODB_STRLEN];
            union { char s[sizeof(double)]; double d; } u;
            u.d = d[i];
            for (size_t j = 0; j < sizeof(double); j++) {
                char c = u.s[j];
                dst[j] = isprint(c) ? c : ' ';            }
            for (int j = sizeof(double); j < ODB_STRLEN; j++) {
                dst[j] = ' ';
            }
        } else {		    
	// In netcdf lat/lon are in degrees 
	// the function 'sanitize_name'  removes '@' character and replace with '_'.
	if ( ABS(d[i]) == mdi )  { 
		buffers[i][k]  =   NAN ;
	}else if (strcmp(cname,"lon_hdr")==0  || strcmp(cname,"lat_hdr")==0  ) {	    	     
             buffers[i][k] =  format_float(  d[i]*RAD2DEG, fmt_float)  ;             
        }else {	
	     buffers[i][k] =   format_float(d[i] ,  fmt_float) ;
	      }
	}
    }  // for ncols  2 -- > fill buffers 
    k++;
    // write  data by CHUNKS 
    if (k == rows_by_chunk) {
        for (int i = 0; i < ncols; i++) {
            if (is_string[i]) {
                size_t start[2] = {global_index, 0};
                size_t count[2] = {k, ODB_STRLEN};
                check = nc_put_vara_text(ncid, varid[i], start, count, str_buffers[i]);
         	   if (check != NC_NOERR) { ERR(check); return PyLong_FromLong(-1) ; }
            } else {
                size_t start[1] = {global_index};
                size_t count[1] = {k};
                check = nc_put_vara_double(ncid, varid[i], start, count, buffers[i]);
	            if (check != NC_NOERR) { ERR(check); return PyLong_FromLong(-1) ; }
            }
        }
        global_index += k;   // Update global index 
        k = 0;
      }  // k == rows_by_chunk 
}
// Final bloc 
if (k > 0) {
    for (int i = 0; i < ncols; i++) {
        if (is_string[i]) {
            size_t start[2] = {global_index, 0};
            size_t count[2] = {k, ODB_STRLEN};
            check = nc_put_vara_text(ncid, varid[i], start, count, str_buffers[i]);
	    if (check != NC_NOERR) { ERR(check); return PyLong_FromLong(-1) ; }
        } else {
            size_t start[1] = {global_index};
            size_t count[1] = {k};
            check = nc_put_vara_double(ncid, varid[i], start, count, buffers[i]);
	    if (check != NC_NOERR) { ERR(check); return PyLong_FromLong(-1) ; }
        }
    }
}

/* get the number of written bytes 
int nvars;
nc_inq_nvars(ncid, &nvars);
size_t total = 0;
for (int v = 0; v < nvars; v++)
    total += nc_var_size_bytes(ncid, varid[v] );*/

if (verbose  ) {
    printf( "%s %s\n" ,  "List of column names in ", ncfile   ) ;
    for ( int c =0; c< ncols ; c++ ){
         printf  ( "%s  :  %s \n" , "Column " ,  colnames[c] )  ;
        }
}


//  Close the NETCDF file  
check = nc_close(ncid);
if (check != NC_NOERR) { ERR(check); return PyLong_FromLong(-1) ; }
// Free buffers 
if (buffers)     free(buffers);
if (str_buffers) free(str_buffers);
// free structures 
if (ci)   odbdump_destroy_colinfo(ci, nci);
if (h)    odbdump_close(h);

// Check file creation and size 
check_file ( ncfile ,   verbose  ) ; 


return PyLong_FromLong(0);
}

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

#include "sqlite3.h"


// Custom
#include "pyspam.h"
#include "progress.h" 
#include "rows.h" 
#include "map_dtype.h"

#define ODB_STRLEN 8  // 8 chars + '\0'


#define CHECK_RC(rc, db, msg) \
    if ((rc) != SQLITE_OK && (rc) != SQLITE_DONE) { \
        fprintf(stderr, "%s: %s\n", msg, sqlite3_errmsg(db)); \
        sqlite3_exec(db, "ROLLBACK;", 0, 0, 0); \
        goto sqlite_error ; \
    }




// Function  : odb2sqlite_method
static PyObject *odb_to_sqlite_method(PyObject *Py_UNUSED(self),
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

     //  Start sqlite3 stuff      
    sqlite3 *db;
    sqlite3_stmt *stmt;
    const char *sqlite_db  = NULL ;
    const char *table_name = NULL ; 

    // Keyword list 
    static char *kwlist[] = {  "database"  , 
	                       "sql_query" , 
			       "nfunc"     ,
			       "outfile" ,
			       "table_name",
			       "fmt_float",
                               "queryfile", 
			       "poolmask" , 
			       "pbar"     , 
			       "verbose"  ,  
			        NULL
                             };

    // Parse keyword args 
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "szis|sizOOO", kwlist,   // 3 requiered , 5 optional 
                                     &database,
                                     &sql_query,
                                     &fcols    ,
				     &sqlite_db,
				     &table_name, 
                                     &fmt_float,
                                     &queryfile,
                                     &poolmask_obj,
                                     &pbar,
                                     &pverb)) {
          return PyLong_FromLong(-1);	
    }
    // Required  
    if (!sql_query && !queryfile) {
      printf( "%s\n" ,  "--odb4py : Either 'sql_query' or 'queryfile' must be provided"  ); 
       return PyLong_FromLong(-1);
    }
    // Convert to string  
    const char *poolmask_str = NULL;
    if (poolmask_obj != Py_None) {
        if (!PyUnicode_Check(poolmask_obj)) {
	  printf( "%s\n", "--odb4py : poolmask must be a string.  ex: '1 2 3 N' or '1:N' N=Number of pools'  \n") ;
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
    int total_rows = getMaxrows(database, sql_query , queryfile , poolmask_str );

    // Check number of rows --> check the query answer 
    if ( total_rows ==0 ) {
      printf( "%s\n",  "--odb4py : The SQL request returned zero rows." )  ; 
      return PyLong_FromLong(-1) ;  
    }
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
        printf( "%s\n", "--odb4py : Failed to open ODB or invalid number of columns" )  ; 
        return PyLong_FromLong(-1)  ; 
    }

    // Number of columns taking into account the number of functions in the query  (col pure - n columns function)
    int ncols = maxcols - fcols;
    if (verbose)   printf("--odb4py : Number of requested columns : %d\n", ncols);


    // Start transaction 
    rc  = sqlite3_open(sqlite_db , &db);
    if ( rc  ){
	printf( "%s\n", "--odb4py : Can't open destination database for SQLITE conversion ");
        return  PyLong_FromLong(-1) ;  
    }
    

    sqlite3_exec(db, "PRAGMA journal_mode=OFF;" , 0, 0, 0);
    sqlite3_exec(db, "PRAGMA synchronous=OFF;"  , 0, 0, 0);
    rc  =  sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, 0);
    CHECK_RC(rc, db, "Begin SQLITE transaction failed"   );

    // = 1 if created 
    int table_created = 0;

    // Rows 
    double *d = malloc(maxcols * sizeof(double));
    int (*nextrow)(void *, void *, int, int *) = odbdump_nextrow;
    int new_dataset=0; 


    // Bytes counter (for the extract & written ONLY without the file structure )
    size_t  total_bytes =0   ; 
    while (nextrow(h, d, maxcols, &new_dataset) > 0) {
        if (lpbar) {  ++ip;            print_progress(ip, prog_max); }   // useful for  large ODBs

        if (new_dataset) {

            ci = odbdump_destroy_colinfo(ci, nci);
            ci = odbdump_create_colinfo(h, &nci);

            const char *default_table = "ODB";
            const char *final_table = default_table;

           if (table_name && table_name[0] != '\0') {
                 final_table = table_name;  // override default
             }

            if (!table_created) {
                 char sql[4096];
                 char insert_sql[4096];
                 char cleaned_table[128];

                 // Copy before sanitizing
                 strncpy(cleaned_table, final_table, sizeof(cleaned_table) - 1);
                 cleaned_table[sizeof(cleaned_table) - 1] = '\0';
                 sanitize_name(cleaned_table);
                 snprintf(sql, sizeof(sql),
                 "CREATE TABLE IF NOT EXISTS \"%s\" (", cleaned_table);

                 snprintf(insert_sql, sizeof(insert_sql),
                 "INSERT INTO \"%s\" VALUES (", cleaned_table);
                
                /*char sql[4096] = "CREATE TABLE IF NOT EXISTS ODB (";
	        char cleaned_table[128];
		if (!table_table) {  
		table_name = "ODB" ;  
		} else {
                strcpy(cleaned_table, table_name);
                sanitize_name(cleaned_table);
                snprintf(sql, sizeof(sql), "CREATE TABLE IF NOT EXISTS %s (", cleaned_table);    // remove blank characters 
                snprintf(sql, sizeof(sql), "CREATE TABLE IF NOT EXISTS \"%s\" (", table_name);
                char insert_sql[4096] = "INSERT INTO ODB VALUES (";
                snprintf(insert_sql, sizeof(insert_sql), "INSERT INTO \"%s\" VALUES (", table_name);
		}*/


                for (int i = 0; i < ncols; i++) {
                    const char *name = ci[i].nickname ? ci[i].nickname : ci[i].name;
                    char cname [128];    // cleaned 

                    strcpy(cname , name);
                    sanitize_name (cname ) ; 
                    int odb_dtype = ci[i].dtnum;

                    strcat(sql, cname);
                    strcat(sql, " ");
		    char *dtype = map_type  ( odb_dtype )  ; 

		    if (  strcmp(dtype ,"INTEGER") ) {
                           strcat(sql, "INTEGER");
		    } else if ( strcmp(dtype,"TEXT") ) {
                          strcat(sql, "TEXT");
		    }
		     else {
                          strcat(sql, "REAL");
		     }                    
		    strcat(insert_sql, "?");
                    if (i < ncols - 1) {
                        strcat(sql, ", ");
                        strcat(insert_sql, ",");
                    }
                }  // for 
                strcat(sql, ");");
                strcat(insert_sql, ");");

		// Create table 
                rc = sqlite3_exec(db, sql, 0, 0, 0);
                CHECK_RC(rc, db, "CREATE TABLE failed");

                rc = sqlite3_prepare_v2(db, insert_sql, -1, &stmt, NULL);
		if (   rc !=  SQLITE_OK ) {
		    printf("%s\n" , "--odb4opy : Prepare SQLITE database failed" )    ;
		    goto sqlite_error  ; 
		}
                table_created = 1;
            } // table 
        }// dataset  

	// Another switch to  insert  rows by datatype 
        for (int i = 0; i < ncols; i++) {
            int type = ci[i].dtnum;
               if ( type  != DATATYPE_STRING && ABS(d[i]) == mdi)  {
	            rc = sqlite3_bind_null  ( stmt ,i+1  );
	       } else {
            switch (type)   {
		case  DATATYPE_STRING : 
			{
                   char cc[sizeof(double)+1];
                   char *scc = cc ;
                   union {  char s[sizeof(double)] ;double d;} u ;  u.d = d[i];
                   for (long unsigned int js=0; js<sizeof(double); js++) { 
			char c = u.s[js]; 
			*scc++ = isprint(c) ? c : '8' ; 
		       }
                     *scc = '\0';
		      total_bytes+= sizeof(  cc) ; 

                      rc = sqlite3_bind_text(stmt, i+1,  cc   , -1, SQLITE_TRANSIENT) ;
                   } 
		break  ; 			
                case DATATYPE_INT1:
                case DATATYPE_INT2:
                case DATATYPE_INT4:
                case DATATYPE_YYYYMMDD:
                case DATATYPE_HHMMSS:		 
           	 rc=sqlite3_bind_int( stmt, i+1 , (int) d[i]);
		 total_bytes+= sizeof( (int) d[i])  ; 
                break  ;
		default :
                 rc=sqlite3_bind_double(stmt, i+1, d[i]);
                 total_bytes+= sizeof( d[i])  ;		 
 		 break  ; 
	          }  // switch 		 

	       if (rc != SQLITE_OK) {
                printf(  "--odb4opy : Bind error col %d: %s\n", i, sqlite3_errmsg(db));
                goto sqlite_error ;
            }
            }
        } // for    insert  

        rc = sqlite3_step(stmt);       
        if (rc != SQLITE_DONE) {
            printf("--odb4opy : Step error: %s\n", sqlite3_errmsg(db));
            goto sqlite_error ;
        }
        sqlite3_reset(stmt);
    }

rc = sqlite3_exec(db, "COMMIT;", 0, 0, 0);
CHECK_RC(rc, db, "Final COMMIT failed for the SQLITE database !");

sqlite_error:
    if (stmt) sqlite3_finalize(stmt);
    if (db)   sqlite3_close(db);
    // ODB vars 
    //if (d)    free(d);
    if (ci)   odbdump_destroy_colinfo(ci, nci);
    if (h)    odbdump_close(h);

if ( verbose )  {
   printf( "%s : %s\n" , "--odb4py : ODB successfully written into the SQLite file" , sqlite_db ) ;
   printf( "%s : %ld Bytes\n" , "--odb4py : Total written data size ",  total_bytes  ) ;
}
return PyLong_FromLong(0);
}

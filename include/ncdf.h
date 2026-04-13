#ifndef NCDF_H
#define NCDF_H
// C
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <string.h>
#include <libgen.h>

// nc header
#include "netcdf.h"

// Define Pi 
#define PI 3.141592653589793
#define RAD2DEG (180.0/PI)


/* Handle errors by printing an error message and exiting with a
* non-zero status. from   https://docs.unidata.ucar.edu/netcdf-c  */
#define ERRCODE 2
#define ERR(e) {printf("Error: %s\n", nc_strerror(e)); exit(ERRCODE);}



typedef struct  {
    int odb_col;
    colinfo_t *meta;
    int is_string;
} nc_column_t;



/*static int   check_scratch  (const char *varname) {   
 * to be used later !!
 * Check whether the *TMPDIR , SCRATCH or SCRATCHDIR  exist and have rw access
    char *val = getenv(varname);
    struct stat info;
    if ( val    == NULL) {
        printf("%s Not defined path  .\n",   val   );
        return -1 ;
     } 
    // W_OK -->>  write access OK
    // R_OK -->>  read access OK
    // X_OK -->>  execute access OK
        if (stat(val , &info) == 0) {
             if (S_ISDIR(info.st_mode)) {
            		 if  (access( val  , W_OK) == 0 ) {        // Exists + dir +  write 
	          	   return 0  ;
		           } else { printf( "The directory %s has no WRITE access\n" ,  val ) ; return -1  ; }			 
		         if ( access( val  , R_OK) == 0 ) {        // Exists + dir +  read 
	                   return 0 ; 
                            } else {printf( "The directory %s has no  READ access\n" ,  val ) ; return -1  ;   } 		
			    } else {	       
                  		 printf( " NOT FOUND directory %s\n", val  ) ;  return -1  ; }
		 } else {printf( " NOT FOUND  directory :%s\n", val  ) ;  return -1  ;  }		 
return 0; 
}*/



void poolfile(char *outfile, 
	      size_t size,
	      char *filename , 
              char *index_str ) {
    const char *dot = strrchr(filename, '.');
    if (dot) {
        size_t base_len = dot - filename;
        snprintf(outfile, size, "%.*s_%s%s", (int)base_len, filename,index_str, dot);
    } else {
        snprintf(outfile, size, "%s_%s",filename, index_str);
    }
}


// The dbname.dd file  path
void build_dd_path(const char *path, char *ddfile, size_t size)
{
    char tmp[512];
    strncpy(tmp, path, sizeof(tmp));
    tmp[sizeof(tmp)-1] = '\0';
    char *base = basename(tmp);
    char dbname[128];
    strncpy(dbname, base, sizeof(dbname));
    dbname[sizeof(dbname)-1] = '\0';

    char *dot = strchr(dbname,'.');
    if (dot)
        *dot = '\0';
    snprintf(ddfile, size, "%s/%s.dd", path, dbname);
}


// read the file  dbname.dd
void   scan_dd_file ( const char *database   , 
                             int *vers , 
			     int *majv ,
		             int *ana_date   , 
			     int *ana_time   ,
			     int *creat_date , 
			     int *creat_time ,
			     int *npools     , 
			     int *ntabs  ) {

char  ddfile [256] ;
char  line   [256];
int minv ;
int modif_date , modif_time  ;

int nfirst_line=6  ; 
build_dd_path(    database    ,ddfile , sizeof(ddfile));

FILE *fp = fopen(ddfile, "r");
if (!fp) {
    return  (void) NULL ;
}
int  i = 0  ;
while (fgets(line, sizeof(line), fp) && i <= ( nfirst_line -1)) {

    switch  (i)  {
      case 0: sscanf(line, "%d %d %d" , &(*vers) , &(*majv) , &minv      ); break  ;
      case 1: sscanf(line, "%d %d",    &(*creat_date) , &(*creat_time )  ); break  ;
      case 2: sscanf(line, "%d %d",  &modif_date, &modif_time     ); break  ;
      case 3: sscanf(line, "%d %d",  &(*ana_date  ), &(*ana_time) ); break  ;
      case 4: sscanf(line, "%d",  &(*npools )  ); break ;
      case 5: sscanf(line, "%d",  &(*ntabs)    ); break ;
      default:
	 (void) 0 ;   // do nothing  
    }
    i++  ;
}
}
// Format datetime to  YY-MM-DD HH:MM:SS UTC 
void format_datetime(int date, int time, char *out){
    int Y = date / 10000;
    int M = (date / 100) % 100;
    int D = date % 100;
    int h = time / 10000;
    int m = (time / 100) % 100;
    int s = time % 100;
    sprintf(out, "%04d-%02d-%02d %02d:%02d:%02d UTC", Y, M, D, h, m, s);
}

// Count number of written bytes 
size_t nc_var_size_bytes(int ncid, int varid)
{
    nc_type vartype;
    int ndims;
    int dimids[NC_MAX_VAR_DIMS];

    size_t typesize;
    size_t dimlen;
    size_t nelems = 1;

    int check  ;  
    check=nc_inq_vartype(ncid, varid, &vartype);
       if (check != NC_NOERR) { ERR(check);    return -1;  }
    check=nc_inq_varndims(ncid, varid, &ndims);
       if (check != NC_NOERR) { ERR(check);    return -1;  }
    check=nc_inq_vardimid(ncid, varid, dimids);
       if (check != NC_NOERR) { ERR(check);    return -1;  }
    check=nc_inq_type(ncid, vartype, NULL, &typesize);
       if (check != NC_NOERR) { ERR(check);    return -1;  }


    for (int i = 0; i < ndims; i++) {
        check=nc_inq_dimlen(ncid, dimids[i], &dimlen);
	   if (check != NC_NOERR) { ERR(check);    return -1;  }
        nelems *= dimlen;
    }
    return nelems * typesize;
}


// remove spaces 
void normalize_spaces( char *s)
{
    char *src = s;
    char *dst = s;
    int space = 0;
    while (*src) {
        if (*src == ' ') {
            if (!space) {
                *dst++ = ' ';
                space = 1; }
        } else {
            *dst++ = *src;
            space = 0;
        }
        src++;
    }
    *dst = '\0';
}



// SQL 
char *read_sql_file(const char *filename){
    FILE *f = fopen(filename, "r");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);
    char *buffer = malloc(size + 1);
    if (!buffer) {
        fclose(f);
        return NULL;
    }
    fread(buffer, 1, size, f);
    buffer[size] = '\0';
    fclose(f);
    return buffer;
}
// Get select statement 
char *extract_select( char *sql)
{
    char *p = sql;
    while (*p) {
        if (strncasecmp(p, "SELECT", 6) == 0) {
            return p;
        }
        p++;
    }
    return NULL;
}

// Trim 
void trim(char *s)
{
    char *start = s;
    char *end;
    if (!s || *s == '\0')
        return;
    while (*start && isspace((unsigned char)*start))
        start++;
    if (start != s)
        memmove(s, start, strlen(start) + 1);
    end = s + strlen(s);
    while (end > s && isspace((unsigned char)*(end - 1)))
        end--;
    *end = '\0';
}

#endif

#include <libgen.h>
#include "netcdf.h"

#define PI 3.141592653589793
#define RAD2DEG (180.0/PI)




typedef struct  {
    int odb_col;
    colinfo_t *meta;
    int is_string;
} nc_column_t;


void sanitize_name(char *name)
{
    for (char *p = name; *p; p++)
    {
        if (*p=='(' || *p==')' ||
            *p=='@' || *p=='.' ||
            *p=='/' )
        {
            *p = '_';
        }
    }
}

/*static void strip_table(char *name)
{
    char *p = strchr(name, '@');
    if (p)
        *p = '\0';
}*/


static void make_printable(char *s, int slen)
{
    for (int j = 0; j < slen; j++) {
        unsigned char c = s[j];
        if (!isprint(c))
            s[j] = ' ';
    }
}




static int is_coord(const char *name)
{
    if (!name) return 0;
    return (!strcmp(name,"lat")  || !strcmp(name,"lon")  || !strcmp(name,"latitude") || !strcmp(name,"longitude"));
}





static void convert_rad_to_deg(double *buffer,
                                   int nrows,
                                   int ncols,
                               nc_column_t *cols)
{
    for (int c = 0; c < ncols; c++)
    {
        if (cols[c].is_string)
            continue;
        const char *name = cols[c].meta->name;
        char varname[128];
        strncpy(varname, name , sizeof(varname)-1);
        varname[sizeof(varname)-1] = '\0';
        sanitize_name(varname);
        if (!is_coord(varname))
            continue;
        for (int r = 0; r < nrows; r++)
        {
            int idx = r*ncols + c;
            buffer[idx] *= RAD2DEG;
        }
    }
}



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


void scan_dd_file ( const char *database  , int *ana_date , int* ana_time  ) {

char  ddfile [256] ;
char  line[256];
int vers , majv , minv ;
/*int creat_date , creat_time  ;
int modif_date , modif_time  ;
int ana_date , ana_time  ;*/
int npools , ntabs  ;

build_dd_path(    database    ,ddfile , sizeof(ddfile));

FILE *fp = fopen(ddfile, "r");
if (!fp) {
    return  (void) NULL ;
}
int  i = 0  ;
while (fgets(line, sizeof(line), fp)) {
    switch  (i)  {
      case 0: sscanf(line, "%d %d %d" , &vers , &majv, &minv );          break  ;
/*/      case 1: sscanf(line, "%d %d",     &creat_date, &creat_time ); break  ;
      case 2: sscanf(line, "%d %d",     &modif_date, &modif_time ); break  ;*/
      case 3: sscanf(line, "%d %d",  &(*ana_date) , &(*ana_time) ); break  ;
      case 4: sscanf(line, "%d",  &npools ); break ;
      case 5: sscanf(line, "%d",  &ntabs ); break ;
      default:
	 continue ;
    }
    i++  ;
}
}


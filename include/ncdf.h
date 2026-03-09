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

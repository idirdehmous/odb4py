#-*- coding: utf-8 -*-

"""
Send the SQL query and retrieve the data !
"""


from datetime import datetime 
# Utils
from odb4py.utils import  OdbObject , SqlParser

# odb4py  core module
from odb4py.core  import  odb_open 

# Start
start  = datetime.now()


# Path
exo_path   ="../odb_samples/metcoop"
dbtype = "CCMA"
dbpath = "/".join(( exo_path  , dbtype))


# Connect 
conn=  odb_open( database = dbpath  )


# Let's get the AMDAR data (obstype =2 ) and the coordinates lat/lon in degrees
# The SQL query
sql_query="SELECT statid , obstype, varno, degrees(lat) ,  degrees(lon) , obsvalue   FROM  hdr, body WHERE obstype==2"


# Check & clean the query
p =SqlParser()

# The number of functions in the SQL statement
nfunc =p.get_nfunc   ( sql_query )

# Check and clean before sending !
sql=p.clean_string ( sql_query )

# Arguments
sql_file= None     # (type -> str     ) The sql file if used instead of and SQL request string
mask    = None     # (type -> str     ) The ODB pools to consider ( must be a string  "1" , "2", "33" ...  , etc )
progress= True     # (type -> bool    ) Progress bar (very useful in the case of large ODBs )
ndigits = 8        # (type -> int     ) Number of decimal digits for floats  (default is 15)
lverb   = False    # (type -> bool    ) Verbosity  on/off
nf      = nfunc    # (type -> integer ) Number of columns considring the functions in the sql statement (degrees, rad, avg etc ...)


# Send the query and fetch the data as a dictionary
data =conn.odb_dict  (database  =dbpath,
                      sql_query =sql    ,
                      nfunc     =nf     ,
                      fmt_float =ndigits,
                      queryfile =sql_file,
                      poolmask  =mask    ,
                      pbar      =progress,
                      verbose   =lverb  )
print( data )

# End
end  = datetime.now()
duration = end -  start
print("Runtime duration:" , duration  )

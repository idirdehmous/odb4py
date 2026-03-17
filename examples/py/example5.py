# -*- coding: utf-8 -*-
import os, sys
from datetime import datetime

# Import odb4py
from  odb4py.utils   import  OdbObject , SqlParser
from  odb4py.core    import  odb_open  ,odb_dict  , odb_close



def Connect ( db_path , ncpu ):
    conn = odb_open( database  =db_path   )
    return conn

def CreateDca (conn, db_path ,db_name   ):
    # Create DCA if not there
    if not os.path.isdir (db_path+"/dca"):
       status =conn.odb_dca ( database=db_path, db= db_name  )
    return status



def FetchData (conn ,  dbpath ,  query  ):
    # Check the  query
    p      =StringParser()
    nfunc  =p.get_nfunc   ( query )    # Parse sql statement and get the number of functions
    sql    =p.clean_string( query  )   # Check and clean before sending !
    nf  =nfunc
    pool= None
    query_file= None

    # If the an error occurs while executing the query a RuntimeError Exception is raised !
    try:
       rows =conn.odbDict (database  = dbpath ,
                     sql_query  = sql    ,
                     nfunc      = nf     ,
                     fmt_float  = 5      ,
                     pbar       = True   ,
                     verbose    = False  ,
                     queryfile  = None   ,
                     poolmask   = None  )
    except:
       RuntimeError
       print("Failed to get data from the ODB {}".format(dbpath) )
    return rows


# Script start time
start = datetime.now()

# Path to ODB directories
# e.g : /path/to/odb/YYYYMMDDHH/CCMA
# Let's use the same ODBs from MetCoOp domain
odb_dir_location  = "/home/user/odb"

# The SQL query
sql_query="select statid,\
           degrees(lat)  ,\
           degrees(lon)  ,\
           varno         ,\
           obsvalue      ,\
           fg_depar      ,\
           an_depar       \
           FROM  hdr, body WHERE an_depar is not NULL"

# Set date/time period (20240110 00h00 --> 20240112 21h00 )
yy= 2024
mm= 1
d1= 10
d2= 12
h1= 0
h2= 21
cycle_inc= 3
dbname="CCMA"

nb_odb  =0    # Number of opened ODBs 
tot_rows=0    # Total fetched rows 


for d in range(d1,  d2 +1 ):
   for h in  range( h1 , h2 +1 , cycle_inc ):

       # Month , day and hour leading zero
       mm= "{:02}".format( mm )
       dd= "{:02}".format( d )
       hh= "{:02}".format( h )
       ddt=str(yy)+str(mm)+dd+hh

       # Set the path
       dbpath = "/".join( (odb_dir_location  , ddt , dbname)  )

       # Reset the paths and IOASSIGN environnment variables for each iteration
       os.environ["ODB_SRCPATH_CCMA" ]=dbpath
       os.environ["ODB_DATAPATH_CCMA"]=dbpath
       os.environ["ODB_IDXPATH_CCMA" ]=dbpath
       os.environ["IOASSIGN"  ]       =dbpath+"/"+"IOASSIGN"

       # Connect and return the connection object
       conn= Connect  (dbpath ,dbname )

       # Create DCA
       try:
          st = CreateDca(conn, dbpath ,dbname  )
       except:
          Exception
          print("Failed to create DCA from the ODB {}".format(dbpath))
          pass

       # Get the data
       row_dict = FetchData  (conn, dbpath , sql_query)

       tot_rows+= len( row_dict["degrees(lat)"] )
       nb_odb  +=1

       # Close the database
       conn.odb_close()


# End script runtime
end = datetime.now()
duration = end - start

print( "Runtime duration         :" , duration )
print( "Total fetched rows       :" , tot_rows )
print( "Number of processed ODBs :" , nb_odb   )
print( "Average number of rows by iteration :"  , tot_rows// nb   )


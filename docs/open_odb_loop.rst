Using odb4py inside loops
=========================

ODB databases are frequently used to perform diagnostics and statistical
analysis over a sequence of dates or time periods. In such workflows,
looping over dates is therefore a common and necessary practice.

During these iterations, both the database path and the associated date
must be updated at each step. If this is not done correctly, **odb4py may continue to operate on the first opened ODB database**. As a result,
identical rows and values may be returned even though the loop variable
(e.g. the date) changes.

.. warning::

   The ODB runtime keeps an internal state based on environment
   variables defining the active database. If these variables are not
   refreshed, odb4py may continue to access the first opened ODB
   database, even if the loop variable changes.

   To ensure that each iteration accesses the correct database, the
   following ODB environment variables must be updated accordingly
   inside the Python loop:

   - ``IOASSIGN``
   - ``ODB_SRCPATH_CCMA``   
   - ``ODB_DATAPATH_CCMA``  
   - ``ODB_IDXPATH_CCMA``

   The following variables have to be used in the case of an `ECMA` database.

   - ``ODB_SRCPATH_ECMA``   
   - ``ODB_DATAPATH_ECMA``
   - ``ODB_IDXPATH_ECMA``  

   Updating these variables forces the ODB runtime to reload the correct
   database context and prevents unintended reuse of a previously opened ODB.


Recommended workflow:

1. Update the database path and environment variables for the current date.
2. Open the database.
3. Execute the required queries.
4. Close the database before moving to the next iteration.

.. code-block:: python 

   # -*- coding: utf-8 -*-
   import os, sys
   from datetime import datetime

   # Import odb4py
   from  odb4py.utils   import  OdbObject , SqlParser
   from  odb4py.core    import  odb_open  ,odb_dict  , odb_close



   def Connect ( db_path , ncpu ):
       conn = odb_open( database  =db_path   )
       return conn

   def CreateDca (conn, db_path ,db_name ):
       # Create DCA if not there
       if not os.path.isdir (db_path+"/dca"):
          status =conn.odb_dca ( database=db_path, db= db_name , ncpu= 4)
       return status



   def FetchData (conn ,  dbpath ,  query  ):
       # Check the  query
       p      =SqlParser()
       nfunc  =p.get_nfunc   ( query )    # Parse sql statement and get the number of functions
       sql    =p.clean_string( query  )   # Check and clean before sending !
       nf  =nfunc
       pool= None
       query_file= None

       # If the an error occurs while executing the query a RuntimeError Exception is raised !
       try:
          rows =conn.odb_dict (database  = dbpath ,
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
    odb_dir_location  = "/home/USER/odb"   # Set to your path 

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
    dbtype   ="CCMA"

    # Number of processed ODBs 
    nb = 0 
    # Total rows (Considering all the ODBs)
    tot_rows=0



    for d in range(d1,  d2 +1 ):
       for h in  range( h1 , h2 +1 , cycle_inc ):

       # Month , day and hour leading zero
       mm= "{:02}".format( mm )
       dd= "{:02}".format( d )
       hh= "{:02}".format( h )
       ddt=str(yy)+str(mm)+dd+hh

       # Set the path
       dbpath = "/".join( (odb_dir_location  , ddt , dbtype)  )

       # Reset the paths and IOASSIGN environnment variables for each iteration
       os.environ["ODB_SRCPATH_CCMA" ]=dbpath
       os.environ["ODB_DATAPATH_CCMA"]=dbpath
       os.environ["ODB_IDXPATH_CCMA" ]=dbpath
       os.environ["IOASSIGN"  ]       =dbpath+"/"+"IOASSIGN"

       # Connect and return the connection object
       conn= odb_open   (dbpath  )

       # Create DCA
       try:
          st = CreateDca(conn, dbpath ,dbtype )
       except:
          Exception
          print("Failed to create DCA from the ODB {}".format(dbpath))
          pass

       # Get the data
       if os.path.isdir( dbpath ):
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
   print( "Average number of rows by iteration :"  , tot_rows// nb_odb )

.. code-block:: bash

   --odb4py : ODB database closed.
   Runtime duration         : 0:00:23.243894
   Total fetched rows       : 412427
   Number of processed ODBs : 24
   Average number of rows by iteration : 17184


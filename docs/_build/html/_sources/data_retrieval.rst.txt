SQL query and data retrieval
============================

The SQL syntax used in **odb4py** follows the same conventions as the
official ODB API.

To perform a data query, one of the functions ``odb_dict`` or ``odb_array`` must be used.
The ``odb_dict`` function returns both the column names and the corresponding row
values organized as a Python dictionary. In this dictionary, column
names are used as keys, and the associated values correspond to the
retrieved ODB data rows.

The ``odb_array`` function returns a **NumPy** array containing only numeric data.
The columns of type string are automatically excluded from the result.

Therefore, if the user knows that the query contains only integer or floating-point
columns, using ``odb_array`` may be slightly faster than ``odb_dict``.

As an optional argument, this method provides the *header* option. When enabled,
this option returns the names of the selected columns together with the NumPy array.

The sql query check 
-------------------
Before executing the query, a preliminary syntax validation step is
performed using the ``get_func`` and ``clean_string`` methods inside the ``utils.SqlParser`` class. This includes:

- Removing non-printable characters if present.
- Filtering out tokens that are not part of the ODB SQL lexicon.
- Parsing SQL tokens to determine the number of ODB SQL functions used within the request.

At the low level, the total number of columns to be retrieved is equal
to the number of explicit column names plus the number of computed
columns resulting from SQL functions.

Consequently, the C backend must internally distinguish between
pure column references and function-based expressions in order to
correctly process and return the requested data.

This preprocessing stage ensures robustness and prevents malformed SQL
queries from reaching the ODB runtime layer.

.. code-block:: python

   #-*- coding: utf-8 -*-  

   from datetime import datetime 

   # Utils  
   from odb4py.utils import  OdbObject , SqlParser 

   # odb4py  core module 
   from odb4py.core  import  odb_connect 
   
   # Start 
   start  = datetime.now()  

   # Path to ODB
   db_path= "/path/to/CCMA"

   # Let's get the AMDAR data (obstype =2 ) and the coordinates lat/lon in degrees 
   # The SQL query  
   sql_query="SELECT statid , obstype, varno, degrees(lat) ,  degrees(lon) , obsvalue   FROM  hdr, body WHERE obstype==2"

   # Check & clean the query 
   p =SqlParser()

   # The number of functions in the SQL statement
   nf =p.get_nfunc   ( sql_query )    

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
   data =conn.odb_dict  (database  =db_path,
                         sql_query =sql    , 
                         nfunc     =nf     ,
                         fmt_float =ndigits,
                         query_file=sql_file,                   
                         poolmask  =mask    , 
                         pbar      =progress, 
                         verbose   =lverb  )
   print( data ) 

   # End 
   end  = datetime.now()
   duration = end -  start
   print("Runtime duration:" , duration  )
 
Output :

.. code-block:: python

   ******** New ODB I/O opened with the following environment
   ******* ODB_WRITE_EMPTY_FILES=0
	  ODB_CONSIDER_TABLES=*
	   ODB_IO_KEEP_INCORE=1
	      ODB_IO_FILESIZE=32 MB
	       ODB_IO_BUFSIZE=4194304 bytes
	       ODB_IO_GRPSIZE=1 (or max no. of pools)
	       ODB_IO_PROFILE=0
	       ODB_IO_VERBOSE=0
	        ODB_IO_METHOD=5
   ODB_CONSIDER_TABLES=*
   ODB_WRITE_TABLES=*
   ***INFO: Poolmasking ignored altogether for database 'CCMA'
   --odb4py : Creating DCA files ...done !

   [##################################################] Complete 100%  (Total: 5102 rows)
   {'statid@hdr':['2YIQTRJA', '2YIQTRJA', '2YIQTRJA'..., ],
    'obstype@hdr':[2, 2, 2,...] ,
    'varno@body':[2, 3, 4 ,...],
    'degrees(lat)':[62.38014, 62.38078, 62.38124,...],
    'degrees(lon)':[1.15, 1.12, 1.14,...] , 
    'obsvalue@body':[216.00144, 28.72984, -2.00899, ... ]}
   
   Runtime duration : 0:00:01.02

ODB data conversion 
===================
odb4py provides a set of utilities to convert ODB datasets into alternative formats, in addition to direct querying capabilities.
These conversion tools are implemented in the ``odb4py.convert`` module.

The following conversion workflows are currently supported:

- Conversion from ODB1 to ODB2
- Conversion from ODB1 to NetCDF
- Conversion from ODB1 to SQLite

.. note::
   It is important to note that conversion to the ODB2 format is not handled internally by odb4py.
   Instead, users can pass the extracted data to the encoder available in the `pyodc <https://pypi.org/project/pyodc>`_  package.

Write extracted data into an ODB2 file
--------------------------------------
The previous code is used here, except that the data have to be written into an ODB2 file.
In this example, the radial doppler wind are selected (``obstype==13`` and ``varno==195``).

.. code-block:: python

   #-*- coding : utf-8 -*-

   from datetime import datetime
   import pandas as pd
   
   # from core module 
   from odb4py.core import odb_open

   # Import pyodc 
   import codc  as odc 

   # Start
   start = datetime.now()

   # ODB path
   dbpath ="/path/to/CCMA"  # or ECMA.<obstype>

      # Get some needed attributes
   db        = OdbObject ( dbpath )
   db_attr   = db.get_attrib()
   db_date   = db_attr["observation_date"]  # Observation datatime

   # For NetCDF filename
   dt = db_date.split()[0]  # Date
   tm = db_date.split()[1]  # Time

   # ODB2 output filename 
   odb2file="radar_dow_"+dt+"_"+tm+".odb2"

   # Set up the sql query :  Get the radial Doppler wind  (varno == 195)
   sql_query="select  statid ,\
              degrees(lat)   ,\
              degrees(lon)   ,\
              varno          ,\
              date           ,\
              time           ,\
              fg_depar       ,\
              an_depar       ,\
              obsvalue       ,\
              FROM hdr,body WHERE obstype ==13 and varno ==195"


   #Parse the query
   p  =SqlParser()
   nf =p.get_nfunc    ( sql_query )
   sql=p.clean_string ( sql_query )

   # Create a connection
   conn = odb_open(dbpath )

   # Get the data as a DataFrame 
   df=pd.DataFrame( conn.odb_dict (database=dbpath,
                     sql_query = sql  ,
                     nfunc     = nf   ,
                     fmt_float = 5    ,
                     queryfile = None ,
                     poolmask  = None ,
                     pbar      = True   ) )

   # Write the data into the ODB2 file                      
   encode_odb(df, odb2file )

   # close  the ODB 
   conn.odb_close()

   # End
   end  = datetime.now()
   duration = end -  start
   print("Runtime duration:" , duration  )

Output 

.. code-block:: bash

   [##################################################] Complete 100%  (Total: 6573 rows)
   --odb4py : ODB database closed.
   Runtime duration: 0:00:01.325594


Check the output ODB2 file   
---------------------------

The output ODB2 file can be checked using the ECMWF  `odc <https://odc.readthedocs.io>`_ tool. 

.. code-block:: bash
   
   odc  count  radar_dow_20230101_000000.odb2
   6573

   odc  header   radar_dow_20230101_000000.odb2

   Header 1. Begin offset: 0, end offset: 281133, number of rows in block: 6573, byteOrder: same
   0. name: statid@hdr, type: STRING, codec: int8_string, width=8, #words=25
   1. name: degrees(lat), type: DOUBLE, codec: long_real, range=<55.520800,71.529550>, hasMissing=false
   2. name: degrees(lon), type: DOUBLE, codec: long_real, range=<3.353190,31.893190>, hasMissing=false
   3. name: varno@body, type: INTEGER, codec: constant, value=195.000000, hasMissing=false
   4. name: date@hdr, type: INTEGER, codec: int16, range=<20221231.000000,20230101.000000>, hasMissing=false
   5. name: time@hdr, type: INTEGER, codec: int32, range=<0.000000,235604.000000>, hasMissing=false
   6. name: fg_depar@body, type: DOUBLE, codec: long_real, range=<-5.985400,5.981810>, hasMissing=false
   7. name: an_depar@body, type: DOUBLE, codec: long_real, range=<-5.596840,5.089180>, hasMissing=true, missingValue=-2147483647.000000
   8. name: obsvalue@body, type: DOUBLE, codec: long_real, range=<-39.312660,38.109210>, hasMissing=false

   
   odc sql -i radar_dow_20230101_000000.odb2 'select statid, lat , lon , varno, date, time ,fg_depar ,an_depar  where  rownumber() < 10'

   statid@hdr	degrees(lat)	degrees(lon)	varno@body	date@hdr	time@hdr	fg_depar@body	an_depar@body	obsvalue@body
   '   nosta'	61.881110	6.848510	195	20221231	235500	2.198550	0.927680	-7.535400
   '   nosta'	62.285970	6.793210	195	20221231	235500	1.093870	1.147740	-9.042490
   '   nosta'	62.666360	7.150330	195	20221231	235500	0.979330	-0.735920	-10.549580
   '   nohgb'	57.422150	6.391940	195	20221231	235501	0.991520	-0.326860	10.549630
   '   nohgb'	58.095420	8.509010	195	20221231	235501	-1.753610	-0.442650	-26.625160
   ...


Convert ODB to Sqlite database
------------------------------

To perform such a conversion, the ``odb2sqlite`` has to be used. The data encoding is handled in the backend, ensuring compatibility with the ODB internal format.SQLite tables are automatically created by mapping the data types returned by the ODB query to the corresponding types supported by SQLite.

| The function returns **0** on success and **-1** on failure.

| Example 


.. code-block:: python 

   #-*- coding : utf-8 -*-

   from datetime import datetime

   # Convert module 
   from odb4py.convert import odb2sqlite 

   # Start
   start = datetime.now()

   # ODB path
   dbpath ="/path/to/CCMA"  # or ECMA.<obstype>

   # The sql query  
   query="select  statid ,\
           degrees(lat)   ,\
           degrees(lon)   ,\
           varno          ,\
           date           ,\
           time           ,\
           fg_depar       ,\
           an_depar       ,\
           obsvalue       ,\
           FROM hdr,body where obstype==13 AND varno ==195"

   # Same as the previous example 
   # ...
   
   
   # Call odb2sqlite method
   outfile="radar_dow.sqlite"
   status =odb2sqlite   (database=dbpath,       
                       sql_query = sql_query,
                       sqlite_db = outfile  ,  
                       nfunc  = nf    , 
                       pbar   = True  , 
                       verbose= True  )

   # End  
   end  = datetime.now()
   duration = end -  start
   print("Runtime duration:" , duration  )


.. code-block:: bash 

   Runtime duration: 0:00:01.988741


To inspect the SQLite database integrity and the extracted data ingestion, the `sqlite3 <https://sqlite.org/>`_ command-line utility has to be used. 
The following sqlite commands : 

 - Open the sqlite output database and see whether the datatypes have been correctly mapped from ODB1 to SQLite.
 - Fetch the first 10 lat/lon and first guess departures.
 - Show the table name and inspect the number of rows.


.. code-block:: bash

   sqlite3 radar_dow.sqlite

.. code-block:: bash 

   sqlite>.fullschema 
   CREATE TABLE obs (statid_hdr TEXT, degrees_lat REAL, degrees_lon REAL, varno_body INTEGER, date_hdr INTEGER, time_hdr INTEGER, fg_depar_body REAL, an_depar_body REAL, obsvalue_body REAL);
   /* No STAT tables available */
   sqlite>
   sqlite>.tables
   ODB 
   
   sqlite> SELECT degrees_lat_, degrees_lon_, obsvalue_body, fg_depar_body  from obs where varno_body ==  195  LIMIT  10 ;
   61.8811145199616|6.84850959712739|-7.53540399999999|2.19854533076505
   62.2859749604917|6.79321489645844|-9.04248999999999|1.09386670675939
   62.6663554590557|7.15033481685942|-10.549576|0.979330563068094
   57.4221472143021|6.39194178590787|10.549628|0.991517874364257
   58.0954189786416|8.50900969550826|-26.62516|-1.7536086583933
   64.4183633340104|11.7318030978241|4.655886|3.95438995017612
   64.749783887069|11.1360987998073|8.277147|-1.41526700691695
   64.91741913086|10.5627709642114|13.450377|-0.724219913199695
   65.0898347853508|10.9459813991416|17.588961|1.93129429946255
   65.2105945134471|10.319780717788|16.554315|-0.330503079630123

   sqlite> SELECT degrees_lat_, degrees_lon_, obsvalue_body, fg_depar_body, COUNT(*) as c from obs where varno_body ==  195 ;
   61.8811145199616|6.84850959712739|-7.53540399999999|2.19854533076505|6573


.. note:: 

   By default, the ``odb2sqlite`` method generates a table named ``ODB``. 
   To support more dynamic workflows, the table name can be given as an argument.
   This flexibility allows users to consolidate results from multiple *ODB* queries into separate SQLite tables within a single output file.

**Example** : Create multiple table SQLite tables according to ODB 'obstype' column. The same MetCOop CCMA is used ( date/time : 20240104 at 00h00 UTC ).

.. code-block:: python 

   #-*- coding : utf-8 -*-

   from datetime import datetime 

   # Import
   from odb4py.convert import  odb2sqlite

   # Path to ODB 
   dbpath="/path/to/CCMA"  # or ECMA.<obstype>

   # Start 
   start   = datetime.now()

   # Parser object 
   p =SqlParser()


   # The sql query
   query="select  statid ,\
           degrees(lat)   ,\
           degrees(lon)   ,\
           varno          ,\
           date           ,\
           time           ,\
           fg_depar       ,\
           an_depar       ,\
           obsvalue       ,\
           FROM hdr,body "

   # Declare the obstype to be extracted 
   obstype_list=[ "1","2", "5", "13"]

   # The output SQLite file 
   sqlite_output="ccma_obstypes.sqlite"

   for obst in  obstype_list :
       """
       THE PATH TO THE ODB IS THE SAME FOR EVERY OTERATION. 
       WE DON'T NEED TO UPDATE THE VARIABLES RELATIVE TO THE PATH 
       INSIDE THE LOOP :
       IOASSIGN 
       ODB_SRCPATH_CCMA 
       ODB_DATAPATH_CCMA  
       """   

       # SQLite table name 
       table_name  = "obstype_"+obst

       # Change the SQL query according to obstype value (Add WHERE condition)
       query_by_obstype = sql_query + " WHERE obstype ==" +obst.rstrip()

       # Parsing the query must be updated, since the query is changing for every iteration
       nf =p.get_nfunc     ( query_by_obstype )
       sql =p.clean_string ( query_by_obstype )

       # Convert
       re =odb2sqlite   ( database  =dbpath,
                         sql_query = sql  ,
                         nfunc     = nf   ,
                         sqlite_db = sqlite_output ,
                         table_name= table_name  ,
                         pbar      = True ,
                         verbose   = True  )
        if re  != 0 :
           print( "Failed to convert data. ODB :" , dbpath  )
           continue 


       # End                   
       end  = datetime.now()
       duration = end -  start
       print("Runtime duration:" , duration  )


.. code-block:: bash 

   ******** New ODB I/O opened with the following environment
   *******	  ODB_CONSIDER_TABLES=*
	   ODB_IO_KEEP_INCORE=1
	      ODB_IO_FILESIZE=32 MB
	       ODB_IO_BUFSIZE=4194304 bytes
	       ODB_IO_GRPSIZE=1 (or max no. of pools)
	       ODB_IO_PROFILE=0
	       ODB_IO_VERBOSE=0
	        ODB_IO_METHOD=5
   ODB_CONSIDER_TABLES=*
   ODB_WRITE_TABLES=*
   --odb4py : Executing query from string: 
     'select  statid ,degrees(lat),degrees(lon),varno ,date  , time,fg_depar, an_depar, obsvalue,FROM hdr,body WHERE obstype ==1'
   --odb4py : Number of requested columns : 9
     [##################################################] Complete 100%  (Total: 3898 rows)
   --odb4py : Rows have been successfully written into the SQLITE db : output.sqlite
   --odb4py : Total written data size  : 215338 Bytes

   --odb4py : Executing query from string:
     'select  statid ,degrees(lat),degrees(lon),varno ,date  , time,fg_depar, an_depar, obsvalue,FROM hdr,body WHERE obstype ==2'

   --odb4py : Number of requested columns : 9
     [##################################################] Complete 100%  (Total: 51 rows)
   --odb4py : Rows have been successfully written into the SQLITE db : output.sqlite
   --odb4py : Total written data size  : 3111 Bytes

   --odb4py : Executing query from string:
     'select  statid ,degrees(lat),degrees(lon),varno ,date  , time,fg_depar, an_depar, obsvalue,FROM hdr,body WHERE obstype ==5'

   --odb4py : Number of requested columns : 9
     [##################################################] Complete 100%  (Total: 15991 rows)
   --odb4py : Rows have been successfully written into the SQLITE db : output.sqlite
   --odb4py : Total written data size  : 962611 Bytes

   --odb4py : Executing query from string:
     'select  statid ,degrees(lat),degrees(lon),varno ,date  , time,fg_depar, an_depar, obsvalue,FROM hdr,body WHERE obstype ==13'

   --odb4py : Number of requested columns : 9
     [##################################################] Complete 100%  (Total: 34818 rows)
   --odb4py : Rows have been successfully written into the SQLITE db : output.sqlite
   --odb4py : Total written data size  : 2010698 Bytes

   Runtime duration: 0:00:05.994993

by inspecting the SQLite file,we can see that a tables is created for each obstype.

.. code-block:: bash  

   sqlite3   ccma_obstypes.sqlite
   SQLite version 3.26.0 2018-12-01 12:34:55
   Enter ".help" for usage hints.
   sqlite> .tables
   odb_obstype_1   odb_obstype_13  odb_obstype_2   odb_obstype_5
   sqlite>






Convert ODB to NetCDF format
----------------------------

In addition to SQLite, odb4py supports the conversion of ODB data to `NetCDF <https://docs.unidata.ucar.edu/netcdf-c/current/file_format_specifications.html>`_ format using the ``odb2nc`` function.
As with the ``odb2sqlite`` function, this conversion does not require an *ODBConnection* object. The function handles the entire process internally: it opens the ODB database, reads the requested data, writes the NetCDF output file, and closes the database automatically. 

| The function returns 0 on success and -1 on failure.

| Example 

.. code-block:: python

   #-*- coding : utf-8 -*-

   from datetime import datetime 

   # utils 
   from odb4py import SqlParser  

   # Import the method 
   from odb4py.convert import odb2nc


   # Start
   end  = datetime.now()

   # ODB path 
   dbpath ="/path/to/CCMA"  # or ECMA.<obstype>

   # Get some needed attributes 
   db        = OdbObject ( dbpath )
   db_attr   = db.get_attrib()
   db_date   = db_attr["observation_date"]  # Observation datatime 
   
   # For NetCDF filename  
   dt = db_date.split()[0]  # Date 
   tm = db_date.split()[1]  # Time 

   # Output filename 
   ncfile  = "radar_dow_"+dt+"_"+tm+".nc"

   # Set up the sql query :  Get the radial Doppler wind  (varno == 195)
   sql_query="select  statid ,\
              degrees(lat)   ,\
              degrees(lon)   ,\
              varno          ,\
              date           ,\
              time           ,\
              fg_depar       ,\
              an_depar       ,\
              obsvalue       ,\
              FROM hdr,body WHERE obstype ==13 and varno ==195"


   #Parse the query
   p  =SqlParser()
   nf =p.get_nfunc    ( sql_query )
   sql=p.clean_string ( sql_query )

   # Fetch the data and convert to NetCDF
   status =odb2nc   (database =dbpath ,      # (dtype -> str  )     ODB path 
                     sql_query=sql    ,      # (dtype -> str  )     The sql query 
                     nfunc    =nf     ,      # (dtype -> integer )  Number of functions found in the sql query 
                     ncfile   =ncfile ,      # (dtype -> str )      The output NetCDF file
                     lalon_deg= True  ,      # (dtype -> boolean)   Encode the corrdinates lat/lon in degrees or radians ( True -> degrees , False -> radians)
                     pbar     = True  ,      # (dtype -> boolean)   Enable the progress bar 
                     verbose  = True  )      # (dtype -> boolean)   verbosity on/off  

   # End           
   end  = datetime.now()
   duration = end -  start
   print("Runtime duration:" , duration  )



.. code-block:: bash

   Writing ODB data into NetCDF file ...
   List of written columns :
   Column   :  statid@hdr
   Column   :  degrees(lat@hdr)
   Column   :  degrees(lon@hdr)
   Column   :  varno@body
   Column   :  date@hdr
   Column   :  time@hdr
   Column   :  fg_depar@body
   Column   :  an_depar@body
   Column   :  obsvalue@body
   ODB data have been successfully written to NetCDF file : radar_dow_20230101_000000.nc
   Total written data size : 2506896 bytes



The **ncdump -h**  command show the structure, the data and metadat of encoded data.

.. code-block:: bash

   netcdf radar_dow_2024011000 {
   dimensions:
        nobs = 7825 ;
        strlen = 8 ;
   variables:
	char statid_hdr(nobs, strlen) ;
	double degrees_lat_(nobs) ;
		degrees_lat_:long_name = "degrees(lat@hdr)" ;
		degrees_lat_:_FillValue = NaN ;
	double degrees_lon_(nobs) ;
		degrees_lon_:long_name = "degrees(lon@hdr)" ;
		degrees_lon_:_FillValue = NaN ;
	double varno_body(nobs) ;
		varno_body:long_name = "varno@body" ;
		varno_body:_FillValue = NaN ;
	double date_hdr(nobs) ;
		date_hdr:long_name = "date@hdr" ;
		date_hdr:_FillValue = NaN ;
	double time_hdr(nobs) ;
		time_hdr:long_name = "time@hdr" ;
		time_hdr:_FillValue = NaN ;
	double fg_depar_body(nobs) ;
		fg_depar_body:long_name = "fg_depar@body" ;
		fg_depar_body:_FillValue = NaN ;
	double an_depar_body(nobs) ;
		an_depar_body:long_name = "an_depar@body" ;
		an_depar_body:_FillValue = NaN ;
	double obsvalue_body(nobs) ;
		obsvalue_body:long_name = "obsvalue@body" ;
		obsvalue_body:_FillValue = NaN ;
        // global attributes:
		:Conventions = "CF-1.10" ;
		:NetCDF_library_version = "4.3.3.1 of Dec 10 2015 16:44:18 $" ;
		:Title = "ODB data in NetCDF format" ;
		:History = "Created by: odb4py python package" ;
		:odb4py_version = "1.3.1" ;
		:Institution = "Royal Meteorological Institute of Belgium (RMI)" ;
		:Native_fomrat = "ECMWF ODB" ;
		:sql_query = "select statid , degrees(lat) , degrees(lon) , varno , date , time , fg_depar , an_depar , obsvalue , FROM hdr,body WHERE obstype ==13 and varno ==195" ;
		:featureType = "point" ;
		:NetCDF_datetime_creation = "2026-03-17 10:14:27 UTC" ;
		:ODB_analysis_datetime = "2024-01-05 00:00:00 UTC" ;
		:ODB_creation_datetime = "2024-11-14 08:44:28 UTC" ;
		:ODB_software_version = 46 ;
		:major_version = 0 ;
		:npools = 128 ;
		:ntables = 393 ;
          }



ODB data conversion 
===================

In addition to querying data directly from an ODB database, *odb4py* provides utilities to convert ODB datasets into other data formats.
These utilities are available in the ``odb4py.convert`` module.

Write data into an ODB2 file 
-----------------------------

Conversion to the *ODB2* format is not performed directly by odb4py.
Instead, the retrieved data can be passed to the ODB2 encoder provided by the **pyodc** package.

A typical workflow consists of:

- Querying data from an ODB database using ``odb_dict``.
- Converting the resulting dictionary into a pandas DataFrame.
- Encoding the DataFrame into an ODB2 file using **pyodc**.

Example
We use the same code as above, except that the data have to be written into an ODB2 file.

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



Convert ODB to NetCDF format
----------------------------
At present, a single conversion function is available: ``odb2nc``, which converts an ODB database into a NetCDF file.

Unlike the query functions, ``odb2nc`` operates independently of an existing database connection.
The function internally opens the ODB database, reads the requested data, writes the NetCDF output file, and closes the database automatically.
Example : 

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
   odb2nc   (database =dbpath ,      # (dtype -> str  )     ODB path 
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
   Total written data size = 2506896 bytes



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



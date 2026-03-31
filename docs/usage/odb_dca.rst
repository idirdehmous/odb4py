Creating Column Direct Access files (DCA)
=========================================
In order to retrieve rows and columns efficiently, ODB relies on
**DCA files (Direct Column Access)**.

DCA files contain structural metadata describing how the data are
physically stored on disk. This includes, for example:

- The starting and ending memory segments of a given table,
- Column sizes,
- Offsets between tables and columns,
- Internal layout information required for fast access.

DCA files can be created automatically when using ODB binaries such as
``odbdump.x`` or ``odbsql.x``. They can also be generated explicitly
using the ``dcagen`` script or the ``dcagen.x`` binary.

Within the **odb4py** package, a dedicated function ``odb_dca`` is provided to
generate DCA files programmatically before executing a query.

Creating DCA files is straightforward: the user must provide the path
to the database. The number of CPUs used for the task and the
verbosity level are optional parameters.

The generated DCA files are written into a subdirectory named
``dca`` inside the corresponding ``ECMA.<obtype>`` or ``CCMA`` directory.
The function returns **0** if the connection succeeds or **-1** if it fails.
Note that in the case of huge ODBs, creating the DCA files might take long time. For this reason, the function has been optimized to create the dca files for the tables found inside each pool (ONLY). In addition, the original options of the script ``dcagen`` can be introduced via a keyword argument called *extra_args* (please refer to the documentation of the ``dcagen`` script)


.. code-block:: python 

   #-*- coding: utf-8 -*-
   import os 
   
   # From  utils 
   from   odb4py.utils import OdbObject 

   # From  core module
   from   odb4py.core  import odb_dca 

   NCPU = 4
   # Path  
   db_type  = "CCMA"
   db_path  = "/path/to/odb/CCMA"  # or ECMA.<obstype>

   # Get the attributes and use the available tables in pool(s)
   db = OdbObject(dbpath)
   db_attr  =db.get_attrib()
   db_tables=db_attr["tables"]


   # Check if the files existe  (Maybe the ODB has been opened before)
   if not os.path.isdir (dca_path ):
      ic =odb_dca (database  = db_path, 
                   dbtype    = db_type, 
                   ncpu      = NCPU ,       # Number of CPUs for parallel run 
                   extra_args=" -u -z ",    # -u means : Update DCA files if found and delete empty files
                   tables    =  db_tables   # Use the one found from attributes 
                   ) 
      if ic < 0 :
         print("Failed to create DCA files")
         sys.exist()


If the verbose is set to *True* the following output is printed on stdout:         

.. code-block:: bash

   --odb4py : Creating DCA files...
   --odb4py : dcagen runs with the command : /usr/local/lib/python3.10/dist-packages/odb4py/bin/dcagen -i 'samples/rmi/ECMA.synop/' -N 4 -q -u -z -P -t body -t conv -t conv_body -t desc -t errstat -t hdr -t index -t modsurf -t poolmask -t timeslot_index
   --odb4py : The dca file will be created for the table :
   body.dca
   conv.dca
   conv_body.dca
   desc.dca
   errstat.dca
   hdr.dca
   index.dca
   modsurf.dca
   poolmask.dca
   radar.dca
   radar_body.dca
   radar_station.dca
   timeslot_index.dca
   update_1.dca
   update_2.dca
   update_3.dca
   update_4.dca
   --odb4py : DCA files creation done.

.. note::

   If the table names are not explicitly provided, the underlying
   ``dcagen`` utility iterates over all possible ODB tables found in the metadata file *ECMA.dd* or *CCMA.dd*. 
   By default in a standard ODB configuration, this corresponds to around **390 tables**.

   In the example above, only **17 tables** are actually present in the
   database. Therefore, not specifying the table list would cause
   ``dcagen`` to loop over all possible ODB tables, which may
   significantly increase the DCA generation time.

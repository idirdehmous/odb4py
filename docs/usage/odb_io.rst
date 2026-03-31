Open and close an ODB
=====================

Since the original ODB system is written in C, handling its data requires
initializing several internal structures such as paths, handlers and pointers.
These operations must be performed in the backend before accessing the data.

To simplify this process, the **odb4py.core** module provides two functions
to open and close an ODB database.

Open an ODB database
--------------------
To open an ODB database, the ``odb_open`` function from ``odb4py.core`` module has to be used.
This function is a wrapper around the ``ODBc_open`` C function in the original ODB code.

It opens the database and returns an *odb.ODBConnection* Python object that represents the active ODB session.
The function initializes the main ODB parameters, including: the database path, the location of the metadata files, the maximum number of ODB handlers etc.


.. code-block:: python

   #-*- coding: utf-8 -*-
   
   # Import function
   from odb4py.core import odb_open 

   # Path to a CCMA database
   dbtype = "CCMA"
   dbpath = "/".join(("/path/to/odb", dbtype))

   # Open the database
   conn  = odb_open  (database =dbpath )
   print( conn )

.. code-block:: bash

   <odb.ODBConnection object at 0x7f7ca23eed30>


Close an ODB database
---------------------
Once the database is no longer needed, it should be closed using the
``odb_close`` method. This function releases allocated handlers,
poolmask arrays (if used), and reinitializes the internal ODB structures.

The function takes no arguments. It automatically looks for all variables
that were initialized during the connection phase.

.. note::
  Most ODB operations are available as methods of the *ODBConnection* object returned by ``odb_open``.
  This means that once a database is opened, all ODB-related functions in ``core`` module can be accessed directly through the connection object.

.. code-block:: python

   #-*- coding: utf-8 -*-
   import sys

   # Import function
   from odb4py.core import odb_open 

   # Path to a CCMA database
   dbtype = "CCMA"
   dbpath = "/".join(("/path/to/odb", dbtype))

   # Open the database
   conn  = odb_open (database =dbpath )

   # Call the other methods and manipulate the data 
   ...
   ...

   # Close the database
   conn.odb_close()

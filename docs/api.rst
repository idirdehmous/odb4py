API documentation
=================

odb4py ``utils`` module
------------------------
Contains a pure python modules with the following classes:



.. py:class:: OdbEnv()

   Manage ODB runtime environment variables, binaries, and libraries.

   This class provides utilities to initialize the ODB environment,
   check required dependencies, and manipulate ODB-related environment variables.

.. note::
   The :class:`OdbEnv` class is automatically initialized using the method ``init()`` below upon importing
   ``odb4py`` or any of its modules. This ensures that the required ODB
   environment variables and paths are configured before any operation is performed.
   This behavior is implemented in the package's ``__init__.py`` file and
   does not require any user intervention.



Environment initialization
--------------------------


.. py:method:: init()

   Initialize the ODB runtime environment.

   This method:

   - Loads the ODB shared library if available.
   - Updates the system ``PATH`` to include ODB binaries.
   - Sets required ODB binary environment variables.

   :returns: *None*


.. py:method:: check_bin()

   Check the availability of required ODB binaries.

   :return: Type *Tuple* (status, message)

   Example of returned values:

   - ``(True, "Found ALL needed binaries")``
   - ``(False, "Binaries", [missing_bins])``


.. py:method:: check_lib()

   Check the availability of required ODB runtime libraries.

   :return: Type *Tuple* -- (status, message)

   Example of returned value:

   - ``(True, "Found ALL needed runtime libraries")``
   - ``(False, "Library not found", [missing_libs])``


Environment variable management
--------------------------------

.. py:method:: update_var(env_vars)

   Update ODB environment variables.
   
   :parameters:

   - **env_vars** : Type *dict*  -- dictionary of environment variables to set.

   :returns: *None*
    Only variables starting with ``ODB_`` are allowed.

   Example:

   .. code-block:: python

      env = OdbEnv()
      env.update_var({"ODB_IO_METHOD": "4"})


.. py:method:: get_var(var_name)

   Retrieve the value of an environment variable.

   :parameters:

   - **var_name**   : Type *str*  -- an ODB environment variable name.

   :returns:  Type *str* or *None* -- value of the variable.



.. py:method:: dump_vars()

   Returns all currently defined ``ODB_*`` environment variables.

   :return: Type *dict* -- dictionary of ODB environment variables.




The ODB object 
---------------

.. py:class:: OdbObject 

   Represents an ODB database and provides access to its structure and metadata.

   *Parameters* : 

   - **database** : Type *str*  -- path to the ODB database.

   The :class:`OdbObject` class provides methods to interact with an ODB database.
   After creating an instance of this class, all operations are performed
   through its methods.

.. py:function:: get_baseame()

   .. code-block:: python 

      # Instance 
      db = OdbObject(database= dbpath)
      db.get_basename()   

   :parameters: 

    - **None**

   :return: Type  *tuple*  (**dbtype**, **obstype**) in the case of ECMA and (**dbtype** , **None**) if a CCMA is used.



.. py:function:: get_size ()

   Parameters : 

    - **None** 

   :return: Type *int*  the total size of the ODB in **Bytes**

.. py:function:: has_flag(dbtype)

   :parameters: 

    - **dbtype**   : *str*  ODB type. 

   :return: Type *Boolean* -- **True** if the ODB has 'dbtype.flags' file **False** otherwise.



.. py:function:: has_ioassign(dbtype)

   :parameters:

    - **dbtype** :  *str*  ODB type.

   :return: Type *Boolean* -- **True** if the ODB has ``IOASSIGN`` file **False** otherwise.



.. py:function:: has_iomap(dbtype)

   :parameters:

    - **dbtype** : *str*  ODB type.

   :return: Type *Boolean* -- **True** if the ODB has 'dbtype.iomap' file **False** otherwise.


.. py:function:: get_pools()

   :parameters:

    - **None** 

   :return: Type *list* -- List of the pools found in the ODB.



.. py:function:: get_tables()

   :parameters: 

    - **None**

   :return: *list* -- List of the tables found inside each pool.


.. py:function:: get_attrib()

   :parameters: 

    - **None**

   :return: Type *dict* -- dictionary grouping all of the attributes found in the opened ODB.
   






.. py:class:: SqlParser 
   
   Contains some functions to parse the SQL query string.

.. py:function:: get_nfunc ( *str* )

    Counts the number of arithmetic,trigonometric ,conversion  or/and aggregation functions in an SQL string.
    The resulting count is intended to be subtracted before filling data
    buffers. Without this adjustment, the resulting value lists may include
    additional empty items represented as <NULL> C pointer.

   :parameters:

    - *arg* *str*  : The SQL query statement

   :return:    Type *int* -- number of functions.



.. py:function::  clean_string( *str* )

   :parameters:

    - *arg* *str*   : The SQL query statement. 

   :return:    Type *str* -- cleaned SQL query by filtering out tokens that are not part of the ODB SQL lexicon and unprintable characters.







odb4py ``core``  module (C API)
-------------------------------

The ``odb4py.core`` module is implemented in C as an extension module (``core.so``). It provides the main methods to perform high-performance access to ODB databases.

.. c:function:: PyObject *odb_open (PyObject *PyUNSED(self) ,PyObject *PyUNSED(args), PyObject *kwargs )

   :parameters:

   **database**  - Path to the ODB (ECMA.<obstype> or CCMA).

   :return:  PyTypeObject  *ODBConnection* or *None* if failed.
   


.. c:function:: PyObject *odb_close(PyObject *PyUNSED(self)  )

   :parameters: 

   **None**

   :return: Type *int* 0 if succeded or *None* if failed.

  
.. c:function:: PyObject *odb_array(PyObject *Py_UNUSED(self), PyObject *args, PyObject *kwargs)

   :parameters: 

    - **database**  : Path to the ODB  , type **str** (required).
    - **sql_query** : SQL string query , type **str** (required).
    - **header**    : If the colnames is returned , type **boolean** (optional).   Default: False 
    - **queryfile** : SQL statement from file , type **str** 'fpath' (optional).   Default: None
    - **poolmask**  : A mask for the pools in the ODB , type **PyObject** list (optional). Default: None
    - **pbar**      : Enable progress bar , type **boolean**  (optional). Default: False 
    - **fmt_float** : Number of the decimal digits for floats , type **int** (optional). Default: 15
    - **verbose**   : Enable more verbosity , type **boolean** (optional).  Default: False     

   :return:          Type *tuple*  -- (*None* , numpy.ndarray) if header = *False* or (*list* , numpy.ndarray) if header = *True*.



.. c:function:: PyObject *odb_dict(PyObject *Py_UNUSED(self), PyObject *args, PyObject *kwargs)

   :parameters: 

    - **database**  : Path to the ODB  , type **str** (required)  
    - **sql_query** : SQL string query , type **str** (required)
    - **nfunc**     : Number of functions in the SQL statement, type **int** (required).      
    - **queryfile** : SQL statement from file , type **str** 'fpath' (optional).   Default: None
    - **poolmask**  : A mask for the pools in the ODB , type **PyObject** list (optional). Default: None
    - **pbar**      : Enable progress bar , type **boolean**  (optional). Default: False 
    - **fmt_float** : Number of the decimal digits for floats , type **int** (optional). Default: 15
    - **verbose**   : Enable more verbosity , type **boolean** (optional).  Default: False

   :return:          Type *dict*  -- a python dictionary with column names as keys and row values as list.



.. c:function:: PyObject *odb_dca(PyObject *Py_UNUSED(self), PyObject *args, PyObject *kwargs)

   :parameters: 

    - **database**  : Path to the ODB  , type **str** (required).
    - **dbtype**    : Database type ECMA or CCMA , type **str** (required).
    - **tables**    : List of the tables found in the pools, type **str** (optional).  Default " ".
    - **ncpu**      : Number of used CPUs when the DCA files(direct column access ) are created, type **int** (optional). Default 4.
    - **extra_args**: Add additional arguments to the ``dcagen``. *See the documentation of dcagen script* , type **str**  (optional). Default "-u"
    - **verbose**   : Enable more verbosity , type **boolean** (optional).  Default (False)

   :return:          Type *int* -- 0 if succeeds or *None* if it fails.   


.. c:function:: PyObject *odb_geopoints(PyObject *Py_UNUSED(self), PyObject *args, PyObject *kwargs)

   :parameters: 

    - **database**  : Path to the ODB  , type **str** (required).
    - **condition** : Additional SQL **WHERE** clause, type **str** (optional). Default: None 
    - **unit**      : Returns coordinates lat/lon in *degrees* or *radians*, type **str** (optional). Default: "degrees".
    - **extent**    : Restricte the query to a given sub-domain delimited by extent=[lon1,lon2,lat1,lat2], type **PyObject** list (optional). Default: None
    - **poolmask**  : Get the data from defined mask,  type **str** (optional). Default: None (All pools)
    - **fmt_float** : Number of the decimal digits for floats , type **int** (optional). Default: 15
    - **pbar**      : Enable progress bar , type **boolean**  (optional). Default: False
    - **verbose**   : Enable more verbosity , type **boolean** (optional).  Default (False)     

   :return:          Type *dict* -- python dictionary with *lat,lon,vertco_reference_1,vertco_reference_2,date,time,obsvalue* as keys and lists of their repective values.



.. c:function:: PyObject *odb_gcdist (PyObject *Py_UNUSED(self), PyObject *args)

   Independent method from the PyTypeObject *ODBConnection*. Performs the computation of the great circle distances between given lat/lon pairs.
   Note that the distances are given in **meter** 

   :parameters:

    - **lon1**   : longitude of the first  array.  (required)
    - **lat1**   : latitude  of the first  array.  (required) 
    - **lon2**   : longitude of the second array. (required)
    - **lat2**   : latitude  of the second array.  (required)

    *lon1,lat1,lon2 and lat2*  are  **numpy.ndarray**.

   :return:       Type **numpy.ndarray**, a square matrix with a shape of len(lon1) x len(lat1) distances computed  between all the lat/lon pairs (distances are in **meter**).



odb4py ``convert`` module (C API)
---------------------------------

The ``odb4py.convert`` is also written in C. At present,  It provides only one function ``odb2nc`` to write the ODB rows in NetCDF format.

.. c:function:: PyObject *odb2nc (PyObject *Py_UNUSED(self), PyObject *args , PyObject *kwargs)

   Independent method from the PyTypeObject *ODBConnection*. 
   Convert the returned ODB rows into NetCDF format (in backend ) 

  :parameter:

   - **database**  : Path to the ODB  , type **str** (required)  
   - **sql_query** : SQL string query , type **str** (required)
   - **ncfile**    : Output NetCDF file name, type **str** 'fpath' (required).
  
  :return:          Type *int* : 0 if succeeds or -1 if it fails.    


.. c:function:: PyObject *odb2sqlite (PyObject *Py_UNUSED(self), PyObject *args , PyObject *kwargs)

   Independent method from the PyTypeObject *ODBConnection*.
   Convert the requested rows into sqlite database.

  :parameter:

   - **database**  : Path to the ODB  , type **str** (required)
   - **sql_query** : SQL string query , type **str** (required)
   - **sqlite_db** : Output sqlite database name, type **str** 'fpath' (required).

  :return:          Type *int* : 0 if succeeds or -1 if it fails.


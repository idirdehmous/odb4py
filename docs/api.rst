API documentation
=================

odb4py ``utils`` module
----------------------
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

   :returns: *Tuple* (status, message)

   Example of returned values:

   - ``(True, "Found ALL needed binaries")``
   - ``(False, "Binaries", [missing_bins])``


.. py:method:: check_lib()

   Check the availability of required ODB runtime libraries.

   :returns: *Tuple* -- (status, message)

   Example of returned value:

   - ``(True, "Found ALL needed runtime libraries")``
   - ``(False, "Library not found", [missing_libs])``


Environment variable management
--------------------------------

.. py:method:: update_var(env_vars)

   Update ODB environment variables.
   
   :parameters:

   **env_vars** : Type *dict*  -- dictionary of environment variables to set.

   :returns: *None*
   Only variables starting with ``ODB_`` are allowed.

   Example:

   .. code-block:: python

      env = OdbEnv()
      env.update_var({"ODB_IO_METHOD": "4"})


.. py:method:: get_var(key, default=None)

   Retrieve the value of an environment variable.

   :parameters:

   - *key*    : Type *str*  environment variable name.

   :returns:  Type *str* or *None* -- value of the variable.



.. py:method:: dump_vars()

   Return all currently defined ``ODB_*`` environment variables.

   :returns: Type *dict*  dictionary of ODB environment variables.




The ODB object 
---------------

.. py:class:: OdbObject 

   Represents an ODB database and provides access to its structure and metadata.

   *Parameters* : 
   **database** : Type *str*  path to the ODB database.

   The :class:`OdbObject` class provides methods to interact with an ODB database.
   After creating an instance of this class, all operations are performed
   through its methods.

.. py:function:: get_baseame ()

   Instance :  db = OdbObject(database= dbpath)
   ``db.get_basename()`` returns the basename of the ODB.
   associated with the object ``db``.

   *Parameters* : *None*
   :return: Type  *tuple*  (**dbtype**, **obstype**) in the case of ECMA and (**dbtype** , **None**) if a CCMA is used.



.. py:function:: get_size ()

   Returns the total size of the ODB in **Bytes**

   *Parameters* : *None* 

.. py:function:: has_flag(*str*  dbtype)

   Returns *Boolean* : **True** if the ODB has dbtype.flags file **False** otherwise.

   *Parameters* *str* :  **dbtype** 

.. py:function:: has_ioassign(*str* dbtype)

   Returns *Boolean* : **True** if the ODB has dbtype.flags file **False** otherwise.

   *Parameters* *str* :  **dbtype**

.. py:function:: has_iomap(*str* dbtype)

   Returns *Boolean* : **True** if the ODB has dbtype.iomap file **False** otherwise.

   *Parameters* *str* :  **dbtype**

.. py:function:: get_pools()

   Returns *list* : List of the pools found in the ODB.

   *Parameters* :  **None**

.. py:function:: get_tables()

   Returns *list* : List of the tables found inside each pool.

   *Parameters*  :  **None**


.. py:function:: get_attrib()

   Returns *dict* : Dictionary grouping all of the general attributes of the opened ODB.
   
   *Parameters* : **None**



.. py:class:: SqlParser 
   
   Contains some functions to parse the SQL query string.

.. py:function:: get_nfunc ( *str* sql_string )

   Returns the number of arithmetic, algebric, aggregation functions present in the SQL string. 
   The number of functions is subtracted before filling the data buffers.If this is not done, the lists of values will contain additionnal empty items with <NULL>. 

   *Parameter* : *str*  : The SQL query. 

.. py:function::  clean_string( *str* sql_string )

   Returns cleaned SQL query by filtering out tokens that are not part of the ODB SQL lexicon and unprintable characters.

   *Parameters* : *str* : The SQL query





odb4py ``core``  module (C API)
-------------------------------

.. c:function:: PyObject *odb_open (PyObject *PyUNSED(self) ,PyObject *PyUNSED(args), PyObject *kwargs )

   :parameter kwargs: *database*  - Path to the ODB (ECMA.<obstype> or CCMA).
   :rtype:  PyTypeObject  *ODBConnection or None if failed.
   


.. c:function:: PyObject *odb_close(PyObject *PyUNSED(self) ,PyObject *PyUNSED(args), PyObject *Py_UNUSED(kwargs )  )

   :parameter: - No argument is given.
   :rtype:  0 if succeded or None if failed.*

  
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

   :return:        : Type *tuple* : (None , numpy.ndarray) if header = *False* or (list , numpy.ndarray) if header = *True*.

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

   :return:        : Type *dict* : a python dictionary with column names as keys and row values as list.



.. c:function:: PyObject *odb_dca(PyObject *Py_UNUSED(self), PyObject *args, PyObject *kwargs)

   :parameters: 
   - **database**  : Path to the ODB  , type **str** (required).
   - **dbtype**    : Database type ECMA or CCMA , type **str** (required).
   - **tables**    : List of the tables found in the pools, type **str** (optional).  Default " ".
   - **ncpu**      : Number of used CPUs when the DCA files(direct column access ) are created, type **int** (optional). Default 4.
   - **extra_args**: Add additional arguments to the ``dcagen``. *See the documentation of dcagen script* , type **str**  (optional). Default "-u"
   - **verbose**   : Enable more verbosity , type **boolean** (optional).  Default (False)

   :return:        : Type *int* : 0 if succeeds or None if it fails.   


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

   :return:        : Type *dict* : python dictionary with *lat,lon,vertco_reference_1,vertco_reference_2,date,time,obsvalue* as keys and lists of their repective values.



.. c:function:: PyObject *odb_gcdist (PyObject *Py_UNUSED(self), PyObject *args)

   :parameters:
   - **lon1**   : longitude of the first array.  (required)
   - **lat1**   : latitude  of the first array.  (required) 
   - **lon2**   : longitude of the second array. (required)
   - **lat2**   : latitude of the second array.  (required)
   *lon1,lat1,lon2 and lat2* have type : **class 'numpy.ndarray'**

   :return:     : Type **numpy.ndarray**, a square matrix with a shape of len(lon1) x len(lat1) distances computed  between all the lat/lon pairs (distances are in **meter**).



odb4py ``convert`` module (C API)
---------------------------------
   
.. c:function:: PyObject *odb2nc (PyObject *Py_UNUSED(self), PyObject *args , PyObject *kwargs)

  :parameter:
   - **database**  : Path to the ODB  , type **str** (required)  
   - **sql_query** : SQL string query , type **str** (required)
   - **ncfile**    : Output NetCDF file name, type **str** 'fpath' (required).
  
  :return:         : Type *int* : 0 if succeeds or None if it fails.    

Inspect the ODB metadata
========================

odb4py provides three main modules:

  - **odb4py.core**   : Low-level functions interfacing Python with the ODB runtime libraries.
  - **odb4py.convert**: Reads ODB data and converts rows into other formats such as ``Sqlite``.
  - **odb4py.utils**  : Pure Python helper utilities to initialize the ODB environment, sanitize SQL queries, and manage auxiliary tasks.

To retrieve the general attributes of an ODB database, only the directory and metadata files are parsed. These attributes can be fetched individually or returned as a dictionary.
In a Python script, a database object must first be instantiated using ``OdbObject`` (from the ``odb_ob.py`` module). The following methods can then be used to retrieve metadata attributes:

- ``get_basename``
- ``get_size``
- ``get_pools``
- ``get_attrib``

Example: An ODB database from RMI,  **ECMA.synop**

.. code-block:: python

   from odb4py import OdbObject

   # Create a 'db' object 
   db = OdbObject("/path/to/ECMA.synop")
   
   for k, v in db_attr.items():
       print( f"{k:<30}: {v}" )



.. code-block:: bash

   name                           :  ECMA.synop
   type                           :  ECMA
   obstype                        :  synop
   date_creation                  :  20240625 181148
   last_modification              :  20240625 181155
   observation_date               :  20211101 030000
   number_pools                   :  4
   odb_total_size                 :  792209 (Bytes)
   Poolmask                       :  ['1', '2', '3', '4']
   tables                         :  ['body', 'conv', 'conv_body', 'desc', 'errstat', 'hdr', 'index', 'modsurf', 'poolmask', 'timeslot_index', 'update_1', 'update_2', 'update_3']
   ioassign_file                  :  samples/rmi/ECMA.synop/IOASSIGN
   flags_file                     :  samples/rmi/ECMA.synop/ECMA.flags
   iomap_file                     :  None
   number_of_considered_tables    :  386
   odb_software_release           :  43 0.000 1

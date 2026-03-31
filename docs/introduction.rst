Introduction 
============

**odb4py** stands for *Observation DataBase for python*. It is a python package implemented in pure C for performance and
reliability. The routines handling the ODB1 format have been derived and
pruned from the ECMWF ODB_API bundle `version 0.18.1 <https://www.ecmwf.int/sites/default/files/elibrary/2013/13861-using-odb-ecmwf.pdf>`_.
necessary components required to build the runtime libraries used by the package.

The package is distributed as manylinux wheels and embeds the required
ODB runtime libraries and binaries, allowing installation via pip without requiring
an external ECMWF ODB installation.

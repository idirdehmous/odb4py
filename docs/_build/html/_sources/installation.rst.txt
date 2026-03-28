Installation 
------------

The odb4py package can be installed from PyPi using the pip command:

.. code-block:: bash

   pip install odb4py

**Requirements**

- 3.9   <=  python  <= 3.13 
- numpy >= 2.x.x
- pandas>= 2.x.x 

**Optional**

- If the conversion to ODB2 format is needed.
- pyodc >= 1.6.0

.. note::
   The `odb` directory included within this package is not identical to the
   official ODB distribution provided in the ECMWF ODB bundle (version 0.18.1).
   For some consistency and optimality reasons with `C/Python API <https://docs.python.org/3/c-api/index.html>`_ the original C routines and/or headers have been modified or removed. 

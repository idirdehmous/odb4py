#define PY_SSIZE_T_CLEAN
#define NPY_NO_DEPRECATED_API NPY_2_0_API_VERSION
#include "pyspam.h"
#include "info_module.h"
#include "meta_module.h"
#include "io_module.h"
#include "dca_module.h"
#include "dict_module.h"
#include "geo_module.h"
#include "dbarray_module.h"
#include "nc_module.h" 


// Define the functions in the modules 
static PyMethodDef module_methods[] = {
    {"odb_connect",  (PyCFunction)(void(*)(void))    odb_connect_method    , METH_VARARGS | METH_KEYWORDS,  "Create ODB connection object"},

    {"odb_close"  ,  (PyCFunction)(void(*)(void))    odb_close_method      , METH_VARARGS | METH_KEYWORDS,  "Close an opened ODB "},

    {"odb_info_tables" ,  (PyCFunction)(void(*)(void))    odb_info_tables_method     , METH_VARARGS | METH_KEYWORDS,  "Show all existing ODB tables"},

    {"odb_info_varno"  ,  (PyCFunction)(void(*)(void))    odb_info_varno_method      , METH_VARARGS | METH_KEYWORDS,  "Print all ODB 'varno' parameters and their description"},

    {"odb_info_funcs",  (PyCFunction)(void(*)(void))  odb_info_funcs_method  , METH_VARARGS | METH_KEYWORDS,  "Print all the possible functions that could be used in ODB sql statement"},

    {"odb_array"  , (PyCFunction)(void(*)(void))  odb_array_method      , METH_VARARGS | METH_KEYWORDS,  "Fetch ODB rows as a numpy array with optional header(colnames)"} ,

    {"odb_dict"   ,    (PyCFunction)(void(*)(void))  odb_dict_method    , METH_VARARGS | METH_KEYWORDS,  "Fetch ODB rows as a python  dictionnary"},
    {"odb_dca"    ,    (PyCFunction)(void(*)(void))  odb_dca_method      , METH_VARARGS | METH_KEYWORDS,  "Create DCA  (Direct Column  Access ) files "},

    {"odb_gcdist" , (PyCFunction)(void(*)(void))  odb_gcdist_method , METH_VARARGS | METH_KEYWORDS,  "Compute great circle distance between numpy lat/lon pairs"},

    {"odb_geopoints" , (PyCFunction)(void(*)(void))  odb_geopoints_method , METH_VARARGS | METH_KEYWORDS,  "Fetch lat/lon/obsvalue according to some additional custom Where statement"},
    {"odb2nc"  ,  (PyCFunction)(void(*)(void))     odb2nc_method   , METH_VARARGS | METH_KEYWORDS,   "Write data into netcdf file  "},

    {"version"   , odbMeta_version, METH_NOARGS, "Return the odb4py version."  },

    {"info"      , odbMeta_info,    METH_NOARGS, "Return build and Python info."},
};


// Define the module itself 
static struct PyModuleDef   odbmodule = {
    PyModuleDef_HEAD_INIT,
    "core",
    "C/Python interface to access the ODB1 IFS/ARPEGE databases\nThe original source code has been developed by Copyright (c) 1997-98, 2000 ECMWF. All Rights Reserved !",
    -1,
    module_methods ,
     .m_slots =NULL
};


// Called first during python call
PyMODINIT_FUNC PyInit_core (void) {

    if (_import_array() < 0) {
        return NULL;
    }

    PyObject*  m  ;
    PyObject* ModuleError ;
    m=PyModule_Create(&odbmodule);
    if ( m == NULL) {
        ModuleError = PyErr_NewException("Failed to create the module : odb4py core", NULL, NULL);
        Py_XINCREF(ModuleError) ;
        return NULL;
}

PyOdbEmptyResultError = PyErr_NewException("odb4py.EmptyResultError", NULL, NULL);
Py_INCREF(PyOdbEmptyResultError);
PyModule_AddObject(m , "EmptyResultError", PyOdbEmptyResultError);

  return m  ;
}


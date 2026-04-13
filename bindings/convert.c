#define PY_SSIZE_T_CLEAN
#define NPY_NO_DEPRECATED_API NPY_2_0_API_VERSION
#include <Python.h>

#include "nc_module.c"
#include "odc_module.c"
#include "sqlite_module.c"

static PyMethodDef  convert_methods[] = {
    {"odb_to_nc"      , (PyCFunction) (void(*)(void)) odb_to_nc_method      , METH_VARARGS | METH_KEYWORDS,  "" },
    {"odb_to_sqlite"  , (PyCFunction) (void(*)(void)) odb_to_sqlite_method  , METH_VARARGS | METH_KEYWORDS,  "" },
    {"odb_to_odb2"    , (PyCFunction) (void(*)(void)) odb_to_odb2_method    , METH_VARARGS | METH_KEYWORDS,  "" },
    {NULL, NULL, 0, NULL}  

};



// Define the module itself 
static struct PyModuleDef   odb_convert = {
    PyModuleDef_HEAD_INIT,
    "convert"         ,
     ""               , 
    -1                ,
    convert_methods   ,
    .m_slots =NULL
};


// Create the IO module 
PyMODINIT_FUNC PyInit_convert   (void) {

    PyObject*  m  ;
    PyObject* ModuleError ;
    m=PyModule_Create(&odb_convert);
    if ( m == NULL) {
        ModuleError = PyErr_NewException("Failed to create the module : odb4py.convert", NULL, NULL);
        Py_XINCREF(ModuleError) ;
        return NULL;
}
return m  ;
}


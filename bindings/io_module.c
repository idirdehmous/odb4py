#define PY_SSIZE_T_CLEAN
#include <Python.h>


#include "info.h"
#include "odb_types.h"
#include "io_module.h"

// odb_open wrapper of ODBc_open  
// A python Type ODBConnction is created and will hold the variable handle created by ODBc_open 
//
PyObject  *odb_open_method(PyObject  *Py_UNUSED(self), 
		           PyObject  *args, 
			   PyObject *kwargs)
{

    const  char *database  = NULL  ;


    static char *kwlist[] = {"database", 
	                      NULL};

    if (!PyArg_ParseTupleAndKeywords(args, 
			             kwargs, "s",
                                     kwlist, 
				     &database))  {	    
        return NULL;
     }    

    ODBConnection *conn =  PyObject_New(ODBConnection, &ODBConnectionType);
    conn->handle = ODBc_open(database, "r",  &conn->npools,  &conn->ntables,  NULL);
    if (conn->handle <= 0) {
        PyErr_SetString(PyExc_RuntimeError,
                        "--odb4py : Failed to open the ODB.");
        Py_DECREF(conn);
	return NULL ; 
    }
    // return connection object  !
    return (PyObject* )conn;
}




// odb_close  Wrapper of ODBc_close method 
// The odb handle object is created and owned by odb_open method 
// the latter is 'alive' util odb_close is called . 
// When called all the allocated arrays , vars and structures are released! 
//
PyObject *odb_close_method(ODBConnection *self, PyObject *Py_UNUSED(args))
{
    if (self->handle > 0)
        ODBc_close(self->handle);
    //printf("--odb4py : ODB database closed.\n");
    self->handle = 0;
    Py_RETURN_NONE;
}


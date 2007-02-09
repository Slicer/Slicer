#ifdef USE_PYTHON
#include "Python.h"
#include <numarray/libnumeric.h>
#include <numarray/numarray.h>
#include <numarray/libnumarray.h>
#include <numarray/nummacro.h>

#include "vtkSystemIncludes.h"
#include "vtkTclUtil.h"
#include "vtkImageData.h"

// Sould look like vtkImageDataToArray interp imagedata
static PyObject* SlicerPython_ToArray ( PyObject* self, PyObject* args )
{
  long addr;
  char *name;
  Tcl_Interp* interp;
  if ( !PyArg_ParseTuple ( args, "ls:_slicer_vtkImageDataToArray", &addr, &name ) ) {
    return NULL;
  }
  interp = (Tcl_Interp*) addr;
  
  // Lookup the image data
  vtkImageData  *id;
  int error = 0;
  id = (vtkImageData *)(vtkTclGetPointerFromObject( name, (char *) "vtkImageData", interp, error ));
  if (error)
    {
    // Raise an error
    return PyErr_Format ( PyExc_TypeError, "vtkImageDataToArray: Could not find vtkImageData" );
    }

  int dims[3];
  id->GetDimensions ( dims );
  NumarrayType t = tDefault;
  // Datatype
  switch ( id->GetScalarType() )
    {
    case VTK_CHAR          : t = tInt8; break;
    case VTK_SIGNED_CHAR   : t = tInt8; break;
    case VTK_UNSIGNED_CHAR : t = tUInt8; break;
    case VTK_SHORT         : t = tInt16; break;
    case VTK_UNSIGNED_SHORT: t = tUInt16; break;
    case VTK_INT           : t = tInt32; break;
    case VTK_UNSIGNED_INT  : t = tUInt32; break;
    case VTK_LONG          : t = tInt64; break;
    case VTK_UNSIGNED_LONG : t = tUInt64; break;
    case VTK_FLOAT         : t = tFloat32; break;
    case VTK_DOUBLE        : t = tFloat64; break;
    default:
      return PyErr_Format ( PyExc_TypeError, "vtkImageDataToArray: Could not find unknown datatatype" );
    }
    
  PyArrayObject* array = NA_NewArray ( (void*)id->GetScalarPointer(), t, 3, dims[2], dims[1], dims[0] );
  return NA_ReturnOutput ( Py_None, array );
}

  

static PyMethodDef moduleMethods[] =
{
  {"vtkImageDataToArray", SlicerPython_ToArray, METH_VARARGS, NULL},
  /* {"ArrayTovtkImageData", SlicerPython_ToArray, METH_VARARGS}, */
  {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC init_slicer(void) {
  import_libnumarray();
  Py_InitModule ( "_slicer", moduleMethods );
}

#endif

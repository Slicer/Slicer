#include "vtkSlicerConfigure.h" /* Slicer3_USE_* */

#ifdef Slicer3_USE_PYTHON
#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif

#ifdef Slicer3_USE_NUMPY
#include <libnumarray.h>
#include <arrayobject.h>
#endif

#include "vtkSystemIncludes.h"
#include "vtkTclUtil.h"
#include "vtkImageData.h"

#ifdef Slicer3_USE_NUMPY
// Should look like vtkImageDataToArray interp imagedata
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

  int dims[3], tempdim;
  id->GetDimensions ( dims );
  // Note: NumPy uses a z,y,x ordering, so swap the 1st and 3rd dimensions!
  tempdim = dims[0];
  dims[0] = dims[2];
  dims[2] = tempdim;
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
    
  // PyArrayObject* array = NA_NewArray ( (void*)id->GetScalarPointer(), t, 3, dims[2], dims[1], dims[0] );
  // PyArrayObject* array = NA_FromDimsTypeAndData ( 3, dims, t, (char*)id->GetScalarPointer() );
  // return NA_ReturnOutput ( Py_None, array );
  // return PyArray_FromDimsAndData ( 3, dims, t, (char*)id->GetScalarPointer() );
  npy_intp dim_ptrs[3];
  dim_ptrs[0] = static_cast<npy_intp> (dims[0]);
  dim_ptrs[1] = static_cast<npy_intp> (dims[1]);
  dim_ptrs[2] = static_cast<npy_intp> (dims[2]);
  PyObject* array = PyArray_SimpleNewFromData ( 3, dim_ptrs, t, (char*)id->GetScalarPointer() );
  return array;
}

  

static PyMethodDef moduleMethods[] =
{
  {"vtkImageDataToArray", SlicerPython_ToArray, METH_VARARGS, NULL},
  /* {"ArrayTovtkImageData", SlicerPython_ToArray, METH_VARARGS}, */
  {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC init_slicer(void) {
  // import_libnumarray();
  import_array();
  Py_InitModule ( "_slicer", moduleMethods );
}

#else

static PyMethodDef moduleMethods[] =
{
  /* {"ArrayTovtkImageData", SlicerPython_ToArray, METH_VARARGS}, */
  {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC init_slicer(void) {
  Py_InitModule ( "_slicer", moduleMethods );
}

#endif

#endif

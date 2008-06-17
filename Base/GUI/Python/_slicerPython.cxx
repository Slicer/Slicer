#include "vtkSlicerConfigure.h" /* Slicer3_USE_* */


#include "vtkSystemIncludes.h"
#include "vtkTclUtil.h"
#include "vtkImageData.h"
#include "vtkDataArray.h"

#ifdef Slicer3_USE_PYTHON
#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif

#ifdef Slicer3_USE_NUMPY
#include <arrayobject.h>
#endif


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
  NPY_TYPES t = NPY_DOUBLE;
  // Datatype
  switch ( id->GetScalarType() )
    {
    case VTK_CHAR          : t = NPY_INT8; break;
    case VTK_SIGNED_CHAR   : t = NPY_INT8; break;
    case VTK_UNSIGNED_CHAR : t = NPY_UINT8; break;
    case VTK_SHORT         : t = NPY_INT16; break;
    case VTK_UNSIGNED_SHORT: t = NPY_UINT16; break;
    case VTK_INT           : t = NPY_INT32; break;
    case VTK_UNSIGNED_INT  : t = NPY_UINT32; break;
    case VTK_LONG          : t = NPY_INT64; break;
    case VTK_UNSIGNED_LONG : t = NPY_UINT64; break;
    case VTK_FLOAT         : t = NPY_FLOAT32; break;
    case VTK_DOUBLE        : t = NPY_FLOAT64; break;
    default:
      return PyErr_Format ( PyExc_TypeError, "vtkImageDataToArray: Could not find unknown datatatype" );
    }
    
  npy_intp dim_ptrs[4];
  dim_ptrs[0] = static_cast<npy_intp> (dims[0]);
  dim_ptrs[1] = static_cast<npy_intp> (dims[1]);
  dim_ptrs[2] = static_cast<npy_intp> (dims[2]);
  dim_ptrs[3] = static_cast<npy_intp> (id->GetNumberOfScalarComponents());
  PyObject* array = NULL;
  if (dim_ptrs[3]==1)
    array = PyArray_SimpleNewFromData ( 3, dim_ptrs, t, (char*)id->GetScalarPointer() );
  else
    array = PyArray_SimpleNewFromData ( 4, dim_ptrs, t, (char*)id->GetScalarPointer() );

  return array;
}

// Should look like vtkDataArrayToArray interp imagedata
static PyObject* SlicerPythonVtkDataArray_ToArray ( PyObject* self, PyObject* args )
{
  long addr;
  char *name;
  Tcl_Interp* interp;
  if ( !PyArg_ParseTuple ( args, "ls:_slicer_vtkDataArrayToArray", &addr, &name ) ) {
    return NULL;
  }
  interp = (Tcl_Interp*) addr;
  
  // Lookup the image data
  vtkDataArray  *da;
  int error = 0;
  da = (vtkDataArray *)(vtkTclGetPointerFromObject( name, (char *) "vtkDataArray", interp, error ));
  if (error)
    {
    // Raise an error
    return PyErr_Format ( PyExc_TypeError, "vtkDataArrayToArray: Could not find vtkDataArray" );
    }

  int dims[2];
  // Note: NumPy uses a z,y,x ordering, so swap the 1st and 3rd dimensions!
  dims[0] = da->GetNumberOfTuples (  );
  dims[1] = da->GetNumberOfComponents( );

  NPY_TYPES t = NPY_DOUBLE;
  // Datatype
  switch ( da->GetDataType() )
    {
    case VTK_CHAR          : t = NPY_INT8; break;
    case VTK_SIGNED_CHAR   : t = NPY_INT8; break;
    case VTK_UNSIGNED_CHAR : t = NPY_UINT8; break;
    case VTK_SHORT         : t = NPY_INT16; break;
    case VTK_UNSIGNED_SHORT: t = NPY_UINT16; break;
    case VTK_INT           : t = NPY_INT32; break;
    case VTK_UNSIGNED_INT  : t = NPY_UINT32; break;
    case VTK_LONG          : t = NPY_INT64; break;
    case VTK_UNSIGNED_LONG : t = NPY_UINT64; break;
    case VTK_FLOAT         : t = NPY_FLOAT32; break;
    case VTK_DOUBLE        : t = NPY_FLOAT64; break;
    default:
      return PyErr_Format ( PyExc_TypeError, "vtkDataArrayToArray: Could not find unknown datatatype" );
    }
    
  npy_intp dim_ptrs[2];
  dim_ptrs[0] = static_cast<npy_intp> (dims[0]);
  dim_ptrs[1] = static_cast<npy_intp> (dims[1]);
  PyObject* array = NULL;
  array = PyArray_SimpleNewFromData ( 2, dim_ptrs, t, (char*)da->GetVoidPointer(0) );

  return array;
} 

static PyMethodDef moduleMethods[] =
{
  {"vtkImageDataToArray", SlicerPython_ToArray, METH_VARARGS, NULL},
  {"vtkDataArrayToArray", SlicerPythonVtkDataArray_ToArray, METH_VARARGS, NULL},
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

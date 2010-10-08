#include "vtkSlicerConfigure.h" /* Slicer_USE_* */


#include "vtkType.h"
#include "vtkSystemIncludes.h"
#include "vtkTclUtil.h"
#include "vtkImageData.h"
#include "vtkDataArray.h"
#include "vtkImageImport.h"
#include "vtkPointData.h"
#include "vtkMatrix4x4.h"

#ifdef Slicer_USE_PYTHON
// Python will define _LARGEFILE_SOURCE to 1, while kwsys simply defines it - undef it here
// to avoid getting a warning during the build
#ifdef _LARGEFILE_SOURCE
#undef _LARGEFILE_SOURCE
#endif 
#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif

#ifdef Slicer_USE_NUMPY
#include "arrayobject.h"
#endif


#ifdef Slicer_USE_NUMPY

static NPY_TYPES getNumpyDataTypeFromVTKDataType( int dataType, bool& success )
{
  NPY_TYPES t = NPY_FLOAT64;

  if ( dataType == VTK_ID_TYPE )
  {
    #if    VTK_SIZEOF_ID_TYPE==1
      dataType = VTK_TYPE_UINT8;
    #elif  VTK_SIZEOF_ID_TYPE==2
      dataType = VTK_TYPE_UINT16;
    #elif  VTK_SIZEOF_ID_TYPE==4
      dataType = VTK_TYPE_UINT32;
    #elif  VTK_SIZEOF_ID_TYPE==8
      dataType = VTK_TYPE_INT64; //In this code VTK_LONG gets mapped tp NPY_INT64
    #else
      dataType = VTK_INT;
    #endif
   }

  success = true;
   
  switch ( dataType )
    {
    case VTK_TYPE_INT8          : t = NPY_INT8; break;
    case VTK_TYPE_UINT8         : t = NPY_UINT8; break;
    case VTK_TYPE_INT16         : t = NPY_INT16; break;
    case VTK_TYPE_UINT16        : t = NPY_UINT16; break;
    case VTK_TYPE_INT32         : t = NPY_INT32; break;
    case VTK_TYPE_UINT32        : t = NPY_UINT32; break;
    case VTK_TYPE_INT64         : t = NPY_INT64; break;
    case VTK_TYPE_UINT64        : t = NPY_UINT64; break;
    case VTK_TYPE_FLOAT32       : t = NPY_FLOAT32; break;
    case VTK_TYPE_FLOAT64       : t = NPY_FLOAT64; break;
    default:
      success = false;
    }

    return t;
}


// Should look like vtkImageDataToArray interp imagedata
static PyObject* SlicerPython_vtkImageDataToArray ( PyObject* self, PyObject* args )
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

  bool success;
  NPY_TYPES t = getNumpyDataTypeFromVTKDataType( id->GetScalarType(), success );
  if (!success)
    {
    PyErr_Format ( PyExc_TypeError, "vtkImageDataToArray: Could not find unknown datatatype" );
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

static PyObject* SlicerPython_ArrayTovtkImageData ( PyObject* self, PyObject* args )
{
  long addr;
  char *name;
  int dimensionality;
  Tcl_Interp* interp;
  PyArrayObject* array = NULL;
  if ( !PyArg_ParseTuple ( args, "lOis:_slicer_ArrayTovtkImageData", &addr, &array, &dimensionality, &name ) ) {
    return NULL;
  }
  interp = (Tcl_Interp*) addr;

  if (!PyArray_Check(array)) {
    return PyErr_Format ( PyExc_TypeError, "ArrayTovtkImageData: array provided is not a ndarray" );
  }

  // Lookup the image data
  vtkImageData  *id;
  int error = 0;
  id = (vtkImageData *)(vtkTclGetPointerFromObject( name, (char *) "vtkImageData", interp, error ));
  if (error)
    {
    // Raise an error
    return PyErr_Format ( PyExc_TypeError, "ArrayTovtkImageData: Could not find vtkImageData" );
    }

  if (dimensionality != 2 && dimensionality != 3)
    {
    return PyErr_Format ( PyExc_ValueError, "ArrayTovtkImageData: dimensionality must be 2 or 3" );
    }

  int ndim = PyArray_NDIM(array);

  // Datatype
  int t = VTK_DOUBLE;
  unsigned long tsize = sizeof(double);
  switch ( PyArray_TYPE(array) )
    {
    case NPY_INT8          : t = VTK_SIGNED_CHAR;    tsize = sizeof(signed char);    break;
    case NPY_UINT8         : t = VTK_UNSIGNED_CHAR;  tsize = sizeof(unsigned char);  break;
    case NPY_INT16         : t = VTK_SHORT;          tsize = sizeof(short);          break;
    case NPY_UINT16        : t = VTK_UNSIGNED_SHORT; tsize = sizeof(unsigned short); break;
    case NPY_INT32         : t = VTK_INT;            tsize = sizeof(int);            break;
    case NPY_UINT32        : t = VTK_UNSIGNED_INT;   tsize = sizeof(unsigned int);   break;
    case NPY_INT64         : t = VTK_LONG;           tsize = sizeof(long);           break;
    case NPY_UINT64        : t = VTK_UNSIGNED_LONG;  tsize = sizeof(unsigned long);  break;
    case NPY_FLOAT32       : t = VTK_FLOAT;          tsize = sizeof(float);          break;
    case NPY_FLOAT64       : t = VTK_DOUBLE;         tsize = sizeof(double);         break;
    default:
      return PyErr_Format ( PyExc_TypeError, "ArrayTovtkImageData: Could not find unknown datatatype" );
    }

  npy_intp* dim_ptrs = PyArray_DIMS(array);

  int dims[3];
  dims[0] = dims[1] = dims[2] = 0;
  int numberOfComponents = 1;

  if (dimensionality == 2)
    {
    switch (ndim)
      {
      case 2:
        dims[0] = static_cast<int>(dim_ptrs[1]);
        dims[1] = static_cast<int>(dim_ptrs[0]);
        dims[2] = 1;
        break;
      case 3:
        dims[0] = static_cast<int>(dim_ptrs[1]);
        dims[1] = static_cast<int>(dim_ptrs[0]);
        dims[2] = 1;
        numberOfComponents = static_cast<int>(dim_ptrs[2]);
        break;
      default:
        return PyErr_Format ( PyExc_ValueError, "ArrayTovtkImageData: Numpy array with inconsistent number of dimensions" );
      }
    }
  else
    {
    switch (ndim)
      {
      case 3:
        dims[0] = static_cast<int>(dim_ptrs[2]);
        dims[1] = static_cast<int>(dim_ptrs[1]);
        dims[2] = static_cast<int>(dim_ptrs[0]);
        break;
      case 4:
        dims[0] = static_cast<int>(dim_ptrs[2]);
        dims[1] = static_cast<int>(dim_ptrs[1]);
        dims[2] = static_cast<int>(dim_ptrs[0]);
        numberOfComponents = static_cast<int>(dim_ptrs[3]);
        break;
      default:
        return PyErr_Format ( PyExc_ValueError, "ArrayTovtkImageData: Numpy array with inconsistent number of dimensions" );
      }
    }

  int size = dims[0] * dims[1] * dims[2] * numberOfComponents * static_cast<int>(tsize); 
  void* ptr = PyArray_DATA(array);

  vtkImageImport* import = vtkImageImport::New();
  import->SetDataScalarType(t);
  import->SetNumberOfScalarComponents(numberOfComponents); 
  import->SetWholeExtent(0,dims[0]-1,0,dims[1]-1,0,dims[2]-1);
  import->SetDataExtentToWholeExtent();
  import->SetDataOrigin(id->GetOrigin());
  import->SetDataSpacing(id->GetSpacing());
  import->CopyImportVoidPointer(ptr,size);
  import->Update();

  id->DeepCopy(import->GetOutput());

  import->Delete();

  Py_INCREF(Py_None);
  return Py_None;
}

// Should look like vtkDataArrayToArray interp imagedata
static PyObject* SlicerPython_vtkDataArrayToArray ( PyObject* self, PyObject* args )
{
  long addr;
  char *name;
  Tcl_Interp* interp;
  if ( !PyArg_ParseTuple ( args, "ls:_slicer_vtkDataArrayToArray", &addr, &name ) ) {
    return NULL;
  }
  interp = (Tcl_Interp*) addr;
  
  // Lookup the data array
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

  bool success;
  NPY_TYPES t = getNumpyDataTypeFromVTKDataType( da->GetDataType(), success );
  if (!success)
  {
    char errmsg[256];
    sprintf( errmsg,  "vtkDataArrayToArray: Could not find unknown datatatype ( %s : %d )", da->GetDataTypeAsString(), da->GetDataType() );
    PyErr_Format ( PyExc_TypeError, "%s", errmsg );
  }
    
  npy_intp dim_ptrs[2];
  dim_ptrs[0] = static_cast<npy_intp> (dims[0]);
  dim_ptrs[1] = static_cast<npy_intp> (dims[1]);
  PyObject* array = NULL;
  array = PyArray_SimpleNewFromData ( 2, dim_ptrs, t, (char*)da->GetVoidPointer(0) );

  return array;
} 

static PyObject* SlicerPython_ArrayTovtkDataArray ( PyObject* self, PyObject* args )
{
  long addr;
  char *name;
  Tcl_Interp* interp;
  PyArrayObject* array = NULL;
  if ( !PyArg_ParseTuple ( args, "lOs:_slicer_ArrayTovtkDataArray", &addr, &array, &name ) ) {
    return NULL;
  }
  interp = (Tcl_Interp*) addr;

  if (!PyArray_Check(array)) {
    return PyErr_Format ( PyExc_TypeError, "ArrayTovtkDataArray: array provided is not a ndarray" );
  }

  // Lookup the data array
  vtkDataArray  *da;
  int error = 0;
  da = (vtkDataArray *)(vtkTclGetPointerFromObject( name, (char *) "vtkDataArray", interp, error ));
  if (error)
    {
    // Raise an error
    return PyErr_Format ( PyExc_TypeError, "vtkDataArrayToArray: Could not find vtkDataArray" );
    }

  int ndim = PyArray_NDIM(array);

  if (ndim != 1 && ndim != 2)
    {      
    return PyErr_Format ( PyExc_ValueError, "ArrayTovtkDataArray: Numpy array must have number of dimensions equal to 1 or 2." );
    }

  // Datatype
  int t = VTK_DOUBLE;
  unsigned long tsize = sizeof(double);
  switch ( PyArray_TYPE(array) )
    {
    case NPY_INT8          : t = VTK_SIGNED_CHAR;    tsize = sizeof(signed char);    break;
    case NPY_UINT8         : t = VTK_UNSIGNED_CHAR;  tsize = sizeof(unsigned char);  break;
    case NPY_INT16         : t = VTK_SHORT;          tsize = sizeof(short);          break;
    case NPY_UINT16        : t = VTK_UNSIGNED_SHORT; tsize = sizeof(unsigned short); break;
    case NPY_INT32         : t = VTK_INT;            tsize = sizeof(int);            break;
    case NPY_UINT32        : t = VTK_UNSIGNED_INT;   tsize = sizeof(unsigned int);   break;
    case NPY_INT64         : t = VTK_LONG;           tsize = sizeof(long);           break;
    case NPY_UINT64        : t = VTK_UNSIGNED_LONG;  tsize = sizeof(unsigned long);  break;
    case NPY_FLOAT32       : t = VTK_FLOAT;          tsize = sizeof(float);          break;
    case NPY_FLOAT64       : t = VTK_DOUBLE;         tsize = sizeof(double);         break;
    default:
      return PyErr_Format ( PyExc_TypeError, "ArrayTovtkDataArray: Could not find unknown datatatype" );
    }

  npy_intp* dim_ptrs = PyArray_DIMS(array);
  
  int numberOfTuples = static_cast<int>(dim_ptrs[0]);
  int numberOfComponents = 1;

  if (ndim == 2)
    {
    numberOfComponents = static_cast<int>(dim_ptrs[1]);
    }

  int size = numberOfTuples * numberOfComponents * static_cast<int>(tsize); 
  void* ptr = PyArray_DATA(array);

  vtkImageImport* import = vtkImageImport::New();
  import->SetDataScalarType(t);
  import->SetNumberOfScalarComponents(numberOfComponents); 
  import->SetWholeExtent(0,numberOfTuples-1,0,0,0,0);
  import->SetDataExtentToWholeExtent();
  import->CopyImportVoidPointer(ptr,size);
  import->Update();

  da->DeepCopy(import->GetOutput()->GetPointData()->GetScalars());

  import->Delete();

  Py_INCREF(Py_None);
  return Py_None;
}

//#define STRING_CALLBACKS

class vtkSlicerPythonCommand : public vtkCommand
{
public:
  static vtkSlicerPythonCommand *New() 
  { 
    return new vtkSlicerPythonCommand; 
  }

  void Execute(vtkObject *caller, unsigned long event, void *callData)
  {
//std::cout<<"Execute!"<<std::endl;
//std::cout<<PyString_AsString(PyObject_Str(this->CallbackCodeObject))<<std::endl;
#ifndef STRING_CALLBACKS
    PyObject_CallFunction(this->CallbackCodeObject,NULL);
#else
    PyRun_SimpleString(this->CallbackCodeString);
#endif
  }

#ifndef STRING_CALLBACKS
  void SetCallbackCodeObject(PyObject* callbackCodeObject)
  {
    this->CallbackCodeObject = callbackCodeObject;
  }  

  PyObject* GetCallbackCodeObject()
  {
    return this->CallbackCodeObject;
  }
#else
  void SetCallbackCodeString(char* callbackCodeString)
  {
    this->CallbackCodeString = callbackCodeString;
  }  

  const char* GetCallbackCodeString()
  {
    return this->CallbackCodeString;
  }
#endif

protected:
  vtkSlicerPythonCommand() 
  {
    this->CallbackCodeObject = NULL;
  }
  ~vtkSlicerPythonCommand()
  {
    if (this->CallbackCodeObject && Py_IsInitialized())
      {
      Py_DECREF(this->CallbackCodeObject);
      }
    this->CallbackCodeObject = NULL;
  }

#ifndef STRING_CALLBACKS
  PyObject* CallbackCodeObject;
#else
  char* CallbackCodeString;
#endif
};

static PyObject* SlicerPython_vtkObjectAddObserver(PyObject* self, PyObject* args)
{
  long addr;
  char *objName;
  char *eventName;
#ifndef STRING_CALLBACKS
  PyObject* codeObject;
  if (!PyArg_ParseTuple(args,"lssO:_slicer_vtkObjectAddObserver",&addr,&objName,&eventName,&codeObject))
#else
  char *codeString;
  if (!PyArg_ParseTuple(args,"lsss:_slicer_vtkObjectAddObserver",&addr,&objName,&eventName,&codeString))
#endif
    {
    return NULL;
    }
  Tcl_Interp* interp;
  interp = (Tcl_Interp*) addr;

#ifndef STRING_CALLBACKS
 Py_INCREF(codeObject);
#endif

  vtkObject *obj;
  int error = 0;
  obj = (vtkObject*)(vtkTclGetPointerFromObject(objName,(char*)"vtkObject",interp,error));
  if (error)
    {
    // Raise an error
    return PyErr_Format(PyExc_TypeError,"Could not find vtkObject");
    }

  unsigned long eventTag = 0;
  vtkSlicerPythonCommand* command = vtkSlicerPythonCommand::New();
#ifndef STRING_CALLBACKS
  command->SetCallbackCodeObject(codeObject);
#else
  command->SetCallbackCodeString(codeString);
#endif
  eventTag = obj->AddObserver(vtkCommand::GetEventIdFromString(eventName),command);
  command->Delete();

  return PyInt_FromLong((long)eventTag);

//std::cout<<"Outside "<<PyString_AsString(PyObject_Str(codeObject))<<std::endl;
//  PyObject_CallFunction(codeObject,NULL);

//  Py_INCREF(Py_None);
//  return Py_None;
}

static PyObject* SlicerPython_vtkObjectAddObserverWithId(PyObject* self, PyObject* args)
{
  long addr;
  char *objName;
  unsigned long eventId;
#ifndef STRING_CALLBACKS
  PyObject* codeObject;
  if (!PyArg_ParseTuple(args,"lskO:_slicer_vtkObjectAddObserver",&addr,&objName,&eventId,&codeObject))
#else
  char *codeString;
  if (!PyArg_ParseTuple(args,"lsks:_slicer_vtkObjectAddObserver",&addr,&objName,&eventId,&codeString))
#endif
    {
    return NULL;
    }
  Tcl_Interp* interp;
  interp = (Tcl_Interp*) addr;

#ifndef STRING_CALLBACKS
 Py_INCREF(codeObject);
#endif

  vtkObject *obj;
  int error = 0;
  obj = (vtkObject*)(vtkTclGetPointerFromObject(objName,(char*)"vtkObject",interp,error));
  if (error)
    {
    // Raise an error
    return PyErr_Format(PyExc_TypeError,"Could not find vtkObject");
    }

  unsigned long eventTag = 0;
  vtkSlicerPythonCommand* command = vtkSlicerPythonCommand::New();
#ifndef STRING_CALLBACKS
  command->SetCallbackCodeObject(codeObject);
#else
  command->SetCallbackCodeString(codeString);
#endif
  eventTag = obj->AddObserver(eventId,command);
  command->Delete();

  return PyInt_FromLong((long)eventTag);

//std::cout<<"Outside "<<PyString_AsString(PyObject_Str(codeObject))<<std::endl;
//  PyObject_CallFunction(codeObject,NULL);

//  Py_INCREF(Py_None);
//  return Py_None;
}

static PyMethodDef moduleMethods[] =
{
  {"vtkImageDataToArray", SlicerPython_vtkImageDataToArray, METH_VARARGS, NULL},
  {"vtkDataArrayToArray", SlicerPython_vtkDataArrayToArray, METH_VARARGS, NULL},
  {"ArrayTovtkImageData", SlicerPython_ArrayTovtkImageData, METH_VARARGS, NULL},
  {"ArrayTovtkDataArray", SlicerPython_ArrayTovtkDataArray, METH_VARARGS, NULL},
  {"vtkObjectAddObserver", SlicerPython_vtkObjectAddObserver, METH_VARARGS, NULL},
  {"vtkObjectAddObserverWithId", SlicerPython_vtkObjectAddObserverWithId, METH_VARARGS, NULL},
//  {"vtkMatrix4x4ToArray", SlicerPythonVtkDataArray_ToArray, METH_VARARGS, NULL},
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

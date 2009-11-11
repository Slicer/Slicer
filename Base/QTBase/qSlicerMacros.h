#ifndef __qSlicerSetGet_h
#define __qSlicerSetGet_h

//
// Slicer constructor declaration.
// Instanciate Internal structure
//
#define qSlicerCxxInternalConstructorMacro(_CLASSTYPE)  \
  _CLASSTYPE::_CLASSTYPE()                              \
  {                                                     \
    this->Internal = new qInternal();                   \
  }
  
//
// Slicer object constructor declaration. 
// Also make sure qInternal is instanciated
//
#define qSlicerCxxInternalConstructor1Macro(_CLASSTYPE, _ARG1) \
  _CLASSTYPE::_CLASSTYPE(_ARG1 parent) :                       \
    Superclass(parent)                                         \
    {                                                          \
    this->Internal = new qInternal();                          \
    }

    
//
// Slicer destructor declaration.
// Delete Internal structure if required
//
#define qSlicerCxxDestructorMacro(_CLASSTYPE)        \
  _CLASSTYPE::~_CLASSTYPE()                          \
  {                                                  \
    if (this->Internal) { delete this->Internal; }   \
  }
  
//
// The SetGet macros are used to interface to Internal instance variable in a standard fashion.
//

#define qSlicerSetInternalCxxMacro(_CLASSTYPE, _TYPE, _NAME, _VARNAME)  \
  void _CLASSTYPE::_NAME(const _TYPE& var)                              \
  {                                                                     \
    Q_ASSERT(this->Internal);                                           \
    this->Internal->_VARNAME =  var;                                    \
  }
  

#define qSlicerGetInternalCxxMacro(_CLASSTYPE, _TYPE, _NAME, _VARNAME)  \
  _TYPE _CLASSTYPE::_NAME()                                             \
  {                                                                     \
    Q_ASSERT(this->Internal);                                           \
    return this->Internal->_VARNAME;                                    \
  }

#endif

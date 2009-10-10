#include "qSlicerKwUIWrapper.h" 

#include "qSlicerApplication.h"

#include <QDebug>

//-----------------------------------------------------------------------------
class qSlicerKwUIWrapper::qInternal
{
public:
  qInternal()
    {
    }
};

//-----------------------------------------------------------------------------
qSlicerKwUIWrapper::qSlicerKwUIWrapper(QWidget *parent) : Superclass(parent)
{
  this->Internal = new qInternal;
}

//-----------------------------------------------------------------------------
qSlicerKwUIWrapper::~qSlicerKwUIWrapper()
{
  delete this->Internal; 
}

//-----------------------------------------------------------------------------
void qSlicerKwUIWrapper::dumpObjectInfo()
{
  this->Superclass::dumpObjectInfo();
}

#include "qSlicerMainWindowCore.h" 

#include <QMainWindow>
#include <QPointer>
#include <QDebug>

//-----------------------------------------------------------------------------
struct qSlicerMainWindowCore::qInternal
{
  qInternal()
    {
    }
  QPointer<QMainWindow> ParentWidget;
};

//-----------------------------------------------------------------------------
qSlicerMainWindowCore::qSlicerMainWindowCore(QMainWindow* parent)
 :Superclass(parent)
{
  this->Internal = new qInternal;
  this->Internal->ParentWidget = parent; 
}

//-----------------------------------------------------------------------------
qSlicerMainWindowCore::~qSlicerMainWindowCore()
{
  delete this->Internal; 
}

//-----------------------------------------------------------------------------
QMainWindow* qSlicerMainWindowCore::widget()
{
  return this->Internal->ParentWidget; 
}

#include "qSlicerAbstractModule.h" 

#include "vtkMRMLScene.h"

#include <QWidget>
#include <QDebug>

//-----------------------------------------------------------------------------
class qSlicerAbstractModule::qInternal
{
public:
  qInternal()
    {
    this->ModuleEnabled = false; 
    this->MRMLScene = vtkMRMLScene::GetActiveScene(); 
    }
  ~qInternal()
    {
    }
  bool                    ModuleEnabled; 
  vtkMRMLScene *          MRMLScene; 
};

//-----------------------------------------------------------------------------
qSlicerAbstractModule::qSlicerAbstractModule(QWidget *parent)
 :Superclass(parent, Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint)
{
  this->Internal = new qInternal;
}

//-----------------------------------------------------------------------------
qSlicerAbstractModule::~qSlicerAbstractModule()
{
  delete this->Internal; 
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModule::dumpObjectInfo()
{
  this->Superclass::dumpObjectInfo(); 
}

//-----------------------------------------------------------------------------
QString qSlicerAbstractModule::moduleName()
{
  return this->metaObject()->className();
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModule::setTitle(const QString& title)
{
  this->Title = title;
}

//-----------------------------------------------------------------------------
QString qSlicerAbstractModule::title()const
{
  return this->Title;
}

//-----------------------------------------------------------------------------
bool qSlicerAbstractModule::moduleEnabled()
{
  return this->Internal->ModuleEnabled; 
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModule::setModuleEnabled(bool value)
{
  this->Internal->ModuleEnabled = value;
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModule::setMRMLScene(vtkMRMLScene * object)
{
  this->Internal->MRMLScene = object; 
}

//-----------------------------------------------------------------------------
vtkMRMLScene* qSlicerAbstractModule::getMRMLScene()
{
  return this->Internal->MRMLScene; 
}

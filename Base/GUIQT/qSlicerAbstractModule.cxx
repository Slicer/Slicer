#include "qSlicerAbstractModule.h" 

#ifdef Slicer3_USE_KWWidgets
#include "qSlicerApplication.h"
#include "vtkSlicerApplication.h"
#include <vtkKWTkUtilities.h>
#endif 

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

//-----------------------------------------------------------------------------
#ifdef Slicer3_USE_KWWidgets
void qSlicerAbstractModule::synchronizeGeometryWithKWModule()
{
  vtkSlicerApplication* slicerApplication = qApp->getSlicerApplication();
  vtkSlicerModuleGUI* module = slicerApplication->GetModuleGUIByName(this->moduleName().toAscii());
  if (!module)
    {
    return;
    }
  vtkKWWidget* widget = 
    module->GetUIPanel() ? module->GetUIPanel()->GetPagesParentWidget() : 0;
  int pos[2];
  int size[2];
  vtkKWTkUtilities::GetWidgetCoordinates(widget, &pos[0], &pos[1]);
  vtkKWTkUtilities::GetWidgetSize(widget, &size[0], &size[1]);
  this->resize(size[0], size[1]);
  this->move(pos[0], pos[1]);
}
#endif

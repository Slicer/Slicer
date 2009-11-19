#include "qSlicerModuleLogic.h"

// SlicerLogic includes
#include "vtkSlicerApplicationLogic.h"

// MRML includes
#include "vtkMRMLScene.h"

// QT includes
#include <QDebug>

// VTK includes
#include "vtkSmartPointer.h"

#include "qSlicerMacros.h"

//-----------------------------------------------------------------------------
struct qSlicerModuleLogic::qInternal
{
  vtkSmartPointer<vtkMRMLScene>              MRMLScene;
  vtkSmartPointer<vtkSlicerApplicationLogic> AppLogic;
};

//-----------------------------------------------------------------------------
qSlicerGetInternalCxxMacro(qSlicerModuleLogic, vtkMRMLScene*, mrmlScene, MRMLScene);
qSlicerSetInternalCxxMacro(qSlicerModuleLogic, vtkMRMLScene*, setMRMLScene, MRMLScene);
qSlicerGetInternalCxxMacro(qSlicerModuleLogic, vtkSlicerApplicationLogic*, appLogic, AppLogic);

//-----------------------------------------------------------------------------
qSlicerCxxInternalConstructor1Macro(qSlicerModuleLogic, QObject*);
//qSlicerCxxDestructorMacro(qSlicerModuleLogic);
//-----------------------------------------------------------------------------
qSlicerModuleLogic::~qSlicerModuleLogic()
{
  delete this->Internal;
}

//-----------------------------------------------------------------------------
void qSlicerModuleLogic::printAdditionalInfo()
{
  //this->Superclass::printAdditionalInfo();
}

//-----------------------------------------------------------------------------
void qSlicerModuleLogic::initialize(vtkSlicerApplicationLogic* appLogic)
{
  Q_ASSERT(appLogic);
  this->Internal->AppLogic = appLogic;

  this->setup();
}

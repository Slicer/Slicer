#include "qSlicerTransformsModuleLogic.h"

#include "vtkSlicerTransformLogic.h"

// QT includes
#include <QString>

//-----------------------------------------------------------------------------
qSlicerTransformsModuleLogic::qSlicerTransformsModuleLogic()
{
}

//-----------------------------------------------------------------------------
vtkMRMLTransformNode* qSlicerTransformsModuleLogic::loadTransform(const QString& fileName)
{
  vtkMRMLTransformNode* res = 0;
  Q_ASSERT(!fileName.isEmpty());
  Q_ASSERT(this->mrmlScene());
  if (fileName.isEmpty() || !this->mrmlScene())
    {
    return res;
    }
  vtkSlicerTransformLogic *logic = vtkSlicerTransformLogic::New();
  res = logic->AddTransform(fileName.toLatin1(), this->mrmlScene());
  logic->Delete();
  return res;
}

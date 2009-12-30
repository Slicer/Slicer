#ifndef __qSlicerTransformsModuleLogic_h
#define __qSlicerTransformsModuleLogic_h

// SlicerQT includes
#include "qSlicerModuleLogic.h"

#include "qSlicerBaseQTCoreModulesExport.h"

class vtkMRMLTransformNode;

class Q_SLICER_BASE_QTCOREMODULES_EXPORT qSlicerTransformsModuleLogic : public qSlicerModuleLogic
{
public:

  typedef qSlicerModuleLogic Superclass;
  qSlicerTransformsModuleLogic();

  // Description:
  // Create a new transform node from a file.
  vtkMRMLTransformNode* loadTransform(const QString& fileName);
  
  //bool saveTransform(vtkMRMLTransformNode*, const QString& fileName);
};

#endif

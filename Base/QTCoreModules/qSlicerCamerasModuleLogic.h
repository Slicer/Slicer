#ifndef __qSlicerCamerasModuleLogic_h
#define __qSlicerCamerasModuleLogic_h

// SlicerQT includes
#include "qSlicerModuleLogic.h"

#include "qSlicerBaseQTCoreModulesExport.h"

// MRML declarations
class vtkMRMLCameraNode;
class vtkMRMLViewNode;

class Q_SLICER_BASE_QTCOREMODULES_EXPORT qSlicerCamerasModuleLogic : public qSlicerModuleLogic
{
public:

  typedef qSlicerModuleLogic Superclass;
  qSlicerCamerasModuleLogic();

  // Description:
  // Associate the camera to a view. Call setActiveTag on the camera with the 
  // view ID. The ActiveTagModifiedEvent will be fired.
  void setCameraToView(vtkMRMLCameraNode* camera, vtkMRMLViewNode* view);
};

#endif

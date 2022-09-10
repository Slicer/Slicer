#ifndef __vtkSlicerAnnotationModuleLogic_h
#define __vtkSlicerAnnotationModuleLogic_h

// Slicer Logic includes
#include "vtkSlicerAnnotationsModuleLogicExport.h"
#include "vtkSlicerModuleLogic.h"

// MRML includes
class vtkMRMLAnnotationHierarchyNode;
class vtkMRMLAnnotationLineDisplayNode;
class vtkMRMLAnnotationNode;
class vtkMRMLAnnotationPointDisplayNode;
class vtkMRMLAnnotationTextDisplayNode;

// VTK includes
#include <vtkStdString.h>

// STD includes
#include <string>

/// \ingroup Slicer_QtModules_Annotation
class VTK_SLICER_ANNOTATIONS_MODULE_LOGIC_EXPORT vtkSlicerAnnotationModuleLogic
  :public vtkSlicerModuleLogic
{
public:
  enum Events{
    RefreshRequestEvent = vtkCommand::UserEvent,
    HierarchyNodeAddedEvent
  };
  static vtkSlicerAnnotationModuleLogic *New();
  vtkTypeMacro(vtkSlicerAnnotationModuleLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  void RegisterNodes() override;

  //
  // SnapShot functionality
  //
  /// Create a snapShot.
  void CreateSnapShot(const char* name, const char* description, int screenshotType, double scaleFactor, vtkImageData* screenshot);

  /// Modify an existing snapShot.
  void ModifySnapShot(vtkStdString id, const char* name, const char* description, int screenshotType, double scaleFactor, vtkImageData* screenshot);

  /// Return the name of an existing annotation snapShot.
  vtkStdString GetSnapShotName(const char* id);

  /// Return the description of an existing annotation snapShot.
  vtkStdString GetSnapShotDescription(const char* id);

  /// Return the screenshotType of an existing annotation snapShot.
  int GetSnapShotScreenshotType(const char* id);

  /// Return the scaleFactor of an existing annotation snapShot.
  double GetSnapShotScaleFactor(const char* id);

  /// Return the screenshot of an existing annotation snapShot.
  vtkImageData* GetSnapShotScreenshot(const char* id);

  /// Check if node id corresponds to a snapShot node
  bool IsSnapshotNode(const char* id);

protected:

  vtkSlicerAnnotationModuleLogic();
  ~vtkSlicerAnnotationModuleLogic() override;

private:
  vtkSlicerAnnotationModuleLogic(const vtkSlicerAnnotationModuleLogic&) = delete;
  void operator=(const vtkSlicerAnnotationModuleLogic&) = delete;
};

#endif

// .NAME vtkMRMLAnnotationSnapshotNode - MRML node to represent scene snapshot including description and screenshot
// .SECTION Description
// n/A
//

#ifndef __vtkMRMLAnnotationSnapshotNode_h
#define __vtkMRMLAnnotationSnapshotNode_h

#include "qSlicerAnnotationModuleExport.h"
#include "vtkMRMLAnnotationControlPointsNode.h" 
#include "vtkMRMLSceneSnapshotNode.h"


class vtkStringArray;
class vtkMRMLStorageNode;

class  Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkMRMLAnnotationSnapshotNode : public vtkMRMLSceneViewNode
{
public:
  static vtkMRMLAnnotationSnapshotNode *New();
  vtkTypeMacro(vtkMRMLAnnotationSnapshotNode,vtkMRMLSceneViewNode);

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();
  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "AnnotationSnapshot";};

  virtual const char* GetIcon() {return ":/Icons/ViewCamera.png";};

  void SetSnapshotDescription(vtkStdString newDescription) {this->m_Description = newDescription;}
  vtkStdString GetSnapshotDescription() {return this->m_Description;}

  void SetScreenshot(vtkImageData* screenshot) {this->m_ScreenShot = screenshot;}
  vtkImageData* GetScreenshot() {return this->m_ScreenShot;}

  void SetScreenshotType(int type) {this->m_ScreenShotType = type;}
  int GetScreenshotType() {return this->m_ScreenShotType;}

  enum
  {
    SnapshotNodeAddedEvent = 0,
    ValueModifiedEvent,
  };

protected:
  vtkMRMLAnnotationSnapshotNode();
  ~vtkMRMLAnnotationSnapshotNode();
  vtkMRMLAnnotationSnapshotNode(const vtkMRMLAnnotationSnapshotNode&);
  void operator=(const vtkMRMLAnnotationSnapshotNode&);

  /// The associated Description
  vtkStdString m_Description;

  /// The vtkImageData of the screenshot
  vtkImageData* m_ScreenShot;

  /// The type of the screenshot
  /// 0: 3D View
  /// 1: Red Slice View
  /// 2: Yellow Slice View
  /// 3: Green Slice View
  /// 4: Full layout
  int m_ScreenShotType;

};

#endif

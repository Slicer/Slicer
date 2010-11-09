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

class  Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkMRMLAnnotationSnapshotNode : public vtkMRMLAnnotationNode
{
public:
  static vtkMRMLAnnotationSnapshotNode *New();
  vtkTypeMacro(vtkMRMLAnnotationSnapshotNode,vtkMRMLAnnotationNode);

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();
  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "AnnotationSnapshot";};



  void GetSnapshotDescription(const char* newDescription) {this->SetText(0, newDescription, 1, 1);}
  vtkStdString GetSnapshotDescription() {return this->GetText(0);}

  void SetSceneSnapshot(vtkMRMLSceneSnapshotNode* snapshot) {this->m_SnapShot = snapshot;}
  vtkMRMLSceneSnapshotNode* GetSceneSnapshot() {return this->m_SnapShot;}

  void SetScreenshot(vtkImageData* screenshot) {this->m_ScreenShot = screenshot;}
  vtkImageData* GetScreenshot() {return this->m_ScreenShot;}

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

  vtkMRMLSceneSnapshotNode* m_SnapShot;

  vtkImageData* m_ScreenShot;

};

#endif

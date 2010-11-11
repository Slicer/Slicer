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

class  Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkMRMLAnnotationSnapshotNode : public vtkMRMLSceneSnapshotNode
{
public:
  static vtkMRMLAnnotationSnapshotNode *New();
  vtkTypeMacro(vtkMRMLAnnotationSnapshotNode,vtkMRMLSceneSnapshotNode);

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();
  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "AnnotationSnapshot";};



  void GetSnapshotDescription(const char* newDescription) {this->m_Description = newDescription;}
  const char* GetSnapshotDescription() {return this->m_Description;}

  //void SetSceneSnapshot(vtkMRMLSceneSnapshotNode* snapshot) {this->m_SnapShot = snapshot;}
  //vtkMRMLSceneSnapshotNode* GetSceneSnapshot() {return this->m_SnapShot;}

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

  const char* m_Description;

  vtkImageData* m_ScreenShot;

};

#endif

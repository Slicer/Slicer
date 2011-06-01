// .NAME vtkMRMLAnnotationSnapshotNode - MRML node to represent scene snapshot including description and screenshot
// .SECTION Description
// n/A
//

#ifndef __vtkMRMLAnnotationSnapshotNode_h
#define __vtkMRMLAnnotationSnapshotNode_h

#include "vtkSlicerAnnotationModuleMRMLExport.h"
#include "vtkMRMLAnnotationControlPointsNode.h" 
#include "vtkMRMLAnnotationNode.h"


class vtkStringArray;
class vtkMRMLStorageNode;

/// \ingroup Slicer_QtModules_Annotation
class  VTK_SLICER_ANNOTATION_MODULE_MRML_EXPORT vtkMRMLAnnotationSnapshotNode : public vtkMRMLAnnotationNode
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

  virtual const char* GetIcon() {return ":/Icons/ViewCamera.png";};

  void SetSnapshotDescription(vtkStdString newDescription) {this->Description = newDescription;}
  vtkStdString GetSnapshotDescription() {return this->Description;}

  void WriteXML(ostream& of, int nIndent);
  void ReadXMLAttributes(const char** atts);

  ///
  /// The attached screenshot
  vtkSetObjectMacro(ScreenShot, vtkImageData);
  vtkGetObjectMacro(ScreenShot, vtkImageData);

  ///
  /// The ScaleFactor of the Screenshot
  vtkGetMacro(ScaleFactor, double);
  vtkSetMacro(ScaleFactor, double);

  ///
  /// The screenshot type
  /// 0: 3D View
  /// 1: Red Slice View
  /// 2: Yellow Slice View
  /// 3: Green Slice View
  /// 4: Full layout
  // TODO use an enum for the types
  void SetScreenShotType(int type) {this->ScreenShotType = type;}
  int GetScreenShotType() {return this->ScreenShotType;}

  ///
  /// Create default storage node or NULL if does not have one
  virtual vtkMRMLStorageNode* CreateDefaultStorageNode();

  /// Utility transformable methods
  virtual bool CanApplyNonLinearTransforms() { return false; }
  virtual void ApplyTransform(vtkMatrix4x4* vtkNotUsed(transformMatrix)) {};
  virtual void ApplyTransform(vtkAbstractTransform* vtkNotUsed(transform)) {};

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
  vtkStdString Description;

  /// The vtkImageData of the screenshot
  vtkImageData* ScreenShot;

  /// The type of the screenshot
  /// 0: 3D View
  /// 1: Red Slice View
  /// 2: Yellow Slice View
  /// 3: Green Slice View
  /// 4: Full layout
  int ScreenShotType;

  double ScaleFactor;

};

#endif

// .NAME vtkMRMLAnnotationSnapshotNode - MRML node to represent scene snapshot including description and screenshot
// .SECTION Description
// n/A
//

#ifndef __vtkMRMLAnnotationSnapshotNode_h
#define __vtkMRMLAnnotationSnapshotNode_h

#include "vtkSlicerAnnotationsModuleMRMLExport.h"
#include "vtkMRMLAnnotationControlPointsNode.h"
#include "vtkMRMLAnnotationNode.h"

#include <vtkStdString.h>
class vtkImageData;
class vtkStringArray;
class vtkMRMLStorageNode;

/// \ingroup Slicer_QtModules_Annotation
class  VTK_SLICER_ANNOTATIONS_MODULE_MRML_EXPORT vtkMRMLAnnotationSnapshotNode : public vtkMRMLAnnotationNode
{
public:
  static vtkMRMLAnnotationSnapshotNode *New();
  vtkTypeMacro(vtkMRMLAnnotationSnapshotNode,vtkMRMLAnnotationNode);

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  vtkMRMLNode* CreateNodeInstance() override;
  // Description:
  // Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "AnnotationSnapshot";}

  const char* GetIcon() override {return ":/Icons/ViewCamera.png";}

  void SetSnapshotDescription(const vtkStdString& newDescription);
  vtkGetMacro(SnapshotDescription, vtkStdString)

  void WriteXML(ostream& of, int nIndent) override;
  void ReadXMLAttributes(const char** atts) override;

  /// The attached screenshot
  virtual void SetScreenShot(vtkImageData* );
  vtkGetObjectMacro(ScreenShot, vtkImageData);

  /// The ScaleFactor of the Screenshot
  vtkGetMacro(ScaleFactor, double);
  vtkSetMacro(ScaleFactor, double);

  /// The screenshot type
  /// 0: 3D View
  /// 1: Red Slice View
  /// 2: Yellow Slice View
  /// 3: Green Slice View
  /// 4: Full layout
  // TODO use an enum for the types
  void SetScreenShotType(int type);
  vtkGetMacro(ScreenShotType, int);

  /// Create default storage node or nullptr if does not have one
  vtkMRMLStorageNode* CreateDefaultStorageNode() override;

  enum
  {
    SnapshotNodeAddedEvent = 0,
    ValueModifiedEvent,
  };

protected:
  vtkMRMLAnnotationSnapshotNode();
  ~vtkMRMLAnnotationSnapshotNode() override;
  vtkMRMLAnnotationSnapshotNode(const vtkMRMLAnnotationSnapshotNode&);
  void operator=(const vtkMRMLAnnotationSnapshotNode&);

  /// The associated Description
  vtkStdString SnapshotDescription;

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

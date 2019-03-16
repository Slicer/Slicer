// .NAME vtkMRMLAnnotationStickyNode - MRML node to represent a fiber bundle from tractography in DTI data.
// .SECTION Description
// Annotation nodes contains control points, internally represented as vtkPolyData.
// A Annotation node contains many control points  and forms the smallest logical unit of tractography
// that MRML will manage/read/write. Each control point has accompanying data.
// Visualization parameters for these nodes are controlled by the vtkMRMLAnnotationTextDisplayNode class.
//

#ifndef __vtkMRMLAnnotationStickyNode_h
#define __vtkMRMLAnnotationStickyNode_h

#include "vtkSlicerAnnotationsModuleMRMLExport.h"
#include "vtkMRMLAnnotationNode.h"
#include "vtkMRMLAnnotationControlPointsNode.h"

#include <vtkStdString.h>
class vtkStringArray;
class vtkMRMLStorageNode;

/// \ingroup Slicer_QtModules_Annotation
class  VTK_SLICER_ANNOTATIONS_MODULE_MRML_EXPORT vtkMRMLAnnotationStickyNode : public vtkMRMLAnnotationControlPointsNode
{
public:
  static vtkMRMLAnnotationStickyNode *New();
  vtkTypeMacro(vtkMRMLAnnotationStickyNode, vtkMRMLAnnotationNode);

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  vtkMRMLNode* CreateNodeInstance() override;
  // Description:
  // Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "AnnotationSticky";}


  int  SetSticky(const char* text,int selectedFlag);

  // Selected and visible are currently always set to 1 and are controlled by selected and visible flag - we can change this later
  void SetStickyText(const char* newLabel) {this->SetText(0,newLabel,1,1);}
  vtkStdString GetStickyText() {return this->GetText(0);}

  int SetStickyCoordinates(double newCoord[3]) {return this->SetControlPoint(0,newCoord,1,1);}

  void Initialize(vtkMRMLScene* mrmlScene) override;


protected:
  vtkMRMLAnnotationStickyNode()  = default;
  ~vtkMRMLAnnotationStickyNode() override  = default;
  vtkMRMLAnnotationStickyNode(const vtkMRMLAnnotationStickyNode&);
  void operator=(const vtkMRMLAnnotationStickyNode&);

};

#endif

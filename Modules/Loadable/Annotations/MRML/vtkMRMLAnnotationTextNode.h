// .NAME vtkMRMLAnnotationTextNode - MRML node to represent a text.
// .SECTION Description
// Annotation nodes contains control points, internally represented as vtkPolyData.
// A Annotation node contains many control points  and forms the smallest logical unit of tractography
// that MRML will manage/read/write. Each control point has accompanying data.
// Visualization parameters for these nodes are controlled by the vtkMRMLAnnotationPointDisplayNode class.
//

#ifndef __vtkMRMLAnnotationTextNode_h
#define __vtkMRMLAnnotationTextNode_h

#include "vtkSlicerAnnotationsModuleMRMLExport.h"
#include "vtkMRMLAnnotationControlPointsNode.h"

#include <vtkStdString.h>
class vtkStringArray;

/// \ingroup Slicer_QtModules_Annotation
class  VTK_SLICER_ANNOTATIONS_MODULE_MRML_EXPORT vtkMRMLAnnotationTextNode : public vtkMRMLAnnotationControlPointsNode
{
public:
  static vtkMRMLAnnotationTextNode *New();
  vtkTypeMacro(vtkMRMLAnnotationTextNode,vtkMRMLAnnotationControlPointsNode);

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  vtkMRMLNode* CreateNodeInstance() override;
  // Description:
  // Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "AnnotationText";}

  const char* GetIcon() override {return ":/Icons/AnnotationText.png";}

  int SetTextCoordinates(double newCoord[3]) {return this->SetControlPoint(0,newCoord,1,1);}
  double* GetTextCoordinates() {return this->GetControlPointCoordinates(0);}

  // normalized viewport coordinates of the lower left of the text box for the caption widget
  int SetCaptionCoordinates(double x, double y);
  int SetCaptionCoordinates(double newCoord[2]);
  double* GetCaptionCoordinates() {return this->GetControlPointCoordinates(1);}

  void SetTextLabel(const char* newLabel) {this->SetText(0, newLabel, 1, 1);}
  vtkStdString GetTextLabel() {return this->GetText(0);}

  enum
  {
    TextNodeAddedEvent = 0,
    ValueModifiedEvent,
  };

protected:
  vtkMRMLAnnotationTextNode();
  ~vtkMRMLAnnotationTextNode() override  = default;
  vtkMRMLAnnotationTextNode(const vtkMRMLAnnotationTextNode&);
  void operator=(const vtkMRMLAnnotationTextNode&);

};

#endif

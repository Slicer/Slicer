// .NAME vtkMRMLAnnotationTextNode - MRML node to represent a text.
// .SECTION Description
// Annotation nodes contains control points, internally represented as vtkPolyData.
// A Annotation node contains many control points  and forms the smallest logical unit of tractography 
// that MRML will manage/read/write. Each control point has accompanying data.  
// Visualization parameters for these nodes are controlled by the vtkMRMLAnnotationPointDisplayNode class.
//

#ifndef __vtkMRMLAnnotationTextNode_h
#define __vtkMRMLAnnotationTextNode_h

#include "qSlicerAnnotationModuleExport.h"
#include "vtkMRMLAnnotationControlPointsNode.h" 


class vtkStringArray;
class vtkMRMLStorageNode;

class  Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkMRMLAnnotationTextNode : public vtkMRMLAnnotationControlPointsNode
{
public:
  static vtkMRMLAnnotationTextNode *New();
  vtkTypeMacro(vtkMRMLAnnotationTextNode,vtkMRMLAnnotationControlPointsNode);

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();
  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "AnnotationText";};

  virtual const char* GetIcon() {return ":/Icons/AnnotationText.png";};

  int SetTextCoordinates(double newCoord[3]) {return this->SetControlPoint(0,newCoord,1,1);}
  double* GetTextCoordinates() {return this->GetControlPointCoordinates(0);}

  // coordinates of the text box for the caption widget
  int SetCaptionCoordinates(double newCoord[3]) {return this->SetControlPoint(1,newCoord,1,1);}
  double* GetCaptionCoordinates() {return this->GetControlPointCoordinates(1);}

  void SetTextLabel(const char* newLabel) {this->SetText(0, newLabel, 1, 1);}
  vtkStdString GetTextLabel() {return this->GetText(0);}

  enum
  {
    TextNodeAddedEvent = 0,
    ValueModifiedEvent,
  };

protected:
  vtkMRMLAnnotationTextNode() { }
  ~vtkMRMLAnnotationTextNode() { }
  vtkMRMLAnnotationTextNode(const vtkMRMLAnnotationTextNode&);
  void operator=(const vtkMRMLAnnotationTextNode&);

};

#endif

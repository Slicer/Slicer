// .NAME vtkMRMLAnnotationFiducialNode - MRML node to represent a fiber bundle from tractography in DTI data.
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
#include "vtkMRMLFiducial.h" 


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

  //int SetTextCoordinates(double newCoord[3]) {return this->SetControlPoint(0,newCoord,1,1);}
  //double* GetTextCoordinates() {return this->GetControlPointCoordinates(0);}

  void SetTextNodeLabel(const char* newLabel) {this->SetText(0, newLabel, 1, 1);}
  vtkStdString GetTextNodeLabel() {return this->GetText(0);}

  void Initialize(vtkMRMLScene* mrmlScene);

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

  void SetTextFromID();

};

#endif

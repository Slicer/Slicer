// .NAME vtkMRMLAnnotationFiducialNode - MRML node to represent a fiber bundle from tractography in DTI data.
// .SECTION Description
// Annotation nodes contains control points, internally represented as vtkPolyData.
// A Annotation node contains many control points  and forms the smallest logical unit of tractography 
// that MRML will manage/read/write. Each control point has accompanying data.  
// Visualization parameters for these nodes are controlled by the vtkMRMLAnnotationPointDisplayNode class.
//

#ifndef __vtkMRMLAnnotationFiducialNode_h
#define __vtkMRMLAnnotationFiducialNode_h

#include "qSlicerAnnotationModuleExport.h"
#include "vtkMRMLAnnotationControlPointsNode.h" 

class vtkStringArray;
class vtkMRMLStorageNode;

class  Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkMRMLAnnotationFiducialNode : public vtkMRMLAnnotationControlPointsNode
{
public:
  static vtkMRMLAnnotationFiducialNode *New();
  vtkTypeMacro(vtkMRMLAnnotationFiducialNode,vtkMRMLAnnotationControlPointsNode);

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();
  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "AnnotationFiducials";};

  int  SetFiducial(const char* label, double newControl[3],int selectedFlag, int visibleFlag);

  // Selected and visible are currently always set to 1 and are controlled by selected and visible flag - we can change this later
  void SetFiducialLabel(const char* newLabel) {this->SetText(0,newLabel,1,1);}
  vtkStdString GetFiducialLabel() {return this->GetText(0);}

  void SetFiducialValue(const char* newValue) {this->SetText(1,newValue,1,1);}
  // return atoi(this->GetText(1).c_str());
  int GetFiducialValue() { 
    return 0;}


  int SetFiducialCoordinates(double newCoord[3]) {return this->SetControlPoint(0,newCoord,1,1);}
  double* GetFiducialCoordinates() {return this->GetControlPointCoordinates(0);}

  enum
  {
    FiducialNodeAddedEvent = 0,
    ValueModifiedEvent,
  };

protected:
  vtkMRMLAnnotationFiducialNode() { }; 
  ~vtkMRMLAnnotationFiducialNode() { };
  vtkMRMLAnnotationFiducialNode(const vtkMRMLAnnotationFiducialNode&);
  void operator=(const vtkMRMLAnnotationFiducialNode&);

  void SetTextFromID();

};

#endif

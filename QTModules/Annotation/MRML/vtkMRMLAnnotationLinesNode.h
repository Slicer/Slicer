// .NAME vtkMRMLAnnotationLinesNode - MRML node to represent a fiber bundle from tractography in DTI data.
// .SECTION Description
// Annotation nodes contains control points, internally represented as vtkPolyData.
// A Annotation node contains many control points  and forms the smallest logical unit of tractography 
// that MRML will manage/read/write. Each control point has accompanying data.  
// Visualization parameters for these nodes are controlled by the vtkMRMLAnnotationLineDisplayNode class.
//

#ifndef __vtkMRMLAnnotationLinesNode_h
#define __vtkMRMLAnnotationLinesNode_h

#include "qSlicerAnnotationModuleExport.h"
#include "vtkMRMLAnnotationControlPointsNode.h" 

class vtkStringArray;
class vtkMRMLStorageNode;
class vtkMRMLAnnotationLineDisplayNode;
 
class  Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkMRMLAnnotationLinesNode : public vtkMRMLAnnotationControlPointsNode
{
public:
  static vtkMRMLAnnotationLinesNode *New();
  vtkTypeMacro(vtkMRMLAnnotationLinesNode,vtkMRMLAnnotationControlPointsNode);
  // void PrintSelf(ostream& os, vtkIndent indent);
  // Description:
  // Just prints short summary 
  virtual void PrintAnnotationInfo(ostream& os, vtkIndent indent, int titleFlag = 1);

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();
  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "AnnotationLines";};

  // Description:
  // Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);
  
  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);


  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  void UpdateScene(vtkMRMLScene *scene);

  // Description:
  // alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );


  // Description:
  // get associated display node or NULL if not set
  vtkMRMLAnnotationLineDisplayNode* GetAnnotationLineDisplayNode();

  // Description:
  // Create default storage node or NULL if does not have one
  virtual vtkMRMLStorageNode* CreateDefaultStorageNode();  

  // Define line between control points 
  int  AddLine(int ctrlPtIdStart, int ctrlPtIdEnd,int selectedFlag, int visibleFlag);
  int SetLine(int id, int ctrlPtIdStart, int ctrlPtIdEnd, int selectedFlag, int visibleFlag);

  int SetControlPoint(int id, double newControl[3],int selectedFlag, int visibleFlag);


  void DeleteLine(int id);
  int GetEndPointsId(vtkIdType lineID, vtkIdType ctrlPtID[2]);
  int GetNumberOfLines();

  // Description:
  // add line display node if not already present
  void CreateAnnotationLineDisplayNode();

  //BTX
  enum 
  {
    LINE_SELECTED = vtkMRMLAnnotationControlPointsNode::NUM_CP_ATTRIBUTE_TYPES,
    LINE_VISIBLE,
    NUM_LINE_ATTRIBUTE_TYPES
  };

  const char *GetAttributeTypesEnumAsString(int val);

  // Description:
  // Initializes all variables associated with annotations
  virtual void ResetAnnotations();

  void Initialize(vtkMRMLScene* mrmlScene); 

protected:
  vtkMRMLAnnotationLinesNode();
  ~vtkMRMLAnnotationLinesNode();
  vtkMRMLAnnotationLinesNode(const vtkMRMLAnnotationLinesNode&);
  void operator=(const vtkMRMLAnnotationLinesNode&);

  // Description:
  // Create Poly data with substructures necessary for this class 
  void CreatePolyData();

  // Description:
  // Initializes control pointes as well as attributes
  void ResetLines(); 

  // Description:
  // Initializes all attributes 
  void ResetLinesAttributesAll(); 

  bool InitializeLinesFlag;
};

#endif

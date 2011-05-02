// .NAME vtkMRMLAnnotationAngleStorageNode - MRML node for representing a volume storage
// .SECTION Description
// vtkMRMLAnnotationAngleStorageNode nodes describe the annotation storage
// node that allows to read/write point data from/to file.

#ifndef __vtkMRMLAnnotationAngleStorageNode_h
#define __vtkMRMLAnnotationAngleStorageNode_h

#include "AnnotationMRMLExport.h"
#include "vtkMRMLAnnotationLinesStorageNode.h"

class vtkMRMLAnnotationAngleNode;

/// \ingroup Slicer_QtModules_Annotation
class  Q_SLICER_QTMODULES_ANNOTATION_MRML_EXPORT vtkMRMLAnnotationAngleStorageNode : public vtkMRMLAnnotationLinesStorageNode
{
  public:
  static vtkMRMLAnnotationAngleStorageNode *New();
  vtkTypeMacro(vtkMRMLAnnotationAngleStorageNode,vtkMRMLAnnotationLinesStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();


  // Description:
  // Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Read data and set it in the referenced node
  // NOTE: Subclasses should implement this method
  virtual int ReadData(vtkMRMLNode *refNode);

  // Description:
  // Write data from a  referenced node
  // NOTE: Subclasses should implement this method
  virtual int WriteData(vtkMRMLNode *refNode);


  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

 // Description:
  // Set dependencies between this node and the parent node
  // when parsing XML file
  virtual void ProcessParentNode(vtkMRMLNode *parentNode);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get node XML tag name (like Storage, Model)
  virtual const char* GetNodeTagName()  {return "AnnotationAngleStorage";};

protected:


  vtkMRMLAnnotationAngleStorageNode();
  ~vtkMRMLAnnotationAngleStorageNode();
  vtkMRMLAnnotationAngleStorageNode(const vtkMRMLAnnotationAngleStorageNode&);
  void operator=(const vtkMRMLAnnotationAngleStorageNode&);

  const char* GetAnnotationStorageType() { return "angle"; }

  int WriteAnnotationAngleProperties(fstream & of, vtkMRMLAnnotationAngleNode *refNode);
  void WriteAnnotationAngleData(fstream& of, vtkMRMLAnnotationAngleNode *refNode);

  int ReadAnnotation(vtkMRMLAnnotationAngleNode *refNode);
  int ReadAnnotationAngleData(vtkMRMLAnnotationAngleNode *refNode, char line[1024], int typeColumn, int line1IDColumn, int line2IDColumn, int selColumn,  int visColumn, int numColumns);
  int ReadAnnotationAngleProperties(vtkMRMLAnnotationAngleNode *refNode, char line[1024], int &typeColumn, int& line1IDColumn,    int& line2Column, int& selColumn, int& visColumn, int& numColumns);

  // Description:
  int WriteData(vtkMRMLNode *refNode, fstream & of);

};

#endif




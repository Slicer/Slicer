// .NAME vtkMRMLAnnotationLinesStorageNode - MRML node for representing a volume storage
// .SECTION Description
// vtkMRMLAnnotationLinesStorageNode nodes describe the annotation storage
// node that allows to read/write point data from/to file.

#ifndef __vtkMRMLAnnotationLinesStorageNode_h
#define __vtkMRMLAnnotationLinesStorageNode_h

#include "AnnotationMRMLExport.h"
#include "vtkMRMLAnnotationControlPointsStorageNode.h"

class vtkMRMLAnnotationLineDisplayNode;
class vtkMRMLAnnotationLinesNode;

/// \ingroup Slicer_QtModules_Annotation
class  Q_SLICER_QTMODULES_ANNOTATION_MRML_EXPORT vtkMRMLAnnotationLinesStorageNode : public vtkMRMLAnnotationControlPointsStorageNode
{
  public:
  static vtkMRMLAnnotationLinesStorageNode *New();
  vtkTypeMacro(vtkMRMLAnnotationLinesStorageNode,vtkMRMLAnnotationControlPointsStorageNode);
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
  virtual const char* GetNodeTagName()  {return "AnnotationLinesStorage";};

protected:


  vtkMRMLAnnotationLinesStorageNode();
  ~vtkMRMLAnnotationLinesStorageNode();
  vtkMRMLAnnotationLinesStorageNode(const vtkMRMLAnnotationLinesStorageNode&);
  void operator=(const vtkMRMLAnnotationLinesStorageNode&);

  const char* GetAnnotationStorageType() { return "line"; }

  void WriteAnnotationLineDisplayProperties(fstream & of, vtkMRMLAnnotationLineDisplayNode *refNode, std::string preposition);
  int WriteAnnotationLinesProperties(fstream & of, vtkMRMLAnnotationLinesNode *refNode);
  void WriteAnnotationLinesData(fstream& of, vtkMRMLAnnotationLinesNode *refNode);

  int ReadAnnotation(vtkMRMLAnnotationLinesNode *refNode);
  int ReadAnnotationLinesData(vtkMRMLAnnotationLinesNode *refNode, char line[1024], int typeColumn, int startIDColumn, int endIDColumn, int selColumn,  int visColumn, int numColumns);
  int ReadAnnotationLineDisplayProperties(vtkMRMLAnnotationLineDisplayNode *refNode, std::string lineString, std::string preposition);
  int ReadAnnotationLinesProperties(vtkMRMLAnnotationLinesNode *refNode, char line[1024], int &typeColumn, int& startIDColumn,    int& endIDColumn, int& selColumn, int& visColumn, int& numColumns);

  // Description:
  int WriteData(vtkMRMLNode *refNode, fstream & of);

};

#endif




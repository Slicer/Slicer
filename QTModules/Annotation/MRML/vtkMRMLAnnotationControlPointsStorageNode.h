// .NAME vtkMRMLAnnotationControlPointsStorageNode - MRML node for representing a volume storage
// .SECTION Description
// vtkMRMLAnnotationControlPointsStorageNode nodes describe the annotation storage
// node that allows to read/write point data from/to file.

#ifndef __vtkMRMLAnnotationControlPointsStorageNode_h
#define __vtkMRMLAnnotationControlPointsStorageNode_h

#include "vtkSlicerAnnotationModuleMRMLExport.h"
#include "vtkMRMLAnnotationStorageNode.h"

class vtkMRMLAnnotationPointDisplayNode;
class vtkMRMLAnnotationControlPointsNode;

/// \ingroup Slicer_QtModules_Annotation
class  VTK_SLICER_ANNOTATION_MODULE_MRML_EXPORT vtkMRMLAnnotationControlPointsStorageNode : public vtkMRMLAnnotationStorageNode
{
  public:
  static vtkMRMLAnnotationControlPointsStorageNode *New();
  vtkTypeMacro(vtkMRMLAnnotationControlPointsStorageNode,vtkMRMLAnnotationStorageNode);
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
  virtual const char* GetNodeTagName()  {return "AnnotationControlPointsStorage";};

  // Description:
  // Initialize all the supported write file types
  virtual void InitializeSupportedWriteFileTypes();

protected:


  vtkMRMLAnnotationControlPointsStorageNode();
  ~vtkMRMLAnnotationControlPointsStorageNode();
  vtkMRMLAnnotationControlPointsStorageNode(const vtkMRMLAnnotationControlPointsStorageNode&);
  void operator=(const vtkMRMLAnnotationControlPointsStorageNode&);

  const char* GetAnnotationStorageType() { return "point"; }

  void WriteAnnotationPointDisplayProperties(fstream & of, vtkMRMLAnnotationPointDisplayNode *refNode, std::string preposition);
  int WriteAnnotationControlPointsProperties(fstream & of, vtkMRMLAnnotationControlPointsNode *refNode);
  void WriteAnnotationControlPointsData(fstream& of, vtkMRMLAnnotationControlPointsNode *refNode);

  int ReadAnnotation(vtkMRMLAnnotationControlPointsNode *refNode);
  int ReadAnnotationControlPointsData(vtkMRMLAnnotationControlPointsNode *refNode, char line[1024], int typeColumn, int xColumn, int yColumn, int zColumn,  
                      int selColumn,  int visColumn, int numColumns);
  int ReadAnnotationPointDisplayProperties(vtkMRMLAnnotationPointDisplayNode *refNode, std::string lineString, std::string preposition);
  int ReadAnnotationControlPointsProperties(vtkMRMLAnnotationControlPointsNode *refNode, char line[1024], int &typeColumn, 
                        int& xColumn,    int& yColumn,     int& zColumn, int& selColumn, int& visColumn, int& numColumns);

  int WriteData(vtkMRMLNode *refNode, fstream &of);
};

#endif




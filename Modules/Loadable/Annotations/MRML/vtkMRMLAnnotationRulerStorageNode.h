// .NAME vtkMRMLAnnotationRulerStorageNode - MRML node for representing a volume storage
// .SECTION Description
// vtkMRMLAnnotationRulerStorageNode nodes describe the annotation storage
// node that allows to read/write point data from/to file.

#ifndef __vtkMRMLAnnotationRulerStorageNode_h
#define __vtkMRMLAnnotationRulerStorageNode_h

#include "vtkSlicerAnnotationsModuleMRMLExport.h"
#include "vtkMRMLAnnotationLinesStorageNode.h"

class vtkMRMLAnnotationRulerNode;

/// \ingroup Slicer_QtModules_Annotation
class  VTK_SLICER_ANNOTATIONS_MODULE_MRML_EXPORT vtkMRMLAnnotationRulerStorageNode
  : public vtkMRMLAnnotationLinesStorageNode
{
  /// Defined as a friend vtkMRMLAnnotationHierarchyStorageNode so that it can
  /// call the protected function WriteData(refNode, of)
  friend class vtkMRMLAnnotationHierarchyStorageNode;

public:
  static vtkMRMLAnnotationRulerStorageNode *New();
  vtkTypeMacro(vtkMRMLAnnotationRulerStorageNode,vtkMRMLAnnotationLinesStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();


  // Description:
  // Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

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
  virtual const char* GetNodeTagName()  {return "AnnotationRulerStorage";};

  /// Read a single ruler from an open list file, called by the hierarchy
  /// storage node
  int ReadOneRuler(fstream & fstr, vtkMRMLAnnotationRulerNode *refNode);

  /// Return true if the node can be read in
  bool CanReadInReferenceNode(vtkMRMLNode* refNode);

protected:
  vtkMRMLAnnotationRulerStorageNode();
  ~vtkMRMLAnnotationRulerStorageNode();
  vtkMRMLAnnotationRulerStorageNode(const vtkMRMLAnnotationRulerStorageNode&);
  void operator=(const vtkMRMLAnnotationRulerStorageNode&);

  const char* GetAnnotationStorageType() { return "ruler"; }

  int WriteAnnotationRulerProperties(fstream & of, vtkMRMLAnnotationRulerNode *refNode);
  void WriteAnnotationRulerData(fstream& of, vtkMRMLAnnotationRulerNode *refNode);

  int ReadAnnotation(vtkMRMLAnnotationRulerNode *refNode);
  int ReadAnnotationRulerData(vtkMRMLAnnotationRulerNode *refNode, char line[1024], int typeColumn, int line1IDColumn, int selColumn,  int visColumn, int numColumns);
  int ReadAnnotationRulerProperties(vtkMRMLAnnotationRulerNode *refNode, char line[1024], int &typeColumn, int& line1IDColumn, int& selColumn, int& visColumn, int& numColumns);

  // Read data and set it in the referenced node
  virtual int ReadDataInternal(vtkMRMLNode *refNode);

  virtual int WriteDataInternal(vtkMRMLNode *refNode, fstream & of);
};

#endif




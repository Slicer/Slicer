// .NAME vtkMRMLAnnotationHierarchyStorageNode - MRML node for representing a hierarchy storage
// .SECTION Description
// vtkMRMLAnnotationHierarchyStorageNode nodes describe the annotation
// hierarchy storage node that writes multiple annotations to one file

#ifndef __vtkMRMLAnnotationHierarchyStorageNode_h
#define __vtkMRMLAnnotationHierarchyStorageNode_h

#include "vtkMRMLAnnotationStorageNode.h"

class vtkMRMLAnnotationTextDisplayNode;
class vtkMRMLAnnotationDisplayNode;
class vtkMRMLAnnotationNode;

/// \ingroup Slicer_QtModules_Annotation
class  VTK_SLICER_ANNOTATIONS_MODULE_MRML_EXPORT vtkMRMLAnnotationHierarchyStorageNode
  : public vtkMRMLAnnotationStorageNode
{
public:
  static vtkMRMLAnnotationHierarchyStorageNode *New();
  vtkTypeMacro(vtkMRMLAnnotationHierarchyStorageNode,vtkMRMLAnnotationStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Get node XML tag name (like Storage, Model)
  virtual const char* GetNodeTagName()  {return "AnnotationHierarchyStorage";};

  /// Return true if the node can be read in
  virtual bool CanReadInReferenceNode(vtkMRMLNode* refNode);

protected:
  vtkMRMLAnnotationHierarchyStorageNode();
  ~vtkMRMLAnnotationHierarchyStorageNode();
  vtkMRMLAnnotationHierarchyStorageNode(const vtkMRMLAnnotationHierarchyStorageNode&);
  void operator=(const vtkMRMLAnnotationHierarchyStorageNode&);

  /// Read data and set it in the referenced node
  virtual int ReadDataInternal(vtkMRMLNode *refNode);

  // Write data from a  referenced node
  virtual int WriteDataInternal(vtkMRMLNode *refNode);
};

#endif




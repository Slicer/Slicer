// .NAME vtkMRMLAnnotationAngleStorageNode - MRML node for representing a volume storage
// .SECTION Description
// vtkMRMLAnnotationAngleStorageNode nodes describe the annotation storage
// node that allows to read/write point data from/to file.

#ifndef __vtkMRMLAnnotationAngleStorageNode_h
#define __vtkMRMLAnnotationAngleStorageNode_h

#include "vtkSlicerAnnotationsModuleMRMLExport.h"
#include "vtkMRMLAnnotationLinesStorageNode.h"

class vtkMRMLAnnotationAngleNode;

/// \ingroup Slicer_QtModules_Annotation
class  VTK_SLICER_ANNOTATIONS_MODULE_MRML_EXPORT vtkMRMLAnnotationAngleStorageNode
  : public vtkMRMLAnnotationLinesStorageNode
{
public:
  static vtkMRMLAnnotationAngleStorageNode *New();
  vtkTypeMacro(vtkMRMLAnnotationAngleStorageNode,vtkMRMLAnnotationLinesStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

  virtual vtkMRMLNode* CreateNodeInstance() VTK_OVERRIDE;

  // Description:
  // Get node XML tag name (like Storage, Model)
  virtual const char* GetNodeTagName() VTK_OVERRIDE {return "AnnotationAngleStorage";}

  /// Return true if the node can be read in
  virtual bool CanReadInReferenceNode(vtkMRMLNode *refNode) VTK_OVERRIDE;

protected:
  vtkMRMLAnnotationAngleStorageNode();
  ~vtkMRMLAnnotationAngleStorageNode();
  vtkMRMLAnnotationAngleStorageNode(const vtkMRMLAnnotationAngleStorageNode&);
  void operator=(const vtkMRMLAnnotationAngleStorageNode&);

  const char* GetAnnotationStorageType() { return "angle"; }

  /// Write angle node properties from the node to the given stream, return 1
  /// on success, 0 on failure
  int WriteAnnotationAngleProperties(fstream & of, vtkMRMLAnnotationAngleNode *refNode);
  int WriteAnnotationAngleData(fstream& of, vtkMRMLAnnotationAngleNode *refNode);

  int ReadAnnotation(vtkMRMLAnnotationAngleNode *refNode);
  int ReadAnnotationAngleData(vtkMRMLAnnotationAngleNode *refNode, char line[1024], int typeColumn, int line1IDColumn, int line2IDColumn, int selColumn,  int visColumn, int numColumns);
  int ReadAnnotationAngleProperties(vtkMRMLAnnotationAngleNode *refNode, char line[1024], int &typeColumn, int& line1IDColumn,    int& line2Column, int& selColumn, int& visColumn, int& numColumns);

  /// Read data and set it in the referenced node
  virtual int ReadDataInternal(vtkMRMLNode *refNode) VTK_OVERRIDE;

  /// Write data from a referenced node to the stream
  virtual int WriteAnnotationDataInternal(vtkMRMLNode *refNode, fstream & of) VTK_OVERRIDE;
};

#endif

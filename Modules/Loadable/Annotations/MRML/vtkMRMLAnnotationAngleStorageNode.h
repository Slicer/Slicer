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
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  // Description:
  // Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override {return "AnnotationAngleStorage";}

  /// Return true if the node can be read in
  bool CanReadInReferenceNode(vtkMRMLNode *refNode) override;

protected:
  vtkMRMLAnnotationAngleStorageNode();
  ~vtkMRMLAnnotationAngleStorageNode() override;
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
  int ReadDataInternal(vtkMRMLNode *refNode) override;

  /// Write data from a referenced node to the stream
  int WriteAnnotationDataInternal(vtkMRMLNode *refNode, fstream & of) override;
};

#endif

// .NAME vtkMRMLAnnotationStorageNode - MRML node for representing a volume storage
// .SECTION Description
// vtkMRMLAnnotationStorageNode nodes describe the annotation storage
// node that allows to read/write point data from/to file.

#ifndef __vtkMRMLAnnotationStorageNode_h
#define __vtkMRMLAnnotationStorageNode_h

#include "vtkSlicerAnnotationsModuleMRMLExport.h"
#include "vtkMRMLStorageNode.h"

class vtkMRMLAnnotationTextDisplayNode;
class vtkMRMLAnnotationDisplayNode;
class vtkMRMLAnnotationNode;

/// \ingroup Slicer_QtModules_Annotation
class  VTK_SLICER_ANNOTATIONS_MODULE_MRML_EXPORT vtkMRMLAnnotationStorageNode : public vtkMRMLStorageNode
{
public:
  static vtkMRMLAnnotationStorageNode *New();
  vtkTypeMacro(vtkMRMLAnnotationStorageNode,vtkMRMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  // Description:
  // Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override {return "AnnotationStorage";}

  /// Return true if the node can be read in
  bool CanReadInReferenceNode(vtkMRMLNode* refNode) override;

protected:
  vtkMRMLAnnotationStorageNode();
  ~vtkMRMLAnnotationStorageNode() override;
  vtkMRMLAnnotationStorageNode(const vtkMRMLAnnotationStorageNode&);
  void operator=(const vtkMRMLAnnotationStorageNode&);

  int WriteAnnotationDisplayProperties(fstream & of, vtkMRMLAnnotationDisplayNode *refNode, std::string preposition);
  int WriteAnnotationTextDisplayProperties(fstream & of, vtkMRMLAnnotationTextDisplayNode *refNode, std::string preposition);

  int WriteAnnotationTextProperties(fstream & of, vtkMRMLAnnotationNode *refNode);
  int WriteAnnotationData(fstream& of, vtkMRMLAnnotationNode *refNode);
  int OpenFileToWrite(fstream& of);

  // Description:
  // Read data related to vtkMRMLAnnotationDisplayNode
  int ReadAnnotationDisplayProperties(vtkMRMLAnnotationDisplayNode *annotationDisplayNode, std::string lineString, std::string preposition);
  int ReadAnnotationTextDisplayProperties(vtkMRMLAnnotationTextDisplayNode *annotationDisplayNode, std::string lineString, std::string preposition);

  int ReadAnnotationTextData(vtkMRMLAnnotationNode *refNode, char line[1024], int typeColumn, int textColumn,  int selColumn,
              int visColumn, int numColumns);
  int ReadAnnotationTextProperties(vtkMRMLAnnotationNode *annotationNode, char line[1024], int &typeColumn, int& annotationColumn, int& selColumn, int& visColumn, int& columnNumber);
  // Description:
  // assumes that ResetAnnotations is executed
  int ReadAnnotation(vtkMRMLAnnotationNode *refNode);

  int OpenFileToRead(fstream& of, vtkMRMLNode *refNode);
  const char* GetAnnotationStorageType() { return "text"; }

  /// Initialize all the supported read file types
  void InitializeSupportedReadFileTypes() override;

  /// Initialize all the supported write file types
  void InitializeSupportedWriteFileTypes() override;

  /// Read data and set it in the referenced node
  int ReadDataInternal(vtkMRMLNode *refNode) override;

  /// Write data from a referenced node
  int WriteDataInternal(vtkMRMLNode *refNode) override;
  /// Write data from a referenced node into a passed stream
  virtual int WriteAnnotationDataInternal(vtkMRMLNode *refNode, fstream &of);

};

#endif

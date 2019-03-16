// .NAME vtkMRMLAnnotationLinesStorageNode - MRML node for representing a volume storage
// .SECTION Description
// vtkMRMLAnnotationLinesStorageNode nodes describe the annotation storage
// node that allows to read/write point data from/to file.

#ifndef __vtkMRMLAnnotationLinesStorageNode_h
#define __vtkMRMLAnnotationLinesStorageNode_h

#include "vtkSlicerAnnotationsModuleMRMLExport.h"
#include "vtkMRMLAnnotationControlPointsStorageNode.h"

class vtkMRMLAnnotationLineDisplayNode;
class vtkMRMLAnnotationLinesNode;

/// \ingroup Slicer_QtModules_Annotation
class  VTK_SLICER_ANNOTATIONS_MODULE_MRML_EXPORT vtkMRMLAnnotationLinesStorageNode
  : public vtkMRMLAnnotationControlPointsStorageNode
{
  public:
  static vtkMRMLAnnotationLinesStorageNode *New();
  vtkTypeMacro(vtkMRMLAnnotationLinesStorageNode,vtkMRMLAnnotationControlPointsStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  // Description:
  // Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  // Description:
  // Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  // Description:
  // Copy the node's attributes to this object
  void Copy(vtkMRMLNode *node) override;

  // Description:
  // Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override {return "AnnotationLinesStorage";}

  /// Return true if the node can be read in
  bool CanReadInReferenceNode(vtkMRMLNode* refNode) override;

protected:
  vtkMRMLAnnotationLinesStorageNode();
  ~vtkMRMLAnnotationLinesStorageNode() override;
  vtkMRMLAnnotationLinesStorageNode(const vtkMRMLAnnotationLinesStorageNode&);
  void operator=(const vtkMRMLAnnotationLinesStorageNode&);

  const char* GetAnnotationStorageType() { return "line"; }

  int WriteAnnotationLineDisplayProperties(fstream & of, vtkMRMLAnnotationLineDisplayNode *refNode, std::string preposition);
  int WriteAnnotationLinesProperties(fstream & of, vtkMRMLAnnotationLinesNode *refNode);
  int WriteAnnotationLinesData(fstream& of, vtkMRMLAnnotationLinesNode *refNode);

  int ReadAnnotation(vtkMRMLAnnotationLinesNode *refNode);
  int ReadAnnotationLinesData(vtkMRMLAnnotationLinesNode *refNode, char line[1024], int typeColumn, int startIDColumn, int endIDColumn, int selColumn,  int visColumn, int numColumns);
  int ReadAnnotationLineDisplayProperties(vtkMRMLAnnotationLineDisplayNode *refNode, std::string lineString, std::string preposition);
  int ReadAnnotationLinesProperties(vtkMRMLAnnotationLinesNode *refNode, char line[1024], int &typeColumn, int& startIDColumn,    int& endIDColumn, int& selColumn, int& visColumn, int& numColumns);

  /// Read data and set it in the referenced node
  int ReadDataInternal(vtkMRMLNode *refNode) override;

  // Description:
  int WriteAnnotationDataInternal(vtkMRMLNode *refNode, fstream & of) override;
};

#endif




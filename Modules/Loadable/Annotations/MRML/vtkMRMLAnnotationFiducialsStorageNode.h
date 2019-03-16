// .NAME vtkMRMLAnnotationFiducialsStorageNode - MRML node for representing a volume storage
// .SECTION Description
// vtkMRMLAnnotationFiducialsStorageNode can be used to read in the old style storage files ending with fcsv

#ifndef __vtkMRMLAnnotationFiducialsStorageNode_h
#define __vtkMRMLAnnotationFiducialsStorageNode_h

#include "vtkSlicerAnnotationsModuleMRMLExport.h"
#include "vtkMRMLAnnotationControlPointsStorageNode.h"

class vtkMRMLAnnotationPointDisplayNode;
class vtkMRMLAnnotationFiducialNode;

/// \ingroup Slicer_QtModules_Annotation
class  VTK_SLICER_ANNOTATIONS_MODULE_MRML_EXPORT vtkMRMLAnnotationFiducialsStorageNode
  : public vtkMRMLAnnotationControlPointsStorageNode
{
  public:
  static vtkMRMLAnnotationFiducialsStorageNode *New();
  vtkTypeMacro(vtkMRMLAnnotationFiducialsStorageNode,vtkMRMLAnnotationControlPointsStorageNode);

  vtkMRMLNode* CreateNodeInstance() override;

  // Description:
  // Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override {return "AnnotationFiducialsStorage";}

  /// utility method called by the annotation hierarchy node to let this
  /// storage node read a single fiducial's data from an already open file
  int ReadOneFiducial(fstream & fstr, vtkMRMLAnnotationFiducialNode *fiducialNode);

  /// Return true if the node can be read in
  bool CanReadInReferenceNode(vtkMRMLNode* refNode) override;

protected:
  vtkMRMLAnnotationFiducialsStorageNode()  = default;
  ~vtkMRMLAnnotationFiducialsStorageNode() override  = default;
  vtkMRMLAnnotationFiducialsStorageNode(const vtkMRMLAnnotationFiducialsStorageNode&);
  void operator=(const vtkMRMLAnnotationFiducialsStorageNode&);

  int ReadAnnotation(vtkMRMLAnnotationFiducialNode *refNode);
  int ReadAnnotationFiducialsData(vtkMRMLAnnotationFiducialNode *refNode, char line[1024], int labelColumn, int xColumn, int yColumn, int zColumn,
                      int selColumn,  int visColumn, int numColumns);
  int ReadAnnotationFiducialsProperties(vtkMRMLAnnotationFiducialNode *refNode, char line[1024], int &labelColumn,
                        int& xColumn,    int& yColumn,     int& zColumn, int& selColumn, int& visColumn, int& numColumns);

  // Description:
  // Initialize all the supported write file types
  void InitializeSupportedWriteFileTypes() override;

  // Read data and set it in the referenced node
  int ReadDataInternal(vtkMRMLNode *refNode) override;

};

#endif

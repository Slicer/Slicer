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
class  VTK_SLICER_ANNOTATIONS_MODULE_MRML_EXPORT vtkMRMLAnnotationFiducialsStorageNode : public vtkMRMLAnnotationControlPointsStorageNode
{
  public:
  static vtkMRMLAnnotationFiducialsStorageNode *New();
  vtkTypeMacro(vtkMRMLAnnotationFiducialsStorageNode,vtkMRMLAnnotationControlPointsStorageNode);

  virtual vtkMRMLNode* CreateNodeInstance();

   // Description:
  // Read data and set it in the referenced node
  // NOTE: Subclasses should implement this method
  int ReadData(vtkMRMLNode *refNode);

  // Description:
  // Get node XML tag name (like Storage, Model)
  virtual const char* GetNodeTagName()  {return "AnnotationFiducialsStorage";};

  // Description:
  // Initialize all the supported write file types
  virtual void InitializeSupportedWriteFileTypes();

protected:


  vtkMRMLAnnotationFiducialsStorageNode() { };
  ~vtkMRMLAnnotationFiducialsStorageNode() { };
  vtkMRMLAnnotationFiducialsStorageNode(const vtkMRMLAnnotationFiducialsStorageNode&);
  void operator=(const vtkMRMLAnnotationFiducialsStorageNode&);

  int ReadAnnotation(vtkMRMLAnnotationFiducialNode *refNode);
  int ReadAnnotationFiducialsData(vtkMRMLAnnotationFiducialNode *refNode, char line[1024], int labelColumn, int xColumn, int yColumn, int zColumn,  
                      int selColumn,  int visColumn, int numColumns);
  int ReadAnnotationFiducialsProperties(vtkMRMLAnnotationFiducialNode *refNode, char line[1024], int &labelColumn, 
                        int& xColumn,    int& yColumn,     int& zColumn, int& selColumn, int& visColumn, int& numColumns);
};

#endif




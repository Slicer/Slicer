// .NAME vtkMRMLAnnotationFiducialsStorageNode - MRML node for representing a volume storage
// .SECTION Description
// vtkMRMLAnnotationFiducialsStorageNode can be used to read in the old style storage files ending with fcsv

#ifndef __vtkMRMLAnnotationFiducialsStorageNode_h
#define __vtkMRMLAnnotationFiducialsStorageNode_h

#include "qSlicerAnnotationModuleExport.h"
#include "vtkMRMLAnnotationControlPointsStorageNode.h"

class vtkMRMLAnnotationPointDisplayNode;
class vtkMRMLAnnotationFiducialNode;


class  Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkMRMLAnnotationFiducialsStorageNode : public vtkMRMLAnnotationControlPointsStorageNode
{
  public:
  static vtkMRMLAnnotationFiducialsStorageNode *New();
  vtkTypeMacro(vtkMRMLAnnotationFiducialsStorageNode,vtkMRMLAnnotationControlPointsStorageNode);

  virtual vtkMRMLNode* CreateNodeInstance();

   // Description:
  // Read data and set it in the referenced node
  // NOTE: Subclasses should implement this method
  int ReadData(); 
  virtual int ReadData(vtkMRMLNode *vtkNotUsed(refNode)) {return this->ReadData();  }

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

  int ReadAnnotation();
  int ReadAnnotationFiducialsData(vtkMRMLAnnotationFiducialNode *refNode, char line[1024], int labelColumn, int xColumn, int yColumn, int zColumn,  
                      int selColumn,  int visColumn, int numColumns);
  int ReadAnnotationFiducialsProperties(vtkMRMLAnnotationFiducialNode *refNode, char line[1024], int &labelColumn, 
                        int& xColumn,    int& yColumn,     int& zColumn, int& selColumn, int& visColumn, int& numColumns);
};

#endif




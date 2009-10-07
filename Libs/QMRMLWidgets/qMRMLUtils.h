#ifndef __qMRMLUtils_h
#define __qMRMLUtils_h

#include "qMRMLWidgetsWin32Header.h"

#include <QString>
#include <QVector>

class vtkMRMLScene; 
class vtkMRMLNode; 
class vtkMRMLLinearTransformNode; 
class vtkTransform; 
class vtkMatrix4x4; 

#define compare_double(x, y) (((x-y)<0.000001) && ((x-y)>-0.000001))

class QMRML_WIDGETS_EXPORT qMRMLUtils
{
  
public:
  typedef qMRMLUtils Self; 

  // Description:
  // Create and add a node using its classname
  static vtkMRMLNode* createAndAddNodeToSceneByClass(vtkMRMLScene * scene, const char* className); 
  static vtkMRMLNode* createAndAddNodeToSceneByClass(vtkMRMLScene * scene, const QString& className); 
  
  // Description:
  // Convert a vtkMatrix to a QVector
  static void vtkMatrixToQVector(vtkMatrix4x4* matrix, QVector<double> & vector); 
  
  // Description:
  static void getTransformInCoordinateSystem(vtkMRMLNode* transformNode, bool global, 
    vtkTransform* transform); 
  static void getTransformInCoordinateSystem(vtkMRMLLinearTransformNode* transformNode, 
    bool global, vtkTransform* transform); 
  
private:
  // Not implemented
  qMRMLUtils();
  virtual ~qMRMLUtils();

};

#endif

#include "qMRMLUtils.h"

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLLinearTransformNode.h>

// VTK includes
#include <vtkTransform.h>
#include <vtkSmartPointer.h>
#include <vtkMatrix4x4.h>

// QT includes
#include <QDebug>

//------------------------------------------------------------------------------
void qMRMLUtils::vtkMatrixToQVector(vtkMatrix4x4* matrix, QVector<double> & vector)
{
  if (!matrix) { return; }
  
  vector.clear();
  
  for (int i=0; i < 4; i++)
    {
    for (int j=0; j < 4; j++)
      {
      vector.append(matrix->GetElement(i,j)); 
      }
    }
}

//------------------------------------------------------------------------------
void qMRMLUtils::getTransformInCoordinateSystem(vtkMRMLNode* node, bool global, 
    vtkTransform* transform)
{
  Self::getTransformInCoordinateSystem(vtkMRMLLinearTransformNode::SafeDownCast( node ), 
    global, transform); 
}

//------------------------------------------------------------------------------
void qMRMLUtils::getTransformInCoordinateSystem(vtkMRMLLinearTransformNode* transformNode, 
  bool global, vtkTransform* transform)
{
  Q_ASSERT(transformNode && transform);
  if (!transformNode || !transform) { return; }
  transform->Identity();

  vtkMatrix4x4 *matrix = transformNode->GetMatrixTransformToParent();
  Q_ASSERT(matrix);
  if (!matrix) { return; }
  transform->SetMatrix(matrix);

  if ( global )
    {
    transform->PostMultiply();
    }
  else
    {
    transform->PreMultiply();
    }
}

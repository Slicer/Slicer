
#include "qMRMLUtils.h"

#include <vtkMRMLScene.h>
#include <vtkMRMLNode.h>

#include <vtkMRMLLinearTransformNode.h>
#include <vtkTransform.h>
#include <vtkSmartPointer.h>
#include <vtkMatrix4x4.h>

#include <QDebug>

//------------------------------------------------------------------------------
vtkMRMLNode* qMRMLUtils::createAndAddNodeToSceneByClass(vtkMRMLScene * scene, const char* className)
{
  return qMRMLUtils::createAndAddNodeToSceneByClass(scene, QString::fromAscii(className) ); 
}

//------------------------------------------------------------------------------
vtkMRMLNode* qMRMLUtils::createAndAddNodeToSceneByClass(vtkMRMLScene * scene, const QString& className)
{
  Q_ASSERT(scene);
  Q_ASSERT(!className.isEmpty());
  if (!scene || className.isEmpty())
    {
    return 0; 
    }

  vtkSmartPointer<vtkMRMLNode> node; 
  node.TakeReference( scene->CreateNodeByClass( className.toAscii().data() ) ); 
  
  Q_ASSERT(node);
  if (node == NULL)
    {
    return 0;
    }
  
  node->SetScene( scene );
  node->SetName( scene->GetUniqueNameByString( 
    scene->GetTagByClassName(className.toAscii().data()) ) );
  /*
    // If there is a Attribute Name-Value specified, then set that
    // attribute on the node
    for (int c=0; c < this->GetNumberOfNodeClasses(); c++)
      {
      if (!strcmp(this->GetNodeClass(c), className))
        {
        if (this->GetNodeAttributeName(c) != NULL)
          {
          node->SetAttribute(this->GetNodeAttributeName(c),
                            this->GetNodeAttributeValue(c));
          }
        break;
        }
      }
  */
  
  vtkMRMLNode * nodeCreated = scene->AddNode(node);
  qDebug() << "createAndAddNodeToSceneByClass - Set name to:" 
           << nodeCreated->GetName() << "(" << nodeCreated->GetID() << ")"; 
  return nodeCreated; 
}

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

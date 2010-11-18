/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QDebug>
#include <QColor>
#include <QStyle>
#include <QPainter>

// qMRML includes
#include "qMRMLUtils.h"

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLViewNode.h>
#include <vtkMRMLLinearTransformNode.h>

// VTK includes
#include <vtkTransform.h>
#include <vtkSmartPointer.h>
#include <vtkMatrix4x4.h>

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
  Q_ASSERT(transform);
  if (!transform)
    {
    return;
    }

  transform->Identity();

  if (!transformNode) 
    { 
    return; 
    }

  vtkMatrix4x4 *matrix = transformNode->GetMatrixTransformToParent();
  Q_ASSERT(matrix);
  if (!matrix) 
    { 
    return; 
    }
  
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

//------------------------------------------------------------------------------
int qMRMLUtils::countVisibleViewNode(vtkMRMLScene* scene)
{
  Q_ASSERT(scene);
  int numberOfVisibleNodes = 0;
  const char* className = "vtkMRMLViewNode";
  int nnodes = scene->GetNumberOfNodesByClass(className);
  for (int n = 0; n < nnodes; n++)
    {
    vtkMRMLViewNode * node = vtkMRMLViewNode::SafeDownCast(scene->GetNthNodeByClass(n, className));
    if (node && node->GetVisibility())
      {
      numberOfVisibleNodes++;
      }
    }
  return numberOfVisibleNodes;
}

// ----------------------------------------------------------------
QPixmap qMRMLUtils::createColorPixmap(QStyle * style, const QColor &color)
{
  if (!style)
    {
    return QPixmap();
    }

  const int size = style->pixelMetric(QStyle::PM_SmallIconSize) - 4;

  // Create a pixmap
  QPixmap colorFieldPixmap(size, size);

  // Fill it with the color
  colorFieldPixmap.fill(color);

  // Make a black rectangle on the border
  QPainter painter(&colorFieldPixmap);
  painter.drawRect(0, 0, size - 1, size - 1);

  return colorFieldPixmap;
}

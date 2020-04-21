/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
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
#include <QStyle>
#include <QPainter>

// CTK includes
#include "ctkVTKWidgetsUtils.h"

// qMRML includes
#include "qMRMLUtils.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkQImageToImageSource.h>
#include <vtkTransform.h>

//-----------------------------------------------------------------------------
qMRMLUtils::qMRMLUtils(QObject* _parent)
  :QObject(_parent)
{
}

//-----------------------------------------------------------------------------
qMRMLUtils::~qMRMLUtils() = default;

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
  Self::getTransformInCoordinateSystem(vtkMRMLTransformNode::SafeDownCast( node ),
    global, transform);
}

//------------------------------------------------------------------------------
void qMRMLUtils::getTransformInCoordinateSystem(vtkMRMLTransformNode* transformNode,
  bool global, vtkTransform* transform)
{
  Q_ASSERT(transform);
  if (!transform)
    {
    return;
    }

  transform->Identity();

  if (!transformNode || !transformNode->IsLinear())
    {
    return;
    }

  vtkNew<vtkMatrix4x4> matrix;
  int matrixDefined=transformNode->GetMatrixTransformToParent(matrix.GetPointer());
  Q_ASSERT(matrixDefined);
  if (!matrixDefined)
    {
    return;
    }

  transform->SetMatrix(matrix.GetPointer());

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

//---------------------------------------------------------------------------
bool qMRMLUtils::qImageToVtkImageData(const QImage& qImage, vtkImageData* vtkimage)
{
  return ctk::qImageToVTKImageData(qImage, vtkimage);
}

//---------------------------------------------------------------------------
bool qMRMLUtils::vtkImageDataToQImage(vtkImageData* vtkimage, QImage& img)
{
  img = ctk::vtkImageDataToQImage(vtkimage);
  return !img.isNull();
}

//-----------------------------------------------------------------------------
void qMRMLUtils::colorToQColor(const double* color, QColor &qcolor)
{
  if (color)
    {
    qcolor = QColor::fromRgbF(color[0], color[1], color[2]);
    }
}

//-----------------------------------------------------------------------------
void qMRMLUtils::qColorToColor(const QColor &qcolor, double* color)
{
  if (color)
    {
    color[0] = qcolor.redF();
    color[1] = qcolor.greenF();
    color[2] = qcolor.blueF();
    }
}

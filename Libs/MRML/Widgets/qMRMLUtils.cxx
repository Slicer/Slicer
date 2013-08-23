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

// qMRML includes
#include "qMRMLUtils.h"

// MRML includes
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkTransform.h>
#include <vtkImageData.h>

//-----------------------------------------------------------------------------
qMRMLUtils::qMRMLUtils(QObject* _parent)
  :QObject(_parent)
{
}

//-----------------------------------------------------------------------------
qMRMLUtils::~qMRMLUtils()
{
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

//---------------------------------------------------------------------------
bool qMRMLUtils::qImageToVtkImageData(const QImage& qImage, vtkImageData* vtkimage)
{
  if (vtkimage == 0)
    {
    return false;
    }

  QImage img = qImage;
  if (qImage.hasAlphaChannel())
    {
    if (qImage.format() != QImage::Format_ARGB32)
      {
      img = qImage.convertToFormat(QImage::Format_ARGB32);
      }
    }
  else
    {
    if (qImage.format() != QImage::Format_RGB32)
      {
      img = qImage.convertToFormat(QImage::Format_RGB32);
      }
    }

  int height = img.height();
  int width = img.width();
  int numcomponents = img.hasAlphaChannel() ? 4 : 3;

  vtkimage->SetWholeExtent(0, width-1, 0, height-1, 0, 0);
  vtkimage->SetSpacing(1.0, 1.0, 1.0);
  vtkimage->SetOrigin(0.0, 0.0, 0.0);
  vtkimage->SetNumberOfScalarComponents(numcomponents);
  vtkimage->SetScalarType(VTK_UNSIGNED_CHAR);
  vtkimage->SetExtent(vtkimage->GetWholeExtent());
  vtkimage->AllocateScalars();
  for(int i=0; i<height; i++)
    {
    unsigned char* row;

    row = static_cast<unsigned char*>(vtkimage->GetScalarPointer(0, height-i-1, 0));
    const QRgb* linePixels = reinterpret_cast<const QRgb*>(img.scanLine(i));

    /*
     * TODO: consider using this to speed up the conversion:
     *
    memcpy(row, linePixels, width*numcomponents);
    */

    unsigned char* pixel;
    pixel = row;
    for(int j=0; j<width; j++)
      {
      const QRgb& col = linePixels[j];
      *pixel++ = qRed(col);
      *pixel++ = qGreen(col);
      *pixel++ = qBlue(col);
      if(numcomponents == 4)
        {
        *pixel++ = qAlpha(col);
        }
      }
    }
  return true;
}

//---------------------------------------------------------------------------
bool qMRMLUtils::vtkImageDataToQImage(vtkImageData* vtkimage, QImage& img)
{
  if (!vtkimage ||
      vtkimage->GetScalarType() != VTK_UNSIGNED_CHAR)
    {
    return false;
    }

  int extent[6];
  vtkimage->GetExtent(extent);
  int width = extent[1]-extent[0]+1;
  int height = extent[3]-extent[2]+1;
  int numcomponents = vtkimage->GetNumberOfScalarComponents();
  if(!(numcomponents == 3 || numcomponents == 4))
    {
    return false;
    }

  QImage newimg(width, height, QImage::Format_ARGB32);

  for(int i=0; i<height; i++)
    {
    QRgb* bits = reinterpret_cast<QRgb*>(newimg.scanLine(i));
    unsigned char* row;
    row = static_cast<unsigned char*>(
      vtkimage->GetScalarPointer(extent[0], extent[2] + height-i-1, extent[4]));
    for(int j=0; j<width; j++)
      {
      unsigned char* data = &row[j*numcomponents];
      bits[j] = numcomponents == 4 ?
        qRgba(data[0], data[1], data[2], data[3]) :
        qRgb(data[0], data[1], data[2]);
      }
    }

  img = newimg;
  return true;
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

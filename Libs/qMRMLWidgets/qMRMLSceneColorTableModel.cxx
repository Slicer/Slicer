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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QDebug>
#include <QIcon>
#include <QPixmap>

// qMRML includes
#include "qMRMLSceneColorTableModel.h"

// MRML includes
#include <vtkMRMLColorNode.h>

// VTK includes
#include <vtkScalarsToColors.h>

//------------------------------------------------------------------------------
class qMRMLSceneColorTableModelPrivate
{
public:
  struct ColorGradient
  {
    ColorGradient();
    void updatePixmap(vtkScalarsToColors* scalarsToColors);

    unsigned long MTime;
    QPixmap       Pixmap;
  };

  mutable QMap<QString, ColorGradient> GradientCache;
};

//------------------------------------------------------------------------------
qMRMLSceneColorTableModelPrivate::ColorGradient::ColorGradient()
{
  this->MTime = 0;
  this->Pixmap = QPixmap(50, 31);
}

//------------------------------------------------------------------------------
void qMRMLSceneColorTableModelPrivate::ColorGradient::updatePixmap(vtkScalarsToColors* scalarsToColors)
{
  const int width = this->Pixmap.width();
  const int height = this->Pixmap.height();

  double* values = new double[width];
  const double* range = scalarsToColors->GetRange();
  for (int i = 0; i < width; ++i)
    {
    values[i] = range[0] + i * (range[1] - range[0]) / (width - 1);
    }

  QImage transferFunctionImage(width, height, QImage::Format_RGB32);
  unsigned char* colors = transferFunctionImage.bits();
  // Map the first line
  scalarsToColors->MapScalarsThroughTable2(
    values, colors, VTK_DOUBLE, width, 1, VTK_RGBA);
  delete [] values;
  // Pixels are not correctly ordered, reorder them correctly
  unsigned char* colorsPtr = colors;
  QRgb* rgbPtr = reinterpret_cast<QRgb*>(colors);
  for (int i = 0; i < width; ++i)
    {
    *(rgbPtr++) = QColor(colorsPtr[0], colorsPtr[1], colorsPtr[2]).rgb();
    colorsPtr += 4;
    }
  // Fill the other lines
  for (int i = 1; i < height; ++i)
    {
    memcpy(colors + i*VTK_RGBA*width, colors, VTK_RGBA*width);
    }

  this->Pixmap = QPixmap::fromImage( transferFunctionImage );
  this->MTime = scalarsToColors->GetMTime();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
qMRMLSceneColorTableModel::qMRMLSceneColorTableModel(QObject *vparent)
  : qMRMLSceneCategoryModel(vparent)
  , d_ptr(new qMRMLSceneColorTableModelPrivate)
{
}

//------------------------------------------------------------------------------
qMRMLSceneColorTableModel::~qMRMLSceneColorTableModel()
{
}

//------------------------------------------------------------------------------
void qMRMLSceneColorTableModel::updateItemFromNode(QStandardItem* item, vtkMRMLNode* node, int column)
{
  Q_D(const qMRMLSceneColorTableModel);
  this->qMRMLSceneModel::updateItemFromNode(item, node, column);
  vtkMRMLColorNode* colorNode = vtkMRMLColorNode::SafeDownCast(node);
  if (colorNode && column == 0)
    {
    if (this->updateGradientFromNode(colorNode))
      {
      qMRMLSceneColorTableModelPrivate::ColorGradient& colorGradient =
        d->GradientCache[colorNode->GetID()];
      //item->setBackground(colorGradient.Gradient);
      item->setIcon(colorGradient.Pixmap);
      }
    }
}

//------------------------------------------------------------------------------
bool qMRMLSceneColorTableModel::updateGradientFromNode(vtkMRMLColorNode* node)const
{
  Q_D(const qMRMLSceneColorTableModel);
  Q_ASSERT(node);
  /// TODO: Improve the cache of the pixmaps, right now, they are not shared
  /// between the different qMRMLSceneColorTableModels.
  qMRMLSceneColorTableModelPrivate::ColorGradient& colorGradient = d->GradientCache[node->GetID()];
  if (!node->GetScalarsToColors() ||
      colorGradient.MTime >= node->GetScalarsToColors()->GetMTime())
    {
    return false;
    }
  /// HACK: The node UserDefined is currently garbage and makes the icon
  /// generation crash.
  if (QString(node->GetName()) == "UserDefined")
    {
    return;
    }
  colorGradient.updatePixmap(node->GetScalarsToColors());
  return true;
}

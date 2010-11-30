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
#include <QFile>
#include <QPainter>

// CTK includes
#include <ctkVTKColorTransferFunction.h>
#include <ctkVTKLookupTable.h>
#include <ctkTransferFunctionRepresentation.h>

// qMRML includes
#include "qMRMLSceneColorTableModel.h"

// MRML includes
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLProceduralColorNode.h>

// VTK includes
#include <vtkColorTransferFunction.h>
#include <vtkLookupTable.h>

//------------------------------------------------------------------------------
class qMRMLSceneColorTableModelPrivate
{
public:
  struct ColorGradient
  {
    ColorGradient();
    unsigned long MTime;
    QGradient     Gradient;
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
    /*
    QString iconFileName(":" + QString(colorNode->GetName()));
    if (!QFile::exists(iconFileName))
      {
      iconFileName = ":" + QString(colorNode->GetTypeAsString());
      }
    if (!QFile::exists(iconFileName))
      {
      iconFileName = ":blankLUT";
      }
    QPixmap lutPixmap(iconFileName);
    item->setIcon(QIcon(lutPixmap));
    */
    if (this->updateGradientFromNode(colorNode))
      {
      qMRMLSceneColorTableModelPrivate::ColorGradient& colorGradient =
        d->GradientCache[colorNode->GetID()];
      item->setBackground(colorGradient.Gradient);
      item->setIcon(colorGradient.Pixmap);
      }
    }
}

//------------------------------------------------------------------------------
bool qMRMLSceneColorTableModel::updateGradientFromNode(vtkMRMLColorNode* node)const
{
  Q_D(const qMRMLSceneColorTableModel);
  Q_ASSERT(node);
  qMRMLSceneColorTableModelPrivate::ColorGradient& colorGradient = d->GradientCache[node->GetID()];
  vtkMRMLProceduralColorNode* proceduralNode = vtkMRMLProceduralColorNode::SafeDownCast(node);
  if ((node->GetLookupTable() &&
       colorGradient.MTime >= node->GetLookupTable()->GetMTime()) ||
      (proceduralNode &&
       colorGradient.MTime >= proceduralNode->GetColorTransferFunction()->GetMTime()))
    {
    return false;
    }
  //qDebug() << " calculate gradient for lookup table: " << node->GetLookupTable();
  ctkTransferFunction* lt = 0;
  if (node->GetLookupTable())
    {
    lt = new ctkVTKLookupTable(node->GetLookupTable());
    }
  else
    {
    lt = new ctkVTKColorTransferFunction(proceduralNode->GetColorTransferFunction());
    }
  ctkTransferFunctionRepresentation  tfr(lt);
  colorGradient.Gradient = tfr.gradient();
  delete lt;
  //qDebug() << " end calculate gradient for lookup table: " << node->GetLookupTable();
  colorGradient.Gradient.setCoordinateMode(QGradient::StretchToDeviceMode);
  QPainter pixmapPainter(&colorGradient.Pixmap);
  pixmapPainter.fillRect(0,0,
                         colorGradient.Pixmap.width(),
                         colorGradient.Pixmap.height(),
                         colorGradient.Gradient);
  QGradientStops gradientStops = colorGradient.Gradient.stops();
  int count = gradientStops.count();
  for (int i = 0; i < count; ++i)
    {
    gradientStops[i].second.setAlpha(64);
    }
  colorGradient.Gradient.setStops(gradientStops);
  colorGradient.MTime = node->GetLookupTable() ? node->GetLookupTable()->GetMTime()
    : proceduralNode->GetColorTransferFunction()->GetMTime();
  return true;
}

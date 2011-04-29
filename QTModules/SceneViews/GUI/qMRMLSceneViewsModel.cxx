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
#include <QMap>
#include <QMimeData>
#include <QSharedPointer>
#include <QStack>
#include <QStringList>
#include <QVector>

// qMRML includes
#include "qMRMLSceneViewsModel.h"
#include "qMRMLSceneModel.h"
#include "qMRMLUtils.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLNode.h>
#include <vtkMRMLSceneViewNode.h>

// VTK includes
#include <vtkVariantArray.h>
#include <typeinfo>
#include <vtkStdString.h>

//------------------------------------------------------------------------------
qMRMLSceneViewsModel::qMRMLSceneViewsModel(QObject *vparent)
  :qMRMLSceneModel(vparent)
//  :qMRMLSceneHierarchyModel(vparent)
{
  this->setListenNodeModifiedEvent(true);
  this->setColumnCount(5);
  this->setHorizontalHeaderLabels(
    QStringList() << "" << "Preview/Edit" << "Restore" << "Name" << "Description");
}

//------------------------------------------------------------------------------
qMRMLSceneViewsModel::~qMRMLSceneViewsModel()
{
}

//------------------------------------------------------------------------------
void qMRMLSceneViewsModel::updateNodeFromItemData(vtkMRMLNode* node, QStandardItem* item)
{
  if (item->column() == qMRMLSceneViewsModel::NameColumn)
    {

    vtkMRMLSceneViewNode* viewNode = vtkMRMLSceneViewNode::SafeDownCast(node);

    if (viewNode)
      {
      // if we have a snapshot node, the name can be changed by editing the textcolumn
      viewNode->SetName(vtkStdString(item->text().toLatin1()));
      }
    }
  else if (item->column() == qMRMLSceneViewsModel::DescriptionColumn)
    {

    vtkMRMLSceneViewNode* viewNode = vtkMRMLSceneViewNode::SafeDownCast(node);

    if (viewNode)
      {
      // if we have a snapshot node, the name can be changed by editing the textcolumn
      viewNode->SetSceneViewDescription(vtkStdString(item->text().toLatin1()));
      }
    }
  //this->m_Widget->refreshTree();
}

//------------------------------------------------------------------------------
void qMRMLSceneViewsModel::updateItemDataFromNode(QStandardItem* item, vtkMRMLNode* node, int column)
{
  vtkMRMLSceneViewNode* viewNode = vtkMRMLSceneViewNode::SafeDownCast(node);

  switch (column)
    {
    case qMRMLSceneViewsModel::ThumbnailColumn:
      if (viewNode)
        {
        if (viewNode->GetScreenshot())
          {
          QImage qimage;
          qMRMLUtils::vtkImageDataToQImage(viewNode->GetScreenshot(),qimage);
          QSize imageSize = qimage.size();
//          std::cout << "Image size = " << imageSize.width() << "x" << imageSize.height() << std::endl;
          QPixmap screenshot;
          screenshot = QPixmap::fromImage(qimage, Qt::AutoColor);
          item->setData(screenshot.scaled(80,80,Qt::KeepAspectRatio,Qt::SmoothTransformation),Qt::DecorationRole);
          item->setData(QSize(80,80),Qt::SizeHintRole);
          }
        else
          {
//          std::cout << "view node's screen shot is null, storage node is " << (viewNode->GetStorageNode() == NULL ? "null" : viewNode->GetStorageNode()->GetID()) << std::endl;
          }
        }
      break;
    case qMRMLSceneViewsModel::RestoreColumn:
      item->setData(QPixmap(":/Icons/Restore.png"),Qt::DecorationRole);
      break;
    case qMRMLSceneViewsModel::NameColumn:
      if (viewNode)
        {
        item->setText(QString(viewNode->GetName()));
        }
      break;
    case qMRMLSceneViewsModel::DescriptionColumn:
      if (viewNode)
        {
        item->setText(QString(viewNode->GetSceneViewDescription()));
        item->setData(QSize(80,80),Qt::SizeHintRole);
        }
      break;
    }
}

//------------------------------------------------------------------------------
QFlags<Qt::ItemFlag> qMRMLSceneViewsModel::nodeFlags(vtkMRMLNode* node, int column)const
{
  QFlags<Qt::ItemFlag> flags = this->qMRMLSceneModel::nodeFlags(node, column);
  // remove the ItemIsEditable flag from any possible item (typically at column 0)
  flags = flags & ~Qt::ItemIsEditable;
  // and set it to the right column
  switch(column)
    {
    case qMRMLSceneViewsModel::NameColumn:
      flags = flags | Qt::ItemIsEditable;
      break;
    case qMRMLSceneViewsModel::DescriptionColumn:
      flags = flags | Qt::ItemIsEditable;
      break;
    default:
      break;
    }
  return flags;
}

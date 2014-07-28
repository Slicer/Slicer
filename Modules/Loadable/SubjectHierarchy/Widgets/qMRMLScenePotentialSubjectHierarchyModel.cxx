/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#include "qMRMLScenePotentialSubjectHierarchyModel.h"

// Subject Hierarchy includes
#include "qMRMLScenePotentialSubjectHierarchyModel_p.h"
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyAbstractPlugin.h"
#include "vtkSlicerSubjectHierarchyModuleLogic.h"

// MRML includes
#include <vtkMRMLNode.h>

// Qt includes
#include <QMimeData>
#include <QTimer>

//------------------------------------------------------------------------------
qMRMLScenePotentialSubjectHierarchyModelPrivate::qMRMLScenePotentialSubjectHierarchyModelPrivate(qMRMLScenePotentialSubjectHierarchyModel& object)
: Superclass(object)
{
}

//------------------------------------------------------------------------------
void qMRMLScenePotentialSubjectHierarchyModelPrivate::init()
{
  Q_Q(qMRMLScenePotentialSubjectHierarchyModel);
  this->Superclass::init();

  QObject::connect( q, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), q, SLOT(onRowsRemoved(QModelIndex,int,int)) );
}


//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
qMRMLScenePotentialSubjectHierarchyModel::qMRMLScenePotentialSubjectHierarchyModel(QObject *vparent)
: Superclass(new qMRMLScenePotentialSubjectHierarchyModelPrivate(*this), vparent)
{
  Q_D(qMRMLScenePotentialSubjectHierarchyModel);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLScenePotentialSubjectHierarchyModel::~qMRMLScenePotentialSubjectHierarchyModel()
{
}

//------------------------------------------------------------------------------
QStringList qMRMLScenePotentialSubjectHierarchyModel::mimeTypes()const
{
  QStringList types;
  types << "application/vnd.text.list";
  return types;
}

//------------------------------------------------------------------------------
QMimeData* qMRMLScenePotentialSubjectHierarchyModel::mimeData(const QModelIndexList &indexes) const
{
  Q_D(const qMRMLScenePotentialSubjectHierarchyModel);

  QMimeData* mimeData = new QMimeData();
  QByteArray encodedData;

  QDataStream stream(&encodedData, QIODevice::WriteOnly);

  foreach (const QModelIndex &index, indexes)
    {
    if (index.isValid())
      {
      d->DraggedNodes << this->mrmlNodeFromIndex(index);
      QString text = data(index, PointerRole).toString();
      stream << text;
      }
    }

  mimeData->setData("application/vnd.text.list", encodedData);
  return mimeData;
}

//------------------------------------------------------------------------------
void qMRMLScenePotentialSubjectHierarchyModel::updateItemDataFromNode(QStandardItem* item, vtkMRMLNode* node, int column)
{
  Q_UNUSED(column);

  item->setText(QString(node->GetName()));
  item->setToolTip(QString(node->GetNodeTagName()) + " type (" + QString(node->GetID()) + ")");
}

//------------------------------------------------------------------------------
void qMRMLScenePotentialSubjectHierarchyModel::updateNodeFromItemData(vtkMRMLNode* node, QStandardItem* item)
{
  node->SetName(item->text().toLatin1().constData());
}

//------------------------------------------------------------------------------
bool qMRMLScenePotentialSubjectHierarchyModel::canBeAChild(vtkMRMLNode* node)const
{
  // If there is at least one plugin that can handle adding the node, then it also can be a child
  QList<qSlicerSubjectHierarchyAbstractPlugin*> foundPlugins =
    qSlicerSubjectHierarchyPluginHandler::instance()->pluginsForAddingToSubjectHierarchyForNode(node);
  if (!foundPlugins.empty())
    {
    return true;
    }

  return false;
}

//------------------------------------------------------------------------------
Qt::DropActions qMRMLScenePotentialSubjectHierarchyModel::supportedDropActions()const
{
  return Qt::MoveAction;
}

//------------------------------------------------------------------------------
void qMRMLScenePotentialSubjectHierarchyModel::onRowsRemoved(const QModelIndex parent, int start, int end)
{
  Q_D(const qMRMLScenePotentialSubjectHierarchyModel);
  Q_UNUSED(parent);
  Q_UNUSED(start);
  Q_UNUSED(end);

  if (d->DraggedNodes.count())
    {
    d->DraggedNodes.clear();
    }
}

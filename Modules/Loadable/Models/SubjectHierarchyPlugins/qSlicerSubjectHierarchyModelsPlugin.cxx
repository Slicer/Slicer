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

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyDefaultPlugin.h"
#include "qSlicerSubjectHierarchyFolderPlugin.h"
#include "qSlicerSubjectHierarchyModelsPlugin.h"
#include "qSlicerSubjectHierarchyPluginHandler.h"

// Terminologies includes
#include "qSlicerTerminologyItemDelegate.h"
#include "vtkSlicerTerminologiesModuleLogic.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLModelDisplayNode.h>

// vtkSegmentationCore includes
#include <vtkSegment.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>

// Qt includes
#include <QDebug>
#include <QStandardItem>
#include <QAction>

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

//-----------------------------------------------------------------------------
class qSlicerSubjectHierarchyModelsPluginPrivate : public QObject
{
  Q_OBJECT
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyModelsPlugin);

protected:
  qSlicerSubjectHierarchyModelsPlugin* const q_ptr;

public:
  qSlicerSubjectHierarchyModelsPluginPrivate(qSlicerSubjectHierarchyModelsPlugin& object);
  ~qSlicerSubjectHierarchyModelsPluginPrivate() override;
  void init();

public:
  QIcon ModelIcon;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyModelsPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyModelsPluginPrivate::qSlicerSubjectHierarchyModelsPluginPrivate(qSlicerSubjectHierarchyModelsPlugin& object)
  : q_ptr(&object)
  , ModelIcon(QIcon(":Icons/Model.png"))
{
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyModelsPluginPrivate::init() {}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyModelsPluginPrivate::~qSlicerSubjectHierarchyModelsPluginPrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyModelsPlugin methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyModelsPlugin::qSlicerSubjectHierarchyModelsPlugin(QObject* parent)
  : Superclass(parent)
  , d_ptr(new qSlicerSubjectHierarchyModelsPluginPrivate(*this))
{
  this->m_Name = QString("Models");

  Q_D(qSlicerSubjectHierarchyModelsPlugin);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyModelsPlugin::~qSlicerSubjectHierarchyModelsPlugin() = default;

//----------------------------------------------------------------------------
double qSlicerSubjectHierarchyModelsPlugin::canAddNodeToSubjectHierarchy(vtkMRMLNode* node, vtkIdType parentItemID /*=vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID*/) const
{
  Q_UNUSED(parentItemID);
  if (!node)
  {
    qCritical() << Q_FUNC_INFO << ": Input node is NULL";
    return 0.0;
  }
  else if (node->IsA("vtkMRMLModelNode"))
  {
    // Node is a model
    return 0.5;
  }
  return 0.0;
}

//---------------------------------------------------------------------------
double qSlicerSubjectHierarchyModelsPlugin::canOwnSubjectHierarchyItem(vtkIdType itemID) const
{
  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return 0.0;
  }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return 0.0;
  }

  // Model
  vtkMRMLNode* associatedNode = shNode->GetItemDataNode(itemID);
  if (associatedNode && associatedNode->IsA("vtkMRMLModelNode"))
  {
    return 0.5; // There may be other plugins that can handle special models better
  }

  return 0.0;
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchyModelsPlugin::roleForPlugin() const
{
  return "Model";
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyModelsPlugin::icon(vtkIdType itemID)
{
  Q_D(qSlicerSubjectHierarchyModelsPlugin);

  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return QIcon();
  }

  if (this->canOwnSubjectHierarchyItem(itemID))
  {
    return d->ModelIcon;
  }

  // Item unknown by plugin
  return QIcon();
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyModelsPlugin::visibilityIcon(int visible)
{
  // Have the default plugin (which is not registered) take care of this
  return qSlicerSubjectHierarchyPluginHandler::instance()->defaultPlugin()->visibilityIcon(visible);
}

//-----------------------------------------------------------------------------
QString qSlicerSubjectHierarchyModelsPlugin::tooltip(vtkIdType itemID) const
{
  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return tr("Invalid item");
  }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return tr("Error");
  }

  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (modelNode == nullptr)
  {
    return tr("Invalid model node");
  }
  QString dataType = tr("Poly data");
  vtkPointSet* data = modelNode->GetPolyData();
  if (data == nullptr)
  {
    data = modelNode->GetUnstructuredGrid();
    dataType = tr("Unstructured grid");
  }
  if (data == nullptr)
  {
    return tr("Model node does not contain data");
  }

  // Get basic tooltip from abstract plugin
  QString tooltipString = QString("%1: %2").arg(Superclass::tooltip(itemID)).arg(dataType);
  tooltipString.append(QString("\n  (") + tr("Points: %1  Cells: %2").arg(data->GetNumberOfPoints()).arg(data->GetNumberOfCells()));

  vtkMRMLModelDisplayNode* displayNode = vtkMRMLModelDisplayNode::SafeDownCast(modelNode->GetDisplayNode());
  if (displayNode && modelNode->GetDisplayVisibility() > 0)
  {
    double color[3] = { 0.0, 0.0, 0.0 };
    displayNode->GetColor(color);
    tooltipString.append(
      QString("  ")
      + tr("Color: %1,%2,%3  Opacity: %4%").arg(int(color[0] * 255.0)).arg(int(color[1] * 255.0)).arg(int(color[2] * 255.0)).arg(int(displayNode->GetOpacity() * 100.0)));
  }
  tooltipString.append(QString(")"));

  return tooltipString;
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModelsPlugin::setDisplayColor(vtkIdType itemID, QColor color, QMap<int, QVariant> terminologyMetaData)
{
  this->setColorAndTerminologyToDisplayableNode(itemID, color, terminologyMetaData, false, true);
}

//-----------------------------------------------------------------------------
QColor qSlicerSubjectHierarchyModelsPlugin::getDisplayColor(vtkIdType itemID, QMap<int, QVariant>& terminologyMetaData) const
{
  return this->colorAndTerminologyFromDisplayableNode(itemID, terminologyMetaData, false);
}

#include "qSlicerSubjectHierarchyModelsPlugin.moc"

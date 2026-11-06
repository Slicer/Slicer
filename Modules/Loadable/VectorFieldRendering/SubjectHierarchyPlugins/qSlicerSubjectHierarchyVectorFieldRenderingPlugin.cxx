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

  This file was developed with funding from the Children's Hospital of
  Philadelphia Frontier Program.

==============================================================================*/

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyVectorFieldRenderingPlugin.h"

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

// VectorFieldRendering includes
#include "vtkMRMLVectorFieldDisplayNode.h"
#include "vtkSlicerVectorFieldRenderingLogic.h"

// MRML includes
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLSubjectHierarchyNode.h>

// Qt includes
#include <QAction>
#include <QDebug>

//-----------------------------------------------------------------------------
class qSlicerSubjectHierarchyVectorFieldRenderingPluginPrivate : public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyVectorFieldRenderingPlugin);

protected:
  qSlicerSubjectHierarchyVectorFieldRenderingPlugin* const q_ptr;

public:
  qSlicerSubjectHierarchyVectorFieldRenderingPluginPrivate(qSlicerSubjectHierarchyVectorFieldRenderingPlugin& object);
  ~qSlicerSubjectHierarchyVectorFieldRenderingPluginPrivate() override;
  void init();

  /// The node of an item if it has a vector field
  vtkMRMLDisplayableNode* vectorFieldNode(vtkIdType itemID) const;

public:
  QAction* ToggleVectorFieldAction{ nullptr };
  QAction* VectorFieldOptionsAction{ nullptr };
};

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyVectorFieldRenderingPluginPrivate::qSlicerSubjectHierarchyVectorFieldRenderingPluginPrivate(qSlicerSubjectHierarchyVectorFieldRenderingPlugin& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyVectorFieldRenderingPluginPrivate::~qSlicerSubjectHierarchyVectorFieldRenderingPluginPrivate() = default;

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyVectorFieldRenderingPluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchyVectorFieldRenderingPlugin);

  this->ToggleVectorFieldAction = new QAction(qSlicerSubjectHierarchyVectorFieldRenderingPlugin::tr("Show as vector field"), q);
  this->ToggleVectorFieldAction->setObjectName("ToggleVectorFieldAction");
  this->ToggleVectorFieldAction->setToolTip(qSlicerSubjectHierarchyVectorFieldRenderingPlugin::tr(
    "Show the vector field with glyphs, a grid, contours or streamlines. While this is checked, the eye icon shows and hides the "
    "vector field with the node."));
  this->ToggleVectorFieldAction->setCheckable(true);
  QObject::connect(this->ToggleVectorFieldAction, SIGNAL(toggled(bool)), q, SLOT(toggleVectorFieldForCurrentItem(bool)));

  this->VectorFieldOptionsAction = new QAction(qSlicerSubjectHierarchyVectorFieldRenderingPlugin::tr("Vector field display settings..."), q);
  this->VectorFieldOptionsAction->setObjectName("VectorFieldOptionsAction");
  this->VectorFieldOptionsAction->setToolTip(
    qSlicerSubjectHierarchyVectorFieldRenderingPlugin::tr("Switch to the Vector Field Rendering module to edit how the vector field is shown"));
  QObject::connect(this->VectorFieldOptionsAction, SIGNAL(triggered()), q, SLOT(showVectorFieldOptionsForCurrentItem()));
}

//-----------------------------------------------------------------------------
vtkMRMLDisplayableNode* qSlicerSubjectHierarchyVectorFieldRenderingPluginPrivate::vectorFieldNode(vtkIdType itemID) const
{
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode || !itemID)
  {
    return nullptr;
  }
  vtkMRMLDisplayableNode* node = vtkMRMLDisplayableNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  return (vtkSlicerVectorFieldRenderingLogic::HasVectorField(node) ? node : nullptr);
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyVectorFieldRenderingPlugin::qSlicerSubjectHierarchyVectorFieldRenderingPlugin(QObject* parent)
  : Superclass(parent)
  , d_ptr(new qSlicerSubjectHierarchyVectorFieldRenderingPluginPrivate(*this))
{
  this->m_Name = QString("VectorFieldRendering");

  Q_D(qSlicerSubjectHierarchyVectorFieldRenderingPlugin);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyVectorFieldRenderingPlugin::~qSlicerSubjectHierarchyVectorFieldRenderingPlugin() = default;

//-----------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyVectorFieldRenderingPlugin::visibilityContextMenuActions() const
{
  Q_D(const qSlicerSubjectHierarchyVectorFieldRenderingPlugin);
  QList<QAction*> actions;
  actions << d->ToggleVectorFieldAction << d->VectorFieldOptionsAction;
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyVectorFieldRenderingPlugin::showVisibilityContextMenuActionsForItem(vtkIdType itemID)
{
  Q_D(qSlicerSubjectHierarchyVectorFieldRenderingPlugin);
  if (!d->vectorFieldNode(itemID))
  {
    return;
  }
  bool wasBlocked = d->ToggleVectorFieldAction->blockSignals(true);
  d->ToggleVectorFieldAction->setChecked(this->isVectorFieldShown(itemID));
  d->ToggleVectorFieldAction->blockSignals(wasBlocked);
  d->ToggleVectorFieldAction->setVisible(true);
  d->VectorFieldOptionsAction->setVisible(true);
}

//---------------------------------------------------------------------------
bool qSlicerSubjectHierarchyVectorFieldRenderingPlugin::isVectorFieldShown(vtkIdType itemID)
{
  Q_D(qSlicerSubjectHierarchyVectorFieldRenderingPlugin);
  // Shown with the node, also while the eye icon has hidden it
  for (vtkMRMLVectorFieldDisplayNode* displayNode : vtkSlicerVectorFieldRenderingLogic::GetVectorFieldDisplayNodes(d->vectorFieldNode(itemID)))
  {
    if (displayNode->GetShowMode() == vtkMRMLDisplayNode::ShowDefault)
    {
      return true;
    }
  }
  return false;
}

//---------------------------------------------------------------------------
bool qSlicerSubjectHierarchyVectorFieldRenderingPlugin::showVectorField(bool show, vtkIdType itemID)
{
  Q_D(qSlicerSubjectHierarchyVectorFieldRenderingPlugin);
  vtkMRMLDisplayableNode* node = d->vectorFieldNode(itemID);
  if (!node)
  {
    return false;
  }
  std::vector<vtkMRMLVectorFieldDisplayNode*> displayNodes = vtkSlicerVectorFieldRenderingLogic::GetVectorFieldDisplayNodes(node);
  if (show && displayNodes.empty())
  {
    // Added shown
    return (vtkSlicerVectorFieldRenderingLogic::AddVectorFieldDisplayNode(node) != nullptr);
  }
  for (vtkMRMLVectorFieldDisplayNode* displayNode : displayNodes)
  {
    vtkSlicerVectorFieldRenderingLogic::SetVectorFieldDisplayNodeShown(displayNode, show);
  }
  return true;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyVectorFieldRenderingPlugin::toggleVectorFieldForCurrentItem(bool show)
{
  vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  if (!currentItemID)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid current item";
    return;
  }
  this->showVectorField(show, currentItemID);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyVectorFieldRenderingPlugin::showVectorFieldOptionsForCurrentItem()
{
  Q_D(qSlicerSubjectHierarchyVectorFieldRenderingPlugin);
  vtkMRMLDisplayableNode* node = d->vectorFieldNode(qSlicerSubjectHierarchyPluginHandler::instance()->currentItem());
  if (!node)
  {
    qCritical() << Q_FUNC_INFO << ": Current item has no vector field";
    return;
  }
  qSlicerAbstractModuleWidget* moduleWidget = qSlicerSubjectHierarchyAbstractPlugin::switchToModule("VectorFieldRendering");
  if (moduleWidget)
  {
    moduleWidget->setEditedNode(node);
  }
}

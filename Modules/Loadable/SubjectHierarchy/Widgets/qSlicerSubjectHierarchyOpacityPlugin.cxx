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
  and was supported through CANARIE's Research Software Program, and Cancer
  Care Ontario.

==============================================================================*/

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyOpacityPlugin.h"

// SubjectHierarchy logic includes
#include "vtkSlicerSubjectHierarchyModuleLogic.h"

// Slicer includes
#include "qSlicerCoreApplication.h"
#include "vtkSlicerApplicationLogic.h"

// MRML includes
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLDisplayNode.h>
#include "vtkMRMLScalarVolumeNode.h"

// CTK includes
#include "ctkDoubleSlider.h"

// Qt includes
#include <QAction>
#include <QDebug>
#include <QMenu>
#include <QWidgetAction>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy_Widgets
class qSlicerSubjectHierarchyOpacityPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyOpacityPlugin);
protected:
  qSlicerSubjectHierarchyOpacityPlugin* const q_ptr;
public:
  qSlicerSubjectHierarchyOpacityPluginPrivate(qSlicerSubjectHierarchyOpacityPlugin& object);
  ~qSlicerSubjectHierarchyOpacityPluginPrivate() override;
  void init();
public:
  QAction* OpacityAction;
  QMenu* OpacityMenu;
  ctkDoubleSlider* OpacitySlider;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyOpacityPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyOpacityPluginPrivate::qSlicerSubjectHierarchyOpacityPluginPrivate(qSlicerSubjectHierarchyOpacityPlugin& object)
: q_ptr(&object)
{
  this->OpacityAction = nullptr;
  this->OpacityMenu = nullptr;
  this->OpacitySlider = nullptr;
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyOpacityPluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchyOpacityPlugin);

  this->OpacityMenu = new QMenu(tr("Opacity"));
  this->OpacitySlider = new ctkDoubleSlider(this->OpacityMenu);
  this->OpacitySlider->setOrientation(Qt::Horizontal);
  this->OpacitySlider->setRange(0.0, 1.0);
  this->OpacitySlider->setSingleStep(0.1);
  QObject::connect(this->OpacitySlider, SIGNAL(valueChanged(double)), q, SLOT(setOpacityForCurrentItem(double)));
  QWidgetAction* opacityAction = new QWidgetAction(this->OpacityMenu);
  opacityAction->setDefaultWidget(this->OpacitySlider);
  this->OpacityMenu->addAction(opacityAction);

  this->OpacityAction = new QAction("Opacity",q);
  this->OpacityAction->setToolTip("Set item opacity in the sub-menu");
  this->OpacityAction->setMenu(this->OpacityMenu);
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyOpacityPluginPrivate::~qSlicerSubjectHierarchyOpacityPluginPrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyOpacityPlugin methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyOpacityPlugin::qSlicerSubjectHierarchyOpacityPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchyOpacityPluginPrivate(*this) )
{
  this->m_Name = QString("Opacity");

  Q_D(qSlicerSubjectHierarchyOpacityPlugin);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyOpacityPlugin::~qSlicerSubjectHierarchyOpacityPlugin() = default;

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyOpacityPlugin::visibilityContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchyOpacityPlugin);

  QList<QAction*> actions;
  actions << d->OpacityAction;
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyOpacityPlugin::showVisibilityContextMenuActionsForItem(vtkIdType itemID)
{
  Q_D(qSlicerSubjectHierarchyOpacityPlugin);

  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  if (!itemID || itemID == shNode->GetSceneItemID())
    {
    // There are no scene actions in this plugin
    return;
    }

  // Show opacity for every non-scene items with display node
  vtkMRMLDisplayNode* displayNode = nullptr;
  vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(displayableNode);
  if (displayableNode)
    {
    displayNode = displayableNode->GetDisplayNode();
    }
  else
    {
    // Folder nodes may have display nodes directly associated
    displayNode = vtkMRMLDisplayNode::SafeDownCast(shNode->GetItemDataNode(itemID));
    }

  if (displayNode)
    {
    d->OpacitySlider->setValue(displayNode->GetOpacity());
    }

  // Show opacity action if there is a valid display node and if the node is not a volume
  d->OpacityAction->setVisible(displayNode && !volumeNode);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyOpacityPlugin::setOpacityForCurrentItem(double opacity)
{
  // Get currently selected node and scene
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }
  vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  if (!currentItemID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current subject hierarchy item!";
    return;
    }

  // Get display node
  vtkMRMLDisplayNode* displayNode = nullptr;
  vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(shNode->GetItemDataNode(currentItemID));
  if (displayableNode)
    {
    displayNode = displayableNode->GetDisplayNode();
    }
  else
    {
    // Folder nodes may have display nodes directly associated
    displayNode = vtkMRMLDisplayNode::SafeDownCast(shNode->GetItemDataNode(currentItemID));
    }
  if (!displayNode)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to find display node for subject hierarchy item " << shNode->GetItemName(currentItemID).c_str();
    return;
    }

  displayNode->SetOpacity(opacity);
}

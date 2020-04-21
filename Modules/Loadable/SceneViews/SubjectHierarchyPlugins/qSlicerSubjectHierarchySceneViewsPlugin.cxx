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

// SubjectHierarchy MRML includes
#include "vtkMRMLSubjectHierarchyNode.h"
#include "vtkMRMLSubjectHierarchyConstants.h"

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchySceneViewsPlugin.h"

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSceneViewNode.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// Qt includes
#include <QDebug>
#include <QStandardItem>
#include <QAction>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy_Widgets
class qSlicerSubjectHierarchySceneViewsPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchySceneViewsPlugin);
protected:
  qSlicerSubjectHierarchySceneViewsPlugin* const q_ptr;
public:
  qSlicerSubjectHierarchySceneViewsPluginPrivate(qSlicerSubjectHierarchySceneViewsPlugin& object);
  ~qSlicerSubjectHierarchySceneViewsPluginPrivate() override;
  void init();
public:
  QIcon SceneViewIcon;

  QAction* RestoreSceneViewAction;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchySceneViewsPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchySceneViewsPluginPrivate::qSlicerSubjectHierarchySceneViewsPluginPrivate(qSlicerSubjectHierarchySceneViewsPlugin& object)
: q_ptr(&object)
{
  this->SceneViewIcon = QIcon(":Icons/SceneView.png");

  this->RestoreSceneViewAction = nullptr;
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchySceneViewsPluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchySceneViewsPlugin);

  this->RestoreSceneViewAction = new QAction("Restore scene view",q);
  QObject::connect(this->RestoreSceneViewAction, SIGNAL(triggered()), q, SLOT(restoreCurrentSceneView()));
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchySceneViewsPluginPrivate::~qSlicerSubjectHierarchySceneViewsPluginPrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchySceneViewsPlugin methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchySceneViewsPlugin::qSlicerSubjectHierarchySceneViewsPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchySceneViewsPluginPrivate(*this) )
{
  this->m_Name = QString("SceneViews");

  Q_D(qSlicerSubjectHierarchySceneViewsPlugin);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchySceneViewsPlugin::~qSlicerSubjectHierarchySceneViewsPlugin() = default;

//----------------------------------------------------------------------------
double qSlicerSubjectHierarchySceneViewsPlugin::canAddNodeToSubjectHierarchy(
  vtkMRMLNode* node, vtkIdType parentItemID/*=vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID*/)const
{
  Q_UNUSED(parentItemID);
  if (!node)
    {
    qCritical() << Q_FUNC_INFO << ": Input node is nullptr!";
    return 0.0;
    }
  else if (node->IsA("vtkMRMLSceneViewNode"))
    {
    // Node is a scene view
    return 1.0;
    }

  return 0.0;
}

//---------------------------------------------------------------------------
double qSlicerSubjectHierarchySceneViewsPlugin::canOwnSubjectHierarchyItem(vtkIdType itemID)const
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

  // Scene view
  vtkMRMLNode* associatedNode = shNode->GetItemDataNode(itemID);
  if (associatedNode && associatedNode->IsA("vtkMRMLSceneViewNode"))
    {
    return 1.0;
    }

  return 0.0;
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchySceneViewsPlugin::roleForPlugin()const
{
  return "SceneView";
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchySceneViewsPlugin::icon(vtkIdType itemID)
{
  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return QIcon();
    }

  Q_D(qSlicerSubjectHierarchySceneViewsPlugin);

  if (this->canOwnSubjectHierarchyItem(itemID))
    {
    return d->SceneViewIcon;
    }

  // Item unknown by plugin
  return QIcon();
}

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchySceneViewsPlugin::itemContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchySceneViewsPlugin);

  QList<QAction*> actions;
  actions << d->RestoreSceneViewAction;
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchySceneViewsPlugin::showContextMenuActionsForItem(vtkIdType itemID)
{
  Q_D(qSlicerSubjectHierarchySceneViewsPlugin);

  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    // There are no scene actions in this plugin
    return;
    }

  // Show restore scene view action for all scene views
  if (this->canOwnSubjectHierarchyItem(itemID))
    {
    d->RestoreSceneViewAction->setVisible(true);
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchySceneViewsPlugin::restoreCurrentSceneView()const
{
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }
  vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  if (currentItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current item!";
    return;
    }
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid MRML scene!";
    return;
    }

  vtkMRMLSceneViewNode* viewNode = vtkMRMLSceneViewNode::SafeDownCast(shNode->GetItemDataNode(currentItemID));
  if (!viewNode)
    {
    qCritical() << Q_FUNC_INFO << ": Could not get scene view node!";
    return;
    }

  scene->SaveStateForUndo();
  viewNode->RestoreScene();
}

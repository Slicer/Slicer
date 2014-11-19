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
  ~qSlicerSubjectHierarchySceneViewsPluginPrivate();
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

  this->RestoreSceneViewAction = NULL;
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchySceneViewsPluginPrivate::~qSlicerSubjectHierarchySceneViewsPluginPrivate()
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchySceneViewsPlugin::qSlicerSubjectHierarchySceneViewsPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchySceneViewsPluginPrivate(*this) )
{
  this->m_Name = QString("SceneViews");

  Q_D(qSlicerSubjectHierarchySceneViewsPlugin);
  d->init();
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchySceneViewsPluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchySceneViewsPlugin);

  this->RestoreSceneViewAction = new QAction("Restore scene view",q);
  QObject::connect(this->RestoreSceneViewAction, SIGNAL(triggered()), q, SLOT(restoreCurrentSceneView()));
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchySceneViewsPlugin::~qSlicerSubjectHierarchySceneViewsPlugin()
{
}

//----------------------------------------------------------------------------
double qSlicerSubjectHierarchySceneViewsPlugin::canAddNodeToSubjectHierarchy(vtkMRMLNode* node, vtkMRMLSubjectHierarchyNode* parent/*=NULL*/)const
{
  Q_UNUSED(parent);
  if (!node)
    {
    qCritical() << "qSlicerSubjectHierarchySceneViewsPlugin::canAddNodeToSubjectHierarchy: Input node is NULL!";
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
double qSlicerSubjectHierarchySceneViewsPlugin::canOwnSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode* node)const
{
  if (!node)
    {
    qCritical() << "qSlicerSubjectHierarchySceneViewsPlugin::canOwnSubjectHierarchyNode: Input node is NULL!";
    return 0.0;
    }

  // Scene view
  vtkMRMLNode* associatedNode = node->GetAssociatedNode();
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
QIcon qSlicerSubjectHierarchySceneViewsPlugin::icon(vtkMRMLSubjectHierarchyNode* node)
{
  if (!node)
    {
    qCritical() << "qSlicerSubjectHierarchySceneViewsPlugin::icon: NULL node given!";
    return QIcon();
    }

  Q_D(qSlicerSubjectHierarchySceneViewsPlugin);

  if (this->canOwnSubjectHierarchyNode(node))
    {
    return d->SceneViewIcon;
    }

  // Node unknown by plugin
  return QIcon();
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchySceneViewsPlugin::editProperties(vtkMRMLSubjectHierarchyNode* node)
{
  Q_UNUSED(node);

  // Switch to scene views module
  qSlicerSubjectHierarchyAbstractPlugin::switchToModule("SceneViews");
}

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchySceneViewsPlugin::nodeContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchySceneViewsPlugin);

  QList<QAction*> actions;
  actions << d->RestoreSceneViewAction;
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchySceneViewsPlugin::showContextMenuActionsForNode(vtkMRMLSubjectHierarchyNode* node)
{
  Q_D(qSlicerSubjectHierarchySceneViewsPlugin);
  this->hideAllContextMenuActions();

  if (!node)
    {
    // There are no scene actions in this plugin
    return;
    }

  // Show restore scene view action for all scene views
  if (this->canOwnSubjectHierarchyNode(node))
    {
    d->RestoreSceneViewAction->setVisible(true);
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchySceneViewsPlugin::restoreCurrentSceneView()const
{
  vtkMRMLSubjectHierarchyNode* currentNode = qSlicerSubjectHierarchyPluginHandler::instance()->currentNode();
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->scene();
  if (!scene || !currentNode || !currentNode->GetAssociatedNode())
    {
    qCritical() << "qSlicerSubjectHierarchySceneViewsPlugin::restoreCurrentSceneView: Invalid current node or MRML scene!";
    return;
    }

  vtkMRMLSceneViewNode* viewNode = vtkMRMLSceneViewNode::SafeDownCast(currentNode->GetAssociatedNode());
  if (!viewNode)
    {
    qCritical() << "qSlicerSubjectHierarchySceneViewsPlugin::restoreCurrentSceneView: Could not get sceneView node!";
    return;
    }

  scene->SaveStateForUndo();
  viewNode->RestoreScene();
}

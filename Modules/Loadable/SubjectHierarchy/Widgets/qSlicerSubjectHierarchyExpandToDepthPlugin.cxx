/*==============================================================================

  Program: 3D Slicer

  Copyright (c) EBATINCA, S.L., Las Palmas de Gran Canaria, Spain.
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, EBATINCA, S.L.

==============================================================================*/

// SubjectHierarchy MRML includes
#include "vtkMRMLSubjectHierarchyNode.h"

// SubjectHierarchy Widgets includes
#include "qMRMLSubjectHierarchyTreeView.h"
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyExpandToDepthPlugin.h"
#include "qSlicerSubjectHierarchyFolderPlugin.h"

// Qt includes
#include <QAction>
#include <QDebug>
#include <QMenu>

//-----------------------------------------------------------------------------
class qSlicerSubjectHierarchyExpandToDepthPluginPrivate : public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyExpandToDepthPlugin);

protected:
  qSlicerSubjectHierarchyExpandToDepthPlugin* const q_ptr;

public:
  qSlicerSubjectHierarchyExpandToDepthPluginPrivate(qSlicerSubjectHierarchyExpandToDepthPlugin& object);
  ~qSlicerSubjectHierarchyExpandToDepthPluginPrivate() override;
  void init();

public:
  QAction* ExpandToDepthAction;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyExpandToDepthPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyExpandToDepthPluginPrivate::qSlicerSubjectHierarchyExpandToDepthPluginPrivate(
  qSlicerSubjectHierarchyExpandToDepthPlugin& object)
  : q_ptr(&object)
  , ExpandToDepthAction(nullptr)
{
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyExpandToDepthPluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchyExpandToDepthPlugin);

  // Set up expand to level action and its menu
  this->ExpandToDepthAction = new QAction(qMRMLSubjectHierarchyTreeView::tr("Expand tree to level..."), q);
  qSlicerSubjectHierarchyAbstractPlugin::setActionPosition(
    this->ExpandToDepthAction, qSlicerSubjectHierarchyAbstractPlugin::SectionFolder, 10);
  // sceneMenuActions.append(this->ExpandToDepthAction);

  QMenu* expandToDepthSubMenu = new QMenu();
  this->ExpandToDepthAction->setMenu(expandToDepthSubMenu);
  QAction* expandToDepth_1 = new QAction("1", q);
  QObject::connect(expandToDepth_1, SIGNAL(triggered()), q, SLOT(expandToDepthFromContextMenu()));
  expandToDepthSubMenu->addAction(expandToDepth_1);
  this->ExpandToDepthAction->setMenu(expandToDepthSubMenu);
  QAction* expandToDepth_2 = new QAction("2", q);
  QObject::connect(expandToDepth_2, SIGNAL(triggered()), q, SLOT(expandToDepthFromContextMenu()));
  expandToDepthSubMenu->addAction(expandToDepth_2);
  this->ExpandToDepthAction->setMenu(expandToDepthSubMenu);
  QAction* expandToDepth_3 = new QAction("3", q);
  QObject::connect(expandToDepth_3, SIGNAL(triggered()), q, SLOT(expandToDepthFromContextMenu()));
  expandToDepthSubMenu->addAction(expandToDepth_3);
  this->ExpandToDepthAction->setMenu(expandToDepthSubMenu);
  QAction* expandToDepth_4 = new QAction("4", q);
  QObject::connect(expandToDepth_4, SIGNAL(triggered()), q, SLOT(expandToDepthFromContextMenu()));
  expandToDepthSubMenu->addAction(expandToDepth_4);
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyExpandToDepthPluginPrivate::~qSlicerSubjectHierarchyExpandToDepthPluginPrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyExpandToDepthPlugin methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyExpandToDepthPlugin::qSlicerSubjectHierarchyExpandToDepthPlugin(QObject* parent)
  : Superclass(parent)
  , d_ptr(new qSlicerSubjectHierarchyExpandToDepthPluginPrivate(*this))
{
  this->m_Name = QString("ExpandToDepth");

  Q_D(qSlicerSubjectHierarchyExpandToDepthPlugin);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyExpandToDepthPlugin::~qSlicerSubjectHierarchyExpandToDepthPlugin() = default;

//-----------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyExpandToDepthPlugin::sceneContextMenuActions() const
{
  Q_D(const qSlicerSubjectHierarchyExpandToDepthPlugin);

  QList<QAction*> actions;
  actions << d->ExpandToDepthAction;
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyExpandToDepthPlugin::showContextMenuActionsForItem(vtkIdType itemID)
{
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
  }

  Q_D(qSlicerSubjectHierarchyExpandToDepthPlugin);

  // Scene
  if (itemID == shNode->GetSceneItemID())
  {
    d->ExpandToDepthAction->setVisible(true);
  }
}

//--------------------------------------------------------------------------
void qSlicerSubjectHierarchyExpandToDepthPlugin::expandToDepthFromContextMenu()
{
  QAction* senderAction = qobject_cast<QAction*>(this->sender());
  if (!senderAction)
  {
    qCritical() << Q_FUNC_INFO << ": Unable to get sender action";
    return;
  }

  int depth = senderAction->text().toInt();

  qMRMLSubjectHierarchyTreeView* currentTreeView = qSlicerSubjectHierarchyPluginHandler::instance()->currentTreeView();
  if (currentTreeView)
  {
    currentTreeView->expandToDepth(depth);
  }
}

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
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyCloneNodePlugin.h"

// SubjectHierarchy logic includes
#include "vtkSlicerSubjectHierarchyModuleLogic.h"

// Slicer includes
#include "qSlicerCoreApplication.h"
#include "vtkSlicerApplicationLogic.h"

// VTK includes
#include <vtkObjectFactory.h>

// Qt includes
#include <QDebug>
#include <QStandardItem>
#include <QAction>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy_Widgets
class qSlicerSubjectHierarchyCloneNodePluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyCloneNodePlugin);
protected:
  qSlicerSubjectHierarchyCloneNodePlugin* const q_ptr;
public:
  qSlicerSubjectHierarchyCloneNodePluginPrivate(qSlicerSubjectHierarchyCloneNodePlugin& object);
  ~qSlicerSubjectHierarchyCloneNodePluginPrivate() override;
  void init();
public:
  QAction* CloneItemAction;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyCloneNodePluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyCloneNodePluginPrivate::qSlicerSubjectHierarchyCloneNodePluginPrivate(qSlicerSubjectHierarchyCloneNodePlugin& object)
: q_ptr(&object)
{
  this->CloneItemAction = nullptr;
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyCloneNodePluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchyCloneNodePlugin);

  this->CloneItemAction = new QAction("Clone",q);
  this->CloneItemAction->setToolTip("Clone this item and its data node if any along with display and storage options");
  QObject::connect(this->CloneItemAction, SIGNAL(triggered()), q, SLOT(cloneCurrentItem()));
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyCloneNodePluginPrivate::~qSlicerSubjectHierarchyCloneNodePluginPrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyCloneNodePlugin methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyCloneNodePlugin::qSlicerSubjectHierarchyCloneNodePlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchyCloneNodePluginPrivate(*this) )
{
  this->m_Name = QString("CloneNode");

  Q_D(qSlicerSubjectHierarchyCloneNodePlugin);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyCloneNodePlugin::~qSlicerSubjectHierarchyCloneNodePlugin() = default;

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyCloneNodePlugin::itemContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchyCloneNodePlugin);

  QList<QAction*> actions;
  actions << d->CloneItemAction;
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyCloneNodePlugin::showContextMenuActionsForItem(vtkIdType itemID)
{
  Q_D(qSlicerSubjectHierarchyCloneNodePlugin);

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

  // Show clone node for every non-scene items
  d->CloneItemAction->setVisible(true);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyCloneNodePlugin::cloneCurrentItem()
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

  vtkIdType clonedItemID = vtkSlicerSubjectHierarchyModuleLogic::CloneSubjectHierarchyItem(shNode, currentItemID);
  if (!clonedItemID)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to clone subject hierarchy item (ID:"
        << currentItemID << ", name:" << shNode->GetItemName(currentItemID).c_str() << ")";
    }

  // Trigger update
  emit requestInvalidateFilter();
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchyCloneNodePlugin::getCloneNodeNamePostfix()
{
  return QString(vtkSlicerSubjectHierarchyModuleLogic::CLONED_NODE_NAME_POSTFIX);
}

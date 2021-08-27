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
#include "qSlicerSubjectHierarchyExportPlugin.h"

// SubjectHierarchy logic includes
#include "vtkSlicerSubjectHierarchyModuleLogic.h"

// Slicer includes
#include "qSlicerApplication.h"
#include "qSlicerIOManager.h"
#include "vtkSlicerApplicationLogic.h"

// VTK includes
#include <vtkObjectFactory.h>

// Qt includes
#include <QDebug>
#include <QStandardItem>
#include <QAction>

// MRML includes
#include <vtkMRMLStorableNode.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy_Widgets
class qSlicerSubjectHierarchyExportPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyExportPlugin);
protected:
  qSlicerSubjectHierarchyExportPlugin* const q_ptr;
public:
  qSlicerSubjectHierarchyExportPluginPrivate(qSlicerSubjectHierarchyExportPlugin& object);
  ~qSlicerSubjectHierarchyExportPluginPrivate() override;
  void init();
public:
  QAction* ExportItemAction;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyExportPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyExportPluginPrivate::qSlicerSubjectHierarchyExportPluginPrivate(qSlicerSubjectHierarchyExportPlugin& object)
: q_ptr(&object)
{
  this->ExportItemAction = nullptr;
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyExportPluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchyExportPlugin);

  this->ExportItemAction = new QAction("Export to file...",q);
  this->ExportItemAction->setToolTip("Export this node to a file");
  // Put towards end of the list, where export features typically would go
  qSlicerSubjectHierarchyAbstractPlugin::setActionPosition(this->ExportItemAction,
    qSlicerSubjectHierarchyAbstractPlugin::SectionDefault, 3);
  QObject::connect(this->ExportItemAction, SIGNAL(triggered()), q, SLOT(exportCurrentItem()));
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyExportPluginPrivate::~qSlicerSubjectHierarchyExportPluginPrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyExportPlugin methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyExportPlugin::qSlicerSubjectHierarchyExportPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchyExportPluginPrivate(*this) )
{
  this->m_Name = QString("Export");

  Q_D(qSlicerSubjectHierarchyExportPlugin);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyExportPlugin::~qSlicerSubjectHierarchyExportPlugin() = default;

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyExportPlugin::itemContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchyExportPlugin);

  QList<QAction*> actions;
  actions << d->ExportItemAction;
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyExportPlugin::showContextMenuActionsForItem(vtkIdType itemID)
{
  Q_D(qSlicerSubjectHierarchyExportPlugin);

  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  if (!itemID || itemID == shNode->GetSceneItemID())
    {
    return; // no export for scene node
    }

  vtkIdType parentItemID = shNode->GetItemParent(itemID);
  if (parentItemID && shNode->IsItemVirtualBranchParent(parentItemID))
    {
    return; // no export for virtual branch items
    }

  vtkMRMLNode* node = shNode->GetItemDataNode(itemID);
  vtkMRMLStorableNode* storableNode = vtkMRMLStorableNode::SafeDownCast(node);

  if (!storableNode)
    {
    return; // only export storable nodes
    }


  d->ExportItemAction->setVisible(true);

}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyExportPlugin::exportCurrentItem()
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

  vtkMRMLNode* node = shNode->GetItemDataNode(currentItemID);
  if (!node)
    {
    qCritical() << Q_FUNC_INFO << ": No node associated to current subject hierarchy item ID!";
    return;
    }

  qSlicerIO::IOProperties properties{};
  properties["nodeID"] = QString(node->GetID());

  qSlicerApplication::application()->ioManager()->openDialog(
    QString("GenericNodeExport"),
    qSlicerFileDialog::Write,
    properties
  );

}

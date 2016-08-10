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
#include "qSlicerSubjectHierarchyCloneNodePlugin.h"

// SubjectHierarchy logic includes
#include "vtkSlicerSubjectHierarchyModuleLogic.h"

// Slicer includes
#include "qSlicerCoreApplication.h"
#include "vtkSlicerApplicationLogic.h"

// MRML includes
#include <vtkMRMLNode.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkCollection.h>
#include <vtksys/SystemTools.hxx>

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
  ~qSlicerSubjectHierarchyCloneNodePluginPrivate();
  void init();
public:
  QAction* CloneNodeAction;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyCloneNodePluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyCloneNodePluginPrivate::qSlicerSubjectHierarchyCloneNodePluginPrivate(qSlicerSubjectHierarchyCloneNodePlugin& object)
: q_ptr(&object)
{
  this->CloneNodeAction = NULL;
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyCloneNodePluginPrivate::~qSlicerSubjectHierarchyCloneNodePluginPrivate()
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyCloneNodePlugin::qSlicerSubjectHierarchyCloneNodePlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchyCloneNodePluginPrivate(*this) )
{
  this->m_Name = QString("CloneNode");

  Q_D(qSlicerSubjectHierarchyCloneNodePlugin);
  d->init();
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyCloneNodePluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchyCloneNodePlugin);

  this->CloneNodeAction = new QAction("Clone node",q);
  QObject::connect(this->CloneNodeAction, SIGNAL(triggered()), q, SLOT(cloneCurrentNode()));
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyCloneNodePlugin::~qSlicerSubjectHierarchyCloneNodePlugin()
{
}

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyCloneNodePlugin::nodeContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchyCloneNodePlugin);

  QList<QAction*> actions;
  actions << d->CloneNodeAction;
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyCloneNodePlugin::showContextMenuActionsForNode(vtkMRMLSubjectHierarchyNode* node)
{
  Q_D(qSlicerSubjectHierarchyCloneNodePlugin);
  this->hideAllContextMenuActions();

  if (!node)
    {
    // There are no scene actions in this plugin
    return;
    }

  // Show clone node for every non-scene nodes
  d->CloneNodeAction->setVisible(true);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyCloneNodePlugin::cloneCurrentNode()
{
  // Get currently selected node and scene
  vtkMRMLSubjectHierarchyNode* currentNode = qSlicerSubjectHierarchyPluginHandler::instance()->currentNode();
  if (!currentNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current subject hierarchy node!";
    return;
    }

  vtkMRMLSubjectHierarchyNode* clonedSubjectHierarchyNode = vtkSlicerSubjectHierarchyModuleLogic::CloneSubjectHierarchyNode(currentNode);
  if (!clonedSubjectHierarchyNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to clone subject hierarchy node" << currentNode->GetNameWithoutPostfix().c_str();
    }

  // Trigger update
  emit requestInvalidateFilter();
}

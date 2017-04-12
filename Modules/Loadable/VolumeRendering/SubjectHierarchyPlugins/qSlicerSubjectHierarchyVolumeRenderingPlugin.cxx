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
#include "qSlicerSubjectHierarchyVolumeRenderingPlugin.h"

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"
#include "qSlicerApplication.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeNode.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// Qt includes
#include <QDebug>
#include <QAction>

// MRML widgets includes
#include "qMRMLNodeComboBox.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy_Widgets
class qSlicerSubjectHierarchyVolumeRenderingPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyVolumeRenderingPlugin);
protected:
  qSlicerSubjectHierarchyVolumeRenderingPlugin* const q_ptr;
public:
  qSlicerSubjectHierarchyVolumeRenderingPluginPrivate(qSlicerSubjectHierarchyVolumeRenderingPlugin& object);
  ~qSlicerSubjectHierarchyVolumeRenderingPluginPrivate();
  void init();
public:
  QAction* ShowVolumeRenderingAction;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyVolumeRenderingPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyVolumeRenderingPluginPrivate::qSlicerSubjectHierarchyVolumeRenderingPluginPrivate(qSlicerSubjectHierarchyVolumeRenderingPlugin& object)
: q_ptr(&object)
, ShowVolumeRenderingAction(NULL)
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyVolumeRenderingPluginPrivate::~qSlicerSubjectHierarchyVolumeRenderingPluginPrivate()
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyVolumeRenderingPlugin::qSlicerSubjectHierarchyVolumeRenderingPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchyVolumeRenderingPluginPrivate(*this) )
{
  this->m_Name = QString("VolumeRendering");

  Q_D(qSlicerSubjectHierarchyVolumeRenderingPlugin);
  d->init();
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyVolumeRenderingPluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchyVolumeRenderingPlugin);

  this->ShowVolumeRenderingAction = new QAction("Show volume rendering...",q);
  this->ShowVolumeRenderingAction->setToolTip(tr("Switch to Volume Rendering module and set up rendering"));
  QObject::connect(this->ShowVolumeRenderingAction, SIGNAL(triggered()), q, SLOT(showVolumeRenderingForCurrentItem()));
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyVolumeRenderingPlugin::~qSlicerSubjectHierarchyVolumeRenderingPlugin()
{
}

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyVolumeRenderingPlugin::visibilityContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchyVolumeRenderingPlugin);

  QList<QAction*> actions;
  actions << d->ShowVolumeRenderingAction;
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyVolumeRenderingPlugin::showVisibilityContextMenuActionsForItem(vtkIdType itemID)
{
  Q_D(qSlicerSubjectHierarchyVolumeRenderingPlugin);
  this->hideAllContextMenuActions();

  if (!itemID)
    {
    // There are no scene actions in this plugin
    return;
    }

  // Volume
  if (qSlicerSubjectHierarchyPluginHandler::instance()->pluginByName("Volumes")->canOwnSubjectHierarchyItem(itemID))
    {
    d->ShowVolumeRenderingAction->setVisible(true);
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyVolumeRenderingPlugin::showVolumeRenderingForCurrentItem()
{
  Q_D(qSlicerSubjectHierarchyVolumeRenderingPlugin);

  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  if (!currentItemID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current item";
    return;
    }

  qSlicerAbstractModuleWidget* moduleWidget = qSlicerSubjectHierarchyAbstractPlugin::switchToModule("VolumeRendering"); 
  if (moduleWidget)
    {
    // Get node selector combobox
    qMRMLNodeComboBox* nodeSelector = moduleWidget->findChild<qMRMLNodeComboBox*>("VolumeNodeComboBox");

    // Choose current data node
    if (nodeSelector)
      {
      nodeSelector->setCurrentNode(shNode->GetItemDataNode(currentItemID));
      }
    }
}

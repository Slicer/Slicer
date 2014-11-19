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
#include "qSlicerSubjectHierarchySegmentPlugin.h"

// Slicer includes
#include "qSlicerCoreApplication.h"
#include "vtkSlicerApplicationLogic.h"

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLSliceNode.h>
#include "vtkMRMLLabelMapVolumeDisplayNode.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkNew.h>
#include <vtkImageData.h>
#include <vtkImageThreshold.h>

// Qt includes
#include <QDebug>
#include <QAction>
#include <QMenu>

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"
#include "qSlicerApplication.h"
#include "PythonQt.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy_Widgets
class qSlicerSubjectHierarchySegmentPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchySegmentPlugin);
protected:
  qSlicerSubjectHierarchySegmentPlugin* const q_ptr;
public:
  qSlicerSubjectHierarchySegmentPluginPrivate(qSlicerSubjectHierarchySegmentPlugin& object);
  ~qSlicerSubjectHierarchySegmentPluginPrivate();
  void init();
public:
  QAction* SegmentWithMenuAction;
  QMenu* SegmentWithMenu;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchySegmentPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchySegmentPluginPrivate::qSlicerSubjectHierarchySegmentPluginPrivate(qSlicerSubjectHierarchySegmentPlugin& object)
: q_ptr(&object)
{
  this->SegmentWithMenuAction = NULL;
  this->SegmentWithMenu = NULL;
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchySegmentPluginPrivate::~qSlicerSubjectHierarchySegmentPluginPrivate()
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchySegmentPlugin::qSlicerSubjectHierarchySegmentPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchySegmentPluginPrivate(*this) )
{
  this->m_Name = QString("Segment");

  Q_D(qSlicerSubjectHierarchySegmentPlugin);
  d->init();
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchySegmentPluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchySegmentPlugin);

  // Create top-level segment action and add it to menu
  this->SegmentWithMenuAction = new QAction("Segment this using...",q);

  this->SegmentWithMenu = new QMenu();
  this->SegmentWithMenuAction->setMenu(this->SegmentWithMenu);

  QAction* editorAction = new QAction("Editor",q);
  QObject::connect(editorAction, SIGNAL(triggered()), q, SLOT(segmentCurrentNodeWithEditor()));
  this->SegmentWithMenu->addAction(editorAction);
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchySegmentPlugin::~qSlicerSubjectHierarchySegmentPlugin()
{
}

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchySegmentPlugin::nodeContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchySegmentPlugin);

  QList<QAction*> actions;
  actions << d->SegmentWithMenuAction;
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchySegmentPlugin::showContextMenuActionsForNode(vtkMRMLSubjectHierarchyNode* node)
{
  Q_D(qSlicerSubjectHierarchySegmentPlugin);
  this->hideAllContextMenuActions();

  if (!node)
    {
    // There are no scene actions in this plugin
    return;
    }

  // Volume
  if (qSlicerSubjectHierarchyPluginHandler::instance()->pluginByName("Volumes")->canOwnSubjectHierarchyNode(node))
    {
    // Get current node
    vtkMRMLSubjectHierarchyNode* currentNode = qSlicerSubjectHierarchyPluginHandler::instance()->currentNode();
    if (!currentNode)
      {
      qCritical() << "qSlicerSubjectHierarchySegmentPlugin::showContextMenuActionsForNode: Invalid current node!";
      return;
      }

    d->SegmentWithMenuAction->setVisible(true);
    }
}

//---------------------------------------------------------------------------
QMenu* qSlicerSubjectHierarchySegmentPlugin::segmentWithMenu()
{
  Q_D(qSlicerSubjectHierarchySegmentPlugin);

  return d->SegmentWithMenu;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchySegmentPlugin::segmentCurrentNodeWithEditor()
{
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->scene();
  vtkMRMLSubjectHierarchyNode* currentNode = qSlicerSubjectHierarchyPluginHandler::instance()->currentNode();
  if (!scene || !currentNode || !currentNode->GetAssociatedNode())
    {
    qCritical() << "qSlicerSubjectHierarchySegmentPlugin::segmentCurrentNodeWithEditor: Invalid current node or scene!";
    return;
    }
  vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(currentNode->GetAssociatedNode());
  if (!volumeNode || volumeNode->GetLabelMap())
    {
    qCritical() << "qSlicerSubjectHierarchySegmentPlugin::segmentCurrentNodeWithEditor: Invalid volume node to segment!";
    return;
    }

  // Create new labelmap as merge volume to use for segmentation
  // (copied from vtkSlicerVolumesLogic::CreateAndAddLabelVolume - was unable to use it
  //  because of circular dependencies. Would make sense to outsource (part of) this to
  //  the Editor module once plugins can be written in python) //TODO
  // create a display node
  vtkNew<vtkMRMLLabelMapVolumeDisplayNode> labelDisplayNode;
  scene->AddNode(labelDisplayNode.GetPointer());
  // create a volume node as copy of source volume
  vtkNew<vtkMRMLScalarVolumeNode> labelNode;
  labelNode->CopyWithScene(volumeNode);
  labelNode->RemoveAllDisplayNodeIDs();
  labelNode->SetAndObserveStorageNodeID(NULL);
  labelNode->SetLabelMap(1);
  // associate it with the source volume
  labelNode->SetAttribute("AssociatedNodeID", volumeNode->GetID());
  labelDisplayNode->SetAndObserveColorNodeID("vtkMRMLColorTableNodeFileGenericAnatomyColors.txt");
  QString labelmapName = QString("%1-label").arg(volumeNode->GetName());
  labelNode->SetName(labelmapName.toLatin1().constData());
  labelNode->SetAndObserveDisplayNodeID( labelDisplayNode->GetID() );
  // make an image data of the same size and shape as the input volume,
  // but filled with zeros
  vtkNew<vtkImageThreshold> thresh;
  thresh->ReplaceInOn();
  thresh->ReplaceOutOn();
  thresh->SetInValue(0);
  thresh->SetOutValue(0);
  thresh->SetOutputScalarType (VTK_SHORT);
#if (VTK_MAJOR_VERSION <= 5)
  thresh->SetInput( volumeNode->GetImageData() );
  thresh->GetOutput()->Update();
  vtkNew<vtkImageData> imageData;
  imageData->DeepCopy( thresh->GetOutput() );
  labelNode->SetAndObserveImageData( imageData.GetPointer() );
#else
  thresh->SetInputData(volumeNode->GetImageData());
  thresh->Update();
  labelNode->SetImageDataConnection( thresh->GetOutputPort() );
#endif
  // add the label volume to the scene
  scene->AddNode(labelNode.GetPointer());

  // Set volume to segment as background volume, foreground to none, and labelmap to new labelmap created for segmentation
  vtkMRMLSelectionNode* selectionNode = qSlicerCoreApplication::application()->applicationLogic()->GetSelectionNode();
  if (!selectionNode)
    {
    qCritical() << "qSlicerSubjectHierarchySegmentPlugin::segmentCurrentNodeWithEditor: Unable to get selection node";
    return;
    }
  vtkMRMLSliceCompositeNode* compositeNode = NULL;
  const int numberOfCompositeNodes = scene->GetNumberOfNodesByClass("vtkMRMLSliceCompositeNode");
  for (int i=0; i<numberOfCompositeNodes; i++)
    {
    compositeNode = vtkMRMLSliceCompositeNode::SafeDownCast ( scene->GetNthNodeByClass( i, "vtkMRMLSliceCompositeNode" ) );
    if (compositeNode)
      {
      compositeNode->SetBackgroundVolumeID(volumeNode->GetID());
      compositeNode->SetForegroundVolumeID(NULL);
      compositeNode->SetLabelVolumeID(labelNode->GetID());
      }
    }

  // Switch to Editor module
  qSlicerAbstractModuleWidget* moduleWidget = qSlicerSubjectHierarchyAbstractPlugin::switchToModule("Editor");

  // Set master and merge volume in Editor
  if (moduleWidget)
    {
    PythonQt::init();
    PythonQtObjectPtr context = PythonQt::self()->getMainModule();
    context.evalScript( QString(
      "masterNode = slicer.mrmlScene.GetNodeByID(%1)"
      "mergeNode = slicer.mrmlScene.GetNodeByID(%2)"
      "editorWidget = slicer.modules.editor.widgetRepresentation().self()"
      "editorWidget.setMasterNode(masterNode)"
      "editorWidget.setMergeNode(mergeNode)" )
      .arg(currentNode->GetAssociatedNode()->GetID()).arg(labelNode->GetID()) );
    }
}

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

==============================================================================*/

// SubjectHierarchy MRML includes
#include "vtkMRMLSubjectHierarchyConstants.h"
#include "vtkMRMLSubjectHierarchyNode.h"
#include "vtkSlicerVolumesLogic.h"

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyViewContextMenuPlugin.h"
#include "qSlicerSubjectHierarchyDefaultPlugin.h"

// Qt includes
#include <QAction>
#include <QMenu>
#include <QClipboard>
#include <QDebug>
#include <QVariant>

// MRML includes
#include <qMRMLSliceView.h>
#include <qMRMLSliceWidget.h>
#include <qMRMLThreeDView.h>
#include <qMRMLThreeDWidget.h>
#include <vtkMRMLAbstractViewNode.h>
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLWindowLevelWidget.h>
#include <vtkMRMLSliceCompositeNode.h>

// Slicer includes
#include <qSlicerApplication.h>
#include <qSlicerLayoutManager.h>
#include "qSlicerAbstractCoreModule.h"
#include "qSlicerModuleManager.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>


// CTK includes
#include "ctkSignalMapper.h"
#include "ctkVTKWidgetsUtils.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy_Widgets
class qSlicerSubjectHierarchyViewContextMenuPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyViewContextMenuPlugin);
protected:
  qSlicerSubjectHierarchyViewContextMenuPlugin* const q_ptr;
public:
  qSlicerSubjectHierarchyViewContextMenuPluginPrivate(qSlicerSubjectHierarchyViewContextMenuPlugin& object);
  ~qSlicerSubjectHierarchyViewContextMenuPluginPrivate() override;
  void init();
public:
  ctkSignalMapper* InteractionModeMapper;
  ctkSignalMapper* PresetModeMapper;
  QMenu*   PresetSubmenu = nullptr;
  QAction* InteractionModeViewTransformAction = nullptr;
  QAction* InteractionModeAdjustWindowLevelAction = nullptr;
  QAction* InteractionModePlaceAction = nullptr;

  QAction* MaximizeViewAction = nullptr;
  QAction* CopyImageAction = nullptr;
  QAction* ConfigureSliceViewAnnotationsAction = nullptr;

  QAction* Separator =  nullptr;
  QAction* CTBonePresetAction = nullptr;
  QAction* CTAirPresetAction = nullptr;
  QAction* PETPresetAction = nullptr;
  QAction* CTAbdomenPresetAction = nullptr;
  QAction* CTBrainPresetAction = nullptr;
  QAction* CTLungPresetAction = nullptr;
  QAction* DTIPresetAction = nullptr;
  QAction* HandlePresetAction = nullptr;

  vtkWeakPointer<vtkMRMLInteractionNode> InteractionNode;
  vtkWeakPointer<vtkMRMLAbstractViewNode> ViewNode;
  vtkWeakPointer<vtkMRMLLayoutNode> LayoutNode;
  vtkMRMLNode*  BackgroundVolumeNode;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyViewContextMenuPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyViewContextMenuPluginPrivate::qSlicerSubjectHierarchyViewContextMenuPluginPrivate(qSlicerSubjectHierarchyViewContextMenuPlugin& object)
: q_ptr(&object)
{
}


//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyViewContextMenuPluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchyViewContextMenuPlugin);

  // Interaction mode

  this->InteractionModeViewTransformAction = new QAction("View transform", q);
  this->InteractionModeViewTransformAction->setObjectName("MouseModeViewTransformAction");
  this->InteractionModeViewTransformAction->setCheckable(true);
  qSlicerSubjectHierarchyAbstractPlugin::setActionPosition(this->InteractionModeViewTransformAction,
  qSlicerSubjectHierarchyAbstractPlugin::SectionInteraction, 0);

  this->InteractionModeAdjustWindowLevelAction = new QAction("Adjust window/level", q);
  this->InteractionModeAdjustWindowLevelAction->setObjectName("MouseModeAdjustWindowLevelAction");
  this->InteractionModeAdjustWindowLevelAction->setCheckable(true);
  qSlicerSubjectHierarchyAbstractPlugin::setActionPosition(this->InteractionModeAdjustWindowLevelAction,
  qSlicerSubjectHierarchyAbstractPlugin::SectionInteraction, 1);

  this->InteractionModePlaceAction = new QAction("Place", q);
  this->InteractionModePlaceAction->setObjectName("MouseModePlaceAction");
  this->InteractionModePlaceAction->setCheckable(true);
  qSlicerSubjectHierarchyAbstractPlugin::setActionPosition(this->InteractionModePlaceAction,
  qSlicerSubjectHierarchyAbstractPlugin::SectionInteraction, 2);

  QActionGroup* interactionModeActions = new QActionGroup(q);
  interactionModeActions->setExclusive(true);

  interactionModeActions->addAction(this->InteractionModeViewTransformAction);
  interactionModeActions->addAction(this->InteractionModeAdjustWindowLevelAction);
  interactionModeActions->addAction(this->InteractionModePlaceAction);

  this->InteractionModeMapper = new ctkSignalMapper(q);
  this->InteractionModeMapper->setMapping(this->InteractionModeViewTransformAction, vtkMRMLInteractionNode::ViewTransform);
  this->InteractionModeMapper->setMapping(this->InteractionModeAdjustWindowLevelAction, vtkMRMLInteractionNode::AdjustWindowLevel);
  this->InteractionModeMapper->setMapping(this->InteractionModePlaceAction, vtkMRMLInteractionNode::Place);
  QObject::connect(interactionModeActions, SIGNAL(triggered(QAction*)), this->InteractionModeMapper, SLOT(map(QAction*)));
  QObject::connect(this->InteractionModeMapper, SIGNAL(mapped(int)), q, SLOT(setInteractionMode(int)));

    // Other

  this->MaximizeViewAction = new QAction(tr("Maximize view"), q);
  this->MaximizeViewAction->setObjectName("MaximizeViewAction");
  this->MaximizeViewAction->setToolTip(tr("Show this view maximized in the view layout"));
  qSlicerSubjectHierarchyAbstractPlugin::setActionPosition(this->MaximizeViewAction,
  qSlicerSubjectHierarchyAbstractPlugin::SectionDefault, 0);
  QObject::connect(this->MaximizeViewAction, SIGNAL(triggered()), q, SLOT(maximizeView()));

  this->CopyImageAction = new QAction(tr("Copy image"), q);
  this->CopyImageAction->setObjectName("CopyImageAction");
  this->CopyImageAction->setToolTip(tr("Copy a screenshot of this view to the clipboard"));
  qSlicerSubjectHierarchyAbstractPlugin::setActionPosition(this->CopyImageAction,
  qSlicerSubjectHierarchyAbstractPlugin::SectionDefault, 1);
  QObject::connect(this->CopyImageAction, SIGNAL(triggered()), q, SLOT(saveScreenshot()));

  this->ConfigureSliceViewAnnotationsAction = new QAction(tr("Configure slice view annotations..."), q);
  this->ConfigureSliceViewAnnotationsAction->setObjectName("ConfigureSliceViewAnnotationsAction");
  this->ConfigureSliceViewAnnotationsAction->setToolTip(tr("Configures display of corner annotations and color bar."));
  qSlicerSubjectHierarchyAbstractPlugin::setActionPosition(this->ConfigureSliceViewAnnotationsAction,
  qSlicerSubjectHierarchyAbstractPlugin::SectionDefault, 2);
  QObject::connect(this->ConfigureSliceViewAnnotationsAction, SIGNAL(triggered()), q, SLOT(configureSliceViewAnnotationsAction()));


    // add volume presets

  this->Separator = new QAction(tr(""), q);
  this->Separator->setObjectName("Separator");
  this->Separator->setToolTip(tr(""));
  this->Separator->setSeparator(true);
  qSlicerSubjectHierarchyAbstractPlugin::setActionPosition(this->Separator,
        qSlicerSubjectHierarchyAbstractPlugin::SectionDefault, 3);

  this->CTBonePresetAction = new QAction(tr("CT Bone"));
  this->CTBonePresetAction->setObjectName("CTBonePresetAction");
  this->CTBonePresetAction->setToolTip(tr("Display slices with CT bone preset"));

  this->CTAirPresetAction = new QAction(tr("CT Air"));
  this->CTAirPresetAction->setObjectName("CTAirPresetAction");
  this->CTAirPresetAction->setToolTip(tr("Display slices with CT air preset"));

  this->PETPresetAction = new QAction(tr("PET"));
  this->PETPresetAction->setObjectName("CTPETPresetAction");
  this->PETPresetAction->setToolTip(tr("Display slices with PET preset"));

  this->CTAbdomenPresetAction = new QAction(tr("CT Abdomen"));
  this->CTAbdomenPresetAction->setObjectName("CTAbdomenPresetAction");
  this->CTAbdomenPresetAction->setToolTip(tr("Display slices with CT abdomen preset"));

  this->CTBrainPresetAction = new QAction(tr("CT Brain"));
  this->CTBrainPresetAction->setObjectName("CTBrainPresetAction");
  this->CTBrainPresetAction->setToolTip(tr("Display slices with CT brain preset"));

  this->CTLungPresetAction = new QAction(tr("CT Lung"));
  this->CTLungPresetAction->setObjectName("CTLungPresetAction");
  this->CTLungPresetAction->setToolTip(tr("Display slices with CT lung preset"));

  this->DTIPresetAction = new QAction(tr("DTI"), q);
  this->DTIPresetAction->setObjectName("DTIPresetAction");
  this->DTIPresetAction->setToolTip(tr("Display slices with DTI preset"));

  this->PresetSubmenu = new QMenu();
  this->PresetSubmenu->addAction(this->CTBonePresetAction);
  this->PresetSubmenu->addAction(this->CTAirPresetAction);
  this->PresetSubmenu->addAction(this->PETPresetAction);
  this->PresetSubmenu->addAction(this->CTAbdomenPresetAction);
  this->PresetSubmenu->addAction(this->CTBrainPresetAction);
  this->PresetSubmenu->addAction(this->CTLungPresetAction);
  this->PresetSubmenu->addAction(this->DTIPresetAction);

  this->HandlePresetAction = new QAction("Window/level presets");
  this->HandlePresetAction->setObjectName("HandlePresetOptions");
  q->setActionPosition(this->HandlePresetAction, 4);
  this->HandlePresetAction->setMenu(this->PresetSubmenu);

  QActionGroup* presetModeActions = new QActionGroup(q);
  presetModeActions->setExclusive(true);

  presetModeActions->addAction(this->CTBonePresetAction);
  presetModeActions->addAction(this->CTAirPresetAction);
  presetModeActions->addAction(this->PETPresetAction);
  presetModeActions->addAction(this->CTAbdomenPresetAction);
  presetModeActions->addAction(this->CTBrainPresetAction);
  presetModeActions->addAction(this->CTLungPresetAction);
  presetModeActions->addAction(this->DTIPresetAction);

  this->PresetModeMapper = new ctkSignalMapper(q);
  this->PresetModeMapper->setMapping(this->CTBonePresetAction, q->ID_CTBone);
  this->PresetModeMapper->setMapping(this->CTAirPresetAction, q->ID_CTAir);
  this->PresetModeMapper->setMapping(this->PETPresetAction, q->ID_PET);
  this->PresetModeMapper->setMapping(this->CTAbdomenPresetAction, q->ID_CTAbdomen);
  this->PresetModeMapper->setMapping(this->CTLungPresetAction, q->ID_CTLung);
  this->PresetModeMapper->setMapping(this->CTBrainPresetAction, q->ID_CTBrain);
  this->PresetModeMapper->setMapping(this->DTIPresetAction, q->ID_DTI);
  QObject::connect(presetModeActions, SIGNAL(triggered(QAction*)), this->PresetModeMapper, SLOT(map(QAction*)));
  QObject::connect(this->PresetModeMapper, SIGNAL(mapped(int)), q, SLOT(setVolumePreset(int)));
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyViewContextMenuPluginPrivate::~qSlicerSubjectHierarchyViewContextMenuPluginPrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyViewContextMenuPlugin methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyViewContextMenuPlugin::qSlicerSubjectHierarchyViewContextMenuPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchyViewContextMenuPluginPrivate(*this) )
{
  this->m_Name = QString("ViewContextMenu");

  Q_D(qSlicerSubjectHierarchyViewContextMenuPlugin);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyViewContextMenuPlugin::~qSlicerSubjectHierarchyViewContextMenuPlugin() = default;

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyViewContextMenuPlugin::viewContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchyViewContextMenuPlugin);
  QList<QAction*> actions;
  actions << d->InteractionModeViewTransformAction
    << d->InteractionModeAdjustWindowLevelAction
    << d->InteractionModePlaceAction
    << d->MaximizeViewAction
    << d->CopyImageAction
    << d->ConfigureSliceViewAnnotationsAction
    << d->Separator
    << d->HandlePresetAction;
  ;
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyViewContextMenuPlugin::showViewContextMenuActionsForItem(vtkIdType itemID, QVariantMap eventData)
{
  Q_D(qSlicerSubjectHierarchyViewContextMenuPlugin);

  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode || !shNode->GetScene())
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }
  if (itemID != shNode->GetSceneItemID())
    {
    return;
    }
  if (!eventData.contains("ViewNodeID"))
    {
    return;
    }
  vtkMRMLAbstractViewNode* viewNode = vtkMRMLAbstractViewNode::SafeDownCast(
    shNode->GetScene()->GetNodeByID(eventData["ViewNodeID"].toString().toStdString()));
  if (!viewNode)
    {
    return;
    }
  vtkMRMLInteractionNode* interactionNode = viewNode->GetInteractionNode();
  if (!interactionNode)
    {
    return;
    }

  d->InteractionModeViewTransformAction->setVisible(true);
  d->InteractionModeAdjustWindowLevelAction->setVisible(true);
  d->InteractionModePlaceAction->setVisible(true);

  int interactionMode = interactionNode->GetCurrentInteractionMode();

  bool wasBlocked = d->InteractionModeViewTransformAction->blockSignals(true);
  d->InteractionModeViewTransformAction->setChecked(interactionMode == vtkMRMLInteractionNode::ViewTransform);
  d->InteractionModeViewTransformAction->blockSignals(wasBlocked);

  wasBlocked = d->InteractionModeAdjustWindowLevelAction->blockSignals(true);
  d->InteractionModeAdjustWindowLevelAction->setChecked(interactionMode == vtkMRMLInteractionNode::AdjustWindowLevel);
  d->InteractionModeAdjustWindowLevelAction->blockSignals(wasBlocked);

  wasBlocked = d->InteractionModePlaceAction->blockSignals(true);
  d->InteractionModePlaceAction->setChecked(interactionMode == vtkMRMLInteractionNode::Place);
  d->InteractionModePlaceAction->blockSignals(wasBlocked);


  // Update view/restore view action
  bool isMaximized = false;
  bool canBeMaximized = false;
  d->LayoutNode = viewNode->GetMaximizedState(isMaximized, canBeMaximized);
  d->MaximizeViewAction->setVisible(canBeMaximized);
  if (canBeMaximized)
    {
    d->MaximizeViewAction->setProperty("maximize", QVariant(!isMaximized));
    if (isMaximized)
      {
      d->MaximizeViewAction->setText(tr("Restore view layout"));
      }
    else
      {
      d->MaximizeViewAction->setText(tr("Maximize view"));
      }
    }

  d->CopyImageAction->setVisible(true);

  bool isSliceViewNode = (vtkMRMLSliceNode::SafeDownCast(viewNode) != nullptr);
  d->ConfigureSliceViewAnnotationsAction->setVisible(isSliceViewNode);


  // Enable preset menus if backround volume node is vtkMRMLScalarVolumeNode
  if (!qSlicerApplication::application()
      || !qSlicerApplication::application()->layoutManager())
  {
      qWarning() << Q_FUNC_INFO << " failed: cannot get layout manager";
      return;
  }

  qMRMLSliceWidget* sliceViewWidget = qSlicerApplication::application()->layoutManager()->sliceWidget(viewNode->GetName());
  if (!sliceViewWidget)
  {
      qWarning() << Q_FUNC_INFO << " failed: cannot get slice view widget";
      return;
  }

  QString backgroundVolumeID = QString(sliceViewWidget->sliceLogic()->GetSliceCompositeNode()->GetBackgroundVolumeID());
  vtkMRMLScene* scene = viewNode->GetScene();
  vtkMRMLNode* backgroundVolumeNodePtr = scene->GetNodeByID(backgroundVolumeID.toUtf8());
  if (backgroundVolumeNodePtr) {
      if (backgroundVolumeNodePtr->IsA("vtkMRMLScalarVolumeNode")) {
          d->Separator->setVisible(isSliceViewNode);
          d->HandlePresetAction->setVisible(isSliceViewNode);
      }
      else {
          d->Separator->setVisible(false);
          d->HandlePresetAction->setVisible(false);

      }
  }

  // Cache nodes to have them available for the menu action execution.
  d->InteractionNode = interactionNode;
  d->ViewNode = viewNode;
  d->BackgroundVolumeNode = backgroundVolumeNodePtr;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyViewContextMenuPlugin::setInteractionMode(int mode)
{
  Q_D(qSlicerSubjectHierarchyViewContextMenuPlugin);
  if (!d->InteractionNode)
    {
    qWarning() << Q_FUNC_INFO << " failed: invalid interaction node";
    return;
    }
  d->InteractionNode->SetCurrentInteractionMode(mode);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyViewContextMenuPlugin::saveScreenshot()
{
  Q_D(qSlicerSubjectHierarchyViewContextMenuPlugin);
  if (!qSlicerApplication::application()
    || !qSlicerApplication::application()->layoutManager())
  {
    qWarning() << Q_FUNC_INFO << " failed: cannot get layout manager";
    return;
  }
  QWidget* widget = qSlicerApplication::application()->layoutManager()->viewWidget(d->ViewNode);

  // Get the inside of the widget (without the view controller bar)
  qMRMLSliceWidget* sliceWidget = qobject_cast<qMRMLSliceWidget*>(widget);
  qMRMLThreeDWidget* threeDWidget = qobject_cast<qMRMLThreeDWidget*>(widget);
  if (sliceWidget)
    {
    widget = sliceWidget->sliceView();
    }
  else if (threeDWidget)
    {
    widget = threeDWidget->threeDView();
    }
  if (!widget)
    {
    qWarning() << Q_FUNC_INFO << " failed: cannot get view widget from layout manager";
    return;
    }

  // Grab image
  QImage screenshot = ctk::grabVTKWidget(widget);

  // Copy to clipboard
  QClipboard* clipboard = QApplication::clipboard();
  if (!clipboard)
    {
    qWarning() << Q_FUNC_INFO << " failed: cannot access the clipboard";
    return;
    }
  clipboard->setImage(screenshot);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyViewContextMenuPlugin::configureSliceViewAnnotationsAction()
{
  Q_D(qSlicerSubjectHierarchyViewContextMenuPlugin);
  qSlicerLayoutManager* layoutManager = qSlicerApplication::application()->layoutManager();
  if (!layoutManager)
    {
    return;
    }
  layoutManager->setCurrentModule("DataProbe");
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyViewContextMenuPlugin::maximizeView()
{
  Q_D(qSlicerSubjectHierarchyViewContextMenuPlugin);
  if (!d->LayoutNode)
    {
    return;
    }
  bool maximizeView = d->MaximizeViewAction->property("maximize").toBool();
  if (maximizeView)
    {
    d->LayoutNode->SetMaximizedViewNode(d->ViewNode);
    }
  else
    {
    d->LayoutNode->SetMaximizedViewNode(nullptr);
    }
}


// --------------------------------------------------------------------------
void qSlicerSubjectHierarchyViewContextMenuPlugin::setVolumePreset(int which)
{
    // call setPreset function from qSlicerScalarVolumeDisplayWidget
    Q_D(qSlicerSubjectHierarchyViewContextMenuPlugin);
    // try opening the Volume module
    qSlicerModuleManager* moduleManager = qSlicerApplication::application()->moduleManager();

    qSlicerAbstractCoreModule* volumeModulePointer = nullptr;

    vtkSlicerVolumesLogic* volumeModulePointerLogic = nullptr;

    if (moduleManager)
    {
        volumeModulePointer = moduleManager->module("Volumes");
        if (!volumeModulePointer)
        {
            QString message = QString("Volume module not found.");
            qWarning() << Q_FUNC_INFO << ": " << message;
            return;
        }
    }
    if (volumeModulePointer)
    {
        volumeModulePointerLogic =
            vtkSlicerVolumesLogic::SafeDownCast(volumeModulePointer->logic());
        if (!volumeModulePointerLogic)
        {
            QString message = QString("Volume module logic not found.");
            qWarning() << Q_FUNC_INFO << ": " << message;
            return;
        }
    }
    if (volumeModulePointerLogic)
    {
        switch(which) {
            case this->ID_CTBone:
                volumeModulePointerLogic->setWindowLevelPreset((vtkMRMLScalarVolumeNode*) d->BackgroundVolumeNode, "CT-Bone");
                break;
            case this->ID_CTAir:
                volumeModulePointerLogic->setWindowLevelPreset((vtkMRMLScalarVolumeNode*)d->BackgroundVolumeNode,"CT-Air");
                break;
            case this->ID_PET:
                volumeModulePointerLogic->setWindowLevelPreset((vtkMRMLScalarVolumeNode*)d->BackgroundVolumeNode, "CT-PET");
                break;
            case this->ID_CTAbdomen:
                volumeModulePointerLogic->setWindowLevelPreset((vtkMRMLScalarVolumeNode*)d->BackgroundVolumeNode, "CT-Abdomen");
                break;
            case this->ID_CTBrain:
                volumeModulePointerLogic->setWindowLevelPreset((vtkMRMLScalarVolumeNode*)d->BackgroundVolumeNode, "CT-Brain");
                break;
            case this->ID_CTLung:
                volumeModulePointerLogic->setWindowLevelPreset((vtkMRMLScalarVolumeNode*)d->BackgroundVolumeNode, "CT-Lung");
                break;
            case this->ID_DTI:
                volumeModulePointerLogic->setWindowLevelPreset((vtkMRMLScalarVolumeNode*)d->BackgroundVolumeNode, "CT-DTI");
                break;
            default:
                QString message = QString("No valid preset ID.");
                qWarning() << Q_FUNC_INFO << ": " << message;
                break;
        }
    }
}
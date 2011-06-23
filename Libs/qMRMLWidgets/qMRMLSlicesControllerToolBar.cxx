/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QAction>
#include <QActionGroup>
#include <QDebug>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QToolButton>
#include <QWidgetAction>

// CTK includes
#include <ctkDoubleSlider.h>
#include <ctkLogger.h>
#include <ctkSliderWidget.h>

// qMRML includes
#include "qMRMLSlicesControllerToolBar.h"
#include "ui_qMRMLSlicesControllerToolBar.h"
#include "qMRMLActionSignalMapper.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLCrosshairNode.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLSliceLogic.h>
#include <vtkMRMLSliceNode.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkSmartPointer.h>

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.libs.qmrmlwidgets.qMRMLSlicesControllerToolBar");
//--------------------------------------------------------------------------

//-----------------------------------------------------------------------------
class qMRMLSlicesControllerToolBarPrivate
  : public Ui_qMRMLSlicesControllerToolBar
{
  Q_DECLARE_PUBLIC(qMRMLSlicesControllerToolBar);
protected:
  qMRMLSlicesControllerToolBar* const q_ptr;
public:
  qMRMLSlicesControllerToolBarPrivate(qMRMLSlicesControllerToolBar& object);

  virtual void setupUi(QWidget* widget);
  vtkSmartPointer<vtkCollection> saveSliceCompositeNodes()const;
  vtkSmartPointer<vtkCollection> saveCrosshairNodes()const;
  vtkSmartPointer<vtkCollection> saveSliceNodes()const;
  vtkMRMLSliceLogic* sliceLogicByName(const QString& name)const;

  vtkMRMLScene*                  MRMLScene;
  vtkSmartPointer<vtkCollection> MRMLSliceLogics;

  ctkSliderWidget*         LabelOpacitySlider;
  QToolButton*             LabelOpacityToggleButton;
  double                   LastLabelOpacity;
  ctkDoubleSlider*         ForegroundOpacitySlider;
  qMRMLActionSignalMapper* AnnotationsMapper;
  qMRMLActionSignalMapper* CompositingMapper;
  qMRMLActionSignalMapper* CrosshairMapper;
  qMRMLActionSignalMapper* CrosshairThicknessMapper;
  qMRMLActionSignalMapper* SpatialUnitsMapper;
  QDoubleSpinBox*          RedSliceFOVSpinBox;
  QDoubleSpinBox*          YellowSliceFOVSpinBox;
  QDoubleSpinBox*          GreenSliceFOVSpinBox;
};
//--------------------------------------------------------------------------
// qMRMLSlicesControllerToolBarPrivate methods

//---------------------------------------------------------------------------
qMRMLSlicesControllerToolBarPrivate::qMRMLSlicesControllerToolBarPrivate(qMRMLSlicesControllerToolBar& object)
  : q_ptr(&object)
{
  this->MRMLScene = 0;
  this->LabelOpacitySlider = 0;
  this->LabelOpacityToggleButton = 0;
  this->LastLabelOpacity = 1.;
  this->ForegroundOpacitySlider = 0;
  this->AnnotationsMapper = 0;
  this->CompositingMapper = 0;
  this->CrosshairMapper = 0;
  this->CrosshairThicknessMapper = 0;
  this->SpatialUnitsMapper = 0;
  this->RedSliceFOVSpinBox = 0;
  this->YellowSliceFOVSpinBox = 0;
  this->GreenSliceFOVSpinBox = 0;
}

//---------------------------------------------------------------------------
void qMRMLSlicesControllerToolBarPrivate::setupUi(QWidget* widget)
{
  Q_Q(qMRMLSlicesControllerToolBar);
  this->Ui_qMRMLSlicesControllerToolBar::setupUi(widget);

  // Features
  QMenu* featuresMenu = new QMenu("Features", q);
  featuresMenu->addAction(actionFiducial_points);
  featuresMenu->addAction(actionFiducial_labels);
  featuresMenu->addAction(actionForeground_grid);
  featuresMenu->addAction(actionBackground_grid);
  featuresMenu->addAction(actionLabel_grid);

  QToolButton* featuresButton = new QToolButton(q);
  featuresButton->setPopupMode(QToolButton::InstantPopup);
  featuresButton->setText("Features");
  featuresButton->setIcon(QIcon(":Icons/VisibleOn.png"));
  featuresButton->setMenu(featuresMenu);
  q->addWidget(featuresButton);

  // Fit to Window
  q->addAction(actionFit_to_Window);
  actionFit_to_Window->setEnabled(this->MRMLSliceLogics.GetPointer() != 0);

  // Label Opacity
  QWidget* labelOpacityWidget = new QWidget(q);
  QHBoxLayout* labelOpacityLayout = new QHBoxLayout(labelOpacityWidget);
  labelOpacityLayout->setContentsMargins(0,0,0,0);
  this->LabelOpacitySlider = new ctkSliderWidget(q);
  this->LabelOpacitySlider->setRange(0., 1.);
  this->LabelOpacitySlider->setValue(1.);
  this->LabelOpacitySlider->setSingleStep(0.05);
  QObject::connect(this->LabelOpacitySlider, SIGNAL(valueChanged(double)),
                   q, SLOT(setLabelOpacity(double)));
  this->LabelOpacityToggleButton = new QToolButton(q);
  this->LabelOpacityToggleButton->setText("Toggle Opacity");
  QIcon visibilityIcon;
  visibilityIcon.addFile(":Icons/VisibleOn.png", QSize(), QIcon::Normal, QIcon::Off);
  visibilityIcon.addFile(":Icons/VisibleOff.png", QSize(), QIcon::Normal, QIcon::On);
  this->LabelOpacityToggleButton->setIcon(visibilityIcon);
  this->LabelOpacityToggleButton->setCheckable(true);
  // clicked is fired only if the user clicks on the button, not programatically
  QObject::connect(this->LabelOpacityToggleButton, SIGNAL(clicked(bool)),
                   q, SLOT(toggleLabelOpacity(bool)));
  labelOpacityLayout->addWidget(this->LabelOpacityToggleButton);
  labelOpacityLayout->addWidget(this->LabelOpacitySlider);
  labelOpacityWidget->setLayout(labelOpacityLayout);
  QMenu* fiducialsMenu = new QMenu(q);
  QWidgetAction* sliderAction = new QWidgetAction(q);
  sliderAction->setDefaultWidget(labelOpacityWidget);
  fiducialsMenu->addAction(sliderAction);
  QToolButton* fiducialsButton = new QToolButton(q);
  fiducialsButton->setPopupMode(QToolButton::InstantPopup);
  fiducialsButton->setText("Fiducials");
  fiducialsButton->setIcon(QIcon(":Icons/SlicesLabelOpacity.png"));
  fiducialsButton->setMenu(fiducialsMenu);
  q->addWidget(fiducialsButton);

  // Annotations
  QActionGroup* annotationsActions = new QActionGroup(q);
  annotationsActions->setExclusive(true);
  annotationsActions->addAction(actionAnnotationNone);
  annotationsActions->addAction(actionAnnotationShow_all);
  annotationsActions->addAction(actionAnnotationShow_label_values_only);
  annotationsActions->addAction(actionAnnotationShow_voxel_and_label_values_only);
  this->AnnotationsMapper = new qMRMLActionSignalMapper(q);
  this->AnnotationsMapper->setMapping(this->actionAnnotationNone,
                                      vtkMRMLSliceCompositeNode::NoAnnotation);
  this->AnnotationsMapper->setMapping(this->actionAnnotationShow_all,
                                      vtkMRMLSliceCompositeNode::All);
  this->AnnotationsMapper->setMapping(this->actionAnnotationShow_label_values_only,
                                      vtkMRMLSliceCompositeNode::LabelValuesOnly);
  this->AnnotationsMapper->setMapping(this->actionAnnotationShow_voxel_and_label_values_only,
                                      vtkMRMLSliceCompositeNode::LabelAndVoxelValuesOnly);
  QObject::connect(annotationsActions, SIGNAL(triggered(QAction*)),
                   this->AnnotationsMapper, SLOT(map(QAction*)));
  QObject::connect(this->AnnotationsMapper, SIGNAL(mapped(int)),
                   q, SLOT(setAnnotationMode(int)));
  QMenu* annotationsMenu = new QMenu("Annotations", q);
  annotationsMenu->addActions(annotationsActions->actions());
  QToolButton* annotationsButton = new QToolButton(q);
  annotationsButton->setPopupMode(QToolButton::InstantPopup);
  annotationsButton->setText("Annotations");
  annotationsButton->setIcon(QIcon(":Icons/SlicesAnnotation.png"));
  annotationsButton->setMenu(annotationsMenu);
  q->addWidget(annotationsButton);

  // Compositing
  QActionGroup* compositingActions = new QActionGroup(q);
  compositingActions->setExclusive(true);
  compositingActions->addAction(actionCompositingAlpha_blend);
  compositingActions->addAction(actionCompositingReverse_alpha_blend);
  compositingActions->addAction(actionCompositingAdd);
  compositingActions->addAction(actionCompositingSubstract);
  this->CompositingMapper = new qMRMLActionSignalMapper(q);
  this->CompositingMapper->setMapping(this->actionCompositingAlpha_blend,
                                      vtkMRMLSliceCompositeNode::Alpha);
  this->CompositingMapper->setMapping(this->actionCompositingReverse_alpha_blend,
                                      vtkMRMLSliceCompositeNode::ReverseAlpha);
  this->CompositingMapper->setMapping(this->actionCompositingAdd,
                                      vtkMRMLSliceCompositeNode::Add);
  this->CompositingMapper->setMapping(this->actionCompositingSubstract,
                                      vtkMRMLSliceCompositeNode::Subtract);
  QObject::connect(compositingActions, SIGNAL(triggered(QAction*)),
                   this->CompositingMapper, SLOT(map(QAction*)));
  QObject::connect(this->CompositingMapper, SIGNAL(mapped(int)),
                   q, SLOT(setCompositing(int)));
  QMenu* compositingMenu = new QMenu("Compositing", q);
  compositingMenu->addActions(compositingActions->actions());
  QToolButton* compositingButton = new QToolButton(q);
  compositingButton->setPopupMode(QToolButton::InstantPopup);
  compositingButton->setText("Compositing");
  compositingButton->setIcon(QIcon(":Icons/SlicesComposite.png"));
  compositingButton->setMenu(compositingMenu);
  q->addWidget(compositingButton);

  // Crosshair
  QActionGroup* crosshairActions = new QActionGroup(q);
  crosshairActions->setExclusive(true);
  crosshairActions->addAction(actionCrosshairNo_crosshair);
  crosshairActions->addAction(actionCrosshairBasic_crosshair);
  crosshairActions->addAction(actionCrosshairBasic_intersection);
  crosshairActions->addAction(actionCrosshairBasic_hashmarks);
  crosshairActions->addAction(actionCrosshairBasic_hashmarks_intersection);
  crosshairActions->addAction(actionCrosshairSmall_basic);
  crosshairActions->addAction(actionCrosshairSmall_basic_intersection);
  this->CrosshairMapper = new qMRMLActionSignalMapper(q);
  this->CrosshairMapper->setMapping(actionCrosshairNo_crosshair,
                                    vtkMRMLCrosshairNode::NoCrosshair);
  this->CrosshairMapper->setMapping(actionCrosshairBasic_crosshair,
                                    vtkMRMLCrosshairNode::ShowBasic);
  this->CrosshairMapper->setMapping(actionCrosshairBasic_intersection,
                                    vtkMRMLCrosshairNode::ShowIntersection);
  this->CrosshairMapper->setMapping(actionCrosshairBasic_hashmarks,
                                    vtkMRMLCrosshairNode::ShowHashmarks);
  this->CrosshairMapper->setMapping(actionCrosshairBasic_hashmarks_intersection,
                                    vtkMRMLCrosshairNode::ShowAll);
  this->CrosshairMapper->setMapping(actionCrosshairSmall_basic,
                                    vtkMRMLCrosshairNode::ShowSmallBasic);
  this->CrosshairMapper->setMapping(actionCrosshairSmall_basic_intersection,
                                    vtkMRMLCrosshairNode::ShowSmallIntersection);
  QObject::connect(crosshairActions, SIGNAL(triggered(QAction*)),
                   this->CrosshairMapper, SLOT(map(QAction*)));
  QObject::connect(this->CrosshairMapper, SIGNAL(mapped(int)),
                   q, SLOT(setCrosshairMode(int)));
  QActionGroup* crosshairThicknessActions = new QActionGroup(q);
  crosshairThicknessActions->setExclusive(true);
  crosshairThicknessActions->addAction(actionCrosshairFine);
  crosshairThicknessActions->addAction(actionCrosshairMedium);
  crosshairThicknessActions->addAction(actionCrosshairThick);
  this->CrosshairThicknessMapper = new qMRMLActionSignalMapper(q);
  this->CrosshairThicknessMapper->setMapping(actionCrosshairFine,
                                             vtkMRMLCrosshairNode::Fine);
  this->CrosshairThicknessMapper->setMapping(actionCrosshairMedium,
                                             vtkMRMLCrosshairNode::Medium);
  this->CrosshairThicknessMapper->setMapping(actionCrosshairThick,
                                             vtkMRMLCrosshairNode::Thick);
  QObject::connect(crosshairThicknessActions, SIGNAL(triggered(QAction*)),
                   this->CrosshairThicknessMapper, SLOT(map(QAction*)));
  QObject::connect(this->CrosshairThicknessMapper, SIGNAL(mapped(int)),
                   q, SLOT(setCrosshairThickness(int)));
  QMenu* crosshairMenu = new QMenu("Crosshair", q);
  crosshairMenu->addAction(actionCrosshairNavigator);
  crosshairMenu->addSeparator();
  crosshairMenu->addActions(crosshairActions->actions());
  crosshairMenu->addSeparator();
  crosshairMenu->addActions(crosshairThicknessActions->actions());
  crosshairMenu->addSeparator();
  crosshairMenu->addAction(actionCrosshairSlice_intersections);
  QToolButton* crosshairButton = new QToolButton(q);
  crosshairButton->setPopupMode(QToolButton::InstantPopup);
  crosshairButton->setText("Crosshair");
  crosshairButton->setIcon(QIcon(":Icons/SlicesCrosshair.png"));
  crosshairButton->setMenu(crosshairMenu);
  q->addWidget(crosshairButton);

  // Spatial Units
  QActionGroup* spatialUnitsActions = new QActionGroup(q);
  spatialUnitsActions->setExclusive(true);
  spatialUnitsActions->addAction(actionSpatialUnitsXYZ);
  spatialUnitsActions->addAction(actionSpatialUnitsIJK);
  spatialUnitsActions->addAction(actionSpatialUnitsRAS);
  spatialUnitsActions->addAction(actionSpatialUnitsIJK_RAS);
  this->SpatialUnitsMapper = new qMRMLActionSignalMapper(q);
  this->SpatialUnitsMapper->setMapping(this->actionSpatialUnitsXYZ,
                                      vtkMRMLSliceCompositeNode::XYZ);
  this->SpatialUnitsMapper->setMapping(this->actionSpatialUnitsIJK,
                                      vtkMRMLSliceCompositeNode::IJK);
  this->SpatialUnitsMapper->setMapping(this->actionSpatialUnitsRAS,
                                      vtkMRMLSliceCompositeNode::RAS);
  this->SpatialUnitsMapper->setMapping(this->actionSpatialUnitsIJK_RAS,
                                      vtkMRMLSliceCompositeNode::IJKAndRAS);
  QObject::connect(spatialUnitsActions, SIGNAL(triggered(QAction*)),
                   this->SpatialUnitsMapper, SLOT(map(QAction*)));
  QObject::connect(this->SpatialUnitsMapper, SIGNAL(mapped(int)),
                   q, SLOT(setAnnotationSpace(int)));
  QMenu* spatialUnitsMenu = new QMenu("Spatial Units", q);
  spatialUnitsMenu->addActions(spatialUnitsActions->actions());
  QToolButton* spatialUnitsButton = new QToolButton(q);
  spatialUnitsButton->setPopupMode(QToolButton::InstantPopup);
  spatialUnitsButton->setText("Spatial Units");
  spatialUnitsButton->setIcon(QIcon(":Icons/SlicesSpatialUnit.png"));
  spatialUnitsButton->setMenu(spatialUnitsMenu);
  q->addWidget(spatialUnitsButton);

  // Field of View
  // red FOV
  QWidget* redSliceFOVWidget = new QWidget(q);
  QHBoxLayout* redSliceFOVLayout = new QHBoxLayout(redSliceFOVWidget);
  redSliceFOVLayout->setContentsMargins(0,0,0,0);
  redSliceFOVLayout->addWidget(new QLabel("Red slice FOV:", q));
  this->RedSliceFOVSpinBox = new QDoubleSpinBox(q);
  this->RedSliceFOVSpinBox->setRange(0., 10000.);
  this->RedSliceFOVSpinBox->setValue(250.);
  QObject::connect(this->RedSliceFOVSpinBox, SIGNAL(valueChanged(double)),
                   q, SLOT(setRedSliceFOV(double)));
  QColor red;
  red.setRgbF(0.952941176471, 0.290196078431, 0.2);
  this->RedSliceFOVSpinBox->setPalette(QPalette(red));
  redSliceFOVLayout->addWidget(this->RedSliceFOVSpinBox);
  redSliceFOVWidget->setLayout(redSliceFOVLayout);
  QWidgetAction* redSliceFOVAction = new QWidgetAction(q);
  redSliceFOVAction->setDefaultWidget(redSliceFOVWidget);
  // yellow FOV
  QWidget* yellowSliceFOVWidget = new QWidget(q);
  QHBoxLayout* yellowSliceFOVLayout = new QHBoxLayout(yellowSliceFOVWidget);
  yellowSliceFOVLayout->setContentsMargins(0,0,0,0);
  yellowSliceFOVLayout->addWidget(new QLabel("Yellow slice FOV:", q));
  this->YellowSliceFOVSpinBox = new QDoubleSpinBox(q);
  this->YellowSliceFOVSpinBox->setRange(0., 10000.);
  this->YellowSliceFOVSpinBox->setValue(250.);
  QObject::connect(this->YellowSliceFOVSpinBox, SIGNAL(valueChanged(double)),
                   q, SLOT(setYellowSliceFOV(double)));
  QColor yellow;
  yellow.setRgbF(0.929411764706, 0.835294117647, 0.298039215686);
  this->YellowSliceFOVSpinBox->setPalette(QPalette(yellow));
  yellowSliceFOVLayout->addWidget(this->YellowSliceFOVSpinBox);
  yellowSliceFOVWidget->setLayout(yellowSliceFOVLayout);
  QWidgetAction* yellowSliceFOVAction = new QWidgetAction(q);
  yellowSliceFOVAction->setDefaultWidget(yellowSliceFOVWidget);
  // green FOV
  QWidget* greenSliceFOVWidget = new QWidget(q);
  QHBoxLayout* greenSliceFOVLayout = new QHBoxLayout(greenSliceFOVWidget);
  greenSliceFOVLayout->setContentsMargins(0,0,0,0);
  greenSliceFOVLayout->addWidget(new QLabel("Green slice FOV:", q));
  this->GreenSliceFOVSpinBox = new QDoubleSpinBox(q);
  this->GreenSliceFOVSpinBox->setRange(0., 10000.);
  this->GreenSliceFOVSpinBox->setValue(250.);
  QObject::connect(this->GreenSliceFOVSpinBox, SIGNAL(valueChanged(double)),
                   q, SLOT(setGreenSliceFOV(double)));
  QColor green;
  green.setRgbF(0.43137254902, 0.690196078431, 0.294117647059);
  this->GreenSliceFOVSpinBox->setPalette(QPalette(green));
  greenSliceFOVLayout->addWidget(this->GreenSliceFOVSpinBox);
  greenSliceFOVWidget->setLayout(greenSliceFOVLayout);
  QWidgetAction* greenSliceFOVAction = new QWidgetAction(q);
  greenSliceFOVAction->setDefaultWidget(greenSliceFOVWidget);
  // menu / tool button FOV
  QMenu* fieldOfViewMenu = new QMenu(q);
  fieldOfViewMenu->addAction(redSliceFOVAction);
  fieldOfViewMenu->addAction(yellowSliceFOVAction);
  fieldOfViewMenu->addAction(greenSliceFOVAction);
  QToolButton* fieldOfViewButton = new QToolButton(q);
  fieldOfViewButton->setPopupMode(QToolButton::InstantPopup);
  fieldOfViewButton->setText("Slices Field of View");
  fieldOfViewButton->setIcon(QIcon(":Icons/SlicesFieldOfView.png"));
  fieldOfViewButton->setMenu(fieldOfViewMenu);
  q->addWidget(fieldOfViewButton);

  // Background / Foreground
  q->addSeparator();
  q->addAction(actionToggleBgFg);
  q->addAction(actionShowBg);
  this->ForegroundOpacitySlider = new ctkDoubleSlider(q->orientation(), q);
  this->ForegroundOpacitySlider->setRange(0., 1.);
  this->ForegroundOpacitySlider->setValue(1.);
  this->ForegroundOpacitySlider->setSingleStep(0.1);
  QObject::connect(q, SIGNAL(orientationChanged(Qt::Orientation)),
                   this->ForegroundOpacitySlider, SLOT(setOrientation(Qt::Orientation)));
  QObject::connect(this->ForegroundOpacitySlider, SIGNAL(valueChanged(double)),
                   q, SLOT(setForegroundOpacity(double)));
  q->addWidget(this->ForegroundOpacitySlider);
  q->addAction(actionShowFg);
}

// --------------------------------------------------------------------------
vtkSmartPointer<vtkCollection> qMRMLSlicesControllerToolBarPrivate::saveSliceCompositeNodes()const
{
  vtkSmartPointer<vtkCollection> nodes;
  if (this->MRMLScene)
    {
    nodes.TakeReference(
      this->MRMLScene->GetNodesByClass("vtkMRMLSliceCompositeNode"));
    }
  if (nodes.GetPointer())
    {
    this->MRMLScene->SaveStateForUndo(nodes);
    }
  return nodes;
}

// --------------------------------------------------------------------------
vtkSmartPointer<vtkCollection> qMRMLSlicesControllerToolBarPrivate::saveCrosshairNodes()const
{
  vtkSmartPointer<vtkCollection> nodes;
  if (this->MRMLScene)
    {
    nodes.TakeReference(this->MRMLScene->GetNodesByClass("vtkMRMLCrosshairNodes"));
    }
  if (nodes)
    {
    this->MRMLScene->SaveStateForUndo(nodes);
    }
  return nodes;
}
// --------------------------------------------------------------------------
vtkSmartPointer<vtkCollection> qMRMLSlicesControllerToolBarPrivate::saveSliceNodes()const
{
  vtkSmartPointer<vtkCollection> nodes;
  if (this->MRMLScene)
    {
    nodes.TakeReference(this->MRMLScene->GetNodesByClass("vtkMRMLSliceNodes"));
    }
  if (nodes)
    {
    this->MRMLScene->SaveStateForUndo(nodes);
    }
  return nodes;
}

// --------------------------------------------------------------------------
vtkMRMLSliceLogic* qMRMLSlicesControllerToolBarPrivate::sliceLogicByName(const QString& name)const
{
  if (!this->MRMLSliceLogics.GetPointer())
    {
    return 0;
    }
  vtkMRMLSliceLogic* logic;
  vtkCollectionSimpleIterator it;
  for (this->MRMLSliceLogics->InitTraversal(it);
       (logic = vtkMRMLSliceLogic::SafeDownCast(
         this->MRMLSliceLogics->GetNextItemAsObject(it)));)
    {
    if (name == logic->GetName())
      {
      return logic;
      }
    }
  return 0;
}

// --------------------------------------------------------------------------
// qMRMLSlicesControllerToolBar methods

// --------------------------------------------------------------------------
qMRMLSlicesControllerToolBar::qMRMLSlicesControllerToolBar(QWidget* _parent) : Superclass(_parent)
  , d_ptr(new qMRMLSlicesControllerToolBarPrivate(*this))
{
  Q_D(qMRMLSlicesControllerToolBar);
  d->setupUi(this);
}

//---------------------------------------------------------------------------
qMRMLSlicesControllerToolBar::~qMRMLSlicesControllerToolBar()
{
}

// --------------------------------------------------------------------------
vtkMRMLScene* qMRMLSlicesControllerToolBar::mrmlScene()const
{
  Q_D(const qMRMLSlicesControllerToolBar);
  return d->MRMLScene;
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLSlicesControllerToolBar);
  qvtkReconnect(d->MRMLScene, scene, vtkMRMLScene::NodeAddedEvent,
                this, SLOT(onMRMLSceneChanged(vtkObject*, void*,  unsigned long, void *)));
  qvtkReconnect(d->MRMLScene, scene, vtkMRMLScene::SceneImportedEvent,
                this, SLOT(onMRMLSceneChanged(vtkObject*, void*,  unsigned long, void *)));
  qvtkReconnect(d->MRMLScene, scene, vtkMRMLScene::SceneRestoredEvent,
                this, SLOT(onMRMLSceneChanged(vtkObject*, void*,  unsigned long, void *)));
  d->MRMLScene = scene;
  if (d->MRMLScene)
    {
    this->onMRMLSceneChanged(d->MRMLScene, reinterpret_cast<void*>(this),
                             vtkMRMLScene::SceneImportedEvent, 0);
    }
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setMRMLSliceLogics(vtkCollection* logics)
{
  Q_D(qMRMLSlicesControllerToolBar);
  d->MRMLSliceLogics = logics;
  d->actionFit_to_Window->setEnabled(logics != 0);
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::onMRMLSceneChanged(
  vtkObject* sender, void* calldata, unsigned long event, void* receiver)
{
  Q_D(qMRMLSlicesControllerToolBar);
  Q_UNUSED(receiver);
  Q_UNUSED(sender);
  Q_ASSERT(d->MRMLScene == sender);
  if (d->MRMLScene->GetIsUpdating())
    {
    return;
    }
  if (event == vtkMRMLScene::NodeAddedEvent)
    {
    this->connectNode(reinterpret_cast<vtkMRMLNode*>(calldata));
    }
  else if (event == vtkMRMLScene::SceneImportedEvent ||
           event == vtkMRMLScene::SceneRestoredEvent)
    {
    vtkCollection* scene = this->mrmlScene()->GetCurrentScene();
    vtkCollectionSimpleIterator it;
    vtkMRMLNode* node = 0;
    for (scene->InitTraversal(it);
         (node = (vtkMRMLNode*)scene->GetNextItemAsObject(it)) ;)
      {
      this->connectNode(node);
      }
    }
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::connectNode(vtkMRMLNode* node)
{
  Q_D(qMRMLSlicesControllerToolBar);
  vtkMRMLSliceCompositeNode* compositeNode = vtkMRMLSliceCompositeNode::SafeDownCast(node);
  vtkMRMLCrosshairNode* crosshairNode = vtkMRMLCrosshairNode::SafeDownCast(node);
  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(node);
  if (compositeNode)
    {
    // only listen to the red slice composite node
    vtkMRMLSliceLogic* redSliceLogic = d->sliceLogicByName("Red");
    if (redSliceLogic &&
        redSliceLogic->GetSliceCompositeNode() == compositeNode)
      {
      qvtkReconnect(compositeNode, vtkCommand::ModifiedEvent,
                    this, SLOT(updateFromCompositeNode(vtkObject*)));
      }
    this->updateFromCompositeNode(node);
    }
  else if (crosshairNode)
    {
    qvtkReconnect(crosshairNode, vtkCommand::ModifiedEvent,
                  this, SLOT(updateFromCrosshairNode(vtkObject*)));
    this->updateFromCrosshairNode(node);
    }
  else if (sliceNode)
    {
    qvtkDisconnect(sliceNode, vtkCommand::ModifiedEvent,
                   this, SLOT(updateFromSliceNode(vtkObject*)));
    qvtkConnect(sliceNode, vtkCommand::ModifiedEvent,
                this, SLOT(updateFromSliceNode(vtkObject*)));
    this->updateFromSliceNode(node);
    }
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::updateFromCompositeNode(vtkObject* node)
{
  Q_D(qMRMLSlicesControllerToolBar);
  vtkMRMLSliceCompositeNode* cnode = vtkMRMLSliceCompositeNode::SafeDownCast(node);
  if (!cnode)
    {
    return;
    }
  d->actionFiducial_points->setChecked(cnode->GetFiducialVisibility());
  d->actionFiducial_labels->setChecked(cnode->GetFiducialLabelVisibility());
  d->actionForeground_grid->setChecked(cnode->GetForegroundGrid());
  d->actionBackground_grid->setChecked(cnode->GetBackgroundGrid());
  d->actionLabel_grid->setChecked(cnode->GetLabelGrid());
  d->LabelOpacitySlider->setValue(cnode->GetLabelOpacity());
  QAction* annotationAction = qobject_cast<QAction*>(
    d->AnnotationsMapper->mapping(cnode->GetAnnotationMode()));
  annotationAction->setChecked(true);
  QAction* compositingAction = qobject_cast<QAction*>(
    d->CompositingMapper->mapping(cnode->GetCompositing()));
  compositingAction->setChecked(true);
  d->actionCrosshairSlice_intersections->setChecked(cnode->GetSliceIntersectionVisibility());
  QAction* spatialUnitsAction = qobject_cast<QAction*>(
    d->SpatialUnitsMapper->mapping(cnode->GetAnnotationSpace()));
  spatialUnitsAction->setChecked(true);
  d->ForegroundOpacitySlider->setValue(cnode->GetForegroundOpacity());
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::updateFromCrosshairNode(vtkObject* node)
{
  Q_D(qMRMLSlicesControllerToolBar);
  vtkMRMLCrosshairNode* xnode = vtkMRMLCrosshairNode::SafeDownCast(node);
  if (!xnode)
    {
    return;
    }
  d->actionCrosshairNavigator->setChecked(xnode->GetNavigation());
  QAction* crosshairAction =
    qobject_cast<QAction*>(d->CrosshairMapper->mapping(xnode->GetCrosshairMode()));
  crosshairAction->setChecked(true);
  QAction* crosshairThicknessAction =
    qobject_cast<QAction*>(d->CrosshairThicknessMapper->mapping(xnode->GetCrosshairThickness()));
  crosshairThicknessAction->setChecked(true);
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::updateFromSliceNode(vtkObject* node)
{
  Q_D(qMRMLSlicesControllerToolBar);
  vtkMRMLSliceNode* snode = vtkMRMLSliceNode::SafeDownCast(node);
  if (!snode)
    {
    return;
    }
  double fov[3];
  snode->GetFieldOfView(fov);
  QString layoutName = QString(snode->GetLayoutName());
  if (layoutName == "Red")
    {
    d->RedSliceFOVSpinBox->setValue(fov[0] < fov[1] ? fov[0] : fov[1]);
    }
  else if(layoutName == "Yellow")
    {
    d->YellowSliceFOVSpinBox->setValue(fov[0] < fov[1] ? fov[0] : fov[1]);
    }
  else if (layoutName == "Green")
    {
    d->GreenSliceFOVSpinBox->setValue(fov[0] < fov[1] ? fov[0] : fov[1]);
    }
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setFiducialPointsVisible(bool visible)
{
  Q_D(qMRMLSlicesControllerToolBar);
  vtkSmartPointer<vtkCollection> nodes = d->saveSliceCompositeNodes();
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLSliceCompositeNode* node = 0;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLSliceCompositeNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    node->SetFiducialVisibility(visible);
    }
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setFiducialLabelsVisible(bool visible)
{
  Q_D(qMRMLSlicesControllerToolBar);
  vtkSmartPointer<vtkCollection> nodes = d->saveSliceCompositeNodes();
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLSliceCompositeNode* node = 0;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLSliceCompositeNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    node->SetFiducialLabelVisibility(visible);
    }
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setForegroundGridVisible(bool visible)
{
  Q_D(qMRMLSlicesControllerToolBar);
  vtkSmartPointer<vtkCollection> nodes = d->saveSliceCompositeNodes();
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLSliceCompositeNode* node = 0;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLSliceCompositeNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    node->SetForegroundGrid(visible);
    }
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setBackgroundGridVisible(bool visible)
{
  Q_D(qMRMLSlicesControllerToolBar);
  vtkSmartPointer<vtkCollection> nodes = d->saveSliceCompositeNodes();
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLSliceCompositeNode* node = 0;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLSliceCompositeNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    node->SetBackgroundGrid(visible);
    }
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setLabelGridVisible(bool visible)
{
  Q_D(qMRMLSlicesControllerToolBar);
  vtkSmartPointer<vtkCollection> nodes = d->saveSliceCompositeNodes();
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLSliceCompositeNode* node = 0;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLSliceCompositeNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    node->SetLabelGrid(visible);
    }
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::fitToWindow()
{
  Q_D(qMRMLSlicesControllerToolBar);
  if (!d->MRMLSliceLogics.GetPointer())
    {
    return;
    }
  vtkSmartPointer<vtkCollection> nodes = d->saveSliceNodes();
  if (!nodes.GetPointer())
    {
    return;
    }

  vtkMRMLSliceLogic* logic;
  vtkCollectionSimpleIterator it;
  for(d->MRMLSliceLogics->InitTraversal(it);
      (logic = vtkMRMLSliceLogic::SafeDownCast(
        d->MRMLSliceLogics->GetNextItemAsObject(it)));)
    {
    logic->FitSliceToAll();
    }
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setLabelOpacity(double value)
{
  Q_D(qMRMLSlicesControllerToolBar);
  // LabelOpacityToggleButton won't fire the clicked(bool) signal here because
  // its check state is set programatically.
  d->LabelOpacityToggleButton->setChecked(value == 0.);
  vtkSmartPointer<vtkCollection> nodes = d->saveSliceCompositeNodes();
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLSliceCompositeNode* node = 0;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLSliceCompositeNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    node->SetLabelOpacity(value);
    }
  if (value != 0.)
    {
    d->LastLabelOpacity = value;
    }
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::toggleLabelOpacity(bool clicked)
{
  Q_D(qMRMLSlicesControllerToolBar);
  d->LabelOpacitySlider->setValue(clicked ? 0. : d->LastLabelOpacity);
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setNavigatorEnabled(bool enable)
{
  Q_D(qMRMLSlicesControllerToolBar);
  vtkSmartPointer<vtkCollection> nodes = d->saveCrosshairNodes();
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLCrosshairNode* node = 0;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLCrosshairNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    node->SetNavigation(enable);
    }
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setSliceIntersectionVisible(bool visible)
{
  Q_D(qMRMLSlicesControllerToolBar);
  vtkSmartPointer<vtkCollection> nodes = d->saveSliceCompositeNodes();
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLSliceCompositeNode* node = 0;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLSliceCompositeNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    node->SetSliceIntersectionVisibility(visible);
    }
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::toggleBackgroundForeground()
{
  Q_D(qMRMLSlicesControllerToolBar);
  d->ForegroundOpacitySlider->setValue(1. - d->ForegroundOpacitySlider->value());
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::showBackground()
{
  Q_D(qMRMLSlicesControllerToolBar);
  d->ForegroundOpacitySlider->setValue(0.);
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::showForeground()
{
  Q_D(qMRMLSlicesControllerToolBar);
  d->ForegroundOpacitySlider->setValue(1.);
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setForegroundOpacity(double value)
{
  Q_D(qMRMLSlicesControllerToolBar);
  vtkSmartPointer<vtkCollection> nodes = d->saveSliceCompositeNodes();
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLSliceCompositeNode* node = 0;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLSliceCompositeNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    node->SetForegroundOpacity(value);
    }
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setAnnotationMode(int mode)
{
  Q_D(qMRMLSlicesControllerToolBar);
  vtkSmartPointer<vtkCollection> nodes = d->saveSliceCompositeNodes();
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLSliceCompositeNode* node = 0;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLSliceCompositeNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    node->SetAnnotationMode(mode);
    }
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setCompositing(int mode)
{
  Q_D(qMRMLSlicesControllerToolBar);
  vtkSmartPointer<vtkCollection> nodes = d->saveSliceCompositeNodes();
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLSliceCompositeNode* node = 0;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLSliceCompositeNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    node->SetCompositing(mode);
    }
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setCrosshairMode(int mode)
{
  Q_D(qMRMLSlicesControllerToolBar);
  vtkSmartPointer<vtkCollection> nodes = d->saveCrosshairNodes();
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLCrosshairNode* node = 0;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLCrosshairNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    node->SetCrosshairMode(mode);
    }
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setCrosshairThickness(int thicknessMode)
{
  Q_D(qMRMLSlicesControllerToolBar);
  vtkSmartPointer<vtkCollection> nodes = d->saveCrosshairNodes();
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLCrosshairNode* node = 0;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLCrosshairNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    node->SetCrosshairThickness(thicknessMode);
    }
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setAnnotationSpace(int spatialUnits)
{
  Q_D(qMRMLSlicesControllerToolBar);
  vtkSmartPointer<vtkCollection> nodes = d->saveSliceCompositeNodes();
  if (!nodes.GetPointer())
    {
    return;
    }
  vtkMRMLSliceCompositeNode* node = 0;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);(node = static_cast<vtkMRMLSliceCompositeNode*>(
                                   nodes->GetNextItemAsObject(it)));)
    {
    node->SetAnnotationSpace(spatialUnits);
    }
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setRedSliceFOV(double fov)
{
  Q_D(qMRMLSlicesControllerToolBar);
  vtkMRMLSliceLogic* redSliceLogic = d->sliceLogicByName("Red");
  if (!redSliceLogic)
    {
    return;
    }
  // The pb with QDoubleSpinBox is that they truncate the resolution of the
  // fov after 2 decimals. Don't fire new events if the new value is below
  // 0.01
  vtkMRMLSliceNode* snode = redSliceLogic->GetSliceNode();
  if (snode)
    {
    double oldFov[3];
    snode->GetFieldOfView(oldFov);
    if (qAbs(qMin(oldFov[0], oldFov[1])- fov) < 0.01)
      {
      return;
      }
    }
  redSliceLogic->FitFOVToBackground(fov);
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setYellowSliceFOV(double fov)
{
  Q_D(qMRMLSlicesControllerToolBar);
  vtkMRMLSliceLogic* yellowSliceLogic = d->sliceLogicByName("Yellow");
  if (!yellowSliceLogic)
    {
    return;
    }
  // The pb with QDoubleSpinBox is that they truncate the resolution of the
  // fov after 2 decimals. Don't fire new events if the new value is below
  // 0.01
  vtkMRMLSliceNode* snode = yellowSliceLogic->GetSliceNode();
  if (snode)
    {
    double oldFov[3];
    snode->GetFieldOfView(oldFov);
    if (qAbs(qMin(oldFov[0], oldFov[1])- fov) < 0.01)
      {
      return;
      }
    }
  yellowSliceLogic->FitFOVToBackground(fov);
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setGreenSliceFOV(double fov)
{
  Q_D(qMRMLSlicesControllerToolBar);
  vtkMRMLSliceLogic* greenSliceLogic = d->sliceLogicByName("Green");
  if (!greenSliceLogic)
    {
    return;
    }
  // The pb with QDoubleSpinBox is that they truncate the resolution of the
  // fov after 2 decimals. Don't fire new events if the new value is below
  // 0.01
  vtkMRMLSliceNode* snode = greenSliceLogic->GetSliceNode();
  if (snode)
    {
    double oldFov[3];
    snode->GetFieldOfView(oldFov);
    if (qAbs(qMin(oldFov[0], oldFov[1])- fov) < 0.01)
      {
      return;
      }
    }
  greenSliceLogic->FitFOVToBackground(fov);
}

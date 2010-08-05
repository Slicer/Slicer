
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
#include <ctkSliderSpinBoxWidget.h>

// qMRML includes
#include "qMRMLSlicesControllerToolBar.h"
#include "ui_qMRMLSlicesControllerToolBar.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLCrosshairNode.h>
#include <vtkMRMLSliceCompositeNode.h>

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.libs.qmrmlwidgets.qMRMLSlicesControllerToolBar");
//--------------------------------------------------------------------------

//-----------------------------------------------------------------------------
qMRMLActionSignalMapper::qMRMLActionSignalMapper(QObject* parentObject)
  :QSignalMapper(parentObject)
{
}

//-----------------------------------------------------------------------------
void qMRMLActionSignalMapper::map(QAction* sender)
{
  this->QSignalMapper::map(qobject_cast<QObject*>(sender));
}

//-----------------------------------------------------------------------------
class qMRMLSlicesControllerToolBarPrivate
  : public ctkPrivate<qMRMLSlicesControllerToolBar>
  , public Ui_qMRMLSlicesControllerToolBar
{
public:
  CTK_DECLARE_PUBLIC(qMRMLSlicesControllerToolBar);
  qMRMLSlicesControllerToolBarPrivate();

  virtual void setupUi(QWidget* widget);
  vtkCollection* saveSliceCompositeNodes()const;
  vtkCollection* saveCrosshairNodes()const;

  vtkMRMLScene*            MRMLScene;
  ctkSliderSpinBoxWidget*  LabelOpacitySlider;
  QToolButton*             LabelOpacityToggleButton;
  double                   LastLabelOpacity;
  ctkDoubleSlider*         OpacitySlider;
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
qMRMLSlicesControllerToolBarPrivate::qMRMLSlicesControllerToolBarPrivate()
{
  this->MRMLScene = 0;
  this->LabelOpacitySlider = 0;
  this->LabelOpacityToggleButton = 0;
  this->LastLabelOpacity = 1.;
  this->OpacitySlider = 0;
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
  CTK_P(qMRMLSlicesControllerToolBar);
  this->Ui_qMRMLSlicesControllerToolBar::setupUi(widget);

  // Features
  QMenu* featuresMenu = new QMenu("Features", p);
  featuresMenu->addAction(actionFiducial_points);
  featuresMenu->addAction(actionFiducial_labels);
  featuresMenu->addAction(actionForeground_grid);
  featuresMenu->addAction(actionBackground_grid);
  featuresMenu->addAction(actionLabel_grid);

  QToolButton* featuresButton = new QToolButton(p);
  featuresButton->setPopupMode(QToolButton::InstantPopup);
  featuresButton->setText("Features");
  featuresButton->setIcon(QIcon(":Icons/SlicesFeaturesVisible.png"));
  featuresButton->setMenu(featuresMenu);
  p->addWidget(featuresButton);

  // Fit to Window
  p->addAction(actionFit_to_Window);

  // Label Opacity
  QWidget* labelOpacityWidget = new QWidget(p);
  QHBoxLayout* labelOpacityLayout = new QHBoxLayout(labelOpacityWidget);
  labelOpacityLayout->setContentsMargins(0,0,0,0);
  this->LabelOpacitySlider = new ctkSliderSpinBoxWidget(p);
  this->LabelOpacitySlider->setRange(0., 1.);
  this->LabelOpacitySlider->setValue(1.);
  this->LabelOpacitySlider->setSingleStep(0.05);
  QObject::connect(this->LabelOpacitySlider, SIGNAL(valueChanged(double)),
                   p, SLOT(setLabelOpacity(double)));
  this->LabelOpacityToggleButton = new QToolButton(p);
  this->LabelOpacityToggleButton->setText("Toggle Opacity");
  QIcon visibilityIcon;
  visibilityIcon.addFile(":Icons/VisibleOn.png", QSize(), QIcon::Normal, QIcon::Off);
  visibilityIcon.addFile(":Icons/VisibleOff.png", QSize(), QIcon::Normal, QIcon::On);
  this->LabelOpacityToggleButton->setIcon(visibilityIcon);
  this->LabelOpacityToggleButton->setCheckable(true);
  // clicked is fired only if the user clicks on the button, not programatically
  QObject::connect(this->LabelOpacityToggleButton, SIGNAL(clicked(bool)),
                   p, SLOT(toggleLabelOpacity(bool)));
  labelOpacityLayout->addWidget(this->LabelOpacityToggleButton);
  labelOpacityLayout->addWidget(this->LabelOpacitySlider);
  labelOpacityWidget->setLayout(labelOpacityLayout);
  QMenu* fiducialsMenu = new QMenu(p);
  QWidgetAction* sliderAction = new QWidgetAction(p);
  sliderAction->setDefaultWidget(labelOpacityWidget);
  fiducialsMenu->addAction(sliderAction);
  QToolButton* fiducialsButton = new QToolButton(p);
  fiducialsButton->setPopupMode(QToolButton::InstantPopup);
  fiducialsButton->setText("Fiducials");
  fiducialsButton->setIcon(QIcon(":Icons/SlicesLabelOpacity.png"));
  fiducialsButton->setMenu(fiducialsMenu);
  p->addWidget(fiducialsButton);

  // Annotations
  QActionGroup* annotationsActions = new QActionGroup(p);
  annotationsActions->setExclusive(true);
  annotationsActions->addAction(actionAnnotationNone);
  annotationsActions->addAction(actionAnnotationShow_all);
  annotationsActions->addAction(actionAnnotationShow_label_values_only);
  annotationsActions->addAction(actionAnnotationShow_voxel_and_label_values_only);
  this->AnnotationsMapper = new qMRMLActionSignalMapper(p);
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
                   p, SLOT(setAnnotationMode(int)));
  QMenu* annotationsMenu = new QMenu("Annotations", p);
  annotationsMenu->addActions(annotationsActions->actions());
  QToolButton* annotationsButton = new QToolButton(p);
  annotationsButton->setPopupMode(QToolButton::InstantPopup);
  annotationsButton->setText("Annotations");
  annotationsButton->setIcon(QIcon(":Icons/SlicesAnnotation.png"));
  annotationsButton->setMenu(annotationsMenu);
  p->addWidget(annotationsButton);

  // Compositing
  QActionGroup* compositingActions = new QActionGroup(p);
  compositingActions->setExclusive(true);
  compositingActions->addAction(actionCompositingAlpha_blend);
  compositingActions->addAction(actionCompositingReverse_alpha_blend);
  compositingActions->addAction(actionCompositingAdd);
  compositingActions->addAction(actionCompositingSubstract);
  this->CompositingMapper = new qMRMLActionSignalMapper(p);
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
                   p, SLOT(setCompositing(int)));
  QMenu* compositingMenu = new QMenu("Compositing", p);
  compositingMenu->addActions(compositingActions->actions());
  QToolButton* compositingButton = new QToolButton(p);
  compositingButton->setPopupMode(QToolButton::InstantPopup);
  compositingButton->setText("Compositing");
  compositingButton->setIcon(QIcon(":Icons/SlicesComposite.png"));
  compositingButton->setMenu(compositingMenu);
  p->addWidget(compositingButton);

  // Crosshair
  QActionGroup* crosshairActions = new QActionGroup(p);
  crosshairActions->setExclusive(true);
  crosshairActions->addAction(actionCrosshairNo_crosshair);
  crosshairActions->addAction(actionCrosshairBasic_crosshair);
  crosshairActions->addAction(actionCrosshairBasic_intersection);
  crosshairActions->addAction(actionCrosshairBasic_hashmarks);
  crosshairActions->addAction(actionCrosshairBasic_hashmarks_intersection);
  crosshairActions->addAction(actionCrosshairSmall_basic);
  crosshairActions->addAction(actionCrosshairSmall_basic_intersection);
  this->CrosshairMapper = new qMRMLActionSignalMapper(p);
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
                   p, SLOT(setCrosshairMode(int)));
  QActionGroup* crosshairThicknessActions = new QActionGroup(p);
  crosshairThicknessActions->setExclusive(true);
  crosshairThicknessActions->addAction(actionCrosshairFine);
  crosshairThicknessActions->addAction(actionCrosshairMedium);
  crosshairThicknessActions->addAction(actionCrosshairThick);
  this->CrosshairThicknessMapper = new qMRMLActionSignalMapper(p);
  this->CrosshairThicknessMapper->setMapping(actionCrosshairFine,
                                             vtkMRMLCrosshairNode::Fine);
  this->CrosshairThicknessMapper->setMapping(actionCrosshairMedium,
                                             vtkMRMLCrosshairNode::Medium);
  this->CrosshairThicknessMapper->setMapping(actionCrosshairThick,
                                             vtkMRMLCrosshairNode::Thick);
  QObject::connect(crosshairThicknessActions, SIGNAL(triggered(QAction*)),
                   this->CrosshairThicknessMapper, SLOT(map(QAction*)));
  QObject::connect(this->CrosshairThicknessMapper, SIGNAL(mapped(int)),
                   p, SLOT(setCrosshairThickness(int)));
  QMenu* crosshairMenu = new QMenu("Crosshair", p);
  crosshairMenu->addAction(actionCrosshairNavigator);
  crosshairMenu->addSeparator();
  crosshairMenu->addActions(crosshairActions->actions());
  crosshairMenu->addSeparator();
  crosshairMenu->addActions(crosshairThicknessActions->actions());
  crosshairMenu->addSeparator();
  crosshairMenu->addAction(actionCrosshairSlice_intersections);
  QToolButton* crosshairButton = new QToolButton(p);
  crosshairButton->setPopupMode(QToolButton::InstantPopup);
  crosshairButton->setText("Crosshair");
  crosshairButton->setIcon(QIcon(":Icons/SlicesCrosshair.png"));
  crosshairButton->setMenu(crosshairMenu);
  p->addWidget(crosshairButton);

  // Spatial Units
  QActionGroup* spatialUnitsActions = new QActionGroup(p);
  spatialUnitsActions->setExclusive(true);
  spatialUnitsActions->addAction(actionSpatialUnitsXYZ);
  spatialUnitsActions->addAction(actionSpatialUnitsIJK);
  spatialUnitsActions->addAction(actionSpatialUnitsRAS);
  spatialUnitsActions->addAction(actionSpatialUnitsIJK_RAS);
  this->SpatialUnitsMapper = new qMRMLActionSignalMapper(p);
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
                   p, SLOT(setAnnotationSpace(int)));
  QMenu* spatialUnitsMenu = new QMenu("Spatial Units", p);
  spatialUnitsMenu->addActions(spatialUnitsActions->actions());
  QToolButton* spatialUnitsButton = new QToolButton(p);
  spatialUnitsButton->setPopupMode(QToolButton::InstantPopup);
  spatialUnitsButton->setText("Spatial Units");
  spatialUnitsButton->setIcon(QIcon(":Icons/SlicesSpatialUnit.png"));
  spatialUnitsButton->setMenu(spatialUnitsMenu);
  p->addWidget(spatialUnitsButton);

  // Field of View
  // red FOV
  QWidget* redSliceFOVWidget = new QWidget(p);
  QHBoxLayout* redSliceFOVLayout = new QHBoxLayout(p);
  redSliceFOVLayout->setContentsMargins(0,0,0,0);
  redSliceFOVLayout->addWidget(new QLabel("Red slice FOV:", p));
  this->RedSliceFOVSpinBox = new QDoubleSpinBox(p);
  this->RedSliceFOVSpinBox->setRange(0., 1000.);
  this->RedSliceFOVSpinBox->setValue(250.);
  QObject::connect(this->RedSliceFOVSpinBox, SIGNAL(valueChanged(double)),
                   p, SLOT(setRedSliceFOV(double)));
  QColor red;
  red.setRgbF(0.952941176471, 0.290196078431, 0.2);
  this->RedSliceFOVSpinBox->setPalette(QPalette(red));
  redSliceFOVLayout->addWidget(this->RedSliceFOVSpinBox);
  redSliceFOVWidget->setLayout(redSliceFOVLayout);
  QWidgetAction* redSliceFOVAction = new QWidgetAction(p);
  redSliceFOVAction->setDefaultWidget(redSliceFOVWidget);
  // yellow FOV
  QWidget* yellowSliceFOVWidget = new QWidget(p);
  QHBoxLayout* yellowSliceFOVLayout = new QHBoxLayout(p);
  yellowSliceFOVLayout->setContentsMargins(0,0,0,0);
  yellowSliceFOVLayout->addWidget(new QLabel("Yellow slice FOV:", p));
  this->YellowSliceFOVSpinBox = new QDoubleSpinBox(p);
  this->YellowSliceFOVSpinBox->setRange(0., 1000.);
  this->YellowSliceFOVSpinBox->setValue(250.);
  QObject::connect(this->YellowSliceFOVSpinBox, SIGNAL(valueChanged(double)),
                   p, SLOT(setYellowSliceFOV(double)));
  QColor yellow;
  yellow.setRgbF(0.929411764706, 0.835294117647, 0.298039215686);
  this->YellowSliceFOVSpinBox->setPalette(QPalette(yellow));
  yellowSliceFOVLayout->addWidget(this->YellowSliceFOVSpinBox);
  yellowSliceFOVWidget->setLayout(yellowSliceFOVLayout);
  QWidgetAction* yellowSliceFOVAction = new QWidgetAction(p);
  yellowSliceFOVAction->setDefaultWidget(yellowSliceFOVWidget);
  // green FOV
  QWidget* greenSliceFOVWidget = new QWidget(p);
  QHBoxLayout* greenSliceFOVLayout = new QHBoxLayout(p);
  greenSliceFOVLayout->setContentsMargins(0,0,0,0);
  greenSliceFOVLayout->addWidget(new QLabel("Green slice FOV:", p));
  this->GreenSliceFOVSpinBox = new QDoubleSpinBox(p);
  this->GreenSliceFOVSpinBox->setRange(0., 1000.);
  this->GreenSliceFOVSpinBox->setValue(250.);
  QObject::connect(this->GreenSliceFOVSpinBox, SIGNAL(valueChanged(double)),
                   p, SLOT(setGreenSliceFOV(double)));
  QColor green;
  green.setRgbF(0.43137254902, 0.690196078431, 0.294117647059);
  this->GreenSliceFOVSpinBox->setPalette(QPalette(green));
  greenSliceFOVLayout->addWidget(this->GreenSliceFOVSpinBox);
  greenSliceFOVWidget->setLayout(greenSliceFOVLayout);
  QWidgetAction* greenSliceFOVAction = new QWidgetAction(p);
  greenSliceFOVAction->setDefaultWidget(greenSliceFOVWidget);
  // menu / tool button FOV
  QMenu* fieldOfViewMenu = new QMenu(p);
  fieldOfViewMenu->addAction(redSliceFOVAction);
  fieldOfViewMenu->addAction(yellowSliceFOVAction);
  fieldOfViewMenu->addAction(greenSliceFOVAction);
  QToolButton* fieldOfViewButton = new QToolButton(p);
  fieldOfViewButton->setPopupMode(QToolButton::InstantPopup);
  fieldOfViewButton->setText("Slices Field of View");
  fieldOfViewButton->setIcon(QIcon(":Icons/SlicesFieldOfView.png"));
  fieldOfViewButton->setMenu(fieldOfViewMenu);
  p->addWidget(fieldOfViewButton);

  // Background / Foreground
  p->addSeparator();
  p->addAction(actionToggleBgFg);
  p->addAction(actionShowBg);
  this->OpacitySlider = new ctkDoubleSlider(p->orientation(), p);
  this->OpacitySlider->setRange(0., 1.);
  this->OpacitySlider->setValue(1.);
  this->OpacitySlider->setSingleStep(0.1);
  QObject::connect(p, SIGNAL(orientationChanged(Qt::Orientation)),
                   this->OpacitySlider, SLOT(setOrientation(Qt::Orientation)));
  QObject::connect(this->OpacitySlider, SIGNAL(valueChanged(double)),
                   p, SLOT(setForegroundOpacity(double)));
  p->addWidget(this->OpacitySlider);
  p->addAction(actionShowFg);
}

// --------------------------------------------------------------------------
vtkCollection* qMRMLSlicesControllerToolBarPrivate::saveSliceCompositeNodes()const
{
  vtkCollection* nodes = this->MRMLScene ?
    this->MRMLScene->GetNodesByClass("vtkMRMLSliceCompositeNode") : 0;
  if (nodes)
    {
    this->MRMLScene->SaveStateForUndo(nodes);
    }
  return nodes;
}

// --------------------------------------------------------------------------
vtkCollection* qMRMLSlicesControllerToolBarPrivate::saveCrosshairNodes()const
{
  vtkCollection* nodes = this->MRMLScene ?
    this->MRMLScene->GetNodesByClass("vtkMRMLCrosshairNodes") : 0;
  if (nodes)
    {
    this->MRMLScene->SaveStateForUndo(nodes);
    }
  return nodes;
}

// --------------------------------------------------------------------------
// qMRMLSlicesControllerToolBar methods

// --------------------------------------------------------------------------
qMRMLSlicesControllerToolBar::qMRMLSlicesControllerToolBar(QWidget* _parent) : Superclass(_parent)
{
  CTK_INIT_PRIVATE(qMRMLSlicesControllerToolBar);
  CTK_D(qMRMLSlicesControllerToolBar);
  d->setupUi(this);
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setMRMLScene(vtkMRMLScene* scene)
{
  CTK_D(qMRMLSlicesControllerToolBar);
  d->MRMLScene = scene;
}

// --------------------------------------------------------------------------
vtkMRMLScene* qMRMLSlicesControllerToolBar::mrmlScene()const
{
  CTK_D(const qMRMLSlicesControllerToolBar);
  return d->MRMLScene;
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setFiducialPointsVisible(bool visible)
{
  CTK_D(qMRMLSlicesControllerToolBar);
  // do it in case if it's not done yet. usually it's a no-op
  d->actionFiducial_points->setChecked(visible);
  vtkCollection* nodes = d->saveSliceCompositeNodes();
  if (!nodes)
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
  nodes->Delete();
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setFiducialLabelsVisible(bool visible)
{
  CTK_D(qMRMLSlicesControllerToolBar);
  // do it in case if it's not done yet. usually it's a no-op
  d->actionFiducial_points->setChecked(visible);
  vtkCollection* nodes = d->saveSliceCompositeNodes();
  if (!nodes)
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
  nodes->Delete();
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setForegroundGridVisible(bool visible)
{
  CTK_D(qMRMLSlicesControllerToolBar);
  // do it in case if it's not done yet. usually it's a no-op
  d->actionForeground_grid->setChecked(visible);
  vtkCollection* nodes = d->saveSliceCompositeNodes();
  if (!nodes)
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
  nodes->Delete();
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setBackgroundGridVisible(bool visible)
{
  CTK_D(qMRMLSlicesControllerToolBar);
  // do it in case if it's not done yet. usually it's a no-op
  d->actionBackground_grid->setChecked(visible);
  vtkCollection* nodes = d->saveSliceCompositeNodes();
  if (!nodes)
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
  nodes->Delete();
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setLabelGridVisible(bool visible)
{
  CTK_D(qMRMLSlicesControllerToolBar);
  // do it in case if it's not done yet. usually it's a no-op
  d->actionLabel_grid->setChecked(visible);
  vtkCollection* nodes = d->saveSliceCompositeNodes();
  if (!nodes)
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
  nodes->Delete();
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::fitToWindow()
{

}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setLabelOpacity(double value)
{
  CTK_D(qMRMLSlicesControllerToolBar);
  // LabelOpacityToggleButton doesn't fire the clicked(bool) signal here because
  // it's check state is set programatically.
  d->LabelOpacityToggleButton->setChecked(value == 0.);
  vtkCollection* nodes = d->saveSliceCompositeNodes();
  if (!nodes)
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
  nodes->Delete();
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::toggleLabelOpacity(bool clicked)
{
  CTK_D(qMRMLSlicesControllerToolBar);
  d->LabelOpacitySlider->setValue(clicked ? 0. : d->LastLabelOpacity);
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setNavigatorEnabled(bool enable)
{
  CTK_D(qMRMLSlicesControllerToolBar);
  // do it in case if it's not done yet. usually it's a no-op
  d->actionCrosshairNavigator->setChecked(enable);
  vtkCollection* nodes = d->saveCrosshairNodes();
  if (!nodes)
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
  nodes->Delete();
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setSliceIntersectionVisible(bool visible)
{
  CTK_D(qMRMLSlicesControllerToolBar);
  // do it in case if it's not done yet. usually it's a no-op
  d->actionCrosshairSlice_intersections->setChecked(visible);
  vtkCollection* nodes = d->saveSliceCompositeNodes();
  if (!nodes)
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
  nodes->Delete();
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::toggleBackgroundForeground()
{
  CTK_D(qMRMLSlicesControllerToolBar);
  d->OpacitySlider->setValue(1. - d->OpacitySlider->value());
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::showBackground()
{
  CTK_D(qMRMLSlicesControllerToolBar);
  d->OpacitySlider->setValue(0.);
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::showForeground()
{
  CTK_D(qMRMLSlicesControllerToolBar);
  d->OpacitySlider->setValue(1.);
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setForegroundOpacity(double value)
{
  CTK_D(qMRMLSlicesControllerToolBar);
  d->OpacitySlider->setValue(value);
  vtkCollection* nodes = d->saveSliceCompositeNodes();
  if (!nodes)
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
  nodes->Delete();
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setAnnotationMode(int mode)
{
  CTK_D(qMRMLSlicesControllerToolBar);
  vtkCollection* nodes = d->saveSliceCompositeNodes();
  if (!nodes)
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
  nodes->Delete();
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setCompositing(int mode)
{
  CTK_D(qMRMLSlicesControllerToolBar);
  vtkCollection* nodes = d->saveSliceCompositeNodes();
  if (!nodes)
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
  nodes->Delete();
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setCrosshairMode(int mode)
{
  CTK_D(qMRMLSlicesControllerToolBar);
  vtkCollection* nodes = d->saveCrosshairNodes();
  if (!nodes)
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
  nodes->Delete();
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setCrosshairThickness(int thicknessMode)
{
  CTK_D(qMRMLSlicesControllerToolBar);
  vtkCollection* nodes = d->saveCrosshairNodes();
  if (!nodes)
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
  nodes->Delete();
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setAnnotationSpace(int spatialUnits)
{
  CTK_D(qMRMLSlicesControllerToolBar);
  vtkCollection* nodes = d->saveSliceCompositeNodes();
  if (!nodes)
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
  nodes->Delete();
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setRedSliceFOV(double fov)
{
  //
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setYellowSliceFOV(double fov)
{
  //
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setGreenSliceFOV(double fov)
{
  //
}

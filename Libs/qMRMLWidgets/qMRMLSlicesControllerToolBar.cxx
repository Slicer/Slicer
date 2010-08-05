
// Qt includes
#include <QAction>
#include <QActionGroup>
#include <QDebug>
#include <QMenu>
#include <QToolButton>

// CTK includes
#include <ctkDoubleSlider.h>
#include <ctkLogger.h>

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
  ctkDoubleSlider*         OpacitySlider;
  qMRMLActionSignalMapper* AnnotationsMapper;
  qMRMLActionSignalMapper* CompositingMapper;
  qMRMLActionSignalMapper* CrosshairMapper;
  qMRMLActionSignalMapper* CrosshairThicknessMapper;
  qMRMLActionSignalMapper* SpatialUnitsMapper;
};
//--------------------------------------------------------------------------
// qMRMLSlicesControllerToolBarPrivate methods

//---------------------------------------------------------------------------
qMRMLSlicesControllerToolBarPrivate::qMRMLSlicesControllerToolBarPrivate()
{
  this->MRMLScene = 0;
  this->OpacitySlider = 0;
  this->AnnotationsMapper = 0;
  this->CompositingMapper = 0;
  this->CrosshairMapper = 0;
  this->CrosshairThicknessMapper = 0;
  this->SpatialUnitsMapper = 0;
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

  // Fiducial Visibility
  QToolButton* fiducialsButton = new QToolButton(p);
  fiducialsButton->setPopupMode(QToolButton::InstantPopup);
  fiducialsButton->setText("Fiducials");
  fiducialsButton->setIcon(QIcon(":Icons/SlicesLabelOpacity.png"));
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
  QToolButton* fieldOfViewButton = new QToolButton(p);
  fieldOfViewButton->setPopupMode(QToolButton::InstantPopup);
  fieldOfViewButton->setText("Slices Field of View");
  fieldOfViewButton->setIcon(QIcon(":Icons/SlicesFieldOfView.png"));
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

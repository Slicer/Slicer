
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

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.libs.qmrmlwidgets.qMRMLSlicesControllerToolBar");
//--------------------------------------------------------------------------

//-----------------------------------------------------------------------------
class qMRMLSlicesControllerToolBarPrivate
  : public ctkPrivate<qMRMLSlicesControllerToolBar>
  , public Ui_qMRMLSlicesControllerToolBar
{
public:
  CTK_DECLARE_PUBLIC(qMRMLSlicesControllerToolBar);
  qMRMLSlicesControllerToolBarPrivate();

  virtual void setupUi(QWidget* widget);

  vtkMRMLScene* MRMLScene;
  ctkDoubleSlider* OpacitySlider;
};
//--------------------------------------------------------------------------
// qMRMLSlicesControllerToolBarPrivate methods

//---------------------------------------------------------------------------
qMRMLSlicesControllerToolBarPrivate::qMRMLSlicesControllerToolBarPrivate()
{
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
  QObject::connect(annotationsActions, SIGNAL(triggered(QAction*)),
          p, SLOT(onAnnotationTriggered(QAction*)));
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
  QObject::connect(compositingActions, SIGNAL(triggered(QAction*)),
          p, SLOT(onCompositingActionTriggered(QAction*)));
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
  QObject::connect(crosshairActions, SIGNAL(triggered(QAction*)),
          p, SLOT(onCrosshairActionTriggered(QAction*)));
  QActionGroup* crosshairThicknessActions = new QActionGroup(p);
  crosshairThicknessActions->setExclusive(true);
  crosshairThicknessActions->addAction(actionCrosshairFine);
  crosshairThicknessActions->addAction(actionCrosshairMedium);
  crosshairThicknessActions->addAction(actionCrosshairThick);
  QObject::connect(crosshairThicknessActions, SIGNAL(triggered(QAction*)),
          p, SLOT(onCrosshairThicknessActionTriggered(QAction*)));
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
  QObject::connect(spatialUnitsActions, SIGNAL(triggered(QAction*)),
          p, SLOT(onSpatialUnitsActionTriggered(QAction*)));
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
  p->addWidget(this->OpacitySlider);
  p->addAction(actionShowFg);
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

}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setFiducialLabelsVisible(bool visible)
{

}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setForegroundGridVisible(bool visible)
{

}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setBackgroundGridVisible(bool visible)
{

}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setLabelGridVisible(bool visible)
{

}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::fitToWindow()
{

}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setNavigatorVisible(bool visible)
{

}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::setSliceIntersectionVisible(bool visible)
{

}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::toggleBackgroundForeground()
{
  CTK_D(qMRMLSlicesControllerToolBar);
  d->OpacitySlider->setValue( 1. - d->OpacitySlider->value() );
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::showBackground()
{
  CTK_D(qMRMLSlicesControllerToolBar);
  d->OpacitySlider->setValue( 0. );
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::showForeground()
{
  CTK_D(qMRMLSlicesControllerToolBar);
  d->OpacitySlider->setValue( 1. );
}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::onAnnotationTriggered(QAction*)
{

}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::onCompositingActionTriggered(QAction*)
{

}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::onCrosshairActionTriggered(QAction*)
{

}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::onCrosshairThicknessActionTriggered(QAction*)
{

}

// --------------------------------------------------------------------------
void qMRMLSlicesControllerToolBar::onSpatialUnitsActionTriggered(QAction*)
{

}

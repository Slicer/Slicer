/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

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
#include <QActionGroup>
#include <QDebug>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QWidgetAction>

// CTK includes
#include <ctkButtonGroup.h>
#include <ctkPopupWidget.h>
#include <ctkSignalMapper.h>
#include <ctkSliderWidget.h>

// qMRML includes
#include "qMRMLColors.h"
#include "qMRMLNodeFactory.h"
#include "qMRMLSceneViewMenu.h"
#include "qMRMLThreeDView.h"
#include "qMRMLThreeDViewControllerWidget_p.h"

// MRML includes
#include <vtkMRMLCameraNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSceneViewNode.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkNew.h>
#include <vtkRenderWindow.h>

//--------------------------------------------------------------------------
// qMRMLThreeDViewControllerWidgetPrivate methods

//---------------------------------------------------------------------------
qMRMLThreeDViewControllerWidgetPrivate::qMRMLThreeDViewControllerWidgetPrivate(qMRMLThreeDViewControllerWidget& object)
  : Superclass(object)
  , CameraNode(nullptr)
  , ThreeDView(nullptr)
  , ViewLogic(nullptr)
  , StereoTypesMapper(nullptr)
  , AnimateViewButtonGroup(nullptr)
  , OrientationMarkerTypesMapper(nullptr)
  , OrientationMarkerSizesMapper(nullptr)
  , RulerTypesMapper(nullptr)
  , RulerColorMapper(nullptr)
  , CenterToolButton(nullptr)
{
}

//---------------------------------------------------------------------------
qMRMLThreeDViewControllerWidgetPrivate::~qMRMLThreeDViewControllerWidgetPrivate() = default;

//---------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidgetPrivate::setupPopupUi()
{
  Q_Q(qMRMLThreeDViewControllerWidget);

  this->Superclass::setupPopupUi();
  this->PopupWidget->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
  this->Ui_qMRMLThreeDViewControllerWidget::setupUi(this->PopupWidget);

  // Look from axes
  QObject::connect(
    this->AxesWidget, SIGNAL(currentAxisChanged(ctkAxesWidget::Axis)), q, SLOT(lookFromAxis(ctkAxesWidget::Axis)));

  // ViewLink button
  QObject::connect(this->ViewLinkButton, SIGNAL(toggled(bool)), q, SLOT(setViewLink(bool)));

  // Orthographic/perspective button
  QObject::connect(this->OrthoButton, SIGNAL(toggled(bool)), q, SLOT(setOrthographicModeEnabled(bool)));

  // ZoomIn, ZoomOut button
  QObject::connect(this->ZoomInButton, SIGNAL(clicked()), q, SLOT(zoomIn()));
  QObject::connect(this->ZoomOutButton, SIGNAL(clicked()), q, SLOT(zoomOut()));

  // ResetFocalPoint button
  this->CenterButton->setDefaultAction(this->actionCenter);
  QObject::connect(this->actionCenter, SIGNAL(triggered()), q, SLOT(resetFocalPoint()));

  QMenu* visibilityMenu = new QMenu(qMRMLThreeDViewControllerWidget::tr("Visibility"), this->PopupWidget);
  visibilityMenu->setObjectName("visibilityMenu");
  this->VisibilityButton->setMenu(visibilityMenu);

  // Show 3D Axis, 3D Axis label
  visibilityMenu->addAction(this->actionSet3DAxisVisible);
  visibilityMenu->addAction(this->actionSet3DAxisLabelVisible);
  QObject::connect(this->actionSet3DAxisVisible, SIGNAL(triggered(bool)), q, SLOT(set3DAxisVisible(bool)));
  QObject::connect(this->actionSet3DAxisLabelVisible, SIGNAL(triggered(bool)), q, SLOT(set3DAxisLabelVisible(bool)));

  this->setupShadowsMenu();

  // OrientationMarker actions
  // Type
  this->OrientationMarkerTypesMapper = new ctkSignalMapper(this->PopupWidget);
  this->OrientationMarkerTypesMapper->setMapping(this->actionOrientationMarkerTypeNone,
                                                 vtkMRMLAbstractViewNode::OrientationMarkerTypeNone);
  this->OrientationMarkerTypesMapper->setMapping(this->actionOrientationMarkerTypeCube,
                                                 vtkMRMLAbstractViewNode::OrientationMarkerTypeCube);
  this->OrientationMarkerTypesMapper->setMapping(this->actionOrientationMarkerTypeHuman,
                                                 vtkMRMLAbstractViewNode::OrientationMarkerTypeHuman);
  this->OrientationMarkerTypesMapper->setMapping(this->actionOrientationMarkerTypeAxes,
                                                 vtkMRMLAbstractViewNode::OrientationMarkerTypeAxes);
  QActionGroup* orientationMarkerTypesActions = new QActionGroup(this->PopupWidget);
  orientationMarkerTypesActions->setExclusive(true);
  orientationMarkerTypesActions->addAction(this->actionOrientationMarkerTypeNone);
  orientationMarkerTypesActions->addAction(this->actionOrientationMarkerTypeCube);
  orientationMarkerTypesActions->addAction(this->actionOrientationMarkerTypeHuman);
  orientationMarkerTypesActions->addAction(this->actionOrientationMarkerTypeAxes);
  QObject::connect(this->OrientationMarkerTypesMapper, SIGNAL(mapped(int)), q, SLOT(setOrientationMarkerType(int)));
  QObject::connect(orientationMarkerTypesActions,
                   SIGNAL(triggered(QAction*)),
                   this->OrientationMarkerTypesMapper,
                   SLOT(map(QAction*)));
  // Size
  this->OrientationMarkerSizesMapper = new ctkSignalMapper(this->PopupWidget);
  this->OrientationMarkerSizesMapper->setMapping(this->actionOrientationMarkerSizeSmall,
                                                 vtkMRMLAbstractViewNode::OrientationMarkerSizeSmall);
  this->OrientationMarkerSizesMapper->setMapping(this->actionOrientationMarkerSizeMedium,
                                                 vtkMRMLAbstractViewNode::OrientationMarkerSizeMedium);
  this->OrientationMarkerSizesMapper->setMapping(this->actionOrientationMarkerSizeLarge,
                                                 vtkMRMLAbstractViewNode::OrientationMarkerSizeLarge);
  QActionGroup* orientationMarkerSizesActions = new QActionGroup(this->PopupWidget);
  orientationMarkerSizesActions->setExclusive(true);
  orientationMarkerSizesActions->addAction(this->actionOrientationMarkerSizeSmall);
  orientationMarkerSizesActions->addAction(this->actionOrientationMarkerSizeMedium);
  orientationMarkerSizesActions->addAction(this->actionOrientationMarkerSizeLarge);
  QObject::connect(this->OrientationMarkerSizesMapper, SIGNAL(mapped(int)), q, SLOT(setOrientationMarkerSize(int)));
  QObject::connect(orientationMarkerSizesActions,
                   SIGNAL(triggered(QAction*)),
                   this->OrientationMarkerSizesMapper,
                   SLOT(map(QAction*)));
  // Menu
  QMenu* orientationMarkerMenu =
    new QMenu(qMRMLThreeDViewControllerWidget::tr("Orientation marker"), this->PopupWidget);
  orientationMarkerMenu->setObjectName("orientationMarkerMenu");
  this->OrientationMarkerButton->setMenu(orientationMarkerMenu);
  orientationMarkerMenu->addActions(orientationMarkerTypesActions->actions());
  orientationMarkerMenu->addSeparator();
  orientationMarkerMenu->addActions(orientationMarkerSizesActions->actions());

  // Ruler actions
  // Type
  this->RulerTypesMapper = new ctkSignalMapper(this->PopupWidget);
  this->RulerTypesMapper->setMapping(this->actionRulerTypeNone, vtkMRMLAbstractViewNode::RulerTypeNone);
  this->RulerTypesMapper->setMapping(this->actionRulerTypeThin, vtkMRMLAbstractViewNode::RulerTypeThin);
  this->RulerTypesMapper->setMapping(this->actionRulerTypeThick, vtkMRMLAbstractViewNode::RulerTypeThick);
  QActionGroup* rulerTypesActions = new QActionGroup(this->PopupWidget);
  rulerTypesActions->setExclusive(true);
  rulerTypesActions->addAction(this->actionRulerTypeNone);
  rulerTypesActions->addAction(this->actionRulerTypeThin);
  rulerTypesActions->addAction(this->actionRulerTypeThick);
  QObject::connect(this->RulerTypesMapper, SIGNAL(mapped(int)), q, SLOT(setRulerType(int)));
  QObject::connect(rulerTypesActions, SIGNAL(triggered(QAction*)), this->RulerTypesMapper, SLOT(map(QAction*)));
  // Color
  this->RulerColorMapper = new ctkSignalMapper(this->PopupWidget);
  this->RulerColorMapper->setMapping(this->actionRulerColorWhite, vtkMRMLAbstractViewNode::RulerColorWhite);
  this->RulerColorMapper->setMapping(this->actionRulerColorBlack, vtkMRMLAbstractViewNode::RulerColorBlack);
  this->RulerColorMapper->setMapping(this->actionRulerColorYellow, vtkMRMLAbstractViewNode::RulerColorYellow);
  QActionGroup* rulerColorActions = new QActionGroup(this->PopupWidget);
  rulerColorActions->setExclusive(true);
  rulerColorActions->addAction(this->actionRulerColorWhite);
  rulerColorActions->addAction(this->actionRulerColorBlack);
  rulerColorActions->addAction(this->actionRulerColorYellow);
  QObject::connect(this->RulerColorMapper, SIGNAL(mapped(int)), q, SLOT(setRulerColor(int)));
  QObject::connect(rulerColorActions, SIGNAL(triggered(QAction*)), this->RulerColorMapper, SLOT(map(QAction*)));

  // Menu
  QMenu* rulerMenu = new QMenu(qMRMLThreeDViewControllerWidget::tr("Ruler"), this->PopupWidget);
  rulerMenu->setObjectName("rulerMenu");
  this->RulerButton->setMenu(rulerMenu);
  rulerMenu->addActions(rulerTypesActions->actions());
  rulerMenu->addSeparator();
  rulerMenu->addActions(rulerColorActions->actions());

  // More controls
  QMenu* moreMenu = new QMenu(qMRMLThreeDViewControllerWidget::tr("More"), this->PopupWidget);
  moreMenu->addAction(this->actionUseDepthPeeling);
  moreMenu->addAction(this->actionStereo);
  moreMenu->addAction(this->actionSetFPSVisible);
  this->MoreToolButton->setMenu(moreMenu);

  // Depth peeling
  QObject::connect(this->actionUseDepthPeeling, SIGNAL(toggled(bool)), q, SLOT(setUseDepthPeeling(bool)));

  // StereoType actions
  this->StereoTypesMapper = new ctkSignalMapper(this->PopupWidget);
  this->StereoTypesMapper->setMapping(this->actionNoStereo, vtkMRMLViewNode::NoStereo);
  this->StereoTypesMapper->setMapping(this->actionSwitchToAnaglyphStereo, vtkMRMLViewNode::Anaglyph);
  this->StereoTypesMapper->setMapping(this->actionSwitchToQuadBufferStereo, vtkMRMLViewNode::QuadBuffer);
  this->StereoTypesMapper->setMapping(this->actionSwitchToInterlacedStereo, vtkMRMLViewNode::Interlaced);
  this->StereoTypesMapper->setMapping(this->actionSwitchToRedBlueStereo, vtkMRMLViewNode::RedBlue);
  this->StereoTypesMapper->setMapping(this->actionSwitchToUserDefinedStereo_1, vtkMRMLViewNode::UserDefined_1);
  this->StereoTypesMapper->setMapping(this->actionSwitchToUserDefinedStereo_2, vtkMRMLViewNode::UserDefined_2);
  this->StereoTypesMapper->setMapping(this->actionSwitchToUserDefinedStereo_3, vtkMRMLViewNode::UserDefined_3);
  QActionGroup* stereoTypesActions = new QActionGroup(this->PopupWidget);
  stereoTypesActions->setExclusive(true);
  stereoTypesActions->addAction(this->actionNoStereo);
  stereoTypesActions->addAction(this->actionSwitchToRedBlueStereo);
  stereoTypesActions->addAction(this->actionSwitchToAnaglyphStereo);
  stereoTypesActions->addAction(this->actionSwitchToInterlacedStereo);
  stereoTypesActions->addAction(this->actionSwitchToQuadBufferStereo);
  stereoTypesActions->addAction(this->actionSwitchToUserDefinedStereo_1);
  stereoTypesActions->addAction(this->actionSwitchToUserDefinedStereo_2);
  stereoTypesActions->addAction(this->actionSwitchToUserDefinedStereo_3);
  QMenu* stereoTypesMenu = new QMenu(qMRMLThreeDViewControllerWidget::tr("Stereo Modes"), this->PopupWidget);
  stereoTypesMenu->setObjectName("stereoTypesMenu");
  stereoTypesMenu->addActions(stereoTypesActions->actions());
  this->actionStereo->setMenu(stereoTypesMenu);
  QObject::connect(this->StereoTypesMapper, SIGNAL(mapped(int)), q, SLOT(setStereoType(int)));
  QObject::connect(stereoTypesActions, SIGNAL(triggered(QAction*)), this->StereoTypesMapper, SLOT(map(QAction*)));
  this->actionSwitchToQuadBufferStereo->setEnabled(false); // Disabled by default

  // FPS
  QObject::connect(this->actionSetFPSVisible, SIGNAL(toggled(bool)), q, SLOT(setFPSVisible(bool)));

  // Background color
  QActionGroup* backgroundColorActions = new QActionGroup(this->PopupWidget);
  backgroundColorActions->setExclusive(true);
  visibilityMenu->addSeparator();
  visibilityMenu->addAction(this->actionSetLightBlueBackground);
  visibilityMenu->addAction(this->actionSetBlackBackground);
  visibilityMenu->addAction(this->actionSetWhiteBackground);
  backgroundColorActions->addAction(this->actionSetLightBlueBackground);
  backgroundColorActions->addAction(this->actionSetBlackBackground);
  backgroundColorActions->addAction(this->actionSetWhiteBackground);
  QObject::connect(this->actionSetLightBlueBackground, SIGNAL(triggered()), q, SLOT(setLightBlueBackground()));
  QObject::connect(this->actionSetWhiteBackground, SIGNAL(triggered()), q, SLOT(setWhiteBackground()));
  QObject::connect(this->actionSetBlackBackground, SIGNAL(triggered()), q, SLOT(setBlackBackground()));

  // SpinView, RockView buttons
  this->AnimateViewButtonGroup = new ctkButtonGroup(this->PopupWidget);
  this->AnimateViewButtonGroup->addButton(this->SpinButton, vtkMRMLViewNode::Spin);
  this->AnimateViewButtonGroup->addButton(this->RockButton, vtkMRMLViewNode::Rock);
  QObject::connect(this->SpinButton, SIGNAL(toggled(bool)), q, SLOT(spinView(bool)));
  QObject::connect(this->RockButton, SIGNAL(toggled(bool)), q, SLOT(rockView(bool)));
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidgetPrivate::setupShadowsMenu()
{
  Q_Q(qMRMLThreeDViewControllerWidget);
  this->ShadowsMenu = new QMenu(qMRMLThreeDViewControllerWidget::tr("Shadows"), this->ShadowsButton);
  this->ShadowsMenu->setObjectName("shadowsMenu");

  this->ShadowsMenu->addAction(this->actionShadowsVisibility);

  QObject::connect(this->actionShadowsVisibility, SIGNAL(toggled(bool)), q, SLOT(setShadowsVisibility(bool)));

  // Size scale
  QMenu* ambientShadowsSizeScaleMenu =
    new QMenu(qMRMLThreeDViewControllerWidget::tr("Size scale"), this->ShadowsButton);
  ambientShadowsSizeScaleMenu->setObjectName("ambienShadowsSizeScale");
  this->AmbientShadowsSizeScaleSlider = new ctkSliderWidget(ambientShadowsSizeScaleMenu);
  this->AmbientShadowsSizeScaleSlider->setToolTip(qMRMLThreeDViewControllerWidget::tr(
    "Size of features to be emphasized by shadows."
    " The scale is logarithmic, default (0.0) corresponds to object size of about 100mm."));
  this->AmbientShadowsSizeScaleSlider->setDecimals(2);
  this->AmbientShadowsSizeScaleSlider->setRange(-3., 3.);
  this->AmbientShadowsSizeScaleSlider->setSingleStep(0.01);
  this->AmbientShadowsSizeScaleSlider->setPageStep(0.1);
  this->AmbientShadowsSizeScaleSlider->setValue(0.);
  QObject::connect(
    this->AmbientShadowsSizeScaleSlider, SIGNAL(valueChanged(double)), q, SLOT(setAmbientShadowsSizeScale(double)));
  this->connect(
    this->actionShadowsVisibility, SIGNAL(toggled(bool)), this->AmbientShadowsSizeScaleSlider, SLOT(setEnabled(bool)));
  QWidgetAction* ambientShadowsSizeScaleAction = new QWidgetAction(ambientShadowsSizeScaleMenu);
  ambientShadowsSizeScaleAction->setDefaultWidget(this->AmbientShadowsSizeScaleSlider);
  ambientShadowsSizeScaleMenu->addAction(ambientShadowsSizeScaleAction);
  this->ShadowsMenu->addMenu(ambientShadowsSizeScaleMenu);

  // Volume opacity threshold
  QMenu* ambientShadowsVolumeOpacityThresholdMenu =
    new QMenu(qMRMLThreeDViewControllerWidget::tr("Volume opacity threshold"), this->ShadowsButton);
  ambientShadowsVolumeOpacityThresholdMenu->setObjectName("ambienShadowsVolumeOpacityThreshold");
  this->AmbientShadowsVolumeOpacityThresholdPercentSlider =
    new ctkSliderWidget(ambientShadowsVolumeOpacityThresholdMenu);
  this->AmbientShadowsVolumeOpacityThresholdPercentSlider->setToolTip(
    qMRMLThreeDViewControllerWidget::tr("Volume rendering opacity above this will cast shadows."));
  this->AmbientShadowsVolumeOpacityThresholdPercentSlider->setSuffix("%");
  this->AmbientShadowsVolumeOpacityThresholdPercentSlider->setDecimals(0);
  this->AmbientShadowsVolumeOpacityThresholdPercentSlider->setRange(0., 100.);
  this->AmbientShadowsVolumeOpacityThresholdPercentSlider->setSingleStep(1.);
  this->AmbientShadowsVolumeOpacityThresholdPercentSlider->setValue(0.);
  QObject::connect(this->AmbientShadowsVolumeOpacityThresholdPercentSlider,
                   SIGNAL(valueChanged(double)),
                   q,
                   SLOT(setAmbientShadowsVolumeOpacityThresholdPercent(double)));
  this->connect(this->actionShadowsVisibility,
                SIGNAL(toggled(bool)),
                this->AmbientShadowsVolumeOpacityThresholdPercentSlider,
                SLOT(setEnabled(bool)));
  QWidgetAction* ambientShadowsVolumeOpacityThresholdAction =
    new QWidgetAction(ambientShadowsVolumeOpacityThresholdMenu);
  ambientShadowsVolumeOpacityThresholdAction->setDefaultWidget(this->AmbientShadowsVolumeOpacityThresholdPercentSlider);
  ambientShadowsVolumeOpacityThresholdMenu->addAction(ambientShadowsVolumeOpacityThresholdAction);
  this->ShadowsMenu->addMenu(ambientShadowsVolumeOpacityThresholdMenu);

  this->ShadowsButton->setMenu(this->ShadowsMenu);
}
//---------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidgetPrivate::init()
{
  Q_Q(qMRMLThreeDViewControllerWidget);
  this->Superclass::init();

  this->CenterToolButton = new QToolButton(q);
  this->CenterToolButton->setAutoRaise(true);
  this->CenterToolButton->setDefaultAction(this->actionCenter);
  this->CenterToolButton->setObjectName("CenterButton_Header");
  this->BarLayout->insertWidget(2, this->CenterToolButton);

  this->ViewLabel->setText(qMRMLThreeDViewControllerWidget::tr("1"));
  this->BarLayout->addStretch(1);

  vtkNew<vtkMRMLViewLogic> defaultLogic;
  q->setViewLogic(defaultLogic.GetPointer());
}

// --------------------------------------------------------------------------
// qMRMLThreeDViewControllerWidget methods

// --------------------------------------------------------------------------
qMRMLThreeDViewControllerWidget::qMRMLThreeDViewControllerWidget(QWidget* parentWidget)
  : Superclass(new qMRMLThreeDViewControllerWidgetPrivate(*this), parentWidget)
{
  Q_D(qMRMLThreeDViewControllerWidget);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLThreeDViewControllerWidget::~qMRMLThreeDViewControllerWidget() = default;

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::setThreeDView(qMRMLThreeDView* view)
{
  Q_D(qMRMLThreeDViewControllerWidget);
  d->ThreeDView = view;
  if (d->ThreeDView != nullptr)
  {
    d->actionSwitchToQuadBufferStereo->setEnabled(d->ThreeDView->renderWindow()->GetStereoCapableWindow());
    // TODO: we could get layout name from the view node and keep it up-to-date using signal connection
    /*
    if (view->mrmlViewNode())
      {
      this->setMRMLViewNode(view->mrmlViewNode());
      }
    else if (this->mrml)
    */
  }
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::setMRMLViewNode(vtkMRMLAbstractViewNode* viewNode)
{
  Q_D(qMRMLThreeDViewControllerWidget);
  Superclass::setMRMLViewNode(viewNode);

  std::string layoutName;
  if (this->mrmlThreeDViewNode())
  {
    if (this->mrmlThreeDViewNode()->GetLayoutName())
    {
      layoutName = this->mrmlThreeDViewNode()->GetLayoutName();
    }
    else
    {
      qCritical() << "qMRMLThreeDViewControllerWidget::setMRMLViewNode failed: invalid layout name";
    }
  }
  d->CameraNode = d->ViewLogic->GetCameraNode(this->mrmlScene(), layoutName.c_str());
  this->qvtkReconnect(d->CameraNode, vtkMRMLCameraNode::CameraInteractionEvent, this, SLOT(updateViewFromMRMLCamera()));

  this->updateViewFromMRMLCamera();
}

//---------------------------------------------------------------------------
vtkMRMLViewNode* qMRMLThreeDViewControllerWidget::mrmlThreeDViewNode() const
{
  Q_D(const qMRMLThreeDViewControllerWidget);
  return vtkMRMLViewNode::SafeDownCast(this->mrmlViewNode());
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::setViewLink(bool linked)
{
  if (!this->mrmlScene())
  {
    return;
  }

  vtkCollection* viewNodes = this->mrmlScene()->GetNodesByClass("vtkMRMLViewNode");
  if (!viewNodes)
  {
    return;
  }

  vtkMRMLViewNode* viewNode = nullptr;
  for (viewNodes->InitTraversal(); (viewNode = vtkMRMLViewNode::SafeDownCast(viewNodes->GetNextItemAsObject()));)
  {
    viewNode->SetLinkedControl(linked);
  }
  viewNodes->Delete();
}

//---------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::setViewLabel(const QString& newViewLabel)
{
  Q_D(qMRMLThreeDViewControllerWidget);
  if (!this->mrmlThreeDViewNode())
  {
    qCritical() << Q_FUNC_INFO << " failed: must set view node first";
    return;
  }
  this->mrmlThreeDViewNode()->SetLayoutLabel(newViewLabel.toUtf8());
}

//---------------------------------------------------------------------------
QString qMRMLThreeDViewControllerWidget::viewLabel() const
{
  Q_D(const qMRMLThreeDViewControllerWidget);
  if (!this->mrmlThreeDViewNode())
  {
    qCritical() << Q_FUNC_INFO << " failed: must set view node first";
    return QString();
  }
  return this->mrmlThreeDViewNode()->GetLayoutLabel();
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::updateWidgetFromMRMLViewLogic()
{
  Q_D(qMRMLThreeDViewControllerWidget);

  if (d->ViewLogic && d->ViewLogic->GetMRMLScene())
  {
    this->setMRMLScene(d->ViewLogic->GetMRMLScene());
  }

  // Update camera node connection
  vtkMRMLCameraNode* cameraNode = (d->ViewLogic ? d->ViewLogic->GetCameraNode() : nullptr);
  if (cameraNode != d->CameraNode)
  {
    this->qvtkReconnect(
      d->CameraNode, cameraNode, vtkMRMLCameraNode::CameraInteractionEvent, this, SLOT(updateViewFromMRMLCamera()));
    d->CameraNode = cameraNode;
    this->updateViewFromMRMLCamera();
  }

  // Update view node connection
  vtkMRMLViewNode* viewNode = (d->ViewLogic ? d->ViewLogic->GetViewNode() : nullptr);
  if (viewNode != this->mrmlThreeDViewNode())
  {
    this->setMRMLViewNode(viewNode);
  }
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::updateWidgetFromMRMLView()
{
  Q_D(qMRMLThreeDViewControllerWidget);
  Superclass::updateWidgetFromMRMLView();
  // Enable buttons
  vtkMRMLViewNode* viewNode = this->mrmlThreeDViewNode();
  QList<QWidget*> widgets;
  widgets << d->AxesWidget << d->CenterButton << d->OrthoButton << d->VisibilityButton << d->ZoomInButton
          << d->ZoomOutButton << d->ShadowsButton << d->RockButton << d->SpinButton << d->MoreToolButton
          << d->OrientationMarkerButton; // RulerButton enable state is not set here (it depends on render mode)
  foreach (QWidget* w, widgets)
  {
    w->setEnabled(viewNode != nullptr);
  }

  if (!viewNode)
  {
    return;
  }

  // In the axes widget the order of labels is: +X, -X, +Z, -Z, +Y, -Y
  // and in the view node axis labels order is: -X, +X, -Y, +Y, -Z, +Z.
  QStringList axesLabels;
  axesLabels << viewNode->GetAxisLabel(1); // +X
  axesLabels << viewNode->GetAxisLabel(0); // -X
  axesLabels << viewNode->GetAxisLabel(5); // +Z
  axesLabels << viewNode->GetAxisLabel(4); // -Z
  axesLabels << viewNode->GetAxisLabel(3); // +Y
  axesLabels << viewNode->GetAxisLabel(2); // -Y
  d->AxesWidget->setAxesLabels(axesLabels);

  // Update view link toggle. Must be done first as its state controls
  // different behaviors when properties are set.
  d->ViewLinkButton->setChecked(viewNode->GetLinkedControl());
  if (viewNode->GetLinkedControl())
  {
    d->ViewLinkButton->setIcon(QIcon(":Icons/LinkOn.png"));
  }
  else
  {
    d->ViewLinkButton->setIcon(QIcon(":Icons/LinkOff.png"));
  }

  d->actionSet3DAxisVisible->setChecked(viewNode->GetBoxVisible());
  d->actionSet3DAxisLabelVisible->setChecked(viewNode->GetAxisLabelsVisible());

  d->actionUseDepthPeeling->setChecked(viewNode->GetUseDepthPeeling());
  d->actionSetFPSVisible->setChecked(viewNode->GetFPSVisible());

  double* color = viewNode->GetBackgroundColor();
  QColor backgroundColor = QColor::fromRgbF(color[0], color[1], color[2]);
  d->actionSetBlackBackground->setChecked(backgroundColor == Qt::black);
  d->actionSetWhiteBackground->setChecked(backgroundColor == Qt::white);
  d->actionSetLightBlueBackground->setChecked(!d->actionSetBlackBackground->isChecked()
                                              && !d->actionSetWhiteBackground->isChecked());

  d->OrthoButton->setChecked(viewNode->GetRenderMode() == vtkMRMLViewNode::Orthographic);

  QAction* action = qobject_cast<QAction*>(d->StereoTypesMapper->mapping(viewNode->GetStereoType()));
  if (action)
  {
    action->setChecked(true);
  }
  action = qobject_cast<QAction*>(d->OrientationMarkerTypesMapper->mapping(viewNode->GetOrientationMarkerType()));
  if (action)
  {
    action->setChecked(true);
  }
  action = qobject_cast<QAction*>(d->OrientationMarkerSizesMapper->mapping(viewNode->GetOrientationMarkerSize()));
  if (action)
  {
    action->setChecked(true);
  }
  action = qobject_cast<QAction*>(d->RulerTypesMapper->mapping(viewNode->GetRulerType()));
  if (action)
  {
    action->setChecked(true);
  }
  d->RulerButton->setEnabled(viewNode->GetRenderMode() == vtkMRMLViewNode::Orthographic);

  d->SpinButton->setChecked(viewNode->GetAnimationMode() == vtkMRMLViewNode::Spin);
  d->RockButton->setChecked(viewNode->GetAnimationMode() == vtkMRMLViewNode::Rock);

  d->ViewLabel->setText(viewNode->GetLayoutLabel());

  double* layoutColorVtk = viewNode->GetLayoutColor();
  QColor layoutColor = QColor::fromRgbF(layoutColorVtk[0], layoutColorVtk[1], layoutColorVtk[2]);
  d->setColor(layoutColor);

  d->actionShadowsVisibility->setChecked(viewNode->GetShadowsVisibility());
  d->AmbientShadowsSizeScaleSlider->setValue(viewNode->GetAmbientShadowsSizeScale());
  d->AmbientShadowsVolumeOpacityThresholdPercentSlider->setValue(viewNode->GetAmbientShadowsVolumeOpacityThreshold()
                                                                 * 100.0);
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::updateViewFromMRMLCamera()
{
  Q_D(qMRMLThreeDViewControllerWidget);
  if (d->CameraNode)
  {
    d->ViewLogic->StartCameraNodeInteraction(vtkMRMLCameraNode::CameraInteractionFlag);
    d->CameraNode->Modified();
    d->ViewLogic->EndCameraNodeInteraction();
  }
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::setQuadBufferStereoSupportEnabled(bool value)
{
  Q_D(qMRMLThreeDViewControllerWidget);
  d->actionSwitchToQuadBufferStereo->setEnabled(value);
}

// --------------------------------------------------------------------------
vtkMRMLViewLogic* qMRMLThreeDViewControllerWidget::viewLogic() const
{
  Q_D(const qMRMLThreeDViewControllerWidget);
  return d->ViewLogic;
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::setViewLogic(vtkMRMLViewLogic* newViewLogic)
{
  Q_D(qMRMLThreeDViewControllerWidget);
  if (d->ViewLogic == newViewLogic)
  {
    return;
  }

  this->qvtkReconnect(
    d->ViewLogic, newViewLogic, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRMLViewLogic()));

  d->ViewLogic = newViewLogic;

  this->updateWidgetFromMRMLViewLogic();
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_D(qMRMLThreeDViewControllerWidget);

  if (this->mrmlScene() == newScene)
  {
    return;
  }

  this->qvtkReconnect(
    this->mrmlScene(), newScene, vtkMRMLScene::EndBatchProcessEvent, this, SLOT(updateWidgetFromMRMLView()));

  d->ViewLogic->SetMRMLScene(newScene);

  this->Superclass::setMRMLScene(newScene);

  if (this->mrmlScene())
  {
    this->updateWidgetFromMRMLView();
  }
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::setOrthographicModeEnabled(bool enabled)
{
  Q_D(qMRMLThreeDViewControllerWidget);

  if (!d->ViewLogic || !this->mrmlThreeDViewNode())
  {
    return;
  }

  d->ViewLogic->StartViewNodeInteraction(vtkMRMLViewNode::RenderModeFlag);
  this->mrmlThreeDViewNode()->SetRenderMode(enabled ? vtkMRMLViewNode::Orthographic : vtkMRMLViewNode::Perspective);
  d->ViewLogic->EndViewNodeInteraction();
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::lookFromAxis(const ctkAxesWidget::Axis& axis)
{
  Q_D(qMRMLThreeDViewControllerWidget);
  if (!d->ThreeDView)
  {
    return;
  }

  d->ViewLogic->StartCameraNodeInteraction(vtkMRMLCameraNode::LookFromAxis);
  d->ThreeDView->lookFromAxis(axis);
  d->ViewLogic->EndCameraNodeInteraction();
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::pitchView()
{
  Q_D(qMRMLThreeDViewControllerWidget);
  if (!d->ThreeDView)
  {
    return;
  }
  d->ThreeDView->pitch();
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::rollView()
{
  Q_D(qMRMLThreeDViewControllerWidget);
  if (!d->ThreeDView)
  {
    return;
  }
  d->ThreeDView->roll();
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::yawView()
{
  Q_D(qMRMLThreeDViewControllerWidget);
  if (!d->ThreeDView)
  {
    return;
  }
  d->ThreeDView->yaw();
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::zoomIn()
{
  Q_D(qMRMLThreeDViewControllerWidget);
  if (!this->mrmlThreeDViewNode())
  {
    return;
  }

  d->ViewLogic->StartCameraNodeInteraction(vtkMRMLCameraNode::ZoomInFlag);
  d->ThreeDView->zoomIn();
  d->ViewLogic->EndCameraNodeInteraction();
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::zoomOut()
{
  Q_D(qMRMLThreeDViewControllerWidget);
  if (!this->mrmlThreeDViewNode())
  {
    return;
  }

  d->ViewLogic->StartCameraNodeInteraction(vtkMRMLCameraNode::ZoomOutFlag);
  d->ThreeDView->zoomOut();
  d->ViewLogic->EndCameraNodeInteraction();
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::spinView(bool enabled)
{
  this->setAnimationMode(enabled ? vtkMRMLViewNode::Spin : vtkMRMLViewNode::Off);
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::rockView(bool enabled)
{
  this->setAnimationMode(enabled ? vtkMRMLViewNode::Rock : vtkMRMLViewNode::Off);
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::setAnimationMode(int newAnimationMode)
{
  Q_D(qMRMLThreeDViewControllerWidget);
  if (!this->mrmlThreeDViewNode())
  {
    return;
  }

  d->ViewLogic->StartViewNodeInteraction(vtkMRMLViewNode::AnimationModeFlag);
  this->mrmlThreeDViewNode()->SetAnimationMode(newAnimationMode);
  d->ViewLogic->EndViewNodeInteraction();
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::resetFocalPoint()
{
  Q_D(qMRMLThreeDViewControllerWidget);
  if (!d->ThreeDView)
  {
    return;
  }

  d->ViewLogic->StartCameraNodeInteraction(vtkMRMLCameraNode::CenterFlag);
  d->ThreeDView->resetFocalPoint();
  d->ViewLogic->EndCameraNodeInteraction();
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::set3DAxisVisible(bool visible)
{
  Q_D(qMRMLThreeDViewControllerWidget);
  if (!this->mrmlThreeDViewNode())
  {
    return;
  }

  d->ViewLogic->StartViewNodeInteraction(vtkMRMLViewNode::BoxVisibleFlag);
  this->mrmlThreeDViewNode()->SetBoxVisible(visible);
  d->ViewLogic->EndViewNodeInteraction();
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::set3DAxisLabelVisible(bool visible)
{
  Q_D(qMRMLThreeDViewControllerWidget);
  if (!this->mrmlThreeDViewNode())
  {
    return;
  }

  d->ViewLogic->StartViewNodeInteraction(vtkMRMLViewNode::BoxLabelVisibileFlag);
  this->mrmlThreeDViewNode()->SetAxisLabelsVisible(visible);
  d->ViewLogic->EndViewNodeInteraction();
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::setUseDepthPeeling(bool use)
{
  Q_D(qMRMLThreeDViewControllerWidget);
  if (!this->mrmlThreeDViewNode())
  {
    return;
  }

  d->ViewLogic->StartViewNodeInteraction(vtkMRMLViewNode::UseDepthPeelingFlag);
  this->mrmlThreeDViewNode()->SetUseDepthPeeling(use ? 1 : 0);
  d->ViewLogic->EndViewNodeInteraction();
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::setFPSVisible(bool visible)
{
  Q_D(qMRMLThreeDViewControllerWidget);
  if (!this->mrmlThreeDViewNode())
  {
    return;
  }

  d->ViewLogic->StartViewNodeInteraction(vtkMRMLViewNode::FPSVisibleFlag);
  this->mrmlThreeDViewNode()->SetFPSVisible(visible ? 1 : 0);
  d->ViewLogic->EndViewNodeInteraction();
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::setLightBlueBackground()
{
  this->setBackgroundColor(QColor::fromRgbF(vtkMRMLViewNode::defaultBackgroundColor()[0],
                                            vtkMRMLViewNode::defaultBackgroundColor()[1],
                                            vtkMRMLViewNode::defaultBackgroundColor()[2]),
                           QColor::fromRgbF(vtkMRMLViewNode::defaultBackgroundColor2()[0],
                                            vtkMRMLViewNode::defaultBackgroundColor2()[1],
                                            vtkMRMLViewNode::defaultBackgroundColor2()[2]));
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::setBlackBackground()
{
  this->setBackgroundColor(Qt::black);
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::setWhiteBackground()
{
  this->setBackgroundColor(Qt::white);
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::setBackgroundColor(const QColor& newColor, QColor newColor2)
{
  Q_D(qMRMLThreeDViewControllerWidget);
  if (!this->mrmlThreeDViewNode())
  {
    return;
  }

  d->ViewLogic->StartViewNodeInteraction(vtkMRMLViewNode::BackgroundColorFlag);

  int wasModifying = this->mrmlThreeDViewNode()->StartModify();
  // The ThreeDView displayable manager will change the background color of
  // the renderer.
  this->mrmlThreeDViewNode()->SetBackgroundColor(newColor.redF(), newColor.greenF(), newColor.blueF());
  if (!newColor2.isValid())
  {
    newColor2 = newColor;
  }
  this->mrmlThreeDViewNode()->SetBackgroundColor2(newColor2.redF(), newColor2.greenF(), newColor2.blueF());
  this->mrmlThreeDViewNode()->EndModify(wasModifying);

  d->ViewLogic->EndViewNodeInteraction();
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::setBoxColor(const QColor& newColor)
{
  Q_D(qMRMLThreeDViewControllerWidget);
  if (!this->mrmlThreeDViewNode())
  {
    return;
  }

  d->ViewLogic->StartViewNodeInteraction(vtkMRMLViewNode::BoxColorFlag);

  int wasModifying = this->mrmlThreeDViewNode()->StartModify();
  // The ThreeDView displayable manager will change the color of BoxAxisActor
  this->mrmlThreeDViewNode()->SetBoxColor(newColor.redF(), newColor.greenF(), newColor.blueF());
  this->mrmlThreeDViewNode()->EndModify(wasModifying);

  d->ViewLogic->EndViewNodeInteraction();
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::setStereoType(int newStereoType)
{
  Q_D(qMRMLThreeDViewControllerWidget);
  if (!this->mrmlThreeDViewNode())
  {
    return;
  }

  d->ViewLogic->StartViewNodeInteraction(vtkMRMLViewNode::StereoTypeFlag);
  this->mrmlThreeDViewNode()->SetStereoType(newStereoType);
  d->ViewLogic->EndViewNodeInteraction();
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::setOrientationMarkerType(int newOrientationMarkerType)
{
  Q_D(qMRMLThreeDViewControllerWidget);
  if (!this->mrmlThreeDViewNode())
  {
    return;
  }

  d->ViewLogic->StartViewNodeInteraction(vtkMRMLViewNode::OrientationMarkerTypeFlag);
  this->mrmlThreeDViewNode()->SetOrientationMarkerType(newOrientationMarkerType);
  d->ViewLogic->EndViewNodeInteraction();
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::setOrientationMarkerSize(int newOrientationMarkerSize)
{
  Q_D(qMRMLThreeDViewControllerWidget);
  if (!this->mrmlThreeDViewNode())
  {
    return;
  }

  d->ViewLogic->StartViewNodeInteraction(vtkMRMLViewNode::OrientationMarkerSizeFlag);
  this->mrmlThreeDViewNode()->SetOrientationMarkerSize(newOrientationMarkerSize);
  d->ViewLogic->EndViewNodeInteraction();
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::setRulerType(int newRulerType)
{
  Q_D(qMRMLThreeDViewControllerWidget);
  if (!this->mrmlThreeDViewNode())
  {
    return;
  }

  d->ViewLogic->StartViewNodeInteraction(vtkMRMLViewNode::RulerTypeFlag);
  this->mrmlThreeDViewNode()->SetRulerType(newRulerType);
  d->ViewLogic->EndViewNodeInteraction();

  // Switch to orthographic render mode automatically if ruler is enabled
  if (newRulerType != vtkMRMLViewNode::RulerTypeNone
      && this->mrmlThreeDViewNode()->GetRenderMode() != vtkMRMLViewNode::Orthographic)
  {
    d->ViewLogic->StartViewNodeInteraction(vtkMRMLViewNode::RenderModeFlag);
    this->mrmlThreeDViewNode()->SetRenderMode(vtkMRMLViewNode::Orthographic);
    d->ViewLogic->EndViewNodeInteraction();
  }
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::setRulerColor(int newRulerColor)
{
  Q_D(qMRMLThreeDViewControllerWidget);
  if (!this->mrmlThreeDViewNode())
  {
    return;
  }

  d->ViewLogic->StartViewNodeInteraction(vtkMRMLViewNode::RulerColorFlag);
  this->mrmlThreeDViewNode()->SetRulerColor(newRulerColor);
  d->ViewLogic->EndViewNodeInteraction();
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::setShadowsVisibility(bool shadows)
{
  Q_D(qMRMLThreeDViewControllerWidget);
  if (!this->mrmlThreeDViewNode())
  {
    return;
  }

  d->ViewLogic->StartViewNodeInteraction(vtkMRMLViewNode::ShadowsVisibilityFlag);
  this->mrmlThreeDViewNode()->SetShadowsVisibility(shadows);
  d->ViewLogic->EndViewNodeInteraction();
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::setAmbientShadowsSizeScale(double value)
{
  Q_D(qMRMLThreeDViewControllerWidget);
  if (!this->mrmlThreeDViewNode())
  {
    return;
  }

  d->ViewLogic->StartViewNodeInteraction(vtkMRMLViewNode::AmbientShadowsSizeScaleFlag);
  this->mrmlThreeDViewNode()->SetAmbientShadowsSizeScale(value);
  d->ViewLogic->EndViewNodeInteraction();
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::setAmbientShadowsVolumeOpacityThresholdPercent(double opacityPercent)
{
  Q_D(qMRMLThreeDViewControllerWidget);
  if (!this->mrmlThreeDViewNode())
  {
    return;
  }

  d->ViewLogic->StartViewNodeInteraction(vtkMRMLViewNode::AmbientShadowsVolumeOpacityThresholdFlag);
  this->mrmlThreeDViewNode()->SetAmbientShadowsVolumeOpacityThreshold(opacityPercent * 0.01);
  d->ViewLogic->EndViewNodeInteraction();
}

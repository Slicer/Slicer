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
#include <QDebug>
#include <QMenu>
#include <QActionGroup>
#include <QInputDialog>

// CTK includes
#include <ctkLogger.h>
#include <ctkButtonGroup.h>

// qMRML includes
#include "qMRMLThreeDViewControllerWidget.h"
#include "qMRMLThreeDViewControllerWidget_p.h"
#include "qMRMLActionSignalMapper.h"
#include "qMRMLNodeFactory.h"
#include "qMRMLSceneViewMenu.h"
#include "qMRMLThreeDView.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLViewNode.h>
#include <vtkMRMLSceneViewNode.h>

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.libs.qmrmlwidgets.qMRMLThreeDViewControllerWidget");
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// qMRMLThreeDViewControllerWidgetPrivate methods

//---------------------------------------------------------------------------
qMRMLThreeDViewControllerWidgetPrivate::qMRMLThreeDViewControllerWidgetPrivate(
  qMRMLThreeDViewControllerWidget& object)
  : q_ptr(&object)
{
  this->ViewNode = 0;
  this->ThreeDView = 0;
}

//---------------------------------------------------------------------------
qMRMLThreeDViewControllerWidgetPrivate::~qMRMLThreeDViewControllerWidgetPrivate()
{
}

//---------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidgetPrivate::setupUi(QWidget* widget)
{
  Q_Q(qMRMLThreeDViewControllerWidget);
  Q_ASSERT(widget == q);
  this->Ui_qMRMLThreeDViewControllerWidget::setupUi(widget);
  
  // Look from axes
  QObject::connect(this->AxesWidget,
                   SIGNAL(currentAxisChanged(ctkAxesWidget::Axis)),
                   q, SLOT(lookFromAxis(const ctkAxesWidget::Axis&)));

  // Pitch, Roll, Yaw buttons
  QObject::connect(this->PitchButton, SIGNAL(clicked()),
                   q, SLOT(pitchView()));
  QObject::connect(this->RollButton, SIGNAL(clicked()),
                   q, SLOT(rollView()));
  QObject::connect(this->YawButton, SIGNAL(clicked()),
                   q, SLOT(yawView()));

  // Orthographic/perspective button
  QObject::connect(this->OrthoButton, SIGNAL(toggled(bool)),
                   q, SLOT(setOrthographicModeEnabled(bool)));

  // ZoomIn, ZoomOut button
  QObject::connect(this->ZoomInButton, SIGNAL(clicked()),
                   q, SLOT(zoomIn()));
  QObject::connect(this->ZoomOutButton, SIGNAL(clicked()),
                   q, SLOT(zoomOut()));

  // ResetFocalPoint button
  QObject::connect(this->CenterButton, SIGNAL(clicked()),
                   q, SLOT(resetFocalPoint()));

  // StereoType actions
  this->StereoTypesMapper = new qMRMLActionSignalMapper(widget);
  this->StereoTypesMapper->setMapping(this->actionNoStereo,
                                      vtkMRMLViewNode::NoStereo);
  this->StereoTypesMapper->setMapping(this->actionSwitchToAnaglyphStereo,
                                      vtkMRMLViewNode::Anaglyph);
  this->StereoTypesMapper->setMapping(this->actionSwitchToCrystalEyesStereo,
                                      vtkMRMLViewNode::CrystalEyes);
  this->StereoTypesMapper->setMapping(this->actionSwitchToInterlacedStereo,
                                      vtkMRMLViewNode::Interlaced);
  this->StereoTypesMapper->setMapping(this->actionSwitchToRedBlueStereo,
                                      vtkMRMLViewNode::RedBlue);
  QActionGroup* stereoTypesActions = new QActionGroup(widget);
  stereoTypesActions->setExclusive(true);
  stereoTypesActions->addAction(this->actionNoStereo);
  stereoTypesActions->addAction(this->actionSwitchToRedBlueStereo);
  stereoTypesActions->addAction(this->actionSwitchToAnaglyphStereo);
  stereoTypesActions->addAction(this->actionSwitchToInterlacedStereo);
  //stereoTypesActions->addAction(this->actionSwitchToCrystalEyesStereo);
  QMenu* stereoTypesMenu = new QMenu("Stereo Modes", widget);
  stereoTypesMenu->addActions(stereoTypesActions->actions());
  this->StereoButton->setMenu(stereoTypesMenu);
  QObject::connect(this->StereoTypesMapper, SIGNAL(mapped(int)),
                   q, SLOT(setStereoType(int)));
  QObject::connect(stereoTypesActions, SIGNAL(triggered(QAction*)),
                   this->StereoTypesMapper, SLOT(map(QAction*)));

  QMenu* visibilityMenu = new QMenu("Visibility", widget);
  this->VisibilityButton->setMenu(visibilityMenu);

  // Show 3D Axis, 3D Axis label
  visibilityMenu->addAction(this->actionSet3DAxisVisible);
  visibilityMenu->addAction(this->actionSet3DAxisLabelVisible);
  QObject::connect(this->actionSet3DAxisVisible, SIGNAL(triggered(bool)),
                   q, SLOT(set3DAxisVisible(bool)));
  QObject::connect(this->actionSet3DAxisLabelVisible, SIGNAL(triggered(bool)),
                   q, SLOT(set3DAxisLabelVisible(bool)));

  // Background color
  QActionGroup* backgroundColorActions = new QActionGroup(widget);
  backgroundColorActions->setExclusive(true);
  visibilityMenu->addAction(this->actionSetLightBlueBackground);
  visibilityMenu->addAction(this->actionSetBlackBackground);
  visibilityMenu->addAction(this->actionSetWhiteBackground);
  backgroundColorActions->addAction(this->actionSetLightBlueBackground);
  backgroundColorActions->addAction(this->actionSetBlackBackground);
  backgroundColorActions->addAction(this->actionSetWhiteBackground);
  QObject::connect(this->actionSetLightBlueBackground, SIGNAL(triggered()),
                   q, SLOT(setLightBlueBackground()));
  QObject::connect(this->actionSetWhiteBackground, SIGNAL(triggered()),
                   q, SLOT(setWhiteBackground()));
  QObject::connect(this->actionSetBlackBackground, SIGNAL(triggered()),
                   q, SLOT(setBlackBackground()));

  // SpinView, RockView buttons
  this->AnimateViewButtonGroup = new ctkButtonGroup(widget);
  this->AnimateViewButtonGroup->addButton(this->SpinButton, vtkMRMLViewNode::Spin);
  this->AnimateViewButtonGroup->addButton(this->RockButton, vtkMRMLViewNode::Rock);
  this->AnimateViewButtonGroup = new ctkButtonGroup(widget);
  this->AnimateViewButtonGroup->addButton(this->SpinButton);
  this->AnimateViewButtonGroup->addButton(this->RockButton);
  QObject::connect(this->SpinButton, SIGNAL(toggled(bool)),
                   q, SLOT(spinView(bool)));
  QObject::connect(this->RockButton, SIGNAL(toggled(bool)),
                   q, SLOT(rockView(bool)));
}

// --------------------------------------------------------------------------
// qMRMLThreeDViewControllerWidget methods

// --------------------------------------------------------------------------
qMRMLThreeDViewControllerWidget::qMRMLThreeDViewControllerWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qMRMLThreeDViewControllerWidgetPrivate(*this))
{
  Q_D(qMRMLThreeDViewControllerWidget);
  d->setupUi(this);
}

// --------------------------------------------------------------------------
qMRMLThreeDViewControllerWidget::~qMRMLThreeDViewControllerWidget()
{
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::setThreeDView(qMRMLThreeDView* view)
{
  Q_D(qMRMLThreeDViewControllerWidget);
  d->ThreeDView = view;
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::setMRMLViewNode(
    vtkMRMLViewNode * viewNode)
{
  Q_D(qMRMLThreeDViewControllerWidget);
  this->qvtkReconnect(d->ViewNode, viewNode, vtkCommand::ModifiedEvent,
                      this, SLOT(updateWidgetFromMRML()));
  d->ViewNode = viewNode;
  this->updateWidgetFromMRML();
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLThreeDViewControllerWidget);
  // Enable buttons
  QList<QWidget*> widgets;
  widgets << d->AxesWidget
    << d->PitchButton << d->RollButton << d->YawButton
    << d->CenterButton << d->OrthoButton << d->VisibilityButton
    << d->ZoomInButton << d->ZoomOutButton << d->StereoButton
    << d->RockButton << d->SpinButton;
  foreach(QWidget* w, widgets)
    {
    w->setEnabled(d->ViewNode != 0);
    }

  if (!d->ViewNode)
    {
    return;
    }

  d->actionSet3DAxisVisible->setChecked(d->ViewNode->GetBoxVisible());
  d->actionSet3DAxisLabelVisible->setChecked(
    d->ViewNode->GetAxisLabelsVisible());

  double* color = d->ViewNode->GetBackgroundColor();
  QColor backgroundColor = QColor::fromRgbF(color[0], color[1], color[2]);
  d->actionSetBlackBackground->setChecked(backgroundColor == Qt::black);
  d->actionSetWhiteBackground->setChecked(backgroundColor == Qt::white);
  d->actionSetLightBlueBackground->setChecked(
    !d->actionSetBlackBackground->isChecked() &&
    !d->actionSetWhiteBackground->isChecked());

  d->OrthoButton->setChecked(
    d->ViewNode->GetRenderMode() == vtkMRMLViewNode::Orthographic);
  
  QAction* action = qobject_cast<QAction*>(d->StereoTypesMapper->mapping(
    d->ViewNode->GetStereoType()));
  action->setChecked(true);
  
  d->SpinButton->setChecked(d->ViewNode->GetAnimationMode() == vtkMRMLViewNode::Spin);
  d->RockButton->setChecked(d->ViewNode->GetAnimationMode() == vtkMRMLViewNode::Rock);
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::setOrthographicModeEnabled(bool enabled)
{
  Q_D(qMRMLThreeDViewControllerWidget);
  d->ViewNode->SetRenderMode(
    enabled ? vtkMRMLViewNode::Orthographic : vtkMRMLViewNode::Perspective);
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::lookFromAxis(const ctkAxesWidget::Axis& axis)
{
  Q_D(qMRMLThreeDViewControllerWidget);
  if (!d->ThreeDView)
    {
    return;
    }
  d->ThreeDView->lookFromViewAxis(axis);
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
  if (!d->ViewNode)
    {
    return;
    }
  d->ThreeDView->zoomIn();
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::zoomOut()
{
  Q_D(qMRMLThreeDViewControllerWidget);
  if (!d->ViewNode)
    {
    return;
    }
  d->ThreeDView->zoomOut();
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
  if (!d->ViewNode)
    {
    return;
    }
  d->ViewNode->SetAnimationMode(newAnimationMode);
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::resetFocalPoint()
{
  Q_D(qMRMLThreeDViewControllerWidget);
  if (!d->ThreeDView)
    {
    return;
    }
  d->ThreeDView->resetFocalPoint();
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::set3DAxisVisible(bool visible)
{
  Q_D(qMRMLThreeDViewControllerWidget);
  if (!d->ViewNode)
    {
    return;
    }
  d->ViewNode->SetBoxVisible(visible);
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::set3DAxisLabelVisible(bool visible)
{
  Q_D(qMRMLThreeDViewControllerWidget);
  if (!d->ViewNode)
    {
    return;
    }
  d->ViewNode->SetAxisLabelsVisible(visible);
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::setLightBlueBackground()
{
  QColor lightBlue = QColor::fromRgbF(0.70196, 0.70196, 0.90588);
  this->setBackgroundColor(lightBlue);
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
void qMRMLThreeDViewControllerWidget::setBackgroundColor(const QColor& newColor)
{
  Q_D(qMRMLThreeDViewControllerWidget);
  if (!d->ViewNode)
    {
    return;
    }
  double rgbF[3];
  rgbF[0] = newColor.redF();
  rgbF[1] = newColor.greenF();
  rgbF[2] = newColor.blueF();
  d->ViewNode->SetBackgroundColor(rgbF);
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewControllerWidget::setStereoType(int newStereoType)
{
  Q_D(qMRMLThreeDViewControllerWidget);
  if (!d->ViewNode)
    {
    return;
    }
  d->ViewNode->SetStereoType(newStereoType);
}

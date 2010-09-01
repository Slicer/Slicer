/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QDebug>
#include <QMenu>

// CTK includes
#include <ctkLogger.h>

// qMRML includes
#include "qMRMLThreeDViewsControllerWidget.h"
#include "qMRMLThreeDViewsControllerWidget_p.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLViewNode.h>


//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.libs.qmrmlwidgets.qMRMLThreeDViewsControllerWidget");
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// qMRMLThreeDViewsControllerWidgetPrivate methods

//---------------------------------------------------------------------------
qMRMLThreeDViewsControllerWidgetPrivate::qMRMLThreeDViewsControllerWidgetPrivate()
{
  this->ActiveMRMLThreeDViewNode = 0;
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewsControllerWidgetPrivate::updateWidgetFromMRML()
{
  Q_ASSERT(this->ActiveMRMLThreeDViewNode);
  this->setOrthographicModeEnabled(
      this->ActiveMRMLThreeDViewNode->GetRenderMode() == vtkMRMLViewNode::Orthographic);
}

//---------------------------------------------------------------------------
void qMRMLThreeDViewsControllerWidgetPrivate::setupUi(qMRMLWidget* widget)
{
  this->Ui_qMRMLThreeDViewsControllerWidget::setupUi(widget);

  // Pitch, Roll, Yaw buttons
  connect(this->PitchButton, SIGNAL(clicked()), SLOT(pitchActiveView()));
  connect(this->RollButton, SIGNAL(clicked()), SLOT(rollActiveView()));
  connect(this->YawButton, SIGNAL(clicked()), SLOT(yawActiveView()));

  // Orthographic/perspective button
  connect(this->OrthoButton, SIGNAL(toggled(bool)), SLOT(setOrthographicModeEnabled(bool)));
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewsControllerWidgetPrivate::setOrthographicModeEnabled(bool enabled)
{
  qDebug() << "setOrthographicModeEnabled" << enabled;
  this->OrthoButton->setChecked(enabled);
  this->ActiveMRMLThreeDViewNode->SetRenderMode(
      enabled ? vtkMRMLViewNode::Orthographic : vtkMRMLViewNode::Perspective);
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewsControllerWidgetPrivate::pitchActiveView()
{
  if (!this->ActiveMRMLThreeDViewNode)
    {
    return;
    }
  this->ActiveMRMLThreeDViewNode->InvokeEvent(vtkMRMLViewNode::PitchViewRequestedEvent);
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewsControllerWidgetPrivate::rollActiveView()
{
  if (!this->ActiveMRMLThreeDViewNode)
    {
    return;
    }
  this->ActiveMRMLThreeDViewNode->InvokeEvent(vtkMRMLViewNode::RollViewRequestedEvent);
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewsControllerWidgetPrivate::yawActiveView()
{
  if (!this->ActiveMRMLThreeDViewNode)
    {
    return;
    }
  this->ActiveMRMLThreeDViewNode->InvokeEvent(vtkMRMLViewNode::YawViewRequestedEvent);
}

// --------------------------------------------------------------------------
// qMRMLThreeDViewsControllerWidget methods

// --------------------------------------------------------------------------
qMRMLThreeDViewsControllerWidget::qMRMLThreeDViewsControllerWidget(QWidget* _parent) : Superclass(_parent)
{
  CTK_INIT_PRIVATE(qMRMLThreeDViewsControllerWidget);
  CTK_D(qMRMLThreeDViewsControllerWidget);
  d->setupUi(this);
}

// --------------------------------------------------------------------------
CTK_SET_CXX(qMRMLThreeDViewsControllerWidget, vtkMRMLViewNode*,
            setActiveMRMLThreeDViewNode, ActiveMRMLThreeDViewNode);


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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QInputDialog>

// CTK includes
#include <ctkLogger.h>
#include <ctkButtonGroup.h>

// qMRML includes
#include "qMRMLThreeDViewsControllerWidget_p.h"
#include "qMRMLNodeFactory.h"
#include "qMRMLSceneViewMenu.h"

// MRML includes
#include <vtkMRMLViewNode.h>
#include <vtkMRMLSceneViewNode.h>


//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.libs.qmrmlwidgets.qMRMLThreeDViewsControllerWidget");
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// qMRMLThreeDViewsControllerWidgetPrivate methods

//---------------------------------------------------------------------------
qMRMLThreeDViewsControllerWidgetPrivate::qMRMLThreeDViewsControllerWidgetPrivate(qMRMLThreeDViewsControllerWidget& object)
  : q_ptr(&object)
{
  this->ActiveMRMLThreeDViewNode = 0;
  this->SceneViewMenu = 0;
  this->DisableMagnification = true;
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewsControllerWidgetPrivate::updateWidgetFromMRML()
{
  // Enable buttons
  this->ScreenshotButton->setEnabled(this->ActiveMRMLThreeDViewNode != 0);
}

//---------------------------------------------------------------------------
void qMRMLThreeDViewsControllerWidgetPrivate::setupUi(qMRMLWidget* widget)
{
  Q_Q(qMRMLThreeDViewsControllerWidget);
  this->Ui_qMRMLThreeDViewsControllerWidget::setupUi(widget);

  // Scene View buttons
  this->SceneViewMenu = new qMRMLSceneViewMenu(widget);
  connect(widget, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
          this->SceneViewMenu, SLOT(setMRMLScene(vtkMRMLScene*)));
  this->SelectSceneViewMenuButton->setMenu(this->SceneViewMenu);

  // Enable magnification by default
  q->setDisableMagnification(false);

  this->setDisplayMode(NavigationDisplayMode);
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewsControllerWidgetPrivate::createSceneView()
{
  Q_Q(qMRMLThreeDViewsControllerWidget);

  // Ask user for a name
  bool ok = false;
  QString sceneViewName = QInputDialog::getText(q, tr("SceneView Name"),
                                               tr("SceneView Name:"), QLineEdit::Normal,
                                               "View", &ok);
  if (!ok || sceneViewName.isEmpty())
    {
    return;
    }

  // Create scene view
  qMRMLNodeFactory nodeFactory;
  nodeFactory.setMRMLScene(q->mrmlScene());
  nodeFactory.setBaseName("vtkMRMLSceneViewNode", sceneViewName);
  vtkMRMLNode * newNode = nodeFactory.createNode("vtkMRMLSceneViewNode");
  vtkMRMLSceneViewNode * newSceneViewNode = vtkMRMLSceneViewNode::SafeDownCast(newNode);
  newSceneViewNode->StoreScene();
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewsControllerWidgetPrivate::setDisplayMode(DisplayMode newMode)
{
  this->StackedWidget->setCurrentIndex(newMode);
}

// --------------------------------------------------------------------------
// qMRMLThreeDViewsControllerWidget methods

// --------------------------------------------------------------------------
qMRMLThreeDViewsControllerWidget::qMRMLThreeDViewsControllerWidget(QWidget* _parent) : Superclass(_parent)
  , d_ptr(new qMRMLThreeDViewsControllerWidgetPrivate(*this))
{
  Q_D(qMRMLThreeDViewsControllerWidget);
  d->setupUi(this);

  connect(d->ScreenshotButton, SIGNAL(clicked()), SIGNAL(screenshotButtonClicked()));
  connect(d->SceneViewButton, SIGNAL(clicked()), SIGNAL(sceneViewButtonClicked()));
}

// --------------------------------------------------------------------------
qMRMLThreeDViewsControllerWidget::~qMRMLThreeDViewsControllerWidget()
{
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewsControllerWidget::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_D(qMRMLThreeDViewsControllerWidget);

  this->Superclass::setMRMLScene(newScene);

  d->SceneViewMenu->setMRMLScene(newScene);
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewsControllerWidget::setActiveMRMLThreeDViewNode(
    vtkMRMLViewNode * newActiveMRMLThreeDViewNode)
{
  Q_D(qMRMLThreeDViewsControllerWidget);

  d->ActiveMRMLThreeDViewNode = newActiveMRMLThreeDViewNode;

  d->NavigationView->setMRMLViewNode(newActiveMRMLThreeDViewNode);
  d->updateWidgetFromMRML();
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewsControllerWidget::setActiveThreeDRenderer(
    vtkRenderer* renderer)
{
  Q_D(qMRMLThreeDViewsControllerWidget);
  d->NavigationView->setRendererToListen(renderer);
}

// --------------------------------------------------------------------------
ctkVTKMagnifyView * qMRMLThreeDViewsControllerWidget::magnifyView()
{
  Q_D(qMRMLThreeDViewsControllerWidget);
  return d->VTKMagnify;
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewsControllerWidget::setDisplayModeToNavigation()
{
  Q_D(qMRMLThreeDViewsControllerWidget);
  d->setDisplayMode(qMRMLThreeDViewsControllerWidgetPrivate::NavigationDisplayMode);
}

// --------------------------------------------------------------------------
void qMRMLThreeDViewsControllerWidget::setDisplayModeToMagnification()
{
  Q_D(qMRMLThreeDViewsControllerWidget);
  d->setDisplayMode(qMRMLThreeDViewsControllerWidgetPrivate::MagnificationDisplayMode);
}

//-----------------------------------------------------------------------------
CTK_GET_CPP(qMRMLThreeDViewsControllerWidget, bool, disableMagnification, DisableMagnification)

void qMRMLThreeDViewsControllerWidget::setDisableMagnification(bool disableMagnification)
{
  Q_D(qMRMLThreeDViewsControllerWidget);

  if (disableMagnification)
    {
    disconnect(d->VTKMagnify, SIGNAL(enteredObservedWidget(QVTKWidget*)),
            this, SLOT(setDisplayModeToMagnification()));
    disconnect(d->VTKMagnify, SIGNAL(leftObservedWidget(QVTKWidget*)),
            this, SLOT(setDisplayModeToNavigation()));
    }
  else
    {
    // Connect the magnify widget to the MRMLThreeDViewsControllerWidget to
    // toggle between navigation (3D) and magnification (2D) modes depending
    // on whether the mouse is within an observed QVTKWidget (i.e. within a
    // ctkVTKSliceView).
    connect(d->VTKMagnify, SIGNAL(enteredObservedWidget(QVTKWidget*)),
            this, SLOT(setDisplayModeToMagnification()));
    connect(d->VTKMagnify, SIGNAL(leftObservedWidget(QVTKWidget*)),
            this, SLOT(setDisplayModeToNavigation()));
    }
  d->DisableMagnification = disableMagnification;
}

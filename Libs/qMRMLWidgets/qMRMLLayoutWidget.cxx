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

// CTK includes
#include <ctkLogger.h>

// qMRML includes
#include "qMRMLLayoutManager.h"
#include "qMRMLLayoutWidget.h"

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.base.qtgui.qMRMLLayoutWidget");
//--------------------------------------------------------------------------

class qMRMLLayoutWidgetPrivate
{
public:
  qMRMLLayoutManager* LayoutManager;
};

//------------------------------------------------------------------------------
// qMRMLLayoutWidget methods

// --------------------------------------------------------------------------
qMRMLLayoutWidget::qMRMLLayoutWidget(QWidget* widget)
  : Superclass(widget)
  , d_ptr(new qMRMLLayoutWidgetPrivate)
{
  Q_D(qMRMLLayoutWidget);
  d->LayoutManager = new qMRMLLayoutManager(this);
  d->LayoutManager->setViewport(this);
}

// --------------------------------------------------------------------------
qMRMLLayoutWidget::~qMRMLLayoutWidget()
{
}

//------------------------------------------------------------------------------
qMRMLLayoutManager* qMRMLLayoutWidget::layoutManager()const
{
  Q_D(const qMRMLLayoutWidget);
  return d->LayoutManager;
}

//------------------------------------------------------------------------------
void qMRMLLayoutWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLLayoutWidget);
  d->LayoutManager->setMRMLScene(scene);
}

//------------------------------------------------------------------------------
vtkMRMLScene* qMRMLLayoutWidget::mrmlScene()const
{
  Q_D(const qMRMLLayoutWidget);
  return d->LayoutManager->mrmlScene();
}

//------------------------------------------------------------------------------
int qMRMLLayoutWidget::layout()const
{
  Q_D(const qMRMLLayoutWidget);
  return d->LayoutManager->layout();
}

//------------------------------------------------------------------------------
void qMRMLLayoutWidget::setLayout(int layout)
{
  Q_D(qMRMLLayoutWidget);
  d->LayoutManager->setLayout(layout);
}

//------------------------------------------------------------------------------
void qMRMLLayoutWidget::switchToOneUpSliceView(const QString& sliceLayoutName)
{
  Q_D(qMRMLLayoutWidget);
  d->LayoutManager->switchToOneUpSliceView(sliceLayoutName);
}

//------------------------------------------------------------------------------
void qMRMLLayoutWidget::switchToConventionalView()
{
  Q_D(qMRMLLayoutWidget);
  d->LayoutManager->switchToConventionalView();
}

//------------------------------------------------------------------------------
void qMRMLLayoutWidget::switchToOneUp3DView()
{
  Q_D(qMRMLLayoutWidget);
  d->LayoutManager->switchToOneUp3DView();
}

//------------------------------------------------------------------------------
void qMRMLLayoutWidget::switchToOneUpRedSliceView()
{
  Q_D(qMRMLLayoutWidget);
  d->LayoutManager->switchToOneUpRedSliceView();
}

//------------------------------------------------------------------------------
void qMRMLLayoutWidget::switchToOneUpYellowSliceView()
{
  Q_D(qMRMLLayoutWidget);
  d->LayoutManager->switchToOneUpYellowSliceView();
}

//------------------------------------------------------------------------------
void qMRMLLayoutWidget::switchToOneUpGreenSliceView()
{
  Q_D(qMRMLLayoutWidget);
  d->LayoutManager->switchToOneUpGreenSliceView();
}

//------------------------------------------------------------------------------
void qMRMLLayoutWidget::switchToFourUpView()
{
  Q_D(qMRMLLayoutWidget);
  d->LayoutManager->switchToFourUpView();
}

//------------------------------------------------------------------------------
void qMRMLLayoutWidget::switchToTabbed3DView()
{
  Q_D(qMRMLLayoutWidget);
  d->LayoutManager->switchToTabbed3DView();
}

//------------------------------------------------------------------------------
void qMRMLLayoutWidget::switchToTabbedSliceView()
{
  Q_D(qMRMLLayoutWidget);
  d->LayoutManager->switchToTabbedSliceView();
}

//------------------------------------------------------------------------------
void qMRMLLayoutWidget::switchToLightboxView()
{
  Q_D(qMRMLLayoutWidget);
  d->LayoutManager->switchToLightboxView();
}

//------------------------------------------------------------------------------
void qMRMLLayoutWidget::switchToCompareView()
{
  Q_D(qMRMLLayoutWidget);
  d->LayoutManager->switchToCompareView();
}

//------------------------------------------------------------------------------
void qMRMLLayoutWidget::switchToCompareWidescreenView()
{
  Q_D(qMRMLLayoutWidget);
  d->LayoutManager->switchToCompareWidescreenView();
}

//------------------------------------------------------------------------------
void qMRMLLayoutWidget::switchToSideBySideLightboxView()
{
  Q_D(qMRMLLayoutWidget);
  d->LayoutManager->switchToSideBySideLightboxView();
}

//------------------------------------------------------------------------------
void qMRMLLayoutWidget::switchToDual3DView()
{
  Q_D(qMRMLLayoutWidget);
  d->LayoutManager->switchToDual3DView();
}

//------------------------------------------------------------------------------
void qMRMLLayoutWidget::switchToNone()
{
  Q_D(qMRMLLayoutWidget);
  d->LayoutManager->switchToNone();
}

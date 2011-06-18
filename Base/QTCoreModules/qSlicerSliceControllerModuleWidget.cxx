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

// qMRMLWidgets includes
#include <qMRMLSliceWidget.h>

// SlicerQt includes
#include "qSlicerSliceControllerModuleWidget.h"
#include "ui_qSlicerSliceControllerModule.h"
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"

//-----------------------------------------------------------------------------
class qSlicerSliceControllerModuleWidgetPrivate:
    public Ui_qSlicerSliceControllerModule
{
public:
};

//-----------------------------------------------------------------------------
qSlicerSliceControllerModuleWidget::qSlicerSliceControllerModuleWidget(QWidget* _parentWidget)
  : Superclass(_parentWidget)
  , d_ptr(new qSlicerSliceControllerModuleWidgetPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerSliceControllerModuleWidget::~qSlicerSliceControllerModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerSliceControllerModuleWidget::setup()
{
  Q_D(qSlicerSliceControllerModuleWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
void qSlicerSliceControllerModuleWidget::setMRMLScene(vtkMRMLScene *newScene)
{
  Q_D(qSlicerSliceControllerModuleWidget);

  this->Superclass::setMRMLScene(newScene);

  qSlicerApplication * app = qSlicerApplication::application();
  if (!app)
    {
    return;
    }
  qSlicerLayoutManager * layoutManager = app->layoutManager();
  if (!layoutManager)
    {
    return;
    }

  // Red
  qMRMLSliceWidget * redSliceWidget = layoutManager->sliceWidget("Red");
  Q_ASSERT(redSliceWidget);
  d->RedSliceControllerWidget->setSliceLogics(layoutManager->mrmlSliceLogics());
  d->RedSliceControllerWidget->setSliceLogic(
    redSliceWidget->sliceController()->sliceLogic());

  // Yellow
  qMRMLSliceWidget * yellowSliceWidget = layoutManager->sliceWidget("Yellow");
  Q_ASSERT(yellowSliceWidget);
  d->YellowSliceControllerWidget->setSliceLogics(layoutManager->mrmlSliceLogics());
  d->YellowSliceControllerWidget->setSliceLogic(
    yellowSliceWidget->sliceController()->sliceLogic());

  // Green
  qMRMLSliceWidget * greenSliceWidget = layoutManager->sliceWidget("Green");
  Q_ASSERT(greenSliceWidget);
  d->GreenSliceControllerWidget->setSliceLogics(layoutManager->mrmlSliceLogics());
  d->GreenSliceControllerWidget->setSliceLogic(
    greenSliceWidget->sliceController()->sliceLogic());
}


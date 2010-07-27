// Qt includes
#include <QDebug>

// qMRMLWidgets includes
#include <qMRMLSliceViewWidget.h>

// SlicerQt includes
#include "qSlicerSliceControllerModuleWidget.h"
#include "ui_qSlicerSliceControllerModule.h"
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"

//-----------------------------------------------------------------------------
class qSlicerSliceControllerModuleWidgetPrivate:
    public ctkPrivate<qSlicerSliceControllerModuleWidget>,
    public Ui_qSlicerSliceControllerModule
{
public:
};

//-----------------------------------------------------------------------------
CTK_CONSTRUCTOR_1_ARG_CXX(qSlicerSliceControllerModuleWidget, QWidget*);

//-----------------------------------------------------------------------------
void qSlicerSliceControllerModuleWidget::setup()
{
  CTK_D(qSlicerSliceControllerModuleWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
void qSlicerSliceControllerModuleWidget::setMRMLScene(vtkMRMLScene *newScene)
{
  CTK_D(qSlicerSliceControllerModuleWidget);

  this->Superclass::setMRMLScene(newScene);

  qSlicerLayoutManager * layoutManager = qSlicerApplication::application()->layoutManager();
  if (!layoutManager)
    {
    return;
    }

  // Red
  qMRMLSliceViewWidget * redSliceViewWidget = layoutManager->sliceView("Red");
  Q_ASSERT(redSliceViewWidget);
  d->RedSliceControllerWidget->setSliceLogic(
      redSliceViewWidget->sliceController()->sliceLogic());

  // Yellow
  qMRMLSliceViewWidget * yellowSliceViewWidget = layoutManager->sliceView("Yellow");
  Q_ASSERT(yellowSliceViewWidget);
  d->YellowSliceControllerWidget->setSliceLogic(
      yellowSliceViewWidget->sliceController()->sliceLogic());

  // Green
  qMRMLSliceViewWidget * greenSliceViewWidget = layoutManager->sliceView("Green");
  Q_ASSERT(greenSliceViewWidget);
  d->GreenSliceControllerWidget->setSliceLogic(
      greenSliceViewWidget->sliceController()->sliceLogic());
}


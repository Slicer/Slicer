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
  qMRMLSliceWidget * redSliceWidget = layoutManager->sliceWidget("Red");
  Q_ASSERT(redSliceWidget);
  d->RedSliceControllerWidget->setSliceLogic(
      redSliceWidget->sliceController()->sliceLogic());

  // Yellow
  qMRMLSliceWidget * yellowSliceWidget = layoutManager->sliceWidget("Yellow");
  Q_ASSERT(yellowSliceWidget);
  d->YellowSliceControllerWidget->setSliceLogic(
      yellowSliceWidget->sliceController()->sliceLogic());

  // Green
  qMRMLSliceWidget * greenSliceWidget = layoutManager->sliceWidget("Green");
  Q_ASSERT(greenSliceWidget);
  d->GreenSliceControllerWidget->setSliceLogic(
      greenSliceWidget->sliceController()->sliceLogic());
}


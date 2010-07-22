
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


//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.libs.qmrmlwidgets.qMRMLThreeDViewsControllerWidget");
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// qMRMLThreeDViewsControllerWidgetPrivate methods

//---------------------------------------------------------------------------
qMRMLThreeDViewsControllerWidgetPrivate::qMRMLThreeDViewsControllerWidgetPrivate()
{
  
}

//---------------------------------------------------------------------------
qMRMLThreeDViewsControllerWidgetPrivate::~qMRMLThreeDViewsControllerWidgetPrivate()
{
}

//---------------------------------------------------------------------------
void qMRMLThreeDViewsControllerWidgetPrivate::setupUi(qMRMLWidget* widget)
{
  //CTK_P(qMRMLThreeDViewsControllerWidget);

  this->Ui_qMRMLThreeDViewsControllerWidget::setupUi(widget);

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


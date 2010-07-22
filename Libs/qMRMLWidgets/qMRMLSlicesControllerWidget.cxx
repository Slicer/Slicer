
// Qt includes
#include <QDebug>
#include <QMenu>

// CTK includes
#include <ctkLogger.h>

// qMRML includes
#include "qMRMLSlicesControllerWidget.h"
#include "qMRMLSlicesControllerWidget_p.h"

// MRML includes
#include <vtkMRMLScene.h>


//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.libs.qmrmlwidgets.qMRMLSlicesControllerWidget");
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// qMRMLSlicesControllerWidgetPrivate methods

//---------------------------------------------------------------------------
qMRMLSlicesControllerWidgetPrivate::qMRMLSlicesControllerWidgetPrivate()
{
  
}

//---------------------------------------------------------------------------
qMRMLSlicesControllerWidgetPrivate::~qMRMLSlicesControllerWidgetPrivate()
{
}

//---------------------------------------------------------------------------
void qMRMLSlicesControllerWidgetPrivate::setupUi(qMRMLWidget* widget)
{
  //CTK_P(qMRMLSlicesControllerWidget);

  this->Ui_qMRMLSlicesControllerWidget::setupUi(widget);

}

// --------------------------------------------------------------------------
// qMRMLSlicesControllerWidget methods

// --------------------------------------------------------------------------
qMRMLSlicesControllerWidget::qMRMLSlicesControllerWidget(QWidget* _parent) : Superclass(_parent)
{
  CTK_INIT_PRIVATE(qMRMLSlicesControllerWidget);
  CTK_D(qMRMLSlicesControllerWidget);
  d->setupUi(this);
}


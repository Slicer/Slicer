// Qt includes
#include <QObject>

// qMRMLWidget includes
#include <qMRMLNodeComboBox.h>

#include "qMRMLAnnotationDisplayNodePointPropertyWidget.h"
#include "qMRMLAnnotationDisplayNodePointPropertyWidget_p.h"

#include "ui_qMRMLAnnotationDisplayNodePointPropertyWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_annotations


//-----------------------------------------------------------------------------
// qMRMLAnnotationDisplayNodePointPropertyWidgetPrivate methods

//-----------------------------------------------------------------------------
qMRMLAnnotationDisplayNodePointPropertyWidgetPrivate::qMRMLAnnotationDisplayNodePointPropertyWidgetPrivate(qMRMLAnnotationDisplayNodePointPropertyWidget& object) :
    q_ptr(&object)
{

}

//-----------------------------------------------------------------------------
void qMRMLAnnotationDisplayNodePointPropertyWidgetPrivate::setupUi(qMRMLAnnotationDisplayNodePropertyWidget * widget)
{
}

//-----------------------------------------------------------------------------
// qMRMLAnnotationDisplayNodePointPropertyWidget methods

//-----------------------------------------------------------------------------
qMRMLAnnotationDisplayNodePointPropertyWidget::qMRMLAnnotationDisplayNodePointPropertyWidget(QWidget *newParent) :
    Superclass(newParent), d_ptr(
        new qMRMLAnnotationDisplayNodePointPropertyWidgetPrivate(*this))
{
  Q_D(qMRMLAnnotationDisplayNodePointPropertyWidget);


}

//-----------------------------------------------------------------------------
qMRMLAnnotationDisplayNodePointPropertyWidget::~qMRMLAnnotationDisplayNodePointPropertyWidget()
{
}


//-----------------------------------------------------------------------------
void qMRMLAnnotationDisplayNodePointPropertyWidget::updateMRMLFromWidget()
{
  Q_D(qMRMLAnnotationDisplayNodePointPropertyWidget);

}

//-----------------------------------------------------------------------------
void qMRMLAnnotationDisplayNodePointPropertyWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLAnnotationDisplayNodePointPropertyWidget);


}


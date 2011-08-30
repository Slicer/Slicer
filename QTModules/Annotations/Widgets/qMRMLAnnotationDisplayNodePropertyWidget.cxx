
// Annotation includes
#include "qMRMLAnnotationDisplayNodePropertyWidget.h"


//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Annotation
class qMRMLAnnotationDisplayNodePropertyWidgetPrivate
{
public:
  qMRMLAnnotationDisplayNodePropertyWidgetPrivate();

};

//-----------------------------------------------------------------------------
// qMRMLAnnotationDisplayNodePropertyWidgetPrivate methods

//-----------------------------------------------------------------------------
qMRMLAnnotationDisplayNodePropertyWidgetPrivate::qMRMLAnnotationDisplayNodePropertyWidgetPrivate()
{

}

//-----------------------------------------------------------------------------
// qMRMLAnnotationDisplayNodePropertyWidget methods

//-----------------------------------------------------------------------------
qMRMLAnnotationDisplayNodePropertyWidget::qMRMLAnnotationDisplayNodePropertyWidget(QWidget *newParent) :
    Superclass(newParent)
  , d_ptr(new qMRMLAnnotationDisplayNodePropertyWidgetPrivate())
{
}

//-----------------------------------------------------------------------------
qMRMLAnnotationDisplayNodePropertyWidget::~qMRMLAnnotationDisplayNodePropertyWidget()
{
}

#ifndef __qMRMLAnnotationDisplayNodePointPropertyWidget_p_h
#define __qMRMLAnnotationDisplayNodePointPropertyWidget_p_h

// Qt includes
#include <QObject>

// CTK includes
#include <ctkPimpl.h>

// Annotations includes
#include "qMRMLAnnotationDisplayNodePointPropertyWidget.h"
#include "qMRMLAnnotationDisplayNodePropertyWidget.h"


#include "ui_qMRMLAnnotationDisplayNodePointPropertyWidget.h"

#include "qSlicerAnnotationsModuleWidgetsExport.h"

class qMRMLAnnotationDisplayNodePropertyWidgetPrivate;

class qMRMLAnnotationDisplayNodePointPropertyWidgetPrivate: public QObject,
    public Ui_qMRMLAnnotationDisplayNodePointPropertyWidget
{
  Q_OBJECT
  Q_DECLARE_PUBLIC(qMRMLAnnotationDisplayNodePointPropertyWidget);
protected:
  qMRMLAnnotationDisplayNodePointPropertyWidget* const q_ptr;

public:

  qMRMLAnnotationDisplayNodePointPropertyWidgetPrivate(qMRMLAnnotationDisplayNodePointPropertyWidget& object);
  typedef qMRMLAnnotationDisplayNodePointPropertyWidgetPrivate Self;
  void setupUi(qMRMLAnnotationDisplayNodePropertyWidget * widget);

};

#endif

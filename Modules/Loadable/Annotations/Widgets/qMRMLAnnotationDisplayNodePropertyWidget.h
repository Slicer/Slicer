
#ifndef __qMRMLAnnotationDisplayNodePropertyWidget_h
#define __qMRMLAnnotationDisplayNodePropertyWidget_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// Slicer includes
#include "qMRMLWidget.h"

#include "qSlicerAnnotationsModuleWidgetsExport.h"

class qMRMLAnnotationDisplayNodePropertyWidgetPrivate;

/// \ingroup Slicer_QtModules_Annotations
class Q_SLICER_MODULE_ANNOTATIONS_WIDGETS_EXPORT qMRMLAnnotationDisplayNodePropertyWidget : public qMRMLWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qMRMLWidget Superclass;
  qMRMLAnnotationDisplayNodePropertyWidget(QWidget *newParent = 0);
  virtual ~qMRMLAnnotationDisplayNodePropertyWidget();

protected:
  QScopedPointer<qMRMLAnnotationDisplayNodePropertyWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLAnnotationDisplayNodePropertyWidget);
  Q_DISABLE_COPY(qMRMLAnnotationDisplayNodePropertyWidget);

};

#endif

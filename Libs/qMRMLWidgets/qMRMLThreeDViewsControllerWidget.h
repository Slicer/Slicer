#ifndef __qMRMLThreeDViewsControllerWidget_h
#define __qMRMLThreeDViewsControllerWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>

// qMRMLWidget includes
#include "qMRMLWidget.h"

#include "qMRMLWidgetsExport.h"

class qMRMLThreeDViewsControllerWidgetPrivate;

class QMRML_WIDGETS_EXPORT qMRMLThreeDViewsControllerWidget : public qMRMLWidget
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef qMRMLWidget Superclass;

  /// Constructors
  explicit qMRMLThreeDViewsControllerWidget(QWidget* parent = 0);
  virtual ~qMRMLThreeDViewsControllerWidget(){}

public slots:


private:
  CTK_DECLARE_PRIVATE(qMRMLThreeDViewsControllerWidget);
};

#endif

#ifndef __qMRMLSlicesControllerWidget_h
#define __qMRMLSlicesControllerWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>

// qMRMLWidget includes
#include "qMRMLWidget.h"

#include "qMRMLWidgetsExport.h"

class qMRMLSlicesControllerWidgetPrivate;

class QMRML_WIDGETS_EXPORT qMRMLSlicesControllerWidget : public qMRMLWidget
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef qMRMLWidget Superclass;

  /// Constructors
  explicit qMRMLSlicesControllerWidget(QWidget* parent = 0);
  virtual ~qMRMLSlicesControllerWidget(){}

public slots:


private:
  CTK_DECLARE_PRIVATE(qMRMLSlicesControllerWidget);
};

#endif

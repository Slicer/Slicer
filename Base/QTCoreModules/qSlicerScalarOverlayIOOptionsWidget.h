#ifndef __qSlicerScalarOverlayIOOptionsWidget_h
#define __qSlicerScalarOverlayIOOptionsWidget_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerIOOptionsWidget.h"
#include "qSlicerBaseQTCoreModulesExport.h"

class qSlicerScalarOverlayIOOptionsWidgetPrivate;

class Q_SLICER_BASE_QTCOREMODULES_EXPORT qSlicerScalarOverlayIOOptionsWidget :
  public qSlicerIOOptionsWidget
{
  Q_OBJECT
public:
  qSlicerScalarOverlayIOOptionsWidget(QWidget *parent=0);

  virtual bool isValid()const;

protected slots:
  void updateProperties();

private:
  CTK_DECLARE_PRIVATE(qSlicerScalarOverlayIOOptionsWidget);
};

#endif

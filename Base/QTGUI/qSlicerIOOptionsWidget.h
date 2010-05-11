#ifndef __qSlicerIOOptionsWidget_h
#define __qSlicerIOOptionsWidget_h

/// Qt includes
#include <QWidget>

/// QtCore includes
#include "qSlicerIOOptions.h"
#include "qSlicerBaseQTGUIExport.h"

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerIOOptionsWidget
  : public QWidget, public qSlicerIOOptions
{
  Q_OBJECT
public:
  explicit qSlicerIOOptionsWidget(QWidget* parent = 0);
  virtual ~qSlicerIOOptionsWidget();
};

#endif

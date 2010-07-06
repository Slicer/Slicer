#ifndef __qSlicerIOOptionsWidget_h
#define __qSlicerIOOptionsWidget_h

/// QtCore includes
#include "qSlicerBaseQTGUIExport.h"
#include "qSlicerIOOptions.h"
#include "qSlicerWidget.h"

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerIOOptionsWidget
  : public qSlicerWidget, public qSlicerIOOptions
{
  Q_OBJECT
public:
  explicit qSlicerIOOptionsWidget(QWidget* parent = 0);
  virtual ~qSlicerIOOptionsWidget();

  /// Returns true if the options have been set and if they are
  /// meaningful
  virtual bool isValid()const;

public slots:
  virtual void setFileName(const QString& fileName);
  virtual void setFileNames(const QStringList& fileNames);

signals:
  void validChanged(bool);
};

#endif

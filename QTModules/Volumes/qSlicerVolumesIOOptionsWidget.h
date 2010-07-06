#ifndef __qSlicerVolumesIOOptionsWidget_h
#define __qSlicerVolumesIOOptionsWidget_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerIOOptionsWidget.h"

// Volumes includes
#include "qSlicerVolumesModuleExport.h"

class qSlicerVolumesIOOptionsWidgetPrivate;

class Q_SLICER_QTMODULES_VOLUMES_EXPORT qSlicerVolumesIOOptionsWidget :
  public qSlicerIOOptionsWidget
{
  Q_OBJECT
public:
  qSlicerVolumesIOOptionsWidget(QWidget *parent=0);

public slots:
  virtual void setFileName(const QString& fileName);
  virtual void setFileNames(const QStringList& fileNames);

protected slots:
  void updateProperties();
private:
  CTK_DECLARE_PRIVATE(qSlicerVolumesIOOptionsWidget);
};

#endif

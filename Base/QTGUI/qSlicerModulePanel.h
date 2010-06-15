#ifndef __qSlicerModulePanel_h
#define __qSlicerModulePanel_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerAbstractModulePanel.h"

#include "qSlicerBaseQTGUIExport.h"

class qSlicerAbstractModule;
class qSlicerModulePanelPrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerModulePanel: public qSlicerAbstractModulePanel
{
  Q_OBJECT
public:
  qSlicerModulePanel(QWidget* parent = 0, Qt::WindowFlags f = 0);

  void clear();

public slots:
  void setModule(const QString& moduleName);

protected slots:
  virtual void addModule(const QString& moduleName);
  virtual void removeModule(const QString& moduleName);
  virtual void removeAllModule();

private:
  CTK_DECLARE_PRIVATE(qSlicerModulePanel);
};

#endif

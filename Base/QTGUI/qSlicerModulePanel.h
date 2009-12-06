#ifndef __qSlicerModulePanel_h
#define __qSlicerModulePanel_h

// SlicerQT includes
#include "qSlicerAbstractModulePanel.h"

// qCTK includes
#include <qCTKPimpl.h>

#include "qSlicerBaseQTGUIWin32Header.h"

class qSlicerAbstractModuleWidget;
class qSlicerModulePanelPrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerModulePanel: public qSlicerAbstractModulePanel
{
  Q_OBJECT
public:
  qSlicerModulePanel(QWidget* parent = 0, Qt::WindowFlags f = 0);
  
  void setModule(qSlicerAbstractModule* module);
  void clear();

public slots:
  void setModule(const QString& moduleTitle);
  void setModuleByName(const QString& moduleName);

protected slots:
  virtual void addModule(qSlicerAbstractModuleWidget* module);
  virtual void removeModule(qSlicerAbstractModuleWidget* module);
  virtual void removeAllModule();

private:
  QCTK_DECLARE_PRIVATE(qSlicerModulePanel);
};

#endif

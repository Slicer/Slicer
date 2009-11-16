#ifndef __qSlicerModulePanel_h
#define __qSlicerModulePanel_h

#include "qSlicerAbstractModulePanel.h"
#include "qSlicerBaseQTGUIWin32Header.h"

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerModulePanel: public qSlicerAbstractModulePanel
{
  Q_OBJECT
public:
  qSlicerModulePanel(QWidget* parent = 0, Qt::WindowFlags f = 0);
  virtual ~qSlicerModulePanel();

  virtual void addModule(qSlicerAbstractModule* module);

private:
  class qInternal;
  qInternal* Internal;
};

#endif

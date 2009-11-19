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

  void setModule(qSlicerAbstractModule* module);
  void clear();

protected:

  virtual void addModule(qSlicerAbstractModule* module);
  virtual void removeModule(qSlicerAbstractModule* module);
  virtual void removeAllModule();

private:
  class qInternal;
  qInternal* Internal;
};

#endif

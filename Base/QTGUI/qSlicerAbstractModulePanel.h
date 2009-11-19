#ifndef __qSlicerAbstractModulePanel_h
#define __qSlicerAbstractModulePanel_h

#include "qSlicerWidget.h"
#include "qSlicerBaseQTGUIWin32Header.h"

class qSlicerAbstractModule;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerAbstractModulePanel: public qSlicerWidget
{
  Q_OBJECT
public:
  qSlicerAbstractModulePanel(QWidget* parent = 0, Qt::WindowFlags f = 0);
  virtual ~qSlicerAbstractModulePanel();
  virtual void addModule(qSlicerAbstractModule* module) = 0;
  virtual void removeModule(qSlicerAbstractModule* module) = 0;
  virtual void removeAllModule() = 0;

signals:
  void moduleAdded(qSlicerAbstractModule* module);
  //void moduleAboutToBeRemoved(qSlicerAbstractModule* module);
  void moduleRemoved(qSlicerAbstractModule* module);
};

#endif

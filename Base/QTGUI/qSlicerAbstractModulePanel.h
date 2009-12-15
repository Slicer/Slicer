#ifndef __qSlicerAbstractModulePanel_h
#define __qSlicerAbstractModulePanel_h

#include "qSlicerWidget.h"
#include "qSlicerBaseQTGUIExport.h"

class qSlicerAbstractModule;
class qSlicerAbstractModule; 

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerAbstractModulePanel: public qSlicerWidget
{
  Q_OBJECT
public:
  qSlicerAbstractModulePanel(QWidget* parent = 0, Qt::WindowFlags f = 0);
  virtual ~qSlicerAbstractModulePanel();
  virtual void addModule(const QString& moduleName) = 0;
  virtual void removeModule(const QString& moduleName) = 0;
  virtual void removeAllModule() = 0;

signals:
  void moduleAdded(const QString& moduleName);
  //void moduleAboutToBeRemoved(const QString& moduleName);
  void moduleRemoved(const QString& moduleName);
};

#endif

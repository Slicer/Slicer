#ifndef __qSlicerModuleSelectorToolBar_h
#define __qSlicerModuleSelectorToolBar_h

// Qt includes
#include <QToolBar>

// CTK includes
#include <ctkPimpl.h>

#include "qSlicerBaseQTGUIExport.h"

class qSlicerModuleSelectorToolBarPrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerModuleSelectorToolBar: public QToolBar
{
  Q_OBJECT
public:
  typedef QToolBar Superclass;
  qSlicerModuleSelectorToolBar(const QString& title, QWidget* parent = 0);
  qSlicerModuleSelectorToolBar(QWidget* parent = 0);

  ///
  /// Add a list of module available for selection
  void addModule(const QString& moduleName);

  ///
  /// Add a list of module available for selection
  inline void addModules(const QStringList& moduleNames);

  ///
  /// Select module by title
  void selectModuleByTitle(const QString& title);

public slots:
  void selectModule(const QString& moduleName);
  void selectNextModule();
  void selectPreviousModule();

signals:
  void moduleSelected(const QString& name);

protected slots:
  void actionSelected(QAction* action);
  void searchModule();

private:
  CTK_DECLARE_PRIVATE(qSlicerModuleSelectorToolBar);
};

//---------------------------------------------------------------------------
void qSlicerModuleSelectorToolBar::addModules(const QStringList& moduleNames)
{
  foreach(const QString& moduleName, moduleNames)
    {
    this->addModule(moduleName);
    }
}

#endif

#ifndef __qSlicerModuleSelectorToolBar_h
#define __qSlicerModuleSelectorToolBar_h

// Qt includes
#include <QToolBar>

// CTK includes
#include <ctkPimpl.h>
#include "qSlicerBaseQTGUIExport.h"

class qSlicerModuleSelectorToolBarPrivate;

///
/// qSlicerModuleSelectorToolBar is a toolbar that can be added in your
/// application and will fired events when the QAction of modules are triggered
/// qSlicerModuleSelectorToolBar supports a tree hierarchy of modules (based on
/// module->category() ), previous/next buttons to browse the history of
/// selected modules. (and a plain history button)
class Q_SLICER_BASE_QTGUI_EXPORT qSlicerModuleSelectorToolBar: public QToolBar
{
  Q_OBJECT
public:
  typedef QToolBar Superclass;

  /// Constructor
  /// title is the name of the toolbar (can appear using right click on the
  /// toolbar area)
  qSlicerModuleSelectorToolBar(const QString& title, QWidget* parent = 0);
  qSlicerModuleSelectorToolBar(QWidget* parent = 0);
  virtual ~qSlicerModuleSelectorToolBar();

  ///
  /// Add a module by name. The module action will be inserted
  /// based on its category.
  void addModule(const QString& moduleName);

  ///
  /// Add a list of module available for selection
  inline void addModules(const QStringList& moduleNames);

  ///
  /// Remove the module from the list of available module
  void removeModule(const QString& moduleName);

public slots:
  ///
  /// Select a module by title. It looks for the module action and triggers it
  void selectModuleByTitle(const QString& title);

  ///
  /// Select a module by name. It looks for the module action and triggers it
  void selectModule(const QString& moduleName);

  void selectNextModule();
  void selectPreviousModule();

signals:
  ///
  /// The signal is fired every time a module is selected. The QAction of the
  /// module is triggered.
  /// TODO: directly connection modules QActions with the module manager
  void moduleSelected(const QString& name);

protected slots:
  void onActionTriggered();
  void actionSelected(QAction* action);
  void searchModule();

protected:
  QScopedPointer<qSlicerModuleSelectorToolBarPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerModuleSelectorToolBar);
  Q_DISABLE_COPY(qSlicerModuleSelectorToolBar);
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

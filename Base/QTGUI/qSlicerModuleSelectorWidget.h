#ifndef __qSlicerModuleSelectorWidget_h
#define __qSlicerModuleSelectorWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>

#include "qSlicerBaseQTGUIExport.h"

class qSlicerModuleSelectorWidgetPrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerModuleSelectorWidget: public QWidget
{
  Q_OBJECT
public:
  typedef QWidget Superclass;
  qSlicerModuleSelectorWidget(QWidget* parent = 0);

  /// 
  /// Add a list of module available for selection
  void addModules(const QStringList& moduleNames);

  /// 
  /// Remove a module from the list given its name
  void removeModule(const QString& name);

signals:
  void moduleSelected(const QString& name);
  
private:
  CTK_DECLARE_PRIVATE(qSlicerModuleSelectorWidget);
};

#endif

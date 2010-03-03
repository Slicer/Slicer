#ifndef __qCTKTreeComboBoxPlugin_h
#define __qCTKTreeComboBoxPlugin_h

#include "qCTKWidgetsAbstractPlugin.h"
#include "qCTKWidgetsPluginExport.h"

class QCTK_WIDGETS_PLUGIN_EXPORT qCTKTreeComboBoxPlugin : public QObject,
                                         public qCTKWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qCTKTreeComboBoxPlugin(QObject *_parent = 0);
  
  QWidget *createWidget(QWidget *_parent);
  QString  domXml() const; 
  QIcon    icon() const;
  QString  includeFile() const;
  bool     isContainer() const;
  QString  name() const;
  
};

#endif

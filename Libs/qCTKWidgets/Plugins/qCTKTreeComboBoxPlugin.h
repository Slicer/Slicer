#ifndef __qCTKTreeComboBoxPlugin_h
#define __qCTKTreeComboBoxPlugin_h

#include "qCTKWidgetsBasePlugin.h"
#include "qCTKWidgetsPluginWin32Header.h"

class QCTK_WIDGETS_PLUGIN_EXPORT qCTKTreeComboBoxPlugin : public QObject,
                                         public qCTKWidgetsBasePlugin
{
  Q_OBJECT

public:
  qCTKTreeComboBoxPlugin(QObject *_parent = 0);
  
  QWidget *createWidget(QWidget *_parent);
  QString domXml() const;
  QString includeFile() const;
  bool isContainer() const;
  QString name() const;
  
};

#endif

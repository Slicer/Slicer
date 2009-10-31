#ifndef __qCTKTitleComboBoxPlugin_h
#define __qCTKTitleComboBoxPlugin_h

#include "qCTKWidgetsBasePlugin.h"
#include "qCTKWidgetsPluginWin32Header.h"

class QCTK_WIDGETS_PLUGIN_EXPORT qCTKTitleComboBoxPlugin : public QObject,
                                         public qCTKWidgetsBasePlugin
{
  Q_OBJECT

public:
  qCTKTitleComboBoxPlugin(QObject *_parent = 0);
  
  QWidget *createWidget(QWidget *_parent);
  QString domXml() const;
  QString includeFile() const;
  bool isContainer() const;
  QString name() const;
  
};

#endif

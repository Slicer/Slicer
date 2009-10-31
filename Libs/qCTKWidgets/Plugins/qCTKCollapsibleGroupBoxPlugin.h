#ifndef __qCTKCollapsibleGroupBoxPlugin_h
#define __qCTKCollapsibleGroupBoxPlugin_h

#include "qCTKWidgetsPlugin.h"
#include "qCTKWidgetsPluginWin32Header.h"

class QCTK_WIDGETS_PLUGIN_EXPORT qCTKCollapsibleGroupBoxPlugin : public QObject,
                                         public qCTKWidgetsPlugin
{
  Q_OBJECT

public:
  qCTKCollapsibleGroupBoxPlugin(QObject *_parent = 0);
  
  QWidget *createWidget(QWidget *_parent);
  QString domXml() const;
  QString includeFile() const;
  bool isContainer() const;
  QString name() const;
  
};

#endif

#ifndef __qCTKCoordinatesWidgetPlugin_h
#define __qCTKCoordinatesWidgetPlugin_h

#include "qCTKWidgetsBasePlugin.h"
#include "qCTKWidgetsPluginWin32Header.h"

class QCTK_WIDGETS_PLUGIN_EXPORT qCTKCoordinatesWidgetPlugin : public QObject,
                                         public qCTKWidgetsBasePlugin
{
  Q_OBJECT

public:
  qCTKCoordinatesWidgetPlugin(QObject *_parent = 0);
  
  QWidget *createWidget(QWidget *_parent);
  QString domXml() const;
  QString includeFile() const;
  bool isContainer() const;
  QString name() const;
  
};

#endif

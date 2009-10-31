#ifndef __qCTKColorPickerButtonPlugin_h
#define __qCTKColorPickerButtonPlugin_h

#include "qCTKWidgetsBasePlugin.h"
#include "qCTKWidgetsPluginWin32Header.h"

class QCTK_WIDGETS_PLUGIN_EXPORT qCTKColorPickerButtonPlugin : public QObject,
                                    public qCTKWidgetsBasePlugin
{
  Q_OBJECT

public:
  qCTKColorPickerButtonPlugin(QObject *_parent = 0);
  
  QWidget *createWidget(QWidget *_parent);
  QString domXml() const;
  QString includeFile() const;
  bool isContainer() const;
  QString name() const;
};

#endif

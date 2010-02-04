#ifndef __qCTKMenuButtonPlugin_h
#define __qCTKMenuButtonPlugin_h

#include "qCTKWidgetsAbstractPlugin.h"

class QCTK_WIDGETS_PLUGIN_EXPORT qCTKMenuButtonPlugin : public QObject,
                                    public qCTKWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qCTKMenuButtonPlugin(QObject *_parent = 0);
  
  QWidget *createWidget(QWidget *_parent);
  QString domXml() const;
  QString includeFile() const;
  bool isContainer() const;
  QString name() const;
};

#endif

#ifndef __qCTKPositionWidgetPlugin_h
#define __qCTKPositionWidgetPlugin_h

#include "qCTKWidgetsPlugin.h"

class qCTKPositionWidgetPlugin : public QObject,
                                         public qCTKWidgetsPlugin
{
  Q_OBJECT

public:
  qCTKPositionWidgetPlugin(QObject *_parent = 0);
  
  QWidget *createWidget(QWidget *_parent);
  QString domXml() const;
  QString includeFile() const;
  bool isContainer() const;
  QString name() const;
  
};

#endif

#ifndef __qCTKCoordinatesWidgetPlugin_h
#define __qCTKCoordinatesWidgetPlugin_h

#include "qCTKWidgetsPlugin.h"

class qCTKCoordinatesWidgetPlugin : public QObject,
                                         public qCTKWidgetsPlugin
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

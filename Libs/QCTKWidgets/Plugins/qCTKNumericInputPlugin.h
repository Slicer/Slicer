#ifndef __qCTKNumericInputPlugin_h
#define __qCTKNumericInputPlugin_h

#include "qCTKWidgetsPlugin.h"

class qCTKNumericInputPlugin : public QObject,
                                         public qCTKWidgetsPlugin
{
  Q_OBJECT

public:
  qCTKNumericInputPlugin(QObject *_parent = 0);
  
  QWidget *createWidget(QWidget *_parent);
  QString domXml() const;
  QString includeFile() const;
  bool isContainer() const;
  QString name() const;
  
};

#endif

#ifndef __qCTKCollapsibleGroupBoxPlugin_h
#define __qCTKCollapsibleGroupBoxPlugin_h

#include "qCTKWidgetsPlugin.h"

class qCTKCollapsibleGroupBoxPlugin : public QObject,
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

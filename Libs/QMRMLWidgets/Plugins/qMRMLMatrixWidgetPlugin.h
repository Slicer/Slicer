#ifndef __qMRMLMatrixWidgetPlugin_h
#define __qMRMLMatrixWidgetPlugin_h

#include "qMRMLWidgetsPlugin.h"

class qMRMLMatrixWidgetPlugin : public QObject,
                                public qMRMLWidgetsPlugin
{
  Q_OBJECT

public:
  qMRMLMatrixWidgetPlugin(QObject *_parent = 0);
  
  QWidget *createWidget(QWidget *_parent);
  QString domXml() const;
  QString includeFile() const;
  bool isContainer() const;
  QString name() const;
  
};

#endif

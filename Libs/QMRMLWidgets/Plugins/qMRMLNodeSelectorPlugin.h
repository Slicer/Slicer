#ifndef __qMRMLNodeSelectorPlugin_h
#define __qMRMLNodeSelectorPlugin_h

#include "qMRMLWidgetsPlugin.h"

class qMRMLNodeSelectorPlugin : public QObject,
                                         public qMRMLWidgetsPlugin
{
  Q_OBJECT

public:
  qMRMLNodeSelectorPlugin(QObject *_parent = 0);
  
  QWidget *createWidget(QWidget *_parent);
  QString domXml() const;
  QString includeFile() const;
  bool isContainer() const;
  QString name() const;
  
};

#endif

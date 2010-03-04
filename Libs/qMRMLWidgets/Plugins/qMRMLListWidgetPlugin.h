#ifndef __qMRMLListWidgetPlugin_h
#define __qMRMLListWidgetPlugin_h

#include "qMRMLWidgetsAbstractPlugin.h"

class QMRML_WIDGETS_PLUGIN_EXPORT qMRMLListWidgetPlugin : public QObject,
                                public qMRMLWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qMRMLListWidgetPlugin(QObject *_parent = 0);
  
  QWidget *createWidget(QWidget *_parent);
  QString  domXml() const;
  QIcon    icon() const;
  QString  includeFile() const;
  bool     isContainer() const;
  QString  name() const;
  
};

#endif

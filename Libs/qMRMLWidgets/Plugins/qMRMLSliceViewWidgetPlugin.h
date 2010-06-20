#ifndef __qMRMLSliceViewWidgetPlugin_h
#define __qMRMLSliceViewWidgetPlugin_h

#include "qMRMLWidgetsAbstractPlugin.h"

class QMRML_WIDGETS_PLUGIN_EXPORT qMRMLSliceViewWidgetPlugin :
  public QObject,
  public qMRMLWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qMRMLSliceViewWidgetPlugin(QObject *_parent = 0);
  
  QWidget *createWidget(QWidget *_parent);
  QString  domXml() const;
//   QIcon    icon() const;
  QString  includeFile() const;
  bool     isContainer() const;
  QString  name() const;
  
};

#endif

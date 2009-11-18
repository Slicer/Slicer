#ifndef __qMRMLNodeTreeSelectorPlugin_h
#define __qMRMLNodeTreeSelectorPlugin_h

#include "qMRMLWidgetsBasePlugin.h"
#include "qMRMLWidgetsPluginWin32Header.h"

class QMRML_WIDGETS_PLUGIN_EXPORT qMRMLNodeTreeSelectorPlugin : public QObject,
                                         public qMRMLWidgetsBasePlugin
{
  Q_OBJECT

public:
  qMRMLNodeTreeSelectorPlugin(QObject *_parent = 0);
  
  QWidget *createWidget(QWidget *_parent);
  QString domXml() const;
  QString includeFile() const;
  bool isContainer() const;
  QString name() const;
  
};

#endif

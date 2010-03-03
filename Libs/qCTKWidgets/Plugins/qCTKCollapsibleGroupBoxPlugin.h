#ifndef __qCTKCollapsibleGroupBoxPlugin_h
#define __qCTKCollapsibleGroupBoxPlugin_h

#include "qCTKWidgetsAbstractPlugin.h"

class QCTK_WIDGETS_PLUGIN_EXPORT qCTKCollapsibleGroupBoxPlugin : public QObject,
                                         public qCTKWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qCTKCollapsibleGroupBoxPlugin(QObject *_parent = 0);
  
  QWidget *createWidget(QWidget *_parent);
  QString  domXml() const;
  QIcon    icon() const;
  QString  includeFile() const;
  bool     isContainer() const;
  QString  name() const;
  
};

#endif

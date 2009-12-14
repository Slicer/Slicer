#ifndef __qCTKCollapsibleWidget2Plugin_h
#define __qCTKCollapsibleWidget2Plugin_h

#include "qCTKWidgetsAbstractPlugin.h"
#include "qCTKWidgetsPluginExport.h"

class QCTK_WIDGETS_PLUGIN_EXPORT qCTKCollapsibleWidget2Plugin : public QObject,
                                         public qCTKWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qCTKCollapsibleWidget2Plugin(QObject *_parent = 0);
  
  QWidget *createWidget(QWidget *_parent);
  QString domXml() const;
  QString includeFile() const;
  bool isContainer() const;
  QString name() const;
  
};

#endif

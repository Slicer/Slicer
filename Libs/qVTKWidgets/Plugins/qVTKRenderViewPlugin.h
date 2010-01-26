#ifndef __qVTKRenderViewPlugin_h
#define __qVTKRenderViewPlugin_h

#include "qVTKWidgetsAbstractPlugin.h"

class QVTK_WIDGETS_PLUGIN_EXPORT qVTKRenderViewPlugin : public QObject,
                                                        public qVTKWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qVTKRenderViewPlugin(QObject *_parent = 0);
  
  QWidget *createWidget(QWidget *_parent);
  QString domXml() const;
  QString includeFile() const;
  bool isContainer() const;
  QString name() const;
  
};

#endif

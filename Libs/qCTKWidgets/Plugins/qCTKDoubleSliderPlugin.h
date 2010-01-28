#ifndef __qCTKDoubleSliderPlugin_h
#define __qCTKDoubleSliderPlugin_h

#include "qCTKWidgetsAbstractPlugin.h"

class QCTK_WIDGETS_PLUGIN_EXPORT qCTKDoubleSliderPlugin : public QObject,
                                                          public qCTKWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qCTKDoubleSliderPlugin(QObject *_parent = 0);

  QWidget *createWidget(QWidget *_parent);
  QString domXml() const;
  QString includeFile() const;
  bool isContainer() const;
  QString name() const;

};

#endif

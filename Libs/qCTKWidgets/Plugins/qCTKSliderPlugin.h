#ifndef __qCTKSliderPlugin_h
#define __qCTKSliderPlugin_h

#include "qCTKWidgetsAbstractPlugin.h"

class QCTK_WIDGETS_PLUGIN_EXPORT qCTKSliderPlugin : public QObject,
                                                          public qCTKWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qCTKSliderPlugin(QObject *_parent = 0);

  QWidget *createWidget(QWidget *_parent);
  QString domXml() const;
  QString includeFile() const;
  bool isContainer() const;
  QString name() const;

};

#endif

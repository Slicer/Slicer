#ifndef __qCTKDoubleRangeSliderPlugin_h
#define __qCTKDoubleRangeSliderPlugin_h

#include "qCTKWidgetsAbstractPlugin.h"

class QCTK_WIDGETS_PLUGIN_EXPORT qCTKDoubleRangeSliderPlugin : public QObject,
                                                          public qCTKWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qCTKDoubleRangeSliderPlugin(QObject *_parent = 0);

  QWidget *createWidget(QWidget *_parent);
  QString  domXml() const;
  QIcon    icon() const;
  QString  includeFile() const;
  bool     isContainer() const;
  QString  name() const;

};

#endif

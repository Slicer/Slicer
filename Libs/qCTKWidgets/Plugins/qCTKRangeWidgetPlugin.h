#ifndef __qCTKRangeWidgetPlugin_h
#define __qCTKRangeWidgetPlugin_h

#include "qCTKWidgetsAbstractPlugin.h"

class QCTK_WIDGETS_PLUGIN_EXPORT qCTKRangeWidgetPlugin : public QObject,
                                                          public qCTKWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qCTKRangeWidgetPlugin(QObject *_parent = 0);

  QWidget *createWidget(QWidget *_parent);
  QString  domXml() const;
  QIcon    icon() const;
  QString  includeFile() const;
  bool     isContainer() const;
  QString  name() const;

};

#endif

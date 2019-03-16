#ifndef __qMRMLVolumeThresholdWidgetPlugin_h
#define __qMRMLVolumeThresholdWidgetPlugin_h

#include "qMRMLWidgetsAbstractPlugin.h"

class QMRML_WIDGETS_PLUGINS_EXPORT qMRMLVolumeThresholdWidgetPlugin : public QObject,
                                         public qMRMLWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qMRMLVolumeThresholdWidgetPlugin(QObject *_parent = nullptr);

  QWidget *createWidget(QWidget *_parent) override;
  QString  domXml() const override;
  QIcon    icon() const override;
  QString  includeFile() const override;
  bool     isContainer() const override;
  QString  name() const override;

};

#endif


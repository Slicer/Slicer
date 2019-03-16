#ifndef __qMRMLThreeDViewInformationWidgetPlugin_h
#define __qMRMLThreeDViewInformationWidgetPlugin_h

#include "qMRMLWidgetsAbstractPlugin.h"

class QMRML_WIDGETS_PLUGINS_EXPORT qMRMLThreeDViewInformationWidgetPlugin : public QObject,
                                         public qMRMLWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qMRMLThreeDViewInformationWidgetPlugin(QObject *_parent = nullptr);

  QWidget *createWidget(QWidget *_parent) override;
  QString  domXml() const override;
  QIcon    icon() const override;
  QString  includeFile() const override;
  bool     isContainer() const override;
  QString  name() const override;

};

#endif

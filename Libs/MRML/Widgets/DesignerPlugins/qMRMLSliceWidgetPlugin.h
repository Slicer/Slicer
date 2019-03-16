#ifndef __qMRMLSliceWidgetPlugin_h
#define __qMRMLSliceWidgetPlugin_h

#include "qMRMLWidgetsAbstractPlugin.h"

class QMRML_WIDGETS_PLUGINS_EXPORT qMRMLSliceWidgetPlugin :
  public QObject,
  public qMRMLWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qMRMLSliceWidgetPlugin(QObject *_parent = nullptr);

  QWidget *createWidget(QWidget *_parent) override;
  QString  domXml() const override;
//   QIcon    icon() const;
  QString  includeFile() const override;
  bool     isContainer() const override;
  QString  name() const override;

};

#endif

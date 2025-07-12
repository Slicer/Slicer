#ifndef __qMRMLScalarsDisplayWidgetPlugin_h
#define __qMRMLScalarsDisplayWidgetPlugin_h

#include "qMRMLWidgetsAbstractPlugin.h"

class QMRML_WIDGETS_PLUGINS_EXPORT qMRMLScalarsDisplayWidgetPlugin
  : public QObject
  , public qMRMLWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qMRMLScalarsDisplayWidgetPlugin(QObject* _parent = nullptr);

  QWidget* createWidget(QWidget* _parent) override;
  QString domXml() const override;
  QIcon icon() const override;
  QString includeFile() const override;
  bool isContainer() const override;
  QString name() const override;
};

#endif

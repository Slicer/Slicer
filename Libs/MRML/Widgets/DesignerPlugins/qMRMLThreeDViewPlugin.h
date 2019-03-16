#ifndef __qMRMLThreeDViewPlugin_h
#define __qMRMLThreeDViewPlugin_h

#include "qMRMLWidgetsAbstractPlugin.h"

class QMRML_WIDGETS_PLUGINS_EXPORT qMRMLThreeDViewPlugin :
  public QObject,
  public qMRMLWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qMRMLThreeDViewPlugin(QObject *_parent = nullptr);

  QWidget *createWidget(QWidget *_parent) override;
  QString  domXml() const override;
//   QIcon    icon() const;
  QString  includeFile() const override;
  bool     isContainer() const override;
  QString  name() const override;

};

#endif

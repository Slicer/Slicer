#ifndef __qMRMLLinearTransformSliderPlugin_h
#define __qMRMLLinearTransformSliderPlugin_h

#include "qMRMLWidgetsAbstractPlugin.h"

class QMRML_WIDGETS_PLUGINS_EXPORT qMRMLLinearTransformSliderPlugin : public QObject,
                                         public qMRMLWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qMRMLLinearTransformSliderPlugin(QObject *_parent = 0);

  QWidget *createWidget(QWidget *_parent);
  QString domXml() const;
  QIcon   icon() const;
  QString includeFile() const;
  bool isContainer() const;
  QString name() const;

};

#endif

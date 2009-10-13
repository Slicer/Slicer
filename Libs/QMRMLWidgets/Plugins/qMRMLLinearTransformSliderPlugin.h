#ifndef __qMRMLLinearTransformSliderPlugin_h
#define __qMRMLLinearTransformSliderPlugin_h

#include "qMRMLWidgetsPlugin.h"

class qMRMLLinearTransformSliderPlugin : public QObject,
                                         public qMRMLWidgetsPlugin
{
  Q_OBJECT

public:
  qMRMLLinearTransformSliderPlugin(QObject *_parent = 0);
  
  QWidget *createWidget(QWidget *_parent);
  QString domXml() const;
  QString includeFile() const;
  bool isContainer() const;
  QString name() const;
  
};

#endif

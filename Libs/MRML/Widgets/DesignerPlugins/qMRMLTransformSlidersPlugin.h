#ifndef __qMRMLTransformSlidersPlugin_h
#define __qMRMLTransformSlidersPlugin_h

#include "qMRMLWidgetsAbstractPlugin.h"

class QMRML_WIDGETS_PLUGINS_EXPORT qMRMLTransformSlidersPlugin : public QObject,
                                    public qMRMLWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qMRMLTransformSlidersPlugin(QObject *_parent = 0);

  QWidget *createWidget(QWidget *_parent);
  QString  domXml() const;
  QIcon    icon() const;
  QString  includeFile() const;
  bool     isContainer() const;
  QString  name() const;
};

#endif

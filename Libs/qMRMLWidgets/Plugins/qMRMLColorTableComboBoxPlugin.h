#ifndef __qMRMLColorTableComboBoxPlugin_h
#define __qMRMLColorTableComboBoxPlugin_h

#include "qMRMLWidgetsAbstractPlugin.h"

class QMRML_WIDGETS_PLUGIN_EXPORT qMRMLColorTableComboBoxPlugin
  : public QObject, public qMRMLWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qMRMLColorTableComboBoxPlugin(QObject *_parent = 0);

  QWidget *createWidget(QWidget *_parent);
  QString  domXml() const;
  QIcon    icon() const;
  QString  includeFile() const;
  bool     isContainer() const;
  QString  name() const;
};

#endif

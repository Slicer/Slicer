#ifndef __qCTKAddRemoveComboBoxPlugin_h
#define __qCTKAddRemoveComboBoxPlugin_h

#include "qCTKWidgetsAbstractPlugin.h"

class QCTK_WIDGETS_PLUGIN_EXPORT qCTKAddRemoveComboBoxPlugin : public QObject,
                                         public qCTKWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qCTKAddRemoveComboBoxPlugin(QObject *_parent = 0);
  
  QWidget *createWidget(QWidget *_parent);
  QString domXml() const;
  QString includeFile() const;
  bool isContainer() const;
  QString name() const;
  
};

#endif

#ifndef __qCTKTitleComboBoxPlugin_h
#define __qCTKTitleComboBoxPlugin_h

#include "qCTKWidgetsPlugin.h"

class qCTKTitleComboBoxPlugin : public QObject,
                                         public qCTKWidgetsPlugin
{
  Q_OBJECT

public:
  qCTKTitleComboBoxPlugin(QObject *_parent = 0);
  
  QWidget *createWidget(QWidget *_parent);
  QString domXml() const;
  QString includeFile() const;
  bool isContainer() const;
  QString name() const;
  
};

#endif

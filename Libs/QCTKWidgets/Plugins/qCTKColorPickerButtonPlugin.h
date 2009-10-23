#ifndef __qCTKColorPickerButtonPlugin_h
#define __qCTKColorPickerButtonPlugin_h

#include "qCTKWidgetsPlugin.h"

class qCTKColorPickerButtonPlugin : public QObject,
                                    public qCTKWidgetsPlugin
{
  Q_OBJECT

public:
  qCTKColorPickerButtonPlugin(QObject *_parent = 0);
  
  QWidget *createWidget(QWidget *_parent);
  QString domXml() const;
  QString includeFile() const;
  bool isContainer() const;
  QString name() const;
};

#endif

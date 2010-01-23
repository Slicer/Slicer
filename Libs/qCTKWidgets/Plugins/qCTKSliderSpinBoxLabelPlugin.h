#ifndef __qCTKSliderSpinBoxLabelPlugin_h
#define __qCTKSliderSpinBoxLabelPlugin_h

#include "qCTKWidgetsAbstractPlugin.h"

class QCTK_WIDGETS_PLUGIN_EXPORT qCTKSliderSpinBoxLabelPlugin : public QObject,
                                                          public qCTKWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qCTKSliderSpinBoxLabelPlugin(QObject *_parent = 0);

  QWidget *createWidget(QWidget *_parent);
  QString domXml() const;
  QString includeFile() const;
  bool isContainer() const;
  QString name() const;

};

#endif

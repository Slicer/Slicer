#include "qCTKSliderSpinBoxLabelPlugin.h"
#include "qCTKSliderSpinBoxLabel.h"

qCTKSliderSpinBoxLabelPlugin::qCTKSliderSpinBoxLabelPlugin(QObject *_parent)
        : QObject(_parent)
{

}

QWidget *qCTKSliderSpinBoxLabelPlugin::createWidget(QWidget *_parent)
{
  qCTKSliderSpinBoxLabel* _widget = new qCTKSliderSpinBoxLabel(_parent);
  return _widget;
}

QString qCTKSliderSpinBoxLabelPlugin::domXml() const
{
  return "<widget class=\"qCTKSliderSpinBoxLabel\" \
          name=\"qCTKSliderSpinBoxLabel\">\n"
          "</widget>\n";
}

QString qCTKSliderSpinBoxLabelPlugin::includeFile() const
{
  return "qCTKSliderSpinBoxLabel.h";
}

bool qCTKSliderSpinBoxLabelPlugin::isContainer() const
{
  return false;
}

QString qCTKSliderSpinBoxLabelPlugin::name() const
{
  return "qCTKSliderSpinBoxLabel";
}

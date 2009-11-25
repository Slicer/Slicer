#include "qCTKSliderPlugin.h"
#include "qCTKSlider.h"

qCTKSliderPlugin::qCTKSliderPlugin(QObject *_parent)
        : QObject(_parent)
{

}

QWidget *qCTKSliderPlugin::createWidget(QWidget *_parent)
{
  qCTKSlider* _widget = new qCTKSlider(_parent);
  return _widget;
}

QString qCTKSliderPlugin::domXml() const
{
  return "<widget class=\"qCTKSlider\" \
          name=\"CTKSlider\">\n"
          "</widget>\n";
}

QString qCTKSliderPlugin::includeFile() const
{
  return "qCTKSlider.h";
}

bool qCTKSliderPlugin::isContainer() const
{
  return false;
}

QString qCTKSliderPlugin::name() const
{
  return "qCTKSlider";
}

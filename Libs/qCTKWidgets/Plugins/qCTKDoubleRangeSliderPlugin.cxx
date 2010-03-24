#include "qCTKDoubleRangeSliderPlugin.h"
#include "qCTKDoubleRangeSlider.h"

// --------------------------------------------------------------------------
qCTKDoubleRangeSliderPlugin::qCTKDoubleRangeSliderPlugin(QObject *_parent)
  : QObject(_parent)
{
}

// --------------------------------------------------------------------------
QWidget *qCTKDoubleRangeSliderPlugin::createWidget(QWidget *_parent)
{
  qCTKDoubleRangeSlider* _widget = new qCTKDoubleRangeSlider(Qt::Horizontal, _parent);
  return _widget;
}

// --------------------------------------------------------------------------
QString qCTKDoubleRangeSliderPlugin::domXml() const
{
  return "<widget class=\"qCTKDoubleRangeSlider\" name=\"CTKSlider\">\n"
    "<property name=\"orientation\">\n"
    "  <enum>Qt::Horizontal</enum>\n"
    " </property>\n"
    "</widget>\n";
}

// --------------------------------------------------------------------------
QIcon qCTKDoubleRangeSliderPlugin::icon() const
{
  return QIcon(":/Icons/hrangeslider.png");
}

// --------------------------------------------------------------------------
QString qCTKDoubleRangeSliderPlugin::includeFile() const
{
  return "qCTKDoubleRangeSlider.h";
}

// --------------------------------------------------------------------------
bool qCTKDoubleRangeSliderPlugin::isContainer() const
{
  return false;
}

// --------------------------------------------------------------------------
QString qCTKDoubleRangeSliderPlugin::name() const
{
  return "qCTKDoubleRangeSlider";
}

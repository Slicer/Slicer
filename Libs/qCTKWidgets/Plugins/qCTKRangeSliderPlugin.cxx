#include "qCTKRangeSliderPlugin.h"
#include "qCTKRangeSlider.h"

// --------------------------------------------------------------------------
qCTKRangeSliderPlugin::qCTKRangeSliderPlugin(QObject *_parent)
  : QObject(_parent)
{
}

// --------------------------------------------------------------------------
QWidget *qCTKRangeSliderPlugin::createWidget(QWidget *_parent)
{
  qCTKRangeSlider* _widget = new qCTKRangeSlider(Qt::Horizontal, _parent);
  return _widget;
}

// --------------------------------------------------------------------------
QString qCTKRangeSliderPlugin::domXml() const
{
  return "<widget class=\"qCTKRangeSlider\" name=\"RangeSlider\">\n"
    "<property name=\"orientation\">\n"
    "  <enum>Qt::Horizontal</enum>\n"
    " </property>\n"
    "</widget>\n";
}

// --------------------------------------------------------------------------
QIcon qCTKRangeSliderPlugin::icon() const
{
  return QIcon(":/Icons/hrangeslider.png");
}

// --------------------------------------------------------------------------
QString qCTKRangeSliderPlugin::includeFile() const
{
  return "qCTKRangeSlider.h";
}

// --------------------------------------------------------------------------
bool qCTKRangeSliderPlugin::isContainer() const
{
  return false;
}

// --------------------------------------------------------------------------
QString qCTKRangeSliderPlugin::name() const
{
  return "qCTKRangeSlider";
}

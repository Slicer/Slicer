#include "qCTKDoubleSliderPlugin.h"
#include "qCTKDoubleSlider.h"

// --------------------------------------------------------------------------
qCTKDoubleSliderPlugin::qCTKDoubleSliderPlugin(QObject *_parent)
  : QObject(_parent)
{
}

// --------------------------------------------------------------------------
QWidget *qCTKDoubleSliderPlugin::createWidget(QWidget *_parent)
{
  qCTKDoubleSlider* _widget = new qCTKDoubleSlider(Qt::Horizontal, _parent);
  return _widget;
}

// --------------------------------------------------------------------------
QString qCTKDoubleSliderPlugin::domXml() const
{
  return "<widget class=\"qCTKDoubleSlider\" name=\"CTKSlider\">\n"
    "<property name=\"orientation\">\n"
    "  <enum>Qt::Horizontal</enum>\n"
    " </property>\n"
    "</widget>\n";
}

// --------------------------------------------------------------------------
QIcon qCTKDoubleSliderPlugin::icon() const
{
  return QIcon(":/Icons/hslider.png");
}

// --------------------------------------------------------------------------
QString qCTKDoubleSliderPlugin::includeFile() const
{
  return "qCTKDoubleSlider.h";
}

// --------------------------------------------------------------------------
bool qCTKDoubleSliderPlugin::isContainer() const
{
  return false;
}

// --------------------------------------------------------------------------
QString qCTKDoubleSliderPlugin::name() const
{
  return "qCTKDoubleSlider";
}

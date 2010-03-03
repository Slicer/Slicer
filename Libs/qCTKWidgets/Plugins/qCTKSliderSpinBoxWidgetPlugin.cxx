#include "qCTKSliderSpinBoxWidgetPlugin.h"
#include "qCTKSliderSpinBoxWidget.h"

qCTKSliderSpinBoxWidgetPlugin::qCTKSliderSpinBoxWidgetPlugin(QObject *_parent)
        : QObject(_parent)
{

}

QWidget *qCTKSliderSpinBoxWidgetPlugin::createWidget(QWidget *_parent)
{
  qCTKSliderSpinBoxWidget* _widget = new qCTKSliderSpinBoxWidget(_parent);
  return _widget;
}

QString qCTKSliderSpinBoxWidgetPlugin::domXml() const
{
  return "<widget class=\"qCTKSliderSpinBoxWidget\" \
          name=\"qCTKSliderSpinBoxWidget\">\n"
          "</widget>\n";
}

// --------------------------------------------------------------------------
QIcon qCTKSliderSpinBoxWidgetPlugin::icon() const
{
  return QIcon(":/Icons/sliderspinbox.png");
}

QString qCTKSliderSpinBoxWidgetPlugin::includeFile() const
{
  return "qCTKSliderSpinBoxWidget.h";
}

bool qCTKSliderSpinBoxWidgetPlugin::isContainer() const
{
  return false;
}

QString qCTKSliderSpinBoxWidgetPlugin::name() const
{
  return "qCTKSliderSpinBoxWidget";
}

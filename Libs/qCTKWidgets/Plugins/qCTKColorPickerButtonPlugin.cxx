#include "qCTKColorPickerButtonPlugin.h"
#include "qCTKColorPickerButton.h"

qCTKColorPickerButtonPlugin::qCTKColorPickerButtonPlugin(QObject *_parent)
        : QObject(_parent)
{
}

QWidget *qCTKColorPickerButtonPlugin::createWidget(QWidget *_parent)
{
  qCTKColorPickerButton* _widget = new qCTKColorPickerButton(_parent);
  return _widget;
}

QString qCTKColorPickerButtonPlugin::domXml() const
{
  return "<widget class=\"qCTKColorPickerButton\" \
          name=\"CTKColorPickerButton\">\n"
          "</widget>\n";
}

QIcon qCTKColorPickerButtonPlugin::icon() const
{
  return QIcon(":/Icons/pushbutton.png");
}

QString qCTKColorPickerButtonPlugin::includeFile() const
{
  return "qCTKColorPickerButton.h";
}

bool qCTKColorPickerButtonPlugin::isContainer() const
{
  return false;
}

QString qCTKColorPickerButtonPlugin::name() const
{
  return "qCTKColorPickerButton";
}

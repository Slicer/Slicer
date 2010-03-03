#include "qCTKNumericInputPlugin.h"
#include "qCTKNumericInput.h"

qCTKNumericInputPlugin::qCTKNumericInputPlugin(QObject *_parent)
        : QObject(_parent)
{

}

QWidget *qCTKNumericInputPlugin::createWidget(QWidget *_parent)
{
  qCTKNumericInput* _widget = new qCTKNumericInput(_parent);
  return _widget;
}

QString qCTKNumericInputPlugin::domXml() const
{
  return "<widget class=\"qCTKNumericInput\" \
          name=\"CTKNumericInput\">\n"
          "</widget>\n";
}

// --------------------------------------------------------------------------
QIcon qCTKNumericInputPlugin::icon() const
{
  return QIcon(":/Icons/doublespinbox.png");
}

QString qCTKNumericInputPlugin::includeFile() const
{
  return "qCTKNumericInput.h";
}

bool qCTKNumericInputPlugin::isContainer() const
{
  return false;
}

QString qCTKNumericInputPlugin::name() const
{
  return "qCTKNumericInput";
}

#include "qCTKRangeWidgetPlugin.h"
#include "qCTKRangeWidget.h"

qCTKRangeWidgetPlugin::qCTKRangeWidgetPlugin(QObject *_parent)
        : QObject(_parent)
{

}

QWidget *qCTKRangeWidgetPlugin::createWidget(QWidget *_parent)
{
  qCTKRangeWidget* _widget = new qCTKRangeWidget(_parent);
  return _widget;
}

QString qCTKRangeWidgetPlugin::domXml() const
{
  return "<widget class=\"qCTKRangeWidget\" \
          name=\"qCTKRangeWidget\">\n"
          "</widget>\n";
}

// --------------------------------------------------------------------------
QIcon qCTKRangeWidgetPlugin::icon() const
{
  return QIcon(":/Icons/hrangeslider.png");
}

QString qCTKRangeWidgetPlugin::includeFile() const
{
  return "qCTKRangeWidget.h";
}

bool qCTKRangeWidgetPlugin::isContainer() const
{
  return false;
}

QString qCTKRangeWidgetPlugin::name() const
{
  return "qCTKRangeWidget";
}

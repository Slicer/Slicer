#include "qCTKPositionWidgetPlugin.h"
#include "qCTKPositionWidget.h"

qCTKPositionWidgetPlugin::qCTKPositionWidgetPlugin(QObject *_parent)
        : QObject(_parent)
{
}

QWidget *qCTKPositionWidgetPlugin::createWidget(QWidget *_parent)
{
  qCTKPositionWidget* _widget = new qCTKPositionWidget(_parent);
  return _widget;
}

QString qCTKPositionWidgetPlugin::domXml() const
{
  return "<widget class=\"qCTKPositionWidget\" \
          name=\"CTKPositionWidget\">\n"
          "</widget>\n";
}

QString qCTKPositionWidgetPlugin::includeFile() const
{
  return "qCTKPositionWidget.h";
}

bool qCTKPositionWidgetPlugin::isContainer() const
{
  return false;
}

QString qCTKPositionWidgetPlugin::name() const
{
  return "qCTKPositionWidget";
}

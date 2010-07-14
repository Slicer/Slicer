#include "qMRMLRangeWidgetPlugin.h"
#include "qMRMLRangeWidget.h"

qMRMLRangeWidgetPlugin::qMRMLRangeWidgetPlugin(QObject *parentWidget)
        : QObject(parentWidget)
{
}

QWidget *qMRMLRangeWidgetPlugin::createWidget(QWidget *parentWidget)
{
  qMRMLRangeWidget* newWidget = new qMRMLRangeWidget(parentWidget);
  return newWidget;
}

QString qMRMLRangeWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLRangeWidget\" \
          name=\"MRMLRangeWidget\">\n"
          "</widget>\n";
}

QIcon qMRMLRangeWidgetPlugin::icon() const
{
  return QIcon(":/Icons/sliderspinbox.png");
}

QString qMRMLRangeWidgetPlugin::includeFile() const
{
  return "qMRMLRangeWidget.h";
}

bool qMRMLRangeWidgetPlugin::isContainer() const
{
  return false;
}

QString qMRMLRangeWidgetPlugin::name() const
{
  return "qMRMLRangeWidget";
}

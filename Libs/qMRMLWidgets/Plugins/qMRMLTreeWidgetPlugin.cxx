#include "qMRMLTreeWidgetPlugin.h"
#include "qMRMLTreeWidget.h"

qMRMLTreeWidgetPlugin::qMRMLTreeWidgetPlugin(QObject *_parent)
        : QObject(_parent)
{
}

QWidget *qMRMLTreeWidgetPlugin::createWidget(QWidget *_parent)
{
  qMRMLTreeWidget* _widget = new qMRMLTreeWidget(_parent);
  return _widget;
}

QString qMRMLTreeWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLTreeWidget\" \
          name=\"MRMLTreeWidget\">\n"
          "</widget>\n";
}

QIcon qMRMLTreeWidgetPlugin::icon() const
{
  return QIcon(":/Icons/listview.png");
}

QString qMRMLTreeWidgetPlugin::includeFile() const
{
  return "qMRMLTreeWidget.h";
}

bool qMRMLTreeWidgetPlugin::isContainer() const
{
  return false;
}

QString qMRMLTreeWidgetPlugin::name() const
{
  return "qMRMLTreeWidget";
}

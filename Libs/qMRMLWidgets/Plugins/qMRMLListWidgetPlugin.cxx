#include "qMRMLListWidgetPlugin.h"
#include "qMRMLListWidget.h"

qMRMLListWidgetPlugin::qMRMLListWidgetPlugin(QObject *_parent)
        : QObject(_parent)
{
}

QWidget *qMRMLListWidgetPlugin::createWidget(QWidget *_parent)
{
  qMRMLListWidget* _widget = new qMRMLListWidget(_parent);
  return _widget;
}

QString qMRMLListWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLListWidget\" \
          name=\"MRMLListWidget\">\n"
          "</widget>\n";
}

QIcon qMRMLListWidgetPlugin::icon() const
{
  return QIcon(":Icons/listbox.png");
}

QString qMRMLListWidgetPlugin::includeFile() const
{
  return "qMRMLListWidget.h";
}

bool qMRMLListWidgetPlugin::isContainer() const
{
  return false;
}

QString qMRMLListWidgetPlugin::name() const
{
  return "qMRMLListWidget";
}

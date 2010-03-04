#include "qMRMLMatrixWidgetPlugin.h"
#include "qMRMLMatrixWidget.h"

qMRMLMatrixWidgetPlugin::qMRMLMatrixWidgetPlugin(QObject *_parent)
        : QObject(_parent)
{
}

QWidget *qMRMLMatrixWidgetPlugin::createWidget(QWidget *_parent)
{
  qMRMLMatrixWidget* _widget = new qMRMLMatrixWidget(_parent);
  return _widget;
}

QString qMRMLMatrixWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLMatrixWidget\" \
          name=\"MRMLMatrixWidget\">\n"
          " <property name=\"geometry\">\n"
          "  <rect>\n"
          "   <x>0</x>\n"
          "   <y>0</y>\n"
          "   <width>200</width>\n"
          "   <height>200</height>\n"
          "  </rect>\n"
          " </property>\n"
          "</widget>\n";
}

QIcon qMRMLMatrixWidgetPlugin::icon() const
{
  return QIcon(":/Icons/table.png");
}

QString qMRMLMatrixWidgetPlugin::includeFile() const
{
  return "qMRMLMatrixWidget.h";
}

bool qMRMLMatrixWidgetPlugin::isContainer() const
{
  return false;
}

QString qMRMLMatrixWidgetPlugin::name() const
{
  return "qMRMLMatrixWidget";
}

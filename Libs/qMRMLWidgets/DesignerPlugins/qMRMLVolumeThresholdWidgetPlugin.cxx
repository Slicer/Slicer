#include "qMRMLVolumeThresholdWidgetPlugin.h"
#include "qMRMLVolumeThresholdWidget.h"

qMRMLVolumeThresholdWidgetPlugin::qMRMLVolumeThresholdWidgetPlugin(QObject *_parent)
        : QObject(_parent)
{
}

QWidget *qMRMLVolumeThresholdWidgetPlugin::createWidget(QWidget *_parent)
{
  qMRMLVolumeThresholdWidget* _widget = new qMRMLVolumeThresholdWidget(_parent);
  return _widget;
}

QString qMRMLVolumeThresholdWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLVolumeThresholdWidget\" \
          name=\"MRMLVolumeThresholdWidget\">\n"
          " <property name=\"geometry\">\n"
          "  <rect>\n"
          "   <x>0</x>\n"
          "   <y>0</y>\n"
          "   <width>200</width>\n"
          "   <height>20</height>\n"
          "  </rect>\n"
          " </property>\n"
          "</widget>\n";
}

QIcon qMRMLVolumeThresholdWidgetPlugin::icon() const
{
  return QIcon(":/Icons/combobox.png");
}

QString qMRMLVolumeThresholdWidgetPlugin::includeFile() const
{
  return "qMRMLVolumeThresholdWidget.h";
}

bool qMRMLVolumeThresholdWidgetPlugin::isContainer() const
{
  return false;
}

QString qMRMLVolumeThresholdWidgetPlugin::name() const
{
  return "qMRMLVolumeThresholdWidget";
}

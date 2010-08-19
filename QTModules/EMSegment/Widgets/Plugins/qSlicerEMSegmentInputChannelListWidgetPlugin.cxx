#include "qSlicerEMSegmentInputChannelListWidgetPlugin.h"
#include "qSlicerEMSegmentInputChannelListWidget.h"

//-----------------------------------------------------------------------------
qSlicerEMSegmentInputChannelListWidgetPlugin::
  qSlicerEMSegmentInputChannelListWidgetPlugin(QObject *newParent) : QObject(newParent)
{
}

//-----------------------------------------------------------------------------
QWidget *qSlicerEMSegmentInputChannelListWidgetPlugin::createWidget(QWidget *newParent)
{
  qSlicerEMSegmentInputChannelListWidget* _widget = 
    new qSlicerEMSegmentInputChannelListWidget(newParent);
  return _widget;
}

//-----------------------------------------------------------------------------
QString qSlicerEMSegmentInputChannelListWidgetPlugin::domXml() const
{
  return "<widget class=\"qSlicerEMSegmentInputChannelListWidget\" \
          name=\"EMSegmentInputChannelListWidget\">\n"
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

//-----------------------------------------------------------------------------
QIcon qSlicerEMSegmentInputChannelListWidgetPlugin::icon() const
{
  return QIcon(":/Icons/table.png");
}

//-----------------------------------------------------------------------------
QString qSlicerEMSegmentInputChannelListWidgetPlugin::includeFile() const
{
  return "qSlicerEMSegmentInputChannelListWidget.h";
}

//-----------------------------------------------------------------------------
bool qSlicerEMSegmentInputChannelListWidgetPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qSlicerEMSegmentInputChannelListWidgetPlugin::name() const
{
  return "qSlicerEMSegmentInputChannelListWidget";
}

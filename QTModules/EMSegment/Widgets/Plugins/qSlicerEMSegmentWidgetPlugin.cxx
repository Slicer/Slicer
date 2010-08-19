#include "qSlicerEMSegmentWidgetPlugin.h"
#include "qSlicerEMSegmentWidget.h"

//-----------------------------------------------------------------------------
qSlicerEMSegmentWidgetPlugin::qSlicerEMSegmentWidgetPlugin(QObject *newParent)
        : QObject(newParent)
{
}

//-----------------------------------------------------------------------------
QWidget *qSlicerEMSegmentWidgetPlugin::createWidget(QWidget *newParent)
{
  qSlicerEMSegmentWidget* _widget = new qSlicerEMSegmentWidget(newParent);
  return _widget;
}

//-----------------------------------------------------------------------------
QString qSlicerEMSegmentWidgetPlugin::domXml() const
{
  return "<widget class=\"qSlicerEMSegmentWidget\" \
          name=\"SlicerEMSegmentWidget\">\n"
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
QIcon qSlicerEMSegmentWidgetPlugin::icon() const
{
  return QIcon(":/Icons/widget.png");
}

//-----------------------------------------------------------------------------
QString qSlicerEMSegmentWidgetPlugin::includeFile() const
{
  return "qSlicerEMSegmentWidget.h";
}

//-----------------------------------------------------------------------------
bool qSlicerEMSegmentWidgetPlugin::isContainer() const
{
  return true;
}

//-----------------------------------------------------------------------------
QString qSlicerEMSegmentWidgetPlugin::name() const
{
  return "qSlicerEMSegmentWidget";
}

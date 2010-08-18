#include "qSlicerEMSegmentAnatomicalTreeWidgetPlugin.h"
#include "qSlicerEMSegmentAnatomicalTreeWidget.h"

//-----------------------------------------------------------------------------
qSlicerEMSegmentAnatomicalTreeWidgetPlugin::qSlicerEMSegmentAnatomicalTreeWidgetPlugin(QObject *_parent)
        : QObject(_parent)
{
}

//-----------------------------------------------------------------------------
QWidget *qSlicerEMSegmentAnatomicalTreeWidgetPlugin::createWidget(QWidget *_parent)
{
  qSlicerEMSegmentAnatomicalTreeWidget* _widget = new qSlicerEMSegmentAnatomicalTreeWidget(_parent);
  return _widget;
}

//-----------------------------------------------------------------------------
QString qSlicerEMSegmentAnatomicalTreeWidgetPlugin::domXml() const
{
  return "<widget class=\"qSlicerEMSegmentAnatomicalTreeWidget\" \
          name=\"SlicerEMSegmentAnatomicalTreeWidget\">\n"
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
QIcon qSlicerEMSegmentAnatomicalTreeWidgetPlugin::icon() const
{
  return QIcon(":/Icons/tree.png");
}

//-----------------------------------------------------------------------------
QString qSlicerEMSegmentAnatomicalTreeWidgetPlugin::includeFile() const
{
  return "qSlicerEMSegmentAnatomicalTreeWidget.h";
}

//-----------------------------------------------------------------------------
bool qSlicerEMSegmentAnatomicalTreeWidgetPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qSlicerEMSegmentAnatomicalTreeWidgetPlugin::name() const
{
  return "qSlicerEMSegmentAnatomicalTreeWidget";
}

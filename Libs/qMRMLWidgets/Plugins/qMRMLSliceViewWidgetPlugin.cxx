
// qMRML includes
#include "qMRMLSliceViewWidgetPlugin.h"
#include "qMRMLSliceViewWidget.h"

//-----------------------------------------------------------------------------
qMRMLSliceViewWidgetPlugin::qMRMLSliceViewWidgetPlugin(QObject *_parent):QObject(_parent)
{
}

//-----------------------------------------------------------------------------
QWidget *qMRMLSliceViewWidgetPlugin::createWidget(QWidget *_parent)
{
  qMRMLSliceViewWidget* _widget = new qMRMLSliceViewWidget("red", _parent);
  return _widget;
}

//-----------------------------------------------------------------------------
QString qMRMLSliceViewWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLSliceViewWidget\" \
          name=\"MRMLSliceViewWidget\">\n"
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
QString qMRMLSliceViewWidgetPlugin::includeFile() const
{
  return "qMRMLSliceViewWidget.h";
}

//-----------------------------------------------------------------------------
bool qMRMLSliceViewWidgetPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qMRMLSliceViewWidgetPlugin::name() const
{
  return "qMRMLSliceViewWidget";
}

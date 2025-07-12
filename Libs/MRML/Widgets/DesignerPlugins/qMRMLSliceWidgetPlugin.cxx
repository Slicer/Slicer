
// qMRML includes
#include "qMRMLSliceWidgetPlugin.h"
#include "qMRMLSliceWidget.h"

//-----------------------------------------------------------------------------
qMRMLSliceWidgetPlugin::qMRMLSliceWidgetPlugin(QObject* _parent)
  : QObject(_parent)
{
}

//-----------------------------------------------------------------------------
QWidget* qMRMLSliceWidgetPlugin::createWidget(QWidget* _parent)
{
  qMRMLSliceWidget* _widget = new qMRMLSliceWidget(_parent);
  return _widget;
}

//-----------------------------------------------------------------------------
QString qMRMLSliceWidgetPlugin::domXml() const
{
  return "<ui language=\"c++\">\n"
         "<widget class=\"qMRMLSliceWidget\" name=\"MRMLSliceViewWidget\">\n"
         "  <property name=\"geometry\">\n"
         "    <rect>\n"
         "      <x>0</x>\n"
         "      <y>0</y>\n"
         "      <width>200</width>\n"
         "      <height>200</height>\n"
         "    </rect>\n"
         "  </property>\n"
         "  <property name=\"sliceViewName\"> <string notr=\"true\"/> </property>\n"
         "  <property name=\"sliceOrientation\"> <string notr=\"true\"/> </property>\n"
         "</widget>\n"
         "</ui>\n";
}

//-----------------------------------------------------------------------------
QString qMRMLSliceWidgetPlugin::includeFile() const
{
  return "qMRMLSliceWidget.h";
}

//-----------------------------------------------------------------------------
bool qMRMLSliceWidgetPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qMRMLSliceWidgetPlugin::name() const
{
  return "qMRMLSliceWidget";
}

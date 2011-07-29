
// qMRML includes
#include "qMRMLThreeDViewPlugin.h"
#include "qMRMLThreeDView.h"

//-----------------------------------------------------------------------------
qMRMLThreeDViewPlugin::qMRMLThreeDViewPlugin(QObject *_parent):QObject(_parent)
{
}

//-----------------------------------------------------------------------------
QWidget *qMRMLThreeDViewPlugin::createWidget(QWidget *_parent)
{
  qMRMLThreeDView* _widget = new qMRMLThreeDView(_parent);
  return _widget;
}

//-----------------------------------------------------------------------------
QString qMRMLThreeDViewPlugin::domXml() const
{
  return "<widget class=\"qMRMLThreeDView\" \
          name=\"MRMLThreeDView\">\n"
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
QString qMRMLThreeDViewPlugin::includeFile() const
{
  return "qMRMLThreeDView.h";
}

//-----------------------------------------------------------------------------
bool qMRMLThreeDViewPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qMRMLThreeDViewPlugin::name() const
{
  return "qMRMLThreeDView";
}

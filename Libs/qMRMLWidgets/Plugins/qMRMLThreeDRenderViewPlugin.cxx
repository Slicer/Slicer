
// qMRML includes
#include "qMRMLThreeDRenderViewPlugin.h"
#include "qMRMLThreeDRenderView.h"

//-----------------------------------------------------------------------------
qMRMLThreeDRenderViewPlugin::qMRMLThreeDRenderViewPlugin(QObject *_parent):QObject(_parent)
{
}

//-----------------------------------------------------------------------------
QWidget *qMRMLThreeDRenderViewPlugin::createWidget(QWidget *_parent)
{
  qMRMLThreeDRenderView* _widget = new qMRMLThreeDRenderView(_parent);
  return _widget;
}

//-----------------------------------------------------------------------------
QString qMRMLThreeDRenderViewPlugin::domXml() const
{
  return "<widget class=\"qMRMLThreeDRenderView\" \
          name=\"MRMLThreeDRenderView\">\n"
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
QString qMRMLThreeDRenderViewPlugin::includeFile() const
{
  return "qMRMLThreeDRenderView.h";
}

//-----------------------------------------------------------------------------
bool qMRMLThreeDRenderViewPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qMRMLThreeDRenderViewPlugin::name() const
{
  return "qMRMLThreeDRenderView";
}

#include "qVTKRenderViewPlugin.h"
#include "qVTKRenderView.h"

qVTKRenderViewPlugin::qVTKRenderViewPlugin(QObject *_parent):QObject(_parent)
{
}

QWidget *qVTKRenderViewPlugin::createWidget(QWidget *_parent)
{
  qVTKRenderView* _widget = new qVTKRenderView(_parent);
  return _widget;
}

QString qVTKRenderViewPlugin::domXml() const
{
  return "<widget class=\"qVTKRenderView\" \
          name=\"VTKRenderView\">\n"
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

QString qVTKRenderViewPlugin::includeFile() const
{
  return "qVTKRenderView.h";
}

bool qVTKRenderViewPlugin::isContainer() const
{
  return false;
}

QString qVTKRenderViewPlugin::name() const
{
  return "qVTKRenderView";
}

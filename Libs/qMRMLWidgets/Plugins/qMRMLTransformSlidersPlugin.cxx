#include "qMRMLTransformSlidersPlugin.h"
#include "qMRMLTransformSliders.h"

qMRMLTransformSlidersPlugin::qMRMLTransformSlidersPlugin(QObject *_parent)
        : QObject(_parent)
{
}

QWidget *qMRMLTransformSlidersPlugin::createWidget(QWidget *_parent)
{
  qMRMLTransformSliders* _widget = new qMRMLTransformSliders(_parent);
  return _widget;
}

QString qMRMLTransformSlidersPlugin::domXml() const
{
  return "<widget class=\"qMRMLTransformSliders\" \
          name=\"MRMLTransformSliders\">\n"
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

QIcon qMRMLTransformSlidersPlugin::icon() const
{
  return QIcon(":/Icons/groupbox.png");
}

QString qMRMLTransformSlidersPlugin::includeFile() const
{
  return "qMRMLTransformSliders.h";
}

bool qMRMLTransformSlidersPlugin::isContainer() const
{
  return false;
}

QString qMRMLTransformSlidersPlugin::name() const
{
  return "qMRMLTransformSliders";
}

#include "qMRMLLinearTransformSliderPlugin.h"
#include "qMRMLLinearTransformSlider.h"

qMRMLLinearTransformSliderPlugin::qMRMLLinearTransformSliderPlugin(QObject *_parent)
        : QObject(_parent)
{
}

QWidget *qMRMLLinearTransformSliderPlugin::createWidget(QWidget *_parent)
{
  qMRMLLinearTransformSlider* _widget = new qMRMLLinearTransformSlider(_parent);
  return _widget;
}

QString qMRMLLinearTransformSliderPlugin::domXml() const
{
  return "<widget class=\"qMRMLLinearTransformSlider\" \
          name=\"MRMLLinearTransformSlider\">\n"
          " <property name=\"geometry\">\n"
          "  <rect>\n"
          "   <x>0</x>\n"
          "   <y>0</y>\n"
          "   <width>200</width>\n"
          "   <height>32</height>\n"
          "  </rect>\n"
          " </property>\n"
          "</widget>\n";
}

QIcon qMRMLLinearTransformSliderPlugin::icon() const
{
  return QIcon(":/Icons/sliderspinbox.png");
}

QString qMRMLLinearTransformSliderPlugin::includeFile() const
{
  return "qMRMLLinearTransformSlider.h";
}

bool qMRMLLinearTransformSliderPlugin::isContainer() const
{
  return false;
}

QString qMRMLLinearTransformSliderPlugin::name() const
{
  return "qMRMLLinearTransformSlider";
}

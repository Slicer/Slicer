#include "qCTKCollapsibleWidget2Plugin.h"
#include "qCTKCollapsibleWidget2.h"

qCTKCollapsibleWidget2Plugin::qCTKCollapsibleWidget2Plugin(QObject *_parent)
        : QObject(_parent)
{
}

QWidget *qCTKCollapsibleWidget2Plugin::createWidget(QWidget *_parent)
{
  qCTKCollapsibleWidget2* _widget = new qCTKCollapsibleWidget2(_parent);
  return _widget;
}


QString qCTKCollapsibleWidget2Plugin::domXml() const
{
  return "<widget class=\"qCTKCollapsibleWidget2\" \
          name=\"CTKCollapsibleWidget2\">\n"
          " <property name=\"geometry\">\n"
          "  <rect>\n"
          "   <x>0</x>\n"
          "   <y>0</y>\n"
          "   <width>300</width>\n"
          "   <height>100</height>\n"
          "  </rect>\n"
          " </property>\n"
          " <property name=\"title\">"
          "  <string>Widget2</string>"
          " </property>"
          "</widget>\n";
}

QString qCTKCollapsibleWidget2Plugin::includeFile() const
{
  return "qCTKCollapsibleWidget2.h";
}

bool qCTKCollapsibleWidget2Plugin::isContainer() const
{
  return true;
}

QString qCTKCollapsibleWidget2Plugin::name() const
{
  return "qCTKCollapsibleWidget2";
}

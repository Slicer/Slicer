#include "qCTKMenuButtonPlugin.h"
#include "qCTKMenuButton.h"

#include <QMenu>

qCTKMenuButtonPlugin::qCTKMenuButtonPlugin(QObject *_parent)
        : QObject(_parent)
{
}

QWidget *qCTKMenuButtonPlugin::createWidget(QWidget *_parent)
{
  qCTKMenuButton* _widget = new qCTKMenuButton(_parent);
  QMenu* dummyMenu = new QMenu(_widget);
  dummyMenu->addAction("foo");
  dummyMenu->addAction("bar");
  dummyMenu->addAction("barbarbarbarbarbarbarbarbarbar");
  _widget->setExtraMenu(dummyMenu);
  return _widget;
}

QString qCTKMenuButtonPlugin::domXml() const
{
  return "<widget class=\"qCTKMenuButton\" name=\"CTKMenuButton\">\n"
         " <property name=\"text\">"
         "  <string>MenuButton</string>"
         " </property>"
         "</widget>\n";
}

QString qCTKMenuButtonPlugin::includeFile() const
{
  return "qCTKMenuButton.h";
}

bool qCTKMenuButtonPlugin::isContainer() const
{
  return false;
}

QString qCTKMenuButtonPlugin::name() const
{
  return "qCTKMenuButton";
}

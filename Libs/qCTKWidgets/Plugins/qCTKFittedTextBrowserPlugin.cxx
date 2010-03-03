#include "qCTKFittedTextBrowserPlugin.h"
#include "qCTKFittedTextBrowser.h"

qCTKFittedTextBrowserPlugin::qCTKFittedTextBrowserPlugin(QObject *_parent)
        : QObject(_parent)
{

}

QWidget *qCTKFittedTextBrowserPlugin::createWidget(QWidget *_parent)
{
  qCTKFittedTextBrowser* _widget = new qCTKFittedTextBrowser(_parent);
  return _widget;
}

QString qCTKFittedTextBrowserPlugin::domXml() const
{
  return "<widget class=\"qCTKFittedTextBrowser\" \
          name=\"CTKFittedTextBrowser\">\n"
          "</widget>\n";
}

// --------------------------------------------------------------------------
QIcon qCTKFittedTextBrowserPlugin::icon() const
{
  return QIcon(":/Icons/textedit.png");
}

QString qCTKFittedTextBrowserPlugin::includeFile() const
{
  return "qCTKFittedTextBrowser.h";
}

bool qCTKFittedTextBrowserPlugin::isContainer() const
{
  return false;
}

QString qCTKFittedTextBrowserPlugin::name() const
{
  return "qCTKFittedTextBrowser";
}

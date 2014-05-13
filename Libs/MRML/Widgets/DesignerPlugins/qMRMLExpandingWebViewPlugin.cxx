#include "qMRMLExpandingWebViewPlugin.h"
#include "qMRMLExpandingWebView.h"

//------------------------------------------------------------------------------
qMRMLExpandingWebViewPlugin::qMRMLExpandingWebViewPlugin(QObject *_parent)
        : QObject(_parent)
{

}

//------------------------------------------------------------------------------
QWidget *qMRMLExpandingWebViewPlugin::createWidget(QWidget *_parent)
{
  qMRMLExpandingWebView* _widget = new qMRMLExpandingWebView(_parent);
  return _widget;
}

//------------------------------------------------------------------------------
QString qMRMLExpandingWebViewPlugin::domXml() const
{
  return "<widget class=\"qMRMLExpandingWebView\" \
          name=\"MRMLExpandingWebView\">\n"
          "</widget>\n";
}

//------------------------------------------------------------------------------
QString qMRMLExpandingWebViewPlugin::includeFile() const
{
  return "qMRMLExpandingWebView.h";
}

//------------------------------------------------------------------------------
bool qMRMLExpandingWebViewPlugin::isContainer() const
{
  return false;
}

//------------------------------------------------------------------------------
QString qMRMLExpandingWebViewPlugin::name() const
{
  return "qMRMLExpandingWebView";
}

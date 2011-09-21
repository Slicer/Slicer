/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#include "qMRMLClipNodeWidgetPlugin.h"
#include "qMRMLClipNodeWidget.h"

//------------------------------------------------------------------------------
qMRMLClipNodeWidgetPlugin::qMRMLClipNodeWidgetPlugin(QObject* pluginParent)
  : QObject(pluginParent)
{
}

//------------------------------------------------------------------------------
QWidget *qMRMLClipNodeWidgetPlugin::createWidget(QWidget* widgetParent)
{
  qMRMLClipNodeWidget* newWidget = new qMRMLClipNodeWidget(widgetParent);
  return newWidget;
}

//------------------------------------------------------------------------------
QString qMRMLClipNodeWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLClipNodeWidget\" \
          name=\"MRMLClipNodeWidget\">\n"
          "</widget>\n";
}

//------------------------------------------------------------------------------
QIcon qMRMLClipNodeWidgetPlugin::icon() const
{
  return QIcon(":/Icons/combobox.png");
}

//------------------------------------------------------------------------------
QString qMRMLClipNodeWidgetPlugin::includeFile() const
{
  return "qMRMLClipNodeWidget.h";
}

//------------------------------------------------------------------------------
bool qMRMLClipNodeWidgetPlugin::isContainer() const
{
  return false;
}

//------------------------------------------------------------------------------
QString qMRMLClipNodeWidgetPlugin::name() const
{
  return "qMRMLClipNodeWidget";
}

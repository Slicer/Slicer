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

#include "qMRMLClipNodeDisplayWidgetPlugin.h"
#include "qMRMLClipNodeDisplayWidget.h"

//------------------------------------------------------------------------------
qMRMLClipNodeDisplayWidgetPlugin::qMRMLClipNodeDisplayWidgetPlugin(QObject* pluginParent)
  : QObject(pluginParent)
{
}

//------------------------------------------------------------------------------
QWidget *qMRMLClipNodeDisplayWidgetPlugin::createWidget(QWidget* widgetParent)
{
  qMRMLClipNodeDisplayWidget* newWidget = new qMRMLClipNodeDisplayWidget(widgetParent);
  return newWidget;
}

//------------------------------------------------------------------------------
QString qMRMLClipNodeDisplayWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLClipNodeDisplayWidget\" \
          name=\"MRMLClipNodeDisplayWidget\">\n"
          "</widget>\n";
}

//------------------------------------------------------------------------------
QIcon qMRMLClipNodeDisplayWidgetPlugin::icon() const
{
  return QIcon(":/Icons/combobox.png");
}

//------------------------------------------------------------------------------
QString qMRMLClipNodeDisplayWidgetPlugin::includeFile() const
{
  return "qMRMLClipNodeDisplayWidget.h";
}

//------------------------------------------------------------------------------
bool qMRMLClipNodeDisplayWidgetPlugin::isContainer() const
{
  return false;
}

//------------------------------------------------------------------------------
QString qMRMLClipNodeDisplayWidgetPlugin::name() const
{
  return "qMRMLClipNodeDisplayWidget";
}

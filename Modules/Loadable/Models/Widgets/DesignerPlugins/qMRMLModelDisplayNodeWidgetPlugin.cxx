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

#include "qMRMLModelDisplayNodeWidgetPlugin.h"
#include "qMRMLModelDisplayNodeWidget.h"

//-----------------------------------------------------------------------------
qMRMLModelDisplayNodeWidgetPlugin::
  qMRMLModelDisplayNodeWidgetPlugin(QObject *newParent) : QObject(newParent)
{
}

//-----------------------------------------------------------------------------
QWidget *qMRMLModelDisplayNodeWidgetPlugin::createWidget(QWidget *newParent)
{
  qMRMLModelDisplayNodeWidget* newWidget =
    new qMRMLModelDisplayNodeWidget(newParent);
  return newWidget;
}

//-----------------------------------------------------------------------------
QString qMRMLModelDisplayNodeWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLModelDisplayNodeWidget\" \
          name=\"EMSegmentInputChannelListWidget\">\n"
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
QIcon qMRMLModelDisplayNodeWidgetPlugin::icon() const
{
  return QIcon(":/Icons/table.png");
}

//-----------------------------------------------------------------------------
QString qMRMLModelDisplayNodeWidgetPlugin::includeFile() const
{
  return "qMRMLModelDisplayNodeWidget.h";
}

//-----------------------------------------------------------------------------
bool qMRMLModelDisplayNodeWidgetPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qMRMLModelDisplayNodeWidgetPlugin::name() const
{
  return "qMRMLModelDisplayNodeWidget";
}

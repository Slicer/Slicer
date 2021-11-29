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

==============================================================================*/

#include "qMRMLColorLegendDisplayNodeWidgetPlugin.h"
#include "qMRMLColorLegendDisplayNodeWidget.h"

//-----------------------------------------------------------------------------
qMRMLColorLegendDisplayNodeWidgetPlugin::qMRMLColorLegendDisplayNodeWidgetPlugin(QObject *newParent)
  : QObject(newParent)
{
}

//-----------------------------------------------------------------------------
QString qMRMLColorLegendDisplayNodeWidgetPlugin::group() const
{
  return "Slicer [MRML Widgets]";
}

//-----------------------------------------------------------------------------
QString qMRMLColorLegendDisplayNodeWidgetPlugin::toolTip() const
{
  return QString();
}

//-----------------------------------------------------------------------------
QString qMRMLColorLegendDisplayNodeWidgetPlugin::whatsThis() const
{
  return QString();
}

//-----------------------------------------------------------------------------
QWidget *qMRMLColorLegendDisplayNodeWidgetPlugin::createWidget(QWidget *newParent)
{
  qMRMLColorLegendDisplayNodeWidget* newWidget =
    new qMRMLColorLegendDisplayNodeWidget(newParent);
  return newWidget;
}

//-----------------------------------------------------------------------------
QString qMRMLColorLegendDisplayNodeWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLColorLegendDisplayNodeWidget\" \
          name=\"ColorLegendDisplayNodeWidget\">\n"
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
QIcon qMRMLColorLegendDisplayNodeWidgetPlugin::icon() const
{
  return QIcon(":/Icons/Colors.png");
}

//-----------------------------------------------------------------------------
QString qMRMLColorLegendDisplayNodeWidgetPlugin::includeFile() const
{
  return "qMRMLColorLegendDisplayNodeWidget.h";
}

//-----------------------------------------------------------------------------
bool qMRMLColorLegendDisplayNodeWidgetPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qMRMLColorLegendDisplayNodeWidgetPlugin::name() const
{
  return "qMRMLColorLegendDisplayNodeWidget";
}

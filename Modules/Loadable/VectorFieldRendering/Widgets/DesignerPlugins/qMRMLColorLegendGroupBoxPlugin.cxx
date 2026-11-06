/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was developed with funding from the Children's Hospital of
  Philadelphia Frontier Program.

==============================================================================*/

#include "qMRMLColorLegendGroupBoxPlugin.h"
#include "qMRMLColorLegendGroupBox.h"

//-----------------------------------------------------------------------------
qMRMLColorLegendGroupBoxPlugin::qMRMLColorLegendGroupBoxPlugin(QObject* newParent)
  : QObject(newParent)
{
}

//-----------------------------------------------------------------------------
QWidget* qMRMLColorLegendGroupBoxPlugin::createWidget(QWidget* newParent)
{
  qMRMLColorLegendGroupBox* newWidget = new qMRMLColorLegendGroupBox(newParent);
  return newWidget;
}

//-----------------------------------------------------------------------------
QString qMRMLColorLegendGroupBoxPlugin::domXml() const
{
  return "<widget class=\"qMRMLColorLegendGroupBox\" name=\"ColorLegendGroupBox\">\n"
         " <property name=\"geometry\">\n"
         "  <rect>\n"
         "   <x>0</x>\n"
         "   <y>0</y>\n"
         "   <width>200</width>\n"
         "   <height>100</height>\n"
         "  </rect>\n"
         " </property>\n"
         "</widget>\n";
}

//-----------------------------------------------------------------------------
QIcon qMRMLColorLegendGroupBoxPlugin::icon() const
{
  return QIcon();
}

//-----------------------------------------------------------------------------
QString qMRMLColorLegendGroupBoxPlugin::includeFile() const
{
  return "qMRMLColorLegendGroupBox.h";
}

//-----------------------------------------------------------------------------
bool qMRMLColorLegendGroupBoxPlugin::isContainer() const
{
  return true;
}

//-----------------------------------------------------------------------------
QString qMRMLColorLegendGroupBoxPlugin::name() const
{
  return "qMRMLColorLegendGroupBox";
}

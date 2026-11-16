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

#include "qMRMLVectorFieldDisplayWidgetPlugin.h"
#include "qMRMLVectorFieldDisplayWidget.h"

//-----------------------------------------------------------------------------
qMRMLVectorFieldDisplayWidgetPlugin::qMRMLVectorFieldDisplayWidgetPlugin(QObject* newParent)
  : QObject(newParent)
{
}

//-----------------------------------------------------------------------------
QWidget* qMRMLVectorFieldDisplayWidgetPlugin::createWidget(QWidget* newParent)
{
  qMRMLVectorFieldDisplayWidget* newWidget = new qMRMLVectorFieldDisplayWidget(newParent);
  return newWidget;
}

//-----------------------------------------------------------------------------
QString qMRMLVectorFieldDisplayWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLVectorFieldDisplayWidget\" name=\"VectorFieldDisplayWidget\">\n"
         " <property name=\"geometry\">\n"
         "  <rect>\n"
         "   <x>0</x>\n"
         "   <y>0</y>\n"
         "   <width>300</width>\n"
         "   <height>600</height>\n"
         "  </rect>\n"
         " </property>\n"
         "</widget>\n";
}

//-----------------------------------------------------------------------------
QIcon qMRMLVectorFieldDisplayWidgetPlugin::icon() const
{
  return QIcon(":/Icons/VectorFieldGlyph.png");
}

//-----------------------------------------------------------------------------
QString qMRMLVectorFieldDisplayWidgetPlugin::includeFile() const
{
  return "qMRMLVectorFieldDisplayWidget.h";
}

//-----------------------------------------------------------------------------
bool qMRMLVectorFieldDisplayWidgetPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qMRMLVectorFieldDisplayWidgetPlugin::name() const
{
  return "qMRMLVectorFieldDisplayWidget";
}

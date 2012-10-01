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

#include "qMRMLCheckableNodeComboBoxPlugin.h"
#include "qMRMLCheckableNodeComboBox.h"

//-----------------------------------------------------------------------------
qMRMLCheckableNodeComboBoxPlugin
::qMRMLCheckableNodeComboBoxPlugin(QObject* pluginParent)
  : QObject(pluginParent)
{
}

//-----------------------------------------------------------------------------
QWidget *qMRMLCheckableNodeComboBoxPlugin::createWidget(QWidget *parentWidget)
{
  qMRMLCheckableNodeComboBox* widget =
    new qMRMLCheckableNodeComboBox(parentWidget);
  return widget;
}

//-----------------------------------------------------------------------------
QString qMRMLCheckableNodeComboBoxPlugin::domXml() const
{
  return "<widget class=\"qMRMLCheckableNodeComboBox\" \
          name=\"CheckableNodeComboBox\">\n"
          " <property name=\"geometry\">\n"
          "  <rect>\n"
          "   <x>0</x>\n"
          "   <y>0</y>\n"
          "   <width>200</width>\n"
          "   <height>20</height>\n"
          "  </rect>\n"
          " </property>\n"
          "</widget>\n";
}

//-----------------------------------------------------------------------------
QIcon qMRMLCheckableNodeComboBoxPlugin::icon() const
{
  return QIcon(":/Icons/combobox.png");
}

//-----------------------------------------------------------------------------
QString qMRMLCheckableNodeComboBoxPlugin::includeFile() const
{
  return "qMRMLCheckableNodeComboBox.h";
}

//-----------------------------------------------------------------------------
bool qMRMLCheckableNodeComboBoxPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qMRMLCheckableNodeComboBoxPlugin::name() const
{
  return "qMRMLCheckableNodeComboBox";
}

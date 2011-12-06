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

#include "qMRMLColorTableComboBoxPlugin.h"
#include "qMRMLColorTableComboBox.h"

//-----------------------------------------------------------------------------
qMRMLColorTableComboBoxPlugin
::qMRMLColorTableComboBoxPlugin(QObject *parentObject)
  : QObject(parentObject)
{
}

//-----------------------------------------------------------------------------
QWidget *qMRMLColorTableComboBoxPlugin::createWidget(QWidget *parentWidget)
{
  qMRMLColorTableComboBox* widget = new qMRMLColorTableComboBox(parentWidget);
  return widget;
}

//-----------------------------------------------------------------------------
QString qMRMLColorTableComboBoxPlugin::domXml() const
{
  return "<widget class=\"qMRMLColorTableComboBox\" \
          name=\"ColorTableComboBox\">\n"
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
QIcon qMRMLColorTableComboBoxPlugin::icon() const
{
  return QIcon(":/Icons/combobox.png");
}

//-----------------------------------------------------------------------------
QString qMRMLColorTableComboBoxPlugin::includeFile() const
{
  return "qMRMLColorTableComboBox.h";
}

//-----------------------------------------------------------------------------
bool qMRMLColorTableComboBoxPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qMRMLColorTableComboBoxPlugin::name() const
{
  return "qMRMLColorTableComboBox";
}

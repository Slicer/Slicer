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

#include "qSlicerPresetComboBoxPlugin.h"
#include "qSlicerPresetComboBox.h"

//-----------------------------------------------------------------------------
qSlicerPresetComboBoxPlugin::qSlicerPresetComboBoxPlugin(QObject *objectParent)
  : QObject(objectParent)
{
}

//-----------------------------------------------------------------------------
QWidget *qSlicerPresetComboBoxPlugin::createWidget(QWidget *widgetParent)
{
  qSlicerPresetComboBox* newWidget = new qSlicerPresetComboBox(widgetParent);
  return newWidget;
}

//-----------------------------------------------------------------------------
QString qSlicerPresetComboBoxPlugin::domXml() const
{
  return "<widget class=\"qSlicerPresetComboBox\" \
          name=\"PresetComboBox\">\n"
          "</widget>\n";
}

//-----------------------------------------------------------------------------
QString qSlicerPresetComboBoxPlugin::includeFile() const
{
  return "qSlicerPresetComboBox.h";
}

//-----------------------------------------------------------------------------
bool qSlicerPresetComboBoxPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qSlicerPresetComboBoxPlugin::name() const
{
  return "qSlicerPresetComboBox";
}

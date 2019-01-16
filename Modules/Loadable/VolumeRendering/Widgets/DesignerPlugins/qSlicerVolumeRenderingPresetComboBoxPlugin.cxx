/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, and Cancer
  Care Ontario.

==============================================================================*/

#include "qSlicerVolumeRenderingPresetComboBoxPlugin.h"
#include "qSlicerVolumeRenderingPresetComboBox.h"

//-----------------------------------------------------------------------------
qSlicerVolumeRenderingPresetComboBoxPlugin::qSlicerVolumeRenderingPresetComboBoxPlugin(QObject *objectParent)
  : QObject(objectParent)
{
}

//-----------------------------------------------------------------------------
QWidget *qSlicerVolumeRenderingPresetComboBoxPlugin::createWidget(QWidget *widgetParent)
{
  qSlicerVolumeRenderingPresetComboBox* newWidget = new qSlicerVolumeRenderingPresetComboBox(widgetParent);
  return newWidget;
}

//-----------------------------------------------------------------------------
QString qSlicerVolumeRenderingPresetComboBoxPlugin::domXml() const
{
  return "<widget class=\"qSlicerVolumeRenderingPresetComboBox\" \
          name=\"VolumeRenderingPresetComboBox\">\n"
          "</widget>\n";
}

//-----------------------------------------------------------------------------
QString qSlicerVolumeRenderingPresetComboBoxPlugin::includeFile() const
{
  return "qSlicerVolumeRenderingPresetComboBox.h";
}

//-----------------------------------------------------------------------------
bool qSlicerVolumeRenderingPresetComboBoxPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qSlicerVolumeRenderingPresetComboBoxPlugin::name() const
{
  return "qSlicerVolumeRenderingPresetComboBox";
}

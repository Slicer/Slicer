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

#include "qSlicerGPUMemoryComboBoxPlugin.h"
#include "qSlicerGPUMemoryComboBox.h"

//-----------------------------------------------------------------------------
qSlicerGPUMemoryComboBoxPlugin::qSlicerGPUMemoryComboBoxPlugin(QObject *objectParent)
  : QObject(objectParent)
{
}

//-----------------------------------------------------------------------------
QWidget *qSlicerGPUMemoryComboBoxPlugin::createWidget(QWidget *widgetParent)
{
  qSlicerGPUMemoryComboBox* newWidget = new qSlicerGPUMemoryComboBox(widgetParent);
  return newWidget;
}

//-----------------------------------------------------------------------------
QString qSlicerGPUMemoryComboBoxPlugin::domXml() const
{
  return "<widget class=\"qSlicerGPUMemoryComboBox\" \
          name=\"GPUMemoryComboBox\">\n"
          "</widget>\n";
}

//-----------------------------------------------------------------------------
QString qSlicerGPUMemoryComboBoxPlugin::includeFile() const
{
  return "qSlicerGPUMemoryComboBox.h";
}

//-----------------------------------------------------------------------------
bool qSlicerGPUMemoryComboBoxPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qSlicerGPUMemoryComboBoxPlugin::name() const
{
  return "qSlicerGPUMemoryComboBox";
}

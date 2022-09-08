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

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#include "qMRMLSubjectHierarchyComboBoxPlugin.h"
#include "qMRMLSubjectHierarchyComboBox.h"

//-----------------------------------------------------------------------------
qMRMLSubjectHierarchyComboBoxPlugin::qMRMLSubjectHierarchyComboBoxPlugin(QObject* pluginParent)
  : QObject(pluginParent)
{
}

//-----------------------------------------------------------------------------
QWidget *qMRMLSubjectHierarchyComboBoxPlugin::createWidget(QWidget* parentWidget)
{
  qMRMLSubjectHierarchyComboBox* pluginWidget =
    new qMRMLSubjectHierarchyComboBox(parentWidget);
  return pluginWidget;
}

//-----------------------------------------------------------------------------
QString qMRMLSubjectHierarchyComboBoxPlugin::domXml() const
{
  return  "<ui language=\"c++\">\n"
    "<widget class=\"qMRMLSubjectHierarchyComboBox\" name=\"SubjectHierarchyComboBox\">\n"
    "  <property name=\"includeItemAttributeNamesFilter\"> <stringlist notr=\"true\"/> </property>\n"
    "  <property name=\"includeNodeAttributeNamesFilter\"> <stringlist notr=\"true\"/> </property>\n"
    "  <property name=\"excludeItemAttributeNamesFilter\"> <stringlist notr=\"true\"/> </property>\n"
    "  <property name=\"excludeNodeAttributeNamesFilter\"> <stringlist notr=\"true\"/> </property>\n"
    "</widget>\n"
    "</ui>\n";
}

//-----------------------------------------------------------------------------
QString qMRMLSubjectHierarchyComboBoxPlugin::includeFile() const
{
  return "qMRMLSubjectHierarchyComboBox.h";
}

//-----------------------------------------------------------------------------
bool qMRMLSubjectHierarchyComboBoxPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qMRMLSubjectHierarchyComboBoxPlugin::name() const
{
  return "qMRMLSubjectHierarchyComboBox";
}

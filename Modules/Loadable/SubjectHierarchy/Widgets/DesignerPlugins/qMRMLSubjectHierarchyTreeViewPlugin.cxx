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

#include "qMRMLSubjectHierarchyTreeViewPlugin.h"
#include "qMRMLSubjectHierarchyTreeView.h"

//-----------------------------------------------------------------------------
qMRMLSubjectHierarchyTreeViewPlugin::qMRMLSubjectHierarchyTreeViewPlugin(QObject* pluginParent)
  : QObject(pluginParent)
{
}

//-----------------------------------------------------------------------------
QWidget *qMRMLSubjectHierarchyTreeViewPlugin::createWidget(QWidget* parentWidget)
{
  qMRMLSubjectHierarchyTreeView* pluginWidget =
    new qMRMLSubjectHierarchyTreeView(parentWidget);
  return pluginWidget;
}

//-----------------------------------------------------------------------------
QString qMRMLSubjectHierarchyTreeViewPlugin::domXml() const
{
  return "<widget class=\"qMRMLSubjectHierarchyTreeView\" \
          name=\"SubjectHierarchyTreeView\">\n"
          "</widget>\n";
}

//-----------------------------------------------------------------------------
QString qMRMLSubjectHierarchyTreeViewPlugin::includeFile() const
{
  return "qMRMLSubjectHierarchyTreeView.h";
}

//-----------------------------------------------------------------------------
bool qMRMLSubjectHierarchyTreeViewPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qMRMLSubjectHierarchyTreeViewPlugin::name() const
{
  return "qMRMLSubjectHierarchyTreeView";
}

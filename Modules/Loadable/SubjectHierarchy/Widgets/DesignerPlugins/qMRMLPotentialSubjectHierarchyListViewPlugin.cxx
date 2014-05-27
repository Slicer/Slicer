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

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#include "qMRMLPotentialSubjectHierarchyListViewPlugin.h"
#include "qMRMLPotentialSubjectHierarchyListView.h"

//-----------------------------------------------------------------------------
qMRMLPotentialSubjectHierarchyListViewPlugin::qMRMLPotentialSubjectHierarchyListViewPlugin(QObject* pluginParent)
  : QObject(pluginParent)
{
}

//-----------------------------------------------------------------------------
QWidget *qMRMLPotentialSubjectHierarchyListViewPlugin::createWidget(QWidget* parentWidget)
{
  qMRMLPotentialSubjectHierarchyListView* pluginWidget =
    new qMRMLPotentialSubjectHierarchyListView(parentWidget);
  return pluginWidget;
}

//-----------------------------------------------------------------------------
QString qMRMLPotentialSubjectHierarchyListViewPlugin::domXml() const
{
  return "<widget class=\"qMRMLPotentialSubjectHierarchyListView\" \
          name=\"PotentialSubjectHierarchyListView\">\n"
          "</widget>\n";
}

//-----------------------------------------------------------------------------
QString qMRMLPotentialSubjectHierarchyListViewPlugin::includeFile() const
{
  return "qMRMLPotentialSubjectHierarchyListView.h";
}

//-----------------------------------------------------------------------------
bool qMRMLPotentialSubjectHierarchyListViewPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qMRMLPotentialSubjectHierarchyListViewPlugin::name() const
{
  return "qMRMLPotentialSubjectHierarchyListView";
}

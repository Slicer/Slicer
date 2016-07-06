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
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#include "qMRMLSegmentationRepresentationsListViewPlugin.h"
#include "qMRMLSegmentationRepresentationsListView.h"

//-----------------------------------------------------------------------------
qMRMLSegmentationRepresentationsListViewPlugin::qMRMLSegmentationRepresentationsListViewPlugin(QObject* pluginParent)
  : QObject(pluginParent)
{
}

//-----------------------------------------------------------------------------
QWidget *qMRMLSegmentationRepresentationsListViewPlugin::createWidget(QWidget* parentWidget)
{
  qMRMLSegmentationRepresentationsListView* pluginWidget =
    new qMRMLSegmentationRepresentationsListView(parentWidget);
  return pluginWidget;
}

//-----------------------------------------------------------------------------
QString qMRMLSegmentationRepresentationsListViewPlugin::domXml() const
{
  return "<widget class=\"qMRMLSegmentationRepresentationsListView\" \
          name=\"SegmentationRepresentationsListView\">\n"
          "</widget>\n";
}

//-----------------------------------------------------------------------------
QString qMRMLSegmentationRepresentationsListViewPlugin::includeFile() const
{
  return "qMRMLSegmentationRepresentationsListView.h";
}

//-----------------------------------------------------------------------------
bool qMRMLSegmentationRepresentationsListViewPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qMRMLSegmentationRepresentationsListViewPlugin::name() const
{
  return "qMRMLSegmentationRepresentationsListView";
}

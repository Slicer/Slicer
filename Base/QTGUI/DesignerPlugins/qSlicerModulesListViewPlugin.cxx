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

#include "qSlicerModulesListViewPlugin.h"
#include "qSlicerModulesListView.h"

qSlicerModulesListViewPlugin::qSlicerModulesListViewPlugin(QObject* parent)
  : QObject(parent)
{
}

QWidget *qSlicerModulesListViewPlugin::createWidget(QWidget* parentWidget)
{
  qSlicerModulesListView* widget = new qSlicerModulesListView(parentWidget);
  return widget;
}

QString qSlicerModulesListViewPlugin::domXml() const
{
  return "<widget class=\"qSlicerModulesListView\" \
          name=\"SlicerModulesListView\">\n"
          "</widget>\n";
}

QString qSlicerModulesListViewPlugin::includeFile() const
{
  return "qSlicerModulesListView.h";
}

bool qSlicerModulesListViewPlugin::isContainer() const
{
  return false;
}

QString qSlicerModulesListViewPlugin::name() const
{
  return "qSlicerModulesListView";
}

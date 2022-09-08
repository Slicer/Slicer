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

// qMRML includes
#include "qMRMLTreeViewPlugin.h"
#include "qMRMLTreeView.h"

// --------------------------------------------------------------------------
qMRMLTreeViewPlugin::qMRMLTreeViewPlugin(QObject *_parent):QObject(_parent)
{
}

// --------------------------------------------------------------------------
QWidget *qMRMLTreeViewPlugin::createWidget(QWidget *_parent)
{
  qMRMLTreeView* _widget = new qMRMLTreeView(_parent);
  return _widget;
}

// --------------------------------------------------------------------------
QString qMRMLTreeViewPlugin::domXml() const
{
  return  "<ui language=\"c++\">\n"
    "<widget class=\"qMRMLTreeView\" name=\"MRMLTreeView\">\n"
    "  <property name=\"nodeTypes\"> <stringlist notr=\"true\"/> </property>\n"
    "  <property name=\"sceneModelType\"> <string notr=\"true\"/> </property>\n"
    "</widget>\n"
    "</ui>\n";
}

// --------------------------------------------------------------------------
QIcon qMRMLTreeViewPlugin::icon() const
{
  return QIcon(":/Icons/listview.png");
}

// --------------------------------------------------------------------------
QString qMRMLTreeViewPlugin::includeFile() const
{
  return "qMRMLTreeView.h";
}

// --------------------------------------------------------------------------
bool qMRMLTreeViewPlugin::isContainer() const
{
  return false;
}

// --------------------------------------------------------------------------
QString qMRMLTreeViewPlugin::name() const
{
  return "qMRMLTreeView";
}

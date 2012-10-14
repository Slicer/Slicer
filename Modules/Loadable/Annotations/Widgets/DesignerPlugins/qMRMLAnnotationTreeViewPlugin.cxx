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

#include "qMRMLAnnotationTreeViewPlugin.h"
#include "qMRMLAnnotationTreeView.h"

//-----------------------------------------------------------------------------
qMRMLAnnotationTreeViewPlugin::qMRMLAnnotationTreeViewPlugin(QObject* pluginParent)
  : QObject(pluginParent)
{
}

//-----------------------------------------------------------------------------
QWidget *qMRMLAnnotationTreeViewPlugin::createWidget(QWidget* parentWidget)
{
  qMRMLAnnotationTreeView* pluginWidget
    = new qMRMLAnnotationTreeView(parentWidget);
  return pluginWidget;
}

//-----------------------------------------------------------------------------
QString qMRMLAnnotationTreeViewPlugin::domXml() const
{
  return "<widget class=\"qMRMLAnnotationTreeView\" \
          name=\"AnnotationTreeView\">\n"
          "</widget>\n";
}

//-----------------------------------------------------------------------------
QString qMRMLAnnotationTreeViewPlugin::includeFile() const
{
  return "qMRMLAnnotationTreeView.h";
}

//-----------------------------------------------------------------------------
bool qMRMLAnnotationTreeViewPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qMRMLAnnotationTreeViewPlugin::name() const
{
  return "qMRMLAnnotationTreeView";
}

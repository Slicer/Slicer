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

#include "qMRMLVolumePropertyNodeWidgetPlugin.h"
#include "qMRMLVolumePropertyNodeWidget.h"

//------------------------------------------------------------------------------
qMRMLVolumePropertyNodeWidgetPlugin
::qMRMLVolumePropertyNodeWidgetPlugin(QObject *parentObject)
  : QObject(parentObject)
{
}

//------------------------------------------------------------------------------
QWidget *qMRMLVolumePropertyNodeWidgetPlugin::createWidget(QWidget *parentWidget)
{
  qMRMLVolumePropertyNodeWidget* newWidget =
    new qMRMLVolumePropertyNodeWidget(parentWidget);
  return newWidget;
}

//------------------------------------------------------------------------------
QString qMRMLVolumePropertyNodeWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLVolumePropertyNodeWidget\" \
          name=\"MRMLVolumePropertyNodeWidget\">\n"
          "</widget>\n";
}

//------------------------------------------------------------------------------
QIcon qMRMLVolumePropertyNodeWidgetPlugin::icon() const
{
  return QIcon();
}

//------------------------------------------------------------------------------
QString qMRMLVolumePropertyNodeWidgetPlugin::includeFile() const
{
  return "qMRMLVolumePropertyNodeWidget.h";
}

//------------------------------------------------------------------------------
bool qMRMLVolumePropertyNodeWidgetPlugin::isContainer() const
{
  return false;
}

//------------------------------------------------------------------------------
QString qMRMLVolumePropertyNodeWidgetPlugin::name() const
{
  return "qMRMLVolumePropertyNodeWidget";
}

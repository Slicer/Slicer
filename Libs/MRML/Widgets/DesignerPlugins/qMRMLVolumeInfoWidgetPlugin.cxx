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

#include "qMRMLVolumeInfoWidgetPlugin.h"
#include "qMRMLVolumeInfoWidget.h"

//------------------------------------------------------------------------------
qMRMLVolumeInfoWidgetPlugin::qMRMLVolumeInfoWidgetPlugin(QObject *parentObject)
  : QObject(parentObject)
{
}

//------------------------------------------------------------------------------
QWidget *qMRMLVolumeInfoWidgetPlugin::createWidget(QWidget *parentWidget)
{
  qMRMLVolumeInfoWidget* newWidget = new qMRMLVolumeInfoWidget(parentWidget);
  return newWidget;
}

//------------------------------------------------------------------------------
QString qMRMLVolumeInfoWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLVolumeInfoWidget\" \
          name=\"MRMLVolumeInfoWidget\">\n"
          "</widget>\n";
}

//------------------------------------------------------------------------------
QIcon qMRMLVolumeInfoWidgetPlugin::icon() const
{
  return QIcon(":/Icons/combobox.png");
}

//------------------------------------------------------------------------------
QString qMRMLVolumeInfoWidgetPlugin::includeFile() const
{
  return "qMRMLVolumeInfoWidget.h";
}

//------------------------------------------------------------------------------
bool qMRMLVolumeInfoWidgetPlugin::isContainer() const
{
  return false;
}

//------------------------------------------------------------------------------
QString qMRMLVolumeInfoWidgetPlugin::name() const
{
  return "qMRMLVolumeInfoWidget";
}

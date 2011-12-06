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

#include "qMRMLModelInfoWidgetPlugin.h"
#include "qMRMLModelInfoWidget.h"

//------------------------------------------------------------------------------
qMRMLModelInfoWidgetPlugin::qMRMLModelInfoWidgetPlugin(QObject *_parent)
  : QObject(_parent)
{
}

//------------------------------------------------------------------------------
QWidget *qMRMLModelInfoWidgetPlugin::createWidget(QWidget *_parent)
{
  qMRMLModelInfoWidget* _widget = new qMRMLModelInfoWidget(_parent);
  return _widget;
}

//------------------------------------------------------------------------------
QString qMRMLModelInfoWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLModelInfoWidget\" \
          name=\"MRMLModelInfoWidget\">\n"
          "</widget>\n";
}

//------------------------------------------------------------------------------
QIcon qMRMLModelInfoWidgetPlugin::icon() const
{
  return QIcon(":/Icons/combobox.png");
}

//------------------------------------------------------------------------------
QString qMRMLModelInfoWidgetPlugin::includeFile() const
{
  return "qMRMLModelInfoWidget.h";
}

//------------------------------------------------------------------------------
bool qMRMLModelInfoWidgetPlugin::isContainer() const
{
  return false;
}

//------------------------------------------------------------------------------
QString qMRMLModelInfoWidgetPlugin::name() const
{
  return "qMRMLModelInfoWidget";
}

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

  This file was originally developed by Johan Andruejol, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#include "qMRMLCoordinatesWidgetPlugin.h"
#include "qMRMLCoordinatesWidget.h"

// --------------------------------------------------------------------------
qMRMLCoordinatesWidgetPlugin::qMRMLCoordinatesWidgetPlugin(QObject *_parent)
: QObject(_parent)
{
}

// --------------------------------------------------------------------------
QWidget *qMRMLCoordinatesWidgetPlugin::createWidget(QWidget *_parent)
{
  qMRMLCoordinatesWidget* _widget = new qMRMLCoordinatesWidget(_parent);
  return _widget;
}

// --------------------------------------------------------------------------
QString qMRMLCoordinatesWidgetPlugin::domXml() const
{
  return  "<ui language=\"c++\">\n"
    "<widget class=\"qMRMLCoordinatesWidget\" name=\"MRMLCoordinatesWidget\">\n"
    "  <property name=\"quantity\"> <string notr=\"true\"/> </property>\n"
    "</widget>\n"
    "</ui>\n";
}

// --------------------------------------------------------------------------
QIcon qMRMLCoordinatesWidgetPlugin::icon() const
{
  return QIcon(":/Icons/spinbox.png");
}

// --------------------------------------------------------------------------
QString qMRMLCoordinatesWidgetPlugin::includeFile() const
{
  return "qMRMLCoordinatesWidget.h";
}

// --------------------------------------------------------------------------
bool qMRMLCoordinatesWidgetPlugin::isContainer() const
{
  return false;
}

// --------------------------------------------------------------------------
QString qMRMLCoordinatesWidgetPlugin::name() const
{
  return "qMRMLCoordinatesWidget";
}

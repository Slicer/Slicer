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

#include "qMRMLUnitWidgetPlugin.h"
#include "qMRMLUnitWidget.h"

//------------------------------------------------------------------------------
qMRMLUnitWidgetPlugin::qMRMLUnitWidgetPlugin(QObject *_parent)
  : QObject(_parent)
{
}

//------------------------------------------------------------------------------
QWidget *qMRMLUnitWidgetPlugin::createWidget(QWidget *_parent)
{
  qMRMLUnitWidget* _widget = new qMRMLUnitWidget(_parent);
  return _widget;
}

//------------------------------------------------------------------------------
QString qMRMLUnitWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLUnitWidget\" \
          name=\"MRMLUnitWidget\">\n"
          "</widget>\n";
}

//------------------------------------------------------------------------------
QString qMRMLUnitWidgetPlugin::includeFile() const
{
  return "qMRMLUnitWidget.h";
}

//------------------------------------------------------------------------------
bool qMRMLUnitWidgetPlugin::isContainer() const
{
  return false;
}

//------------------------------------------------------------------------------
QString qMRMLUnitWidgetPlugin::name() const
{
  return "qMRMLUnitWidget";
}

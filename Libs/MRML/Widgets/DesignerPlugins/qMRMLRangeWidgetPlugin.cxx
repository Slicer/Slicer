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

#include "qMRMLRangeWidgetPlugin.h"
#include "qMRMLRangeWidget.h"

qMRMLRangeWidgetPlugin::qMRMLRangeWidgetPlugin(QObject *parentWidget)
        : QObject(parentWidget)
{
}

QWidget *qMRMLRangeWidgetPlugin::createWidget(QWidget *parentWidget)
{
  qMRMLRangeWidget* newWidget = new qMRMLRangeWidget(parentWidget);
  return newWidget;
}

QString qMRMLRangeWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLRangeWidget\" \
          name=\"MRMLRangeWidget\">\n"
          "</widget>\n";
}

QIcon qMRMLRangeWidgetPlugin::icon() const
{
  return QIcon(":/Icons/sliderspinbox.png");
}

QString qMRMLRangeWidgetPlugin::includeFile() const
{
  return "qMRMLRangeWidget.h";
}

bool qMRMLRangeWidgetPlugin::isContainer() const
{
  return false;
}

QString qMRMLRangeWidgetPlugin::name() const
{
  return "qMRMLRangeWidget";
}

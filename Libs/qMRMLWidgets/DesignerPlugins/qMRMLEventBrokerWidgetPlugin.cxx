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

#include "qMRMLEventBrokerWidgetPlugin.h"
#include "qMRMLEventBrokerWidget.h"

qMRMLEventBrokerWidgetPlugin::qMRMLEventBrokerWidgetPlugin(QObject *parentWidget)
  : QObject(parentWidget)
{
}

QWidget *qMRMLEventBrokerWidgetPlugin::createWidget(QWidget *parentWidget)
{
  qMRMLEventBrokerWidget* newWidget =
    new qMRMLEventBrokerWidget(parentWidget);
  return newWidget;
}

QString qMRMLEventBrokerWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLEventBrokerWidget\" \
          name=\"EventBrokerWidget\">\n"
          " <property name=\"geometry\">\n"
          "  <rect>\n"
          "   <x>0</x>\n"
          "   <y>0</y>\n"
          "   <width>200</width>\n"
          "   <height>200</height>\n"
          "  </rect>\n"
          " </property>\n"
          "</widget>\n";
}

QIcon qMRMLEventBrokerWidgetPlugin::icon() const
{
  return QIcon(":/Icons/table.png");
}

QString qMRMLEventBrokerWidgetPlugin::includeFile() const
{
  return "qMRMLEventBrokerWidget.h";
}

bool qMRMLEventBrokerWidgetPlugin::isContainer() const
{
  return false;
}

QString qMRMLEventBrokerWidgetPlugin::name() const
{
  return "qMRMLEventBrokerWidget";
}

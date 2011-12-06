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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#include "qMRMLMatrixWidgetPlugin.h"
#include "qMRMLMatrixWidget.h"

qMRMLMatrixWidgetPlugin::qMRMLMatrixWidgetPlugin(QObject *_parent)
        : QObject(_parent)
{
}

QWidget *qMRMLMatrixWidgetPlugin::createWidget(QWidget *_parent)
{
  qMRMLMatrixWidget* _widget = new qMRMLMatrixWidget(_parent);
  return _widget;
}

QString qMRMLMatrixWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLMatrixWidget\" \
          name=\"MRMLMatrixWidget\">\n"
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

QIcon qMRMLMatrixWidgetPlugin::icon() const
{
  return QIcon(":/Icons/table.png");
}

QString qMRMLMatrixWidgetPlugin::includeFile() const
{
  return "qMRMLMatrixWidget.h";
}

bool qMRMLMatrixWidgetPlugin::isContainer() const
{
  return false;
}

QString qMRMLMatrixWidgetPlugin::name() const
{
  return "qMRMLMatrixWidget";
}

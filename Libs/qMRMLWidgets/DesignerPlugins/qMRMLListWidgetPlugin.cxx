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

#include "qMRMLListWidgetPlugin.h"
#include "qMRMLListWidget.h"

qMRMLListWidgetPlugin::qMRMLListWidgetPlugin(QObject *_parent)
        : QObject(_parent)
{
}

QWidget *qMRMLListWidgetPlugin::createWidget(QWidget *_parent)
{
  qMRMLListWidget* _widget = new qMRMLListWidget(_parent);
  return _widget;
}

QString qMRMLListWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLListWidget\" \
          name=\"MRMLListWidget\">\n"
          "</widget>\n";
}

QIcon qMRMLListWidgetPlugin::icon() const
{
  return QIcon(":Icons/listbox.png");
}

QString qMRMLListWidgetPlugin::includeFile() const
{
  return "qMRMLListWidget.h";
}

bool qMRMLListWidgetPlugin::isContainer() const
{
  return false;
}

QString qMRMLListWidgetPlugin::name() const
{
  return "qMRMLListWidget";
}

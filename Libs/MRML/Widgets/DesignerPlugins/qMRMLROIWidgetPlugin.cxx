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

#include "qMRMLROIWidgetPlugin.h"
#include "qMRMLROIWidget.h"

qMRMLROIWidgetPlugin::qMRMLROIWidgetPlugin(QObject *_parent)
        : QObject(_parent)
{
}

QWidget *qMRMLROIWidgetPlugin::createWidget(QWidget *_parent)
{
  qMRMLROIWidget* _widget = new qMRMLROIWidget(_parent);
  return _widget;
}

QString qMRMLROIWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLROIWidget\" \
          name=\"MRMLROIWidget\">\n"
          "</widget>\n";
}

QIcon qMRMLROIWidgetPlugin::icon() const
{
  return QIcon(":/Icons/sliderspinbox.png");
}

QString qMRMLROIWidgetPlugin::includeFile() const
{
  return "qMRMLROIWidget.h";
}

bool qMRMLROIWidgetPlugin::isContainer() const
{
  return false;
}

QString qMRMLROIWidgetPlugin::name() const
{
  return "qMRMLROIWidget";
}

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

#include "qMRMLDisplayNodeWidgetPlugin.h"
#include "qMRMLDisplayNodeWidget.h"

//------------------------------------------------------------------------------
qMRMLDisplayNodeWidgetPlugin::qMRMLDisplayNodeWidgetPlugin(QObject *_parent)
        : QObject(_parent)
{
}

//------------------------------------------------------------------------------
QWidget *qMRMLDisplayNodeWidgetPlugin::createWidget(QWidget *_parent)
{
  qMRMLDisplayNodeWidget* _widget = new qMRMLDisplayNodeWidget(_parent);
  return _widget;
}

//------------------------------------------------------------------------------
QString qMRMLDisplayNodeWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLDisplayNodeWidget\" \
          name=\"MRMLDisplayNodeWidget\">\n"
          "</widget>\n";
}

//------------------------------------------------------------------------------
QIcon qMRMLDisplayNodeWidgetPlugin::icon() const
{
  return QIcon(":/Icons/combobox.png");
}

//------------------------------------------------------------------------------
QString qMRMLDisplayNodeWidgetPlugin::includeFile() const
{
  return "qMRMLDisplayNodeWidget.h";
}

//------------------------------------------------------------------------------
bool qMRMLDisplayNodeWidgetPlugin::isContainer() const
{
  return false;
}

//------------------------------------------------------------------------------
QString qMRMLDisplayNodeWidgetPlugin::name() const
{
  return "qMRMLDisplayNodeWidget";
}

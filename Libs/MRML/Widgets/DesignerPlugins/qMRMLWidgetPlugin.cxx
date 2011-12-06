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

// qMRML includes
#include "qMRMLWidgetPlugin.h"
#include "qMRMLWidget.h"

// --------------------------------------------------------------------------
qMRMLWidgetPlugin::qMRMLWidgetPlugin(QObject *_parent):QObject(_parent)
{
}

// --------------------------------------------------------------------------
QWidget *qMRMLWidgetPlugin::createWidget(QWidget *_parent)
{
  qMRMLWidget* _widget = new qMRMLWidget(_parent);
  return _widget;
}

// --------------------------------------------------------------------------
QString qMRMLWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLWidget\" \
          name=\"MRMLWidget\">\n"
          "</widget>\n";
}

// --------------------------------------------------------------------------
QIcon qMRMLWidgetPlugin::icon() const
{
  return QIcon(":/Icons/widget.png");
}

// --------------------------------------------------------------------------
QString qMRMLWidgetPlugin::includeFile() const
{
  return "qMRMLWidget.h";
}

// --------------------------------------------------------------------------
bool qMRMLWidgetPlugin::isContainer() const
{
  return true;
}

// --------------------------------------------------------------------------
QString qMRMLWidgetPlugin::name() const
{
  return "qMRMLWidget";
}

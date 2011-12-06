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

// qMRML includes
#include "qMRMLNavigationViewPlugin.h"
#include "qMRMLNavigationView.h"

//-----------------------------------------------------------------------------
qMRMLNavigationViewPlugin::qMRMLNavigationViewPlugin(QObject *_parent):QObject(_parent)
{
}

//-----------------------------------------------------------------------------
QWidget *qMRMLNavigationViewPlugin::createWidget(QWidget *_parent)
{
  qMRMLNavigationView* _widget = new qMRMLNavigationView(_parent);
  return _widget;
}

//-----------------------------------------------------------------------------
QString qMRMLNavigationViewPlugin::domXml() const
{
  return "<widget class=\"qMRMLNavigationView\" \
          name=\"MRMLNavigationView\">\n"
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

//-----------------------------------------------------------------------------
QString qMRMLNavigationViewPlugin::includeFile() const
{
  return "qMRMLNavigationView.h";
}

//-----------------------------------------------------------------------------
bool qMRMLNavigationViewPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qMRMLNavigationViewPlugin::name() const
{
  return "qMRMLNavigationView";
}

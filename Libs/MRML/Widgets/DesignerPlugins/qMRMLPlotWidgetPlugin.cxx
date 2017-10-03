/*==============================================================================

  Copyright (c) Kapteyn Astronomical Institute
  University of Groningen, Groningen, Netherlands. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Davide Punzo, Kapteyn Astronomical Institute,
  and was supported through the European Research Council grant nr. 291531.

==============================================================================*/

// qMRML includes
#include "qMRMLPlotWidgetPlugin.h"
#include "qMRMLPlotWidget.h"

//-----------------------------------------------------------------------------
qMRMLPlotWidgetPlugin::qMRMLPlotWidgetPlugin(QObject *_parent):QObject(_parent)
{
}

//-----------------------------------------------------------------------------
QWidget *qMRMLPlotWidgetPlugin::createWidget(QWidget *_parent)
{
  qMRMLPlotWidget* _widget = new qMRMLPlotWidget(_parent);
  return _widget;
}

//-----------------------------------------------------------------------------
QString qMRMLPlotWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLPlotWidget\" \
          name=\"MRMLPlotViewWidget\">\n"
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
QString qMRMLPlotWidgetPlugin::includeFile() const
{
  return "qMRMLPlotWidget.h";
}

//-----------------------------------------------------------------------------
bool qMRMLPlotWidgetPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qMRMLPlotWidgetPlugin::name() const
{
  return "qMRMLPlotWidget";
}

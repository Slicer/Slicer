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

#include "qMRMLPlotViewInformationWidgetPlugin.h"
#include "qMRMLPlotViewInformationWidget.h"

// --------------------------------------------------------------------------
qMRMLPlotViewInformationWidgetPlugin::qMRMLPlotViewInformationWidgetPlugin(QObject *_parent)
        : QObject(_parent)
{
}

// --------------------------------------------------------------------------
QWidget *qMRMLPlotViewInformationWidgetPlugin::createWidget(QWidget *_parent)
{
  qMRMLPlotViewInformationWidget* _widget = new qMRMLPlotViewInformationWidget(_parent);
  return _widget;
}

// --------------------------------------------------------------------------
QString qMRMLPlotViewInformationWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLPlotViewInformationWidget\" \
          name=\"MRMLPlotViewInformationWidget\">\n"
          "</widget>\n";
}

// --------------------------------------------------------------------------
QIcon qMRMLPlotViewInformationWidgetPlugin::icon() const
{
  return QIcon();
}

// --------------------------------------------------------------------------
QString qMRMLPlotViewInformationWidgetPlugin::includeFile() const
{
  return "qMRMLPlotViewInformationWidget.h";
}

// --------------------------------------------------------------------------
bool qMRMLPlotViewInformationWidgetPlugin::isContainer() const
{
  return false;
}

// --------------------------------------------------------------------------
QString qMRMLPlotViewInformationWidgetPlugin::name() const
{
  return "qMRMLPlotViewInformationWidget";
}

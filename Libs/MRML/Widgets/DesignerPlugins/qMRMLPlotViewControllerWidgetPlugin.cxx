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

#include "qMRMLPlotViewControllerWidgetPlugin.h"
#include "qMRMLPlotViewControllerWidget.h"

// --------------------------------------------------------------------------
qMRMLPlotViewControllerWidgetPlugin::qMRMLPlotViewControllerWidgetPlugin(QObject *_parent)
        : QObject(_parent)
{
}

// --------------------------------------------------------------------------
QWidget *qMRMLPlotViewControllerWidgetPlugin::createWidget(QWidget *_parent)
{
  qMRMLPlotViewControllerWidget* _widget = new qMRMLPlotViewControllerWidget(_parent);
  return _widget;
}

// --------------------------------------------------------------------------
QString qMRMLPlotViewControllerWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLPlotViewControllerWidget\" \
          name=\"MRMLPlotViewControllerWidget\">\n"
          "</widget>\n";
}

// --------------------------------------------------------------------------
QIcon qMRMLPlotViewControllerWidgetPlugin::icon() const
{
  return QIcon();
}

// --------------------------------------------------------------------------
QString qMRMLPlotViewControllerWidgetPlugin::includeFile() const
{
  return "qMRMLPlotViewControllerWidget.h";
}

// --------------------------------------------------------------------------
bool qMRMLPlotViewControllerWidgetPlugin::isContainer() const
{
  return false;
}

// --------------------------------------------------------------------------
QString qMRMLPlotViewControllerWidgetPlugin::name() const
{
  return "qMRMLPlotViewControllerWidget";
}

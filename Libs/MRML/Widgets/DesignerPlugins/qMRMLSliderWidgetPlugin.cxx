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

  This file was originally developed by Johan Andruejol, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#include "qMRMLSliderWidgetPlugin.h"
#include "qMRMLSliderWidget.h"

// --------------------------------------------------------------------------
qMRMLSliderWidgetPlugin::qMRMLSliderWidgetPlugin(QObject *_parent)
: QObject(_parent)
{
}

// --------------------------------------------------------------------------
QWidget *qMRMLSliderWidgetPlugin::createWidget(QWidget *_parent)
{
  qMRMLSliderWidget* _widget = new qMRMLSliderWidget(_parent);
  return _widget;
}

// --------------------------------------------------------------------------
QString qMRMLSliderWidgetPlugin::domXml() const
{
  return "<widget class=\"qMRMLSliderWidget\" \
                  name=\"MRMLSliderWidget\">\n"
          "</widget>\n";
}

// --------------------------------------------------------------------------
QIcon qMRMLSliderWidgetPlugin::icon() const
{
  return QIcon(":/Icons/sliderspinbox.png");
}

// --------------------------------------------------------------------------
QString qMRMLSliderWidgetPlugin::includeFile() const
{
  return "qMRMLSliderWidget.h";
}

// --------------------------------------------------------------------------
bool qMRMLSliderWidgetPlugin::isContainer() const
{
  return false;
}

// --------------------------------------------------------------------------
QString qMRMLSliderWidgetPlugin::name() const
{
  return "qMRMLSliderWidget";
}

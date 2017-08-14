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

#include "qSlicerStyle.h"
#include "qSlicerWidgetPlugin.h"
#include "qSlicerWidget.h"

// --------------------------------------------------------------------------
qSlicerWidgetPlugin::qSlicerWidgetPlugin(QObject* parent)
  : QObject(parent)
{
}

// --------------------------------------------------------------------------
QWidget *qSlicerWidgetPlugin::createWidget(QWidget* parentWidget)
{
  qSlicerWidget* widget = new qSlicerWidget(parentWidget);
  QPalette slicerPalette = widget->palette();

  // Apply Slicer Palette using the non-member function defined in qSlicerApplication
  //qSlicerApplyPalette(slicerPalette);
  qSlicerStyle style;

  widget->setPalette(style.standardPalette());
  widget->setAutoFillBackground(true);
  return widget;
}

// --------------------------------------------------------------------------
QString qSlicerWidgetPlugin::domXml() const
{
  return "<widget class=\"qSlicerWidget\" name=\"SlicerWidget\">\n"
    " <property name=\"geometry\">\n"
    "  <rect>\n"
    "   <x>0</x>\n"
    "   <y>0</y>\n"
    "   <width>100</width>\n"
    "   <height>100</height>\n"
    "  </rect>\n"
    " </property>\n"
    "</widget>\n";
}

// --------------------------------------------------------------------------
QIcon qSlicerWidgetPlugin::icon() const
{
  return QIcon(":/Icons/widget.png");
}

// --------------------------------------------------------------------------
QString qSlicerWidgetPlugin::includeFile() const
{
  return "qSlicerWidget.h";
}

// --------------------------------------------------------------------------
bool qSlicerWidgetPlugin::isContainer() const
{
  return true;
}

// --------------------------------------------------------------------------
QString qSlicerWidgetPlugin::name() const
{
  return "qSlicerWidget";
}

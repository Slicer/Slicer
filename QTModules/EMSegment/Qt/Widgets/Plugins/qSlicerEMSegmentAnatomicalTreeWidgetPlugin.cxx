/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#include "qSlicerEMSegmentAnatomicalTreeWidgetPlugin.h"
#include "qSlicerEMSegmentAnatomicalTreeWidget.h"

//-----------------------------------------------------------------------------
qSlicerEMSegmentAnatomicalTreeWidgetPlugin::qSlicerEMSegmentAnatomicalTreeWidgetPlugin(QObject *_parent)
        : QObject(_parent)
{
}

//-----------------------------------------------------------------------------
QWidget *qSlicerEMSegmentAnatomicalTreeWidgetPlugin::createWidget(QWidget *_parent)
{
  qSlicerEMSegmentAnatomicalTreeWidget* _widget = new qSlicerEMSegmentAnatomicalTreeWidget(_parent);
  return _widget;
}

//-----------------------------------------------------------------------------
QString qSlicerEMSegmentAnatomicalTreeWidgetPlugin::domXml() const
{
  return "<widget class=\"qSlicerEMSegmentAnatomicalTreeWidget\" \
          name=\"EMSegmentAnatomicalTreeWidget\">\n"
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
QIcon qSlicerEMSegmentAnatomicalTreeWidgetPlugin::icon() const
{
  return QIcon(":/Icons/tree.png");
}

//-----------------------------------------------------------------------------
QString qSlicerEMSegmentAnatomicalTreeWidgetPlugin::includeFile() const
{
  return "qSlicerEMSegmentAnatomicalTreeWidget.h";
}

//-----------------------------------------------------------------------------
bool qSlicerEMSegmentAnatomicalTreeWidgetPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qSlicerEMSegmentAnatomicalTreeWidgetPlugin::name() const
{
  return "qSlicerEMSegmentAnatomicalTreeWidget";
}

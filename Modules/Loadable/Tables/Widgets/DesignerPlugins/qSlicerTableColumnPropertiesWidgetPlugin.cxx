/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Matthew Holden, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#include "qSlicerTableColumnPropertiesWidgetPlugin.h"
#include "qSlicerTableColumnPropertiesWidget.h"

//------------------------------------------------------------------------------
qSlicerTableColumnPropertiesWidgetPlugin
::qSlicerTableColumnPropertiesWidgetPlugin(QObject *_parent)
  : QObject(_parent)
{

}

//------------------------------------------------------------------------------
QWidget *qSlicerTableColumnPropertiesWidgetPlugin
::createWidget(QWidget *_parent)
{
  qSlicerTableColumnPropertiesWidget* _widget
    = new qSlicerTableColumnPropertiesWidget(_parent);
  return _widget;
}

//------------------------------------------------------------------------------
QString qSlicerTableColumnPropertiesWidgetPlugin
::domXml() const
{
  return "<widget class=\"qSlicerTableColumnPropertiesWidget\" \
          name=\"SlicerTableColumnPropertiesWidget\">\n"
          "</widget>\n";
}

//------------------------------------------------------------------------------
QString qSlicerTableColumnPropertiesWidgetPlugin
::includeFile() const
{
  return "qSlicerTableColumnPropertiesWidget.h";
}

//------------------------------------------------------------------------------
bool qSlicerTableColumnPropertiesWidgetPlugin
::isContainer() const
{
  return false;
}

//------------------------------------------------------------------------------
QString qSlicerTableColumnPropertiesWidgetPlugin
::name() const
{
  return "qSlicerTableColumnPropertiesWidget";
}

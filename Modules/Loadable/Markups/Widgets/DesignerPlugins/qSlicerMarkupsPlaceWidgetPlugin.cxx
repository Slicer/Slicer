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

#include "qSlicerMarkupsPlaceWidgetPlugin.h"
#include "qSlicerMarkupsPlaceWidget.h"

//------------------------------------------------------------------------------
qSlicerMarkupsPlaceWidgetPlugin
::qSlicerMarkupsPlaceWidgetPlugin(QObject *_parent)
  : QObject(_parent)
{

}

//------------------------------------------------------------------------------
QWidget *qSlicerMarkupsPlaceWidgetPlugin
::createWidget(QWidget *_parent)
{
  qSlicerMarkupsPlaceWidget* _widget
    = new qSlicerMarkupsPlaceWidget(_parent);
  return _widget;
}

//------------------------------------------------------------------------------
QString qSlicerMarkupsPlaceWidgetPlugin
::domXml() const
{
  return "<widget class=\"qSlicerMarkupsPlaceWidget\" \
          name=\"MarkupsPlaceWidget\">\n"
          "</widget>\n";
}

//------------------------------------------------------------------------------
QString qSlicerMarkupsPlaceWidgetPlugin
::includeFile() const
{
  return "qSlicerMarkupsPlaceWidget.h";
}

//------------------------------------------------------------------------------
bool qSlicerMarkupsPlaceWidgetPlugin
::isContainer() const
{
  return false;
}

//------------------------------------------------------------------------------
QString qSlicerMarkupsPlaceWidgetPlugin
::name() const
{
  return "qSlicerMarkupsPlaceWidget";
}

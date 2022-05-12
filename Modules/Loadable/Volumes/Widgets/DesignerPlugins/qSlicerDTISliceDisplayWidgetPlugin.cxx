/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Brigham and Women's Hospital

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#include "qSlicerDTISliceDisplayWidgetPlugin.h"
#include "qSlicerDTISliceDisplayWidget.h"

//------------------------------------------------------------------------------
qSlicerDTISliceDisplayWidgetPlugin
::qSlicerDTISliceDisplayWidgetPlugin(QObject *_parent)
  : QObject(_parent)
{

}

//------------------------------------------------------------------------------
QWidget *qSlicerDTISliceDisplayWidgetPlugin
::createWidget(QWidget *_parent)
{
  qSlicerDTISliceDisplayWidget* _widget
    = new qSlicerDTISliceDisplayWidget(_parent);
  return _widget;
}

//------------------------------------------------------------------------------
QString qSlicerDTISliceDisplayWidgetPlugin
::domXml() const
{
  return "<widget class=\"qSlicerDTISliceDisplayWidget\" \
          name=\"VolumeDisplayWidget\">\n"
          "</widget>\n";
}

//------------------------------------------------------------------------------
QString qSlicerDTISliceDisplayWidgetPlugin
::includeFile() const
{
  return "qSlicerDTISliceDisplayWidget.h";
}

//------------------------------------------------------------------------------
bool qSlicerDTISliceDisplayWidgetPlugin
::isContainer() const
{
  return false;
}

//------------------------------------------------------------------------------
QString qSlicerDTISliceDisplayWidgetPlugin
::name() const
{
  return "qSlicerDTISliceDisplayWidget";
}

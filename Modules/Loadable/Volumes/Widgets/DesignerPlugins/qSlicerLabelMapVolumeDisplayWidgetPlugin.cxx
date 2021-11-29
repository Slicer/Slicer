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

#include "qSlicerLabelMapVolumeDisplayWidgetPlugin.h"
#include "qSlicerLabelMapVolumeDisplayWidget.h"

//------------------------------------------------------------------------------
qSlicerLabelMapVolumeDisplayWidgetPlugin
::qSlicerLabelMapVolumeDisplayWidgetPlugin(QObject *_parent)
  : QObject(_parent)
{

}

//------------------------------------------------------------------------------
QWidget *qSlicerLabelMapVolumeDisplayWidgetPlugin
::createWidget(QWidget *_parent)
{
  qSlicerLabelMapVolumeDisplayWidget* _widget
    = new qSlicerLabelMapVolumeDisplayWidget(_parent);
  return _widget;
}

//------------------------------------------------------------------------------
QString qSlicerLabelMapVolumeDisplayWidgetPlugin
::domXml() const
{
  return "<widget class=\"qSlicerLabelMapVolumeDisplayWidget\" \
          name=\"VolumeDisplayWidget\">\n"
          "</widget>\n";
}

//------------------------------------------------------------------------------
QString qSlicerLabelMapVolumeDisplayWidgetPlugin
::includeFile() const
{
  return "qSlicerLabelMapVolumeDisplayWidget.h";
}

//------------------------------------------------------------------------------
bool qSlicerLabelMapVolumeDisplayWidgetPlugin
::isContainer() const
{
  return false;
}

//------------------------------------------------------------------------------
QString qSlicerLabelMapVolumeDisplayWidgetPlugin
::name() const
{
  return "qSlicerLabelMapVolumeDisplayWidget";
}

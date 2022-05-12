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

#include "qSlicerVolumeDisplayWidgetPlugin.h"
#include "qSlicerVolumeDisplayWidget.h"

//------------------------------------------------------------------------------
qSlicerVolumeDisplayWidgetPlugin
::qSlicerVolumeDisplayWidgetPlugin(QObject *_parent)
  : QObject(_parent)
{

}

//------------------------------------------------------------------------------
QWidget *qSlicerVolumeDisplayWidgetPlugin
::createWidget(QWidget *_parent)
{
  qSlicerVolumeDisplayWidget* _widget
    = new qSlicerVolumeDisplayWidget(_parent);
  return _widget;
}

//------------------------------------------------------------------------------
QString qSlicerVolumeDisplayWidgetPlugin
::domXml() const
{
  return "<widget class=\"qSlicerVolumeDisplayWidget\" \
          name=\"VolumeDisplayWidget\">\n"
          "</widget>\n";
}

//------------------------------------------------------------------------------
QString qSlicerVolumeDisplayWidgetPlugin
::includeFile() const
{
  return "qSlicerVolumeDisplayWidget.h";
}

//------------------------------------------------------------------------------
bool qSlicerVolumeDisplayWidgetPlugin
::isContainer() const
{
  return false;
}

//------------------------------------------------------------------------------
QString qSlicerVolumeDisplayWidgetPlugin
::name() const
{
  return "qSlicerVolumeDisplayWidget";
}

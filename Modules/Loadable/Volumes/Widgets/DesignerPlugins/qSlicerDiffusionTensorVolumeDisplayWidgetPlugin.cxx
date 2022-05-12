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

#include "qSlicerDiffusionTensorVolumeDisplayWidgetPlugin.h"
#include "qSlicerDiffusionTensorVolumeDisplayWidget.h"

//------------------------------------------------------------------------------
qSlicerDiffusionTensorVolumeDisplayWidgetPlugin
::qSlicerDiffusionTensorVolumeDisplayWidgetPlugin(QObject *_parent)
  : QObject(_parent)
{

}

//------------------------------------------------------------------------------
QWidget *qSlicerDiffusionTensorVolumeDisplayWidgetPlugin
::createWidget(QWidget *_parent)
{
  qSlicerDiffusionTensorVolumeDisplayWidget* _widget
    = new qSlicerDiffusionTensorVolumeDisplayWidget(_parent);
  return _widget;
}

//------------------------------------------------------------------------------
QString qSlicerDiffusionTensorVolumeDisplayWidgetPlugin
::domXml() const
{
  return "<widget class=\"qSlicerDiffusionTensorVolumeDisplayWidgetPlugin\" \
          name=\"VolumeDisplayWidget\">\n"
          "</widget>\n";
}

//------------------------------------------------------------------------------
QString qSlicerDiffusionTensorVolumeDisplayWidgetPlugin
::includeFile() const
{
  return "qSlicerDiffusionTensorVolumeDisplayWidget.h";
}

//------------------------------------------------------------------------------
bool qSlicerDiffusionTensorVolumeDisplayWidgetPlugin
::isContainer() const
{
  return false;
}

//------------------------------------------------------------------------------
QString qSlicerDiffusionTensorVolumeDisplayWidgetPlugin
::name() const
{
  return "qSlicerDiffusionTensorVolumeDisplayWidget";
}

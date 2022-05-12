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

#include "qSlicerDiffusionWeightedVolumeDisplayWidgetPlugin.h"
#include "qSlicerDiffusionWeightedVolumeDisplayWidget.h"

//------------------------------------------------------------------------------
qSlicerDiffusionWeightedVolumeDisplayWidgetPlugin
::qSlicerDiffusionWeightedVolumeDisplayWidgetPlugin(QObject *_parent)
  : QObject(_parent)
{

}

//------------------------------------------------------------------------------
QWidget *qSlicerDiffusionWeightedVolumeDisplayWidgetPlugin
::createWidget(QWidget *_parent)
{
  qSlicerDiffusionWeightedVolumeDisplayWidget* _widget
    = new qSlicerDiffusionWeightedVolumeDisplayWidget(_parent);
  return _widget;
}

//------------------------------------------------------------------------------
QString qSlicerDiffusionWeightedVolumeDisplayWidgetPlugin
::domXml() const
{
  return "<widget class=\"qSlicerDiffusionWeightedVolumeDisplayWidget\" \
          name=\"VolumeDisplayWidget\">\n"
          "</widget>\n";
}

//------------------------------------------------------------------------------
QString qSlicerDiffusionWeightedVolumeDisplayWidgetPlugin
::includeFile() const
{
  return "qSlicerDiffusionWeightedVolumeDisplayWidget.h";
}

//------------------------------------------------------------------------------
bool qSlicerDiffusionWeightedVolumeDisplayWidgetPlugin
::isContainer() const
{
  return false;
}

//------------------------------------------------------------------------------
QString qSlicerDiffusionWeightedVolumeDisplayWidgetPlugin
::name() const
{
  return "qSlicerDiffusionWeightedVolumeDisplayWidget";
}

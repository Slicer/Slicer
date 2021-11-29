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

#include "qSlicerScalarVolumeDisplayWidgetPlugin.h"
#include "qSlicerScalarVolumeDisplayWidget.h"

//------------------------------------------------------------------------------
qSlicerScalarVolumeDisplayWidgetPlugin
::qSlicerScalarVolumeDisplayWidgetPlugin(QObject *_parent)
  : QObject(_parent)
{

}

//------------------------------------------------------------------------------
QWidget *qSlicerScalarVolumeDisplayWidgetPlugin
::createWidget(QWidget *_parent)
{
  qSlicerScalarVolumeDisplayWidget* _widget
    = new qSlicerScalarVolumeDisplayWidget(_parent);
  return _widget;
}

//------------------------------------------------------------------------------
QString qSlicerScalarVolumeDisplayWidgetPlugin
::domXml() const
{
  return "<widget class=\"qSlicerScalarVolumeDisplayWidget\" \
          name=\"VolumeDisplayWidget\">\n"
          "</widget>\n";
}

//------------------------------------------------------------------------------
QString qSlicerScalarVolumeDisplayWidgetPlugin
::includeFile() const
{
  return "qSlicerScalarVolumeDisplayWidget.h";
}

//------------------------------------------------------------------------------
bool qSlicerScalarVolumeDisplayWidgetPlugin
::isContainer() const
{
  return false;
}

//------------------------------------------------------------------------------
QString qSlicerScalarVolumeDisplayWidgetPlugin
::name() const
{
  return "qSlicerScalarVolumeDisplayWidget";
}

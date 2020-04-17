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

#include "qMRMLSequenceBrowserSeekWidgetPlugin.h"
#include "qMRMLSequenceBrowserSeekWidget.h"

//------------------------------------------------------------------------------
qMRMLSequenceBrowserSeekWidgetPlugin
::qMRMLSequenceBrowserSeekWidgetPlugin(QObject *_parent)
  : QObject(_parent)
{

}

//------------------------------------------------------------------------------
QWidget *qMRMLSequenceBrowserSeekWidgetPlugin
::createWidget(QWidget *_parent)
{
  qMRMLSequenceBrowserSeekWidget* _widget
    = new qMRMLSequenceBrowserSeekWidget(_parent);
  return _widget;
}

//------------------------------------------------------------------------------
QString qMRMLSequenceBrowserSeekWidgetPlugin
::domXml() const
{
  return "<widget class=\"qMRMLSequenceBrowserSeekWidget\" \
          name=\"SequenceBrowserSeekWidget\">\n"
          "</widget>\n";
}

//------------------------------------------------------------------------------
QString qMRMLSequenceBrowserSeekWidgetPlugin
::includeFile() const
{
  return "qMRMLSequenceBrowserSeekWidget.h";
}

//------------------------------------------------------------------------------
bool qMRMLSequenceBrowserSeekWidgetPlugin
::isContainer() const
{
  return false;
}

//------------------------------------------------------------------------------
QString qMRMLSequenceBrowserSeekWidgetPlugin
::name() const
{
  return "qMRMLSequenceBrowserSeekWidget";
}

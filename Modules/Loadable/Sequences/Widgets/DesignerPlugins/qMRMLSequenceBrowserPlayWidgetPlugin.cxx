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

#include "qMRMLSequenceBrowserPlayWidgetPlugin.h"
#include "qMRMLSequenceBrowserPlayWidget.h"

//------------------------------------------------------------------------------
qMRMLSequenceBrowserPlayWidgetPlugin
::qMRMLSequenceBrowserPlayWidgetPlugin(QObject *_parent)
  : QObject(_parent)
{

}

//------------------------------------------------------------------------------
QWidget *qMRMLSequenceBrowserPlayWidgetPlugin
::createWidget(QWidget *_parent)
{
  qMRMLSequenceBrowserPlayWidget* _widget
    = new qMRMLSequenceBrowserPlayWidget(_parent);
  return _widget;
}

//------------------------------------------------------------------------------
QString qMRMLSequenceBrowserPlayWidgetPlugin
::domXml() const
{
  return "<widget class=\"qMRMLSequenceBrowserPlayWidget\" \
          name=\"SequenceBrowserPlayWidget\">\n"
          "</widget>\n";
}

//------------------------------------------------------------------------------
QString qMRMLSequenceBrowserPlayWidgetPlugin
::includeFile() const
{
  return "qMRMLSequenceBrowserPlayWidget.h";
}

//------------------------------------------------------------------------------
bool qMRMLSequenceBrowserPlayWidgetPlugin
::isContainer() const
{
  return false;
}

//------------------------------------------------------------------------------
QString qMRMLSequenceBrowserPlayWidgetPlugin
::name() const
{
  return "qMRMLSequenceBrowserPlayWidget";
}

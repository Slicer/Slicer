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

#include "qMRMLSequenceEditWidgetPlugin.h"
#include "qMRMLSequenceEditWidget.h"

//------------------------------------------------------------------------------
qMRMLSequenceEditWidgetPlugin ::qMRMLSequenceEditWidgetPlugin(QObject* _parent)
  : QObject(_parent)
{
}

//------------------------------------------------------------------------------
QWidget* qMRMLSequenceEditWidgetPlugin ::createWidget(QWidget* _parent)
{
  qMRMLSequenceEditWidget* _widget = new qMRMLSequenceEditWidget(_parent);
  return _widget;
}

//------------------------------------------------------------------------------
QString qMRMLSequenceEditWidgetPlugin ::domXml() const
{
  return "<widget class=\"qMRMLSequenceEditWidget\" \
          name=\"SequenceEditWidget\">\n"
         "</widget>\n";
}

//------------------------------------------------------------------------------
QString qMRMLSequenceEditWidgetPlugin ::includeFile() const
{
  return "qMRMLSequenceEditWidget.h";
}

//------------------------------------------------------------------------------
bool qMRMLSequenceEditWidgetPlugin ::isContainer() const
{
  return false;
}

//------------------------------------------------------------------------------
QString qMRMLSequenceEditWidgetPlugin ::name() const
{
  return "qMRMLSequenceEditWidget";
}

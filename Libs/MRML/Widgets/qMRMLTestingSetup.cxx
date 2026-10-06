/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#include "qMRMLTestingSetup.h"

// QtTesting includes
#ifdef MRML_WIDGETS_USE_QTTESTING
# include <pqEventDispatcher.h>
#endif

// STD includes
#include <iostream>

//-----------------------------------------------------------------------------
qMRMLTestingSetup::qMRMLTestingSetup(int argc, char* argv[])
{
  QStringList arguments;
  for (int i = 1; i < argc; ++i)
  {
    arguments << QString::fromLocal8Bit(argv[i]);
  }
  this->parse(arguments);
  if (!this->isValid())
  {
    // Use std::cerr (instead of Qt logging) to make sure the error is visible in the test output
    std::cerr << "Invalid test arguments: " << qPrintable(this->ErrorString) << std::endl;
    return;
  }
  this->applySettings();
}

//-----------------------------------------------------------------------------
void qMRMLTestingSetup::parse(const QStringList& arguments)
{
  for (int i = 0; i < arguments.size(); ++i)
  {
    const QString& argument = arguments[i];
    if (argument == "-I")
    {
      this->Interactive = true;
    }
    else if (argument == "--event-playback-delay")
    {
      if (i + 1 >= arguments.size())
      {
        this->ErrorString = "Missing value for option --event-playback-delay";
        continue;
      }
      ++i;
      bool valid = false;
      const int delay = arguments[i].toInt(&valid);
      if (!valid || delay < 0)
      {
        this->ErrorString = QString("Invalid value for option --event-playback-delay: '%1' (expected a non-negative integer)").arg(arguments[i]);
        continue;
      }
      this->EventPlaybackDelay = delay;
    }
    else
    {
      this->PositionalArguments << argument;
    }
  }
}

//-----------------------------------------------------------------------------
bool qMRMLTestingSetup::isValid() const
{
  return this->ErrorString.isEmpty();
}

//-----------------------------------------------------------------------------
QString qMRMLTestingSetup::errorString() const
{
  return this->ErrorString;
}

//-----------------------------------------------------------------------------
bool qMRMLTestingSetup::interactive() const
{
  return this->Interactive;
}

//-----------------------------------------------------------------------------
QStringList qMRMLTestingSetup::positionalArguments() const
{
  return this->PositionalArguments;
}

//-----------------------------------------------------------------------------
QString qMRMLTestingSetup::positionalArgument(int index, const QString& defaultValue /*=QString()*/) const
{
  return this->PositionalArguments.value(index, defaultValue);
}

//-----------------------------------------------------------------------------
int qMRMLTestingSetup::eventPlaybackDelay() const
{
  return this->EventPlaybackDelay;
}

//-----------------------------------------------------------------------------
void qMRMLTestingSetup::applySettings() const
{
#ifdef MRML_WIDGETS_USE_QTTESTING
  if (this->EventPlaybackDelay >= 0)
  {
    pqEventDispatcher::setEventPlaybackDelay(this->EventPlaybackDelay);
  }
#endif
}

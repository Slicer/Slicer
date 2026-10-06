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

// Qt includes
#include <QByteArray>
#include <QList>
#include <QStringList>

// Slicer includes
#include "vtkSlicerConfigure.h"

// CTK includes
#include <ctkCoreTestingMacros.h>

// QtTesting includes
#ifdef Slicer_USE_QtTesting
# include <pqEventDispatcher.h>
#endif

// qMRML includes
#include "qMRMLTestingSetup.h"

// STD includes
#include <cstdlib>
#include <vector>

namespace
{

//-----------------------------------------------------------------------------
/// Command-line arguments of a test function: the test name followed by the specified arguments.
class TestArguments
{
public:
  TestArguments(const QStringList& arguments)
  {
    this->Arguments << QByteArray("qMRMLTestingSetupTest1");
    for (const QString& argument : arguments)
    {
      this->Arguments << argument.toLocal8Bit();
    }
    for (QByteArray& argument : this->Arguments)
    {
      this->Argv.push_back(argument.data());
    }
  }
  int argc() { return static_cast<int>(this->Argv.size()); }
  char** argv() { return this->Argv.data(); }

private:
  QList<QByteArray> Arguments;
  std::vector<char*> Argv;
};

} // namespace

//-----------------------------------------------------------------------------
int qMRMLTestingSetupTest1(int argc, char* argv[])
{
  Q_UNUSED(argc);
  Q_UNUSED(argv);

  {
    // No arguments
    TestArguments args{ QStringList() };
    qMRMLTestingSetup setup(args.argc(), args.argv());
    CHECK_BOOL(setup.isValid(), true);
    CHECK_BOOL(setup.interactive(), false);
    CHECK_INT(setup.eventPlaybackDelay(), -1);
    CHECK_INT(setup.positionalArguments().size(), 0);
    CHECK_QSTRING(setup.positionalArgument(0), QString());
    CHECK_QSTRING(setup.positionalArgument(5, "x"), QString("x"));
  }

  {
    // Positional arguments are kept in order, options are recognized anywhere,
    // and the specified playback delay is applied
    TestArguments args(QStringList() << "-I" << "srcDir" << "--event-playback-delay" << "250" << "scene.mrml");
    qMRMLTestingSetup setup(args.argc(), args.argv());
    CHECK_BOOL(setup.isValid(), true);
    CHECK_BOOL(setup.interactive(), true);
    CHECK_INT(setup.eventPlaybackDelay(), 250);
    CHECK_QSTRINGLIST(setup.positionalArguments(), QStringList() << "srcDir" << "scene.mrml");
    CHECK_QSTRING(setup.positionalArgument(0), QString("srcDir"));
    CHECK_QSTRING(setup.positionalArgument(1), QString("scene.mrml"));
#ifdef Slicer_USE_QtTesting
    CHECK_INT(pqEventDispatcher::eventPlaybackDelay(), 250);
#endif
  }

  {
    // Interactive option in the middle and at the end
    TestArguments argsMiddle(QStringList() << "a" << "-I" << "b");
    qMRMLTestingSetup setupMiddle(argsMiddle.argc(), argsMiddle.argv());
    CHECK_BOOL(setupMiddle.interactive(), true);
    CHECK_QSTRINGLIST(setupMiddle.positionalArguments(), QStringList() << "a" << "b");
    TestArguments argsEnd(QStringList() << "a" << "b" << "-I");
    qMRMLTestingSetup setupEnd(argsEnd.argc(), argsEnd.argv());
    CHECK_BOOL(setupEnd.interactive(), true);
  }

  {
    // Zero playback delay
    TestArguments args(QStringList() << "srcDir" << "--event-playback-delay" << "0");
    qMRMLTestingSetup setup(args.argc(), args.argv());
    CHECK_BOOL(setup.isValid(), true);
    CHECK_INT(setup.eventPlaybackDelay(), 0);
    CHECK_QSTRINGLIST(setup.positionalArguments(), QStringList() << "srcDir");
#ifdef Slicer_USE_QtTesting
    CHECK_INT(pqEventDispatcher::eventPlaybackDelay(), 0);
#endif
  }

  {
    // Playback delay is not changed if it is not specified
    TestArguments args(QStringList() << "srcDir");
    qMRMLTestingSetup setup(args.argc(), args.argv());
    CHECK_INT(setup.eventPlaybackDelay(), -1);
#ifdef Slicer_USE_QtTesting
    CHECK_INT(pqEventDispatcher::eventPlaybackDelay(), 0);
#endif
  }

  {
    // Missing value: invalid, settings are not applied
    TestArguments args(QStringList() << "srcDir" << "--event-playback-delay");
    qMRMLTestingSetup setup(args.argc(), args.argv());
    CHECK_BOOL(setup.isValid(), false);
    CHECK_BOOL(setup.errorString().isEmpty(), false);
    CHECK_INT(setup.eventPlaybackDelay(), -1);
  }

  {
    // Invalid values: settings are not applied
    TestArguments argsText(QStringList() << "--event-playback-delay" << "fast");
    CHECK_BOOL(qMRMLTestingSetup(argsText.argc(), argsText.argv()).isValid(), false);
    TestArguments argsNegative(QStringList() << "--event-playback-delay" << "-5");
    CHECK_BOOL(qMRMLTestingSetup(argsNegative.argc(), argsNegative.argv()).isValid(), false);
#ifdef Slicer_USE_QtTesting
    CHECK_INT(pqEventDispatcher::eventPlaybackDelay(), 0);
#endif
  }

  return EXIT_SUCCESS;
}

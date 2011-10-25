/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// QT includes
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QStringList>

// SlicerQt includes
#include "qSlicerUtils.h"

// SlicerLogic includes
#include "vtkSlicerApplicationLogic.h"

//------------------------------------------------------------------------------
bool qSlicerUtils::isExecutableName(const QString& name)
{
  QStringList extensions;
  extensions << ".bat" << ".com" << ".sh" << ".csh" << ".tcsh"
             << ".pl" << ".py" << ".tcl" << ".m" << ".exe";

  foreach(const QString& extension, extensions)
    {
    if (name.endsWith(extension, Qt::CaseInsensitive))
      {
      return true;
      }
    }
  return false;
}

//------------------------------------------------------------------------------
bool qSlicerUtils::isCLIExecutable(const QString& filePath)
{
#ifdef _WIN32
  return filePath.endsWith(".exe", Qt::CaseInsensitive);
#else
  return !QFileInfo(filePath).fileName().contains('.');
#endif
}

//-----------------------------------------------------------------------------
bool qSlicerUtils::isCLILoadableModule(const QString& filePath)
{
  // See http://stackoverflow.com/questions/899422/regular-expression-for-a-string-that-does-not-start-with-a-sequence
  QRegExp regex("(lib.+Lib\\.(so|dylib))|((?!lib).+Lib\\.(dll|DLL))");
  return regex.exactMatch(QFileInfo(filePath).fileName());
}

//-----------------------------------------------------------------------------
bool qSlicerUtils::isLoadableModule(const QString& filePath)
{
  // See http://stackoverflow.com/questions/899422/regular-expression-for-a-string-that-does-not-start-with-a-sequence
  QRegExp regex("(libqSlicer.+Module\\.(so|dylib))|((?!lib)qSlicer.+Module\\.(dll|DLL))");
  return regex.exactMatch(QFileInfo(filePath).fileName());
}

//------------------------------------------------------------------------------
QString qSlicerUtils::searchTargetInIntDir(const QString& directory, const QString& target)
{
#ifdef _WIN32
  QStringList intDirs;
  intDirs << "." << "Debug" << "RelWithDebInfo" << "Release" << "MinSizeRel";
  QString intDir = directory + "/%2/" + target;
  foreach(const QString& subdir, intDirs)
    {
    if (QFile::exists(intDir.arg(subdir)))
      {
      return directory+"/"+subdir+"/";
      }
    }
  return QString();
#else
  Q_UNUSED(target);
  return directory;
#endif
}

//------------------------------------------------------------------------------
QString qSlicerUtils::executableExtension()
{
#ifdef _WIN32
  return QLatin1String(".exe");
#else
  return QString();
#endif
}

//-----------------------------------------------------------------------------
QString qSlicerUtils::extractModuleNameFromLibraryName(const QString& libraryName)
{
  QFileInfo libraryPath(libraryName);
  QString moduleName = libraryPath.baseName();

  // Remove prefix 'lib' if needed
  if (moduleName.indexOf("lib") == 0)
    {
    moduleName.remove(0, 3);
    }

  // Remove prefix 'qSlicer' if needed
  if (moduleName.indexOf("qSlicer") == 0)
    {
    moduleName.remove(0, 7);
    }

  // Remove suffix 'Module' if needed
  int index = moduleName.lastIndexOf("Module");
  if (index != -1)
    {
    moduleName.remove(index, 6);
    }

  // Remove suffix 'Lib' if needed
  index = moduleName.lastIndexOf("Lib");
  if (index == (moduleName.size() - 3))
    {
    moduleName.remove(index, 3);
    }

  return moduleName;
}

//-----------------------------------------------------------------------------
QString qSlicerUtils::extractModuleNameFromClassName(const QString& className)
{
  QString moduleName(className);

  // Remove prefix 'qSlicer' if needed
  if (moduleName.indexOf("qSlicer") == 0)
    {
    moduleName.remove(0, 7);
    }

  // Remove suffix 'Module' if needed
  int index = moduleName.lastIndexOf("Module");
  if (index == (moduleName.size() - 6))
    {
    moduleName.remove(index, 6);
    }

  return moduleName;
}

//-----------------------------------------------------------------------------
bool qSlicerUtils::isPluginInstalled(const QString& filePath, const QString& applicationHomeDir)
{
  return vtkSlicerApplicationLogic::IsPluginInstalled(filePath.toStdString(), applicationHomeDir.toStdString());
}

//-----------------------------------------------------------------------------
QString qSlicerUtils::pathWithoutIntDir(const QString& path, const QString& subDirWithoutIntDir)
{
  QString tmp;
  return qSlicerUtils::pathWithoutIntDir(path, subDirWithoutIntDir, tmp);
}

//-----------------------------------------------------------------------------
QString qSlicerUtils::pathWithoutIntDir(const QString& path,
                                        const QString& subDirWithoutIntDir,
                                        QString& intDir)
{
  QDir pathAsDir(path);
  if (!qSlicerUtils::pathEndsWith(path, subDirWithoutIntDir))
    {
    intDir = pathAsDir.dirName();
    pathAsDir.cdUp();
    }
  return pathAsDir.path();
}

//-----------------------------------------------------------------------------
bool qSlicerUtils::pathEndsWith(const QString& path, const QString& relativePath)
{
  QString relativePathWithSlashNoEndingSlash = QDir::fromNativeSeparators(relativePath);
  relativePathWithSlashNoEndingSlash.remove(QRegExp("\\/$")); // Remove the last '/' if any
  int pathComponentCount = relativePathWithSlashNoEndingSlash.count("/");
  if (QDir::isRelativePath(relativePath))
    {
    ++pathComponentCount;
    }

  if (!QDir(path).cd(QString("../").repeated(pathComponentCount) + relativePath))
    {
    return false;
    }
  return true;
}

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

// Qt includes
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QRegExp>
#include <QStringList>

// Slicer includes
#include "qSlicerUtils.h"
#include "qSlicerAbstractCoreModule.h"

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
  if (isCLIScriptedExecutable(filePath))
    {
    return true;
    }

#ifdef _WIN32
  return ( filePath.endsWith(".exe", Qt::CaseInsensitive) ||
           filePath.endsWith(".bat", Qt::CaseInsensitive) );
#else
  return !QFileInfo(filePath).fileName().contains('.');
#endif
}

//------------------------------------------------------------------------------
bool qSlicerUtils::isCLIScriptedExecutable(const QString& filePath)
{
  // check if .py file starts with `#!` magic
  //   note: uses QTextStream to avoid issues with BOM.
  QFile scriptFile(filePath);
  QTextStream scriptStream(&scriptFile);

  if ( (filePath.endsWith(".py", Qt::CaseInsensitive)) &&
       (scriptFile.open(QIODevice::ReadOnly))          &&
       (scriptStream.readLine(2).startsWith("#!")) )
    {
    return true;
    }
  return false;
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

//-----------------------------------------------------------------------------
bool qSlicerUtils::isTestingModule(qSlicerAbstractCoreModule* module)
{
  const QStringList& categories = module->categories();
  foreach(const QString & category, categories)
    {
    if (category.split('.').takeFirst() != "Testing")
      {
      return false;
      }
    }
  return true;
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
  if (index != -1 && index == (moduleName.size() - 3))
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
bool qSlicerUtils::isPluginBuiltIn(const QString& filePath, const QString& applicationHomeDir)
{
  return vtkSlicerApplicationLogic::IsPluginBuiltIn(filePath.toStdString(), applicationHomeDir.toStdString());
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
    if (!qSlicerUtils::pathEndsWith(pathAsDir.path(), subDirWithoutIntDir))
      {
      intDir.clear();
      return path;
      }
    }
  return pathAsDir.path();
}

//-----------------------------------------------------------------------------
bool qSlicerUtils::pathEndsWith(const QString& inputPath, const QString& path)
{
#ifdef Q_OS_WIN32
  Qt::CaseSensitivity sensitivity = Qt::CaseInsensitive;
#else
  Qt::CaseSensitivity sensitivity = Qt::CaseSensitive;
#endif
  return QDir::cleanPath(QDir::fromNativeSeparators(inputPath)).
      endsWith(QDir::cleanPath(QDir::fromNativeSeparators(path)), sensitivity);
}

//-----------------------------------------------------------------------------
bool qSlicerUtils::setPermissionsRecursively(const QString &path,
                                             QFile::Permissions directoryPermissions,
                                             QFile::Permissions filePermissions)
{
  if (!QFile::exists(path))
    {
    qCritical() << "qSlicerUtils::setPermissionsRecursively: Failed to set permissions of nonexistent file" << path;
    return false;
    }

  foreach(const QFileInfo &info, QDir(path).entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot))
    {
    if (info.isDir())
      {
      if (directoryPermissions & QFile::ExeOwner
             || directoryPermissions & QFile::ExeGroup
             || directoryPermissions & QFile::ExeOther)
        {
        // If executable bit is on /a/b/c/d, we should start with a, b, c, then d
        if (!QFile::setPermissions(info.filePath(), directoryPermissions))
          {
          qCritical() << "qSlicerUtils::setPermissionsRecursively: Failed to set permissions on directory" << info.filePath();
          return false;
          }
        if (!qSlicerUtils::setPermissionsRecursively(info.filePath(), directoryPermissions, filePermissions))
          {
          return false;
          }
        }
      else
        {
        // .. otherwise we should start with d, c, b, then a
        if (!qSlicerUtils::setPermissionsRecursively(info.filePath(), directoryPermissions, filePermissions))
          {
          return false;
          }
        if (!QFile::setPermissions(info.filePath(), directoryPermissions))
          {
          qCritical() << "qSlicerUtils::setPermissionsRecursively: Failed to set permissions on directory" << info.filePath();
          return false;
          }
        }
      }
    else if (info.isFile())
      {
      if (!QFile::setPermissions(info.filePath(), filePermissions))
        {
        qCritical() << "qSlicerUtils::setPermissionsRecursively: Failed to set permissions on file" << info.filePath();
        return false;
        }
      }
    else
      {
      qWarning() << "qSlicerUtils::setPermissionsRecursively: Unhandled item" << info.filePath();
      }
    }
  return true;
}

//-----------------------------------------------------------------------------
QString qSlicerUtils::replaceWikiUrlVersion(const QString& text, const QString& version)
{
  QString updatedText = text;
  QRegExp rx("http[s]?\\:\\/\\/[a-zA-Z0-9\\-\\._\\?\\,\\'\\/\\\\\\+&amp;%\\$#\\=~]*");
  int pos = 0;
  while ((pos = rx.indexIn(updatedText, pos)) != -1)
    {
    // Given an URL matching the regular expression reported above, this second
    // expression will replace the first occurrence of "Documentation/<StringWithLetterOrNumberOrDot>/"
    // with "Documentation/<version>/"
    QString updatedURL = rx.cap(0).replace(QRegExp("Documentation\\/[a-zA-Z0-9\\.]+"), "Documentation/" +version);
    updatedText.replace(pos, rx.matchedLength(), updatedURL);
    pos += updatedURL.length();
    }

  return updatedText;
}

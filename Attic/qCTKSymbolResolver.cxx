/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

// Qt includes
#include <QLibrary>

// CTK includes
#include "qCTKSymbolResolver.h"

#if !defined(_WIN32)
#include "ctkBinaryFileDescriptor.h"
#endif

//-----------------------------------------------------------------------------
class qCTKSymbolResolverPrivate: public ctkPrivate<qCTKSymbolResolver>
{
public:
  typedef qCTKSymbolResolverPrivate Self;
  qCTKSymbolResolverPrivate();

  static bool useBFD(const QString& fileName);
  
  QLibrary Library;
#if !defined(_WIN32)
  ctkBinaryFileDescriptor BinaryFileDescriptor;
#endif

  enum StateType { Undefined = 0, UsingQLibrary, UsingBFD };
  StateType State;
};

// --------------------------------------------------------------------------
// qCTKSymbolResolverPrivate methods

// --------------------------------------------------------------------------
qCTKSymbolResolverPrivate::qCTKSymbolResolverPrivate()
{
  this->State = Undefined; 
  
}

// --------------------------------------------------------------------------
bool qCTKSymbolResolverPrivate::useBFD(const QString& fileName)
{
  return !QLibrary::isLibrary(fileName);
}

// --------------------------------------------------------------------------
// qCTKSymbolResolver methods

// --------------------------------------------------------------------------
qCTKSymbolResolver::qCTKSymbolResolver()
{
  CTK_INIT_PRIVATE(qCTKSymbolResolver);
}

// --------------------------------------------------------------------------
qCTKSymbolResolver::~qCTKSymbolResolver()
{
}

// --------------------------------------------------------------------------
QString qCTKSymbolResolver::errorString() const
{
  CTK_D(const qCTKSymbolResolver);
  if (d->State == qCTKSymbolResolverPrivate::UsingQLibrary)
    {
    return d->Library.errorString();
    }
  else
    {
    return QString();
    }
}

// --------------------------------------------------------------------------
QString qCTKSymbolResolver::fileName() const
{
  CTK_D(const qCTKSymbolResolver);
  if (d->State == qCTKSymbolResolverPrivate::UsingQLibrary)
    {
    return d->Library.fileName();
    }
#if !defined(_WIN32)
  else if (d->State == qCTKSymbolResolverPrivate::UsingBFD)
    {
    return d->BinaryFileDescriptor.fileName();
    }
#endif
  else
    {
    return QString();
    }
}

// --------------------------------------------------------------------------
bool qCTKSymbolResolver::isLoaded() const
{
  CTK_D(const qCTKSymbolResolver);
  if (d->State == qCTKSymbolResolverPrivate::UsingQLibrary)
    {
    return d->Library.isLoaded();
    }
#if !defined(_WIN32)
  else if (d->State == qCTKSymbolResolverPrivate::UsingBFD)
    {
    return d->BinaryFileDescriptor.isLoaded();
    }
#endif
  else
    {
    return false;
    }
}

// --------------------------------------------------------------------------
bool qCTKSymbolResolver::load()
{
  CTK_D(qCTKSymbolResolver);
  if (d->State == qCTKSymbolResolverPrivate::UsingQLibrary)
    {
    return d->Library.load();
    }
#if !defined(_WIN32)
  else if (d->State == qCTKSymbolResolverPrivate::UsingBFD)
    {
    return d->BinaryFileDescriptor.load();
    }
#endif
  else
    {
    return false;
    }
}

// --------------------------------------------------------------------------
// QLibrary::LoadHints qCTKSymbolResolver::loadHints() const
// {
//   CTK_D(const qCTKSymbolResolver);
//   if (d->State == qCTKSymbolResolverPrivate::UsingQLibrary)
//     {
//     return d->Library.loadHints();
//     }
//   else
//     {
//     return 0;
//     }
// }

// --------------------------------------------------------------------------
void * qCTKSymbolResolver::resolve(const char * symbol)
{
  CTK_D(qCTKSymbolResolver);
  if (d->State == qCTKSymbolResolverPrivate::UsingQLibrary)
    {
    return d->Library.resolve(symbol);
    }
#if !defined(_WIN32)
  else if (d->State == qCTKSymbolResolverPrivate::UsingBFD)
    {
    return d->BinaryFileDescriptor.resolve(symbol);
    }
#endif
  else
    {
    return 0;
    }
}

// --------------------------------------------------------------------------
void qCTKSymbolResolver::setFileName(const QString & _fileName)
{
  CTK_D(qCTKSymbolResolver);
  if (d->State == qCTKSymbolResolverPrivate::UsingQLibrary)
    {
    d->Library.setFileName(_fileName);
    }
#if !defined(_WIN32)
  else if (d->State == qCTKSymbolResolverPrivate::UsingBFD)
    {
    d->BinaryFileDescriptor.setFileName(_fileName);
    }
#endif
}

// --------------------------------------------------------------------------
void qCTKSymbolResolver::setFileNameAndVersion(const QString & _fileName, int versionNumber)
{
  CTK_D(qCTKSymbolResolver);
  if (d->State == qCTKSymbolResolverPrivate::UsingQLibrary)
    {
    d->Library.setFileNameAndVersion(_fileName, versionNumber);
    }
#if !defined(_WIN32)
  else if (d->State == qCTKSymbolResolverPrivate::UsingBFD)
    {
    d->BinaryFileDescriptor.setFileName(_fileName);
    }
#endif
}

// --------------------------------------------------------------------------
void qCTKSymbolResolver::setFileNameAndVersion(const QString & _fileName, const QString & version)
{
  CTK_D(qCTKSymbolResolver);
  if (d->State == qCTKSymbolResolverPrivate::UsingQLibrary)
    {
    d->Library.setFileNameAndVersion(_fileName, version);
    }
#if !defined(_WIN32)
  else if (d->State == qCTKSymbolResolverPrivate::UsingBFD)
    {
    d->BinaryFileDescriptor.setFileName(_fileName);
    }
#endif
}

// // --------------------------------------------------------------------------
// void qCTKSymbolResolver::setLoadHints(QLibrary::LoadHints hints)
// {
//   CTK_D(qCTKSymbolResolver);
//   if (d->State == qCTKSymbolResolverPrivate::UsingQLibrary)
//     {
//     d->Library.setLoadHints(hints);
//     }
// }

// --------------------------------------------------------------------------
bool qCTKSymbolResolver::unload()
{
  CTK_D(qCTKSymbolResolver);
  if (d->State == qCTKSymbolResolverPrivate::UsingQLibrary)
    {
    return d->Library.unload();
    }
#if !defined(_WIN32)
  else if (d->State == qCTKSymbolResolverPrivate::UsingBFD)
    {
    return d->BinaryFileDescriptor.unload();
    }
#endif
  else
    {
    return false;
    }
}

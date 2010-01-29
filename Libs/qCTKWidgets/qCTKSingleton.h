/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

#ifndef __qCTKSingleton_h
#define __qCTKSingleton_h

//
/// Singleton definition and declaration helpers
//
/// See http://www.parashift.com/c++-faq-lite/ctors.html#faq-10.12
/// and http://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Nifty_Counter
//
/// Inspired from VTK/Utilities/kwsys/SystemTools class
//

//-----------------------------------------------------------------------------
/// Should be included as a class protected member
//
#define QCTK_SINGLETON_DECLARE(NAME)      \
static NAME* Instance;                    \
static void classInitialize();            \
static void classFinalize();              \
friend class NAME##Initialize;
  
//-----------------------------------------------------------------------------
/// Help macro allowing to declare the utility class to make sure
/// NAME is initialized before it is used.
//
/// Should be added at the bottom of the header file, after the class declaration
//
/// The instance (NAME##Initializer) will show up in any translation unit
/// that uses NAME.  It will make sure NAME is initialized before it is used.
/// 
#define QCTK_SINGLETON_DECLARE_INITIALIZER(EXPORT_DIRECTIVE,NAME)  \
class EXPORT_DIRECTIVE NAME##Initialize                            \
{                                                                  \
public:                                                            \
  typedef NAME##Initialize Self;                                   \
                                                                   \
  NAME##Initialize();                                              \
  ~NAME##Initialize();                                             \
private:                                                           \
  static unsigned int Count;                                       \
};                                                                 \
                                                                   \
static NAME##Initialize NAME##Initializer;


//-----------------------------------------------------------------------------
//
/// Implementation of NAME##Initialize class.
//
/// Note: NAME##Initialize::Count and NAME::Instance Must NOT be initialized.
/// Default initialization to zero is necessary.
//
#define QCTK_SINGLETON_DEFINE_INITIALIZER(NAME)     \
NAME##Initialize::NAME##Initialize()                \
{                                                   \
  if(++Self::Count == 1)                            \
    { NAME::classInitialize(); }                    \
}                                                   \
                                                    \
NAME##Initialize::~NAME##Initialize()               \
{                                                   \
  if(--Self::Count == 0)                            \
    { NAME::classFinalize(); }                      \
}                                                   \
                                                    \
unsigned int NAME##Initialize::Count;               \
NAME* NAME::Instance;


//----------------------------------------------------------------------------
//
/// This should be added at the end of the CXX file
//
#define QCTK_SINGLETON_DEFINE(NAME)                 \
void NAME::classInitialize()                        \
{                                                   \
  Self::Instance = new NAME;                        \
}                                                   \
                                                    \
void NAME::classFinalize()                          \
{                                                   \
  delete Self::Instance;                            \
}                                                   \
                                                    \
QCTK_SINGLETON_DEFINE_INITIALIZER(NAME)             


//----------------------------------------------------------------------------
//
/// Helper macro
//
/// TODO Documentation
//
#define QCTK_SINGLETON_DECLARE_PRIVATE(PUB)      \
PUB(const PUB&);                                 \
void operator=(const PUB&);                      \
QCTK_DECLARE_PRIVATE(PUB);           

#endif //__qCTKSingleton_h

/****************************************************************************
 **
 ** Copyright (C) Qxt Foundation. Some rights reserved.
 **
 ** This file is part of the QxtCore module of the Qxt library.
 **
 ** This library is free software; you can redistribute it and/or modify it
 ** under the terms of the Common Public License, version 1.0, as published
 ** by IBM, and/or under the terms of the GNU Lesser General Public License,
 ** version 2.1, as published by the Free Software Foundation.
 **
 ** This file is provided "AS IS", without WARRANTIES OR CONDITIONS OF ANY
 ** KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
 ** WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY OR
 ** FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** You should have received a copy of the CPL and the LGPL along with this
 ** file. See the LICENSE file and the cpl1.0.txt/lgpl-2.1.txt files
 ** included with the source distribution for more information.
 ** If you did not receive a copy of the licenses, contact the Qxt Foundation.
 **
 ** <http://libqxt.org>  <foundation@libqxt.org>
 **
 ****************************************************************************/

/****************************************************************************
** This file is derived from code bearing the following notice:
** The sole author of this file, Adam Higerd, has explicitly disclaimed all
** copyright interest and protection for the content within. This file has
** been placed in the public domain according to United States copyright
** statute and case law. In jurisdictions where this public domain dedication
** is not legally recognized, anyone who receives a copy of this file is
** permitted to use, modify, duplicate, and redistribute this file, in whole
** or in part, with no restrictions or conditions. In these jurisdictions,
** this file shall be copyright (C) 2006-2008 by Adam Higerd.
****************************************************************************/

/**
\class qCTKPimpl qCTKPimpl
\ingroup QxtCore
\brief Hide private details of a class

Application code generally doesn't have to be concerned about hiding its
implementation details, but when writing library code it is important to
maintain a constant interface, both source and binary. Maintaining a constant
source interface is easy enough, but keeping the binary interface constant
means moving implementation details into a private class. The PIMPL, or
d-pointer, idiom is a common method of implementing this separation. qCTKPimpl
offers a convenient way to connect the public and private sides of your class.

\section start Getting Started
Before you declare the public class, you need to make a forward declaration
of the private class. The private class must have the same name as the public
class, followed by the word Private. For example, a class named MyTest would
declare the private class with:
\code
class MyTestPrivate;
\endcode

\subsection pub The Public Class
Generally, you shouldn't keep any data members in the public class without a
good reason. Functions that are part of the public interface should be declared
in the public class, and functions that need to be available to subclasses (for
calling or overriding) should be in the protected section of the public class.
To connect the private class to the public class, include the
QCTK_DECLARE_PRIVATE macro in the private section of the public class. In the
example above, the private class is connected as follows:
\code
private:
    QCTK_DECLARE_PRIVATE(MyTest);
\endcode

Additionally, you must include the QCTK_INIT_PRIVATE macro in the public class's
constructor. Continuing with the MyTest example, your constructor might look
like this:
\code
MyTest::MyTest() {
    // initialization
    QCTK_INIT_PRIVATE(MyTest);
}
\endcode

\subsection priv The Private Class
As mentioned above, data members should usually be kept in the private class.
This allows the memory layout of the private class to change without breaking
binary compatibility for the public class. Functions that exist only as
implementation details, or functions that need access to private data members,
should be implemented here.

To define the private class, inherit from the template qCTKPrivate class, and
include the QCTK_DECLARE_PUBLIC macro in its public section. The template
parameter should be the name of the public class. For example:
\code
class MyTestPrivate : public qCTKPrivate<MyTest> {
public:
    MyTestPrivate();
    QCTK_DECLARE_PUBLIC(MyTest);
};
\endcode

\section cross Accessing Private Members
Use the qctk_d() function (actually a function-like object) from functions in
the public class to access the private class. Similarly, functions in the
private class can invoke functions in the public class by using the qctk_p()
function (this one's actually a function).

For example, assume that MyTest has methods named getFoobar and doBaz(),
and MyTestPrivate has a member named foobar and a method named doQuux().
The code might resemble this example:
\code
int MyTest::getFoobar() {
    return qctk_d().foobar;
}

void MyTestPrivate::doQuux() {
    qctk_p().doBaz(foobar);
}
\endcode
*/

#ifndef __qCTKPimpl_h
#define __qCTKPimpl_h

/*! \relates qCTKPimpl
 * Declares that a public class has a related private class.
 *
 * This should be put in the private section of the public class. The parameter is the name of the public class.
 */
#define QCTK_DECLARE_PRIVATE(PUB) friend class PUB##Private; qCTKPrivateInterface<PUB, PUB##Private> qctk_d;
/*! \relates qCTKPimpl
 * Declares that a private class has a related public class.
 *
 * This may be put anywhere in the declaration of the private class. The parameter is the name of the public class.
 */
#define QCTK_DECLARE_PUBLIC(PUB) friend class PUB;
/*! \relates qCTKPimpl
 * Initializes resources owned by the private class.
 *
 * This should be called from the public class's constructor,
 * before qctk_d() is used for the first time. The parameter is the name of the public class.
 */
#define QCTK_INIT_PRIVATE(PUB) qctk_d.setPublic(this)
/*! \relates qCTKPimpl
 * Returns a reference in the current scope named "d" to the private class.
 *
 * This function is only available in a class using \a QCTK_DECLARE_PRIVATE.
 */
#define QCTK_D(PUB) PUB##Private& d = qctk_d()
/*! \relates qCTKPimpl
 * Creates a reference in the current scope named "q" to the public class.
 *
 * This macro only works in a class using \a QCTK_DECLARE_PUBLIC.
 */
#define QCTK_P(PUB) PUB& p = qctk_p()

#ifdef QCTK_DOXYGEN_RUN
/*! \relates qCTKPimpl
 * Returns a reference to the private class.
 *
 * This function is only available in a class using \a QCTK_DECLARE_PRIVATE.
 */
qCTKPrivate<PUB>& qctk_d();

/*! \relates qCTKPimpl
 * Returns a const reference to the private class.
 *
 * This function is only available in a class using \a QCTK_DECLARE_PRIVATE.
 * This overload will be automatically used in const functions.
 */
const qCTKPrivate<PUB>& qctk_d();

/*! \relates qCTKPimpl
 * Returns a reference to the public class.
 *
 * This function is only available in a class using \a QCTK_DECLARE_PUBLIC.
 */
PUB& qctk_p();

/*! \relates qCTKPimpl
 * Returns a const reference to the public class.
 *
 * This function is only available in a class using \a QCTK_DECLARE_PUBLIC.
 * This overload will be automatically used in const functions.
 */
const PUB& qctk_p();
#endif

#ifndef QCTK_DOXYGEN_RUN
template <typename PUB>
class qCTKPrivate
{
public:
    virtual ~qCTKPrivate()
    {}
    inline void QCTK_setPublic(PUB* pub)
    {
        qctk_p_ptr = pub;
    }

protected:
    inline PUB& qctk_p()
    {
        return *qctk_p_ptr;
    }
    inline const PUB& qctk_p() const
    {
        return *qctk_p_ptr;
    }

private:
    PUB* qctk_p_ptr;
};

template <typename PUB, typename PVT>
class qCTKPrivateInterface
{
    friend class qCTKPrivate<PUB>;
public:
    qCTKPrivateInterface()
    {
        pvt = new PVT;
    }
    ~qCTKPrivateInterface()
    {
        delete pvt;
    }

    inline void setPublic(PUB* pub)
    {
        pvt->QCTK_setPublic(pub);
    }
    inline PVT& operator()()
    {
        return *static_cast<PVT*>(pvt);
    }
    inline const PVT& operator()() const
    {
        return *static_cast<PVT*>(pvt);
    }
private:
    qCTKPrivate<PUB>* pvt;
};
#endif

#endif

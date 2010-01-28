/**
\class qCTKPimpl qCTKPimpl
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
class, followed by the word Private.

\subsection pub The Public Class
Generally, you shouldn't keep any data members in the public class without a
good reason. Functions that are part of the public interface should be declared
in the public class, and functions that need to be available to subclasses (for
calling or overriding) should be in the protected section of the public class.
To connect the private class to the public class, include the
QCTK_DECLARE_PRIVATE macro in the private section of the public class. In the
example above, the private class is connected as follows:

Additionally, you must include the QCTK_INIT_PRIVATE macro in the public class's
constructor.

\subsection priv The Private Class
As mentioned above, data members should usually be kept in the private class.
This allows the memory layout of the private class to change without breaking
binary compatibility for the public class. Functions that exist only as
implementation details, or functions that need access to private data members,
should be implemented here.

To define the private class, inherit from the template qCTKPrivate class, and
include the QCTK_DECLARE_PUBLIC macro in its public section. The template
parameter should be the name of the public class.

\section cross Accessing Private Members
Use the qctk_d() function (actually a function-like object) from functions in
the public class to access the private class. Similarly, functions in the
private class can invoke functions in the public class by using the qctk_p()
function (this one's actually a function).
*/

#ifndef __qCTKPimpl_h
#define __qCTKPimpl_h

#include <QtGlobal>

/*! \relates qCTKPimpl
 * Define a public class constructor with no argument
 *
 * Also make sure the Pimpl is initalized
 */
#define QCTK_CONSTRUCTOR_NO_ARG_CXX(PUB) \
  PUB::PUB()                             \
    {                                    \
    QCTK_INIT_PRIVATE(PUB);              \
    }

/*! \relates qCTKPimpl
 * Define a public class constructor with one argument
 *
 * Also make sure the Pimpl is initalized
 */
#define QCTK_CONSTRUCTOR_1_ARG_CXX(PUB, _ARG1)  \
  PUB::PUB(_ARG1 _parent) :                     \
    Superclass( _parent )                       \
    {                                           \
    QCTK_INIT_PRIVATE(PUB);                     \
    }
    
/*! \relates qCTKPimpl
 * Define the setter in the public class.
 *
 * This should be put in the .cxx file of the public class. The parameter are
 * the name of the public class (PUB), the type of the argument to return (_TYPE),
 * the name of the getter(_NAME) and the name of the variable in the Private class(_VARNAME).
 */
#define QCTK_SET_CXX(PUB, _TYPE, _NAME, _VARNAME)   \
  void PUB::_NAME(_TYPE var)                        \
  {                                                 \
    qctk_d.ref()._VARNAME =  var;                   \
  }

/*! \relates qCTKPimpl
 * Define the setter in the public class.
 *
 * This should be put in the .cxx file of the public class. The parameter are
 * the name of the public class (PUB), the type of the argument to return (_TYPE),
 * the name of the setter(_NAME) and the name of the variable in the Private class(_VARNAME).
 */
#define QCTK_GET_CXX(PUB, _TYPE, _NAME, _VARNAME)  \
  _TYPE PUB::_NAME()const                          \
  {                                                \
    return qctk_d.ref()._VARNAME;                  \
  }

/*! \relates qCTKPimpl
 * Declares that a public class has a related private class.
 *
 * This should be put in the private section of the public class. The parameter is the name of the public class.
 * For convenience, this macro also add 'typedef PUB Self;'
 */
#define QCTK_DECLARE_PRIVATE(PUB)                                          \
friend class PUB##Private; qCTKPrivateInterface<PUB, PUB##Private> qctk_d; \
typedef PUB Self;

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
 * Returns a pointer (or reference) in the current scope named "d" to the private class.
 *
 * This function is only available in a class using \a QCTK_DECLARE_PRIVATE.
 */
#define QCTK_D(PUB) PUB##Private* d = qctk_d()
#define QCTK_D_REF(PUB) PUB##Private& d = qctk_d.ref()
/*! \relates qCTKPimpl
 * Creates a pointer ( or reference) in the current scope named "q" to the public class.
 *
 * This macro only works in a class using \a QCTK_DECLARE_PUBLIC.
 */
#define QCTK_P(PUB) PUB* p = qctk_p()
#define QCTK_P_REF(PUB) PUB& p = qctk_p_ref()

#ifdef QCTK_DOXYGEN_RUN
/*! \relates qCTKPimpl
 * Returns a pointer to the private class.
 *
 * This function is only available in a class using \a QCTK_DECLARE_PRIVATE.
 */
qCTKPrivate<PUB>* qctk_d();

/*! \relates qCTKPimpl
 * Returns a const pointer to the private class.
 *
 * This function is only available in a class using \a QCTK_DECLARE_PRIVATE.
 * This overload will be automatically used in const functions.
 */
const qCTKPrivate<PUB>* qctk_d();

/*! \relates qCTKPimpl
 * Returns a reference or pointer to the public class.
 *
 * This function is only available in a class using \a QCTK_DECLARE_PUBLIC.
 */
PUB& qctk_p_ref();
PUB* qctk_p();

/*! \relates qCTKPimpl
 * Returns a const reference or pointer to the public class.
 *
 * This function is only available in a class using \a QCTK_DECLARE_PUBLIC.
 * This overload will be automatically used in const functions.
 */
const PUB& qctk_p_ref();
const PUB* qctk_p();
#endif

template <typename PUB>
class qCTKPrivate
{
public:
  virtual ~qCTKPrivate(){}
  inline void QCTK_setPublic(PUB* pub)
    {
    Q_ASSERT(pub);
    qctk_p_ptr = pub;
    }

protected:
  inline PUB& qctk_p_ref()
    {
    Q_ASSERT(this->qctk_p_ptr);
    return *this->qctk_p_ptr;
    }
  inline const PUB& qctk_p_ref() const
    {
    Q_ASSERT(this->qctk_p_ptr);
    return *this->qctk_p_ptr;
    }

  inline PUB* qctk_p()
    {
    Q_ASSERT(this->qctk_p_ptr);
    return this->qctk_p_ptr;
    }
  inline const PUB* qctk_p() const
    {
    Q_ASSERT(this->qctk_p_ptr);
    return this->qctk_p_ptr;
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
    this->pvt = new PVT;
    }
  ~qCTKPrivateInterface()
    {
    delete this->pvt;
    }

  inline void setPublic(PUB* pub)
    {
    Q_ASSERT(pub);
    this->pvt->QCTK_setPublic(pub);
    }
  inline PVT& ref()
    {
    return *static_cast<PVT*>(this->pvt);
    }
  inline const PVT& ref() const
    {
    return *static_cast<PVT*>(this->pvt);
    }
  inline PVT* operator()()
    {
    return static_cast<PVT*>(this->pvt);
    }
  inline const PVT* operator()() const
    {
    return static_cast<PVT*>(this->pvt);
    }
private:
  qCTKPrivate<PUB>* pvt;
};

#endif

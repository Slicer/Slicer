#ifndef __BRAINSMacro_h
#define __BRAINSMacro_h

/**
  * This class encapulates all the functionality needed to run the BRAINSFit
  *program without any command line options.
  *
  * It is a class that has all the functionality after the command line options
  *are processed and the images pre-processed, and
  * returns all binary versions of the objects for post-processing.
  *
  * NOTE:  This class is not templated!
  */

#if defined( ITK_LEAN_AND_MEAN ) || defined( __BORLANDC__ ) || defined( NDEBUG )
#  define VECTORitkDebugMacro(s, type, x)
#else
#  define VECTORitkDebugMacro(s, type, x)                               \
    {                                                                   \
    if ( this->GetDebug() && ::itk::Object::GetGlobalWarningDisplay() ) \
      {                                                                 \
      ::itk::OStringStream itkmsg;                                      \
      itkmsg << "Debug: In " __FILE__ ", line " << __LINE__ << "\n"     \
             << this->GetNameOfClass() << " (" << this << "):" << s;    \
      for ( type::const_iterator it = x.begin(); it != x.end(); it++ )  \
        {                                                               \
        itkmsg << " "                                                   \
               << *( it );                                              \
        }                                                               \
      itkmsg << "\n\n";                                                 \
      ::itk::OutputWindowDisplayDebugText( itkmsg.str().c_str() );      \
      }                                                                 \
    }
#endif
/** Set built-in type.  Creates member Set"name"() (e.g., SetVisibility()); */
#define VECTORitkSetMacro(name, type)                          \
  virtual void Set ## name (const type _arg)                   \
    {                                                          \
    VECTORitkDebugMacro("setting " # name " to ", type, _arg); \
    type::const_iterator fromIt = _arg.begin();                \
    type::const_iterator toIt = this->m_ ## name.begin();      \
      {                                                        \
      this->m_ ## name.resize( _arg.size() );                  \
      this->m_ ## name = _arg;                                 \
      this->Modified();                                        \
      }                                                        \
    }

/** Get built-in type.  Creates member Get"name"() (e.g., GetVisibility()); */
#define VECTORitkGetConstMacro(name, type)                                      \
  virtual type Get ## name ()                                                   \
    {                                                                           \
    VECTORitkDebugMacro("returning " << # name " of ", type, this->m_ ## name); \
    return this->m_ ## name;                                                    \
    }

#endif // __BRAINSMACRO_h

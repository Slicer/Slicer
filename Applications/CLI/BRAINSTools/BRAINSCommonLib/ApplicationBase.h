#ifndef __ApplicationBase_h
#define __ApplicationBase_h

#include "itkObjectFactory.h"
#include "itkObject.h"

namespace itk
{
/** \class ApplicationBase
  *
  * This class ties together an input parser, a preprocessor,
  * a registrator components to
  * form a deformable registration/atlas segmentation application.
  *
  */
template< typename TParser,
          typename TPreprocessor,
          typename TRegistrator >
class ApplicationBase:public Object
{
public:

  /** Standard class typedefs. */
  typedef ApplicationBase            Self;
  typedef Object                     Superclass;
  typedef SmartPointer< Self >       Pointer;
  typedef SmartPointer< const Self > ConstPointer;

  /** Run-time type information (and related methods). */
  itkTypeMacro(MIMApplication, Object);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Input parser type. */
  typedef TParser                      ParserType;
  typedef typename ParserType::Pointer ParserPointer;

  /** Preprocessor type. */
  typedef TPreprocessor                      PreprocessorType;
  typedef typename PreprocessorType::Pointer PreprocessorPointer;

  /** Registrator type. */
  typedef TRegistrator                      RegistratorType;
  typedef typename RegistratorType::Pointer RegistratorPointer;

  /** Set Debug mode */
  itkSetMacro(OutDebug, bool);
  itkGetConstMacro(OutDebug, bool);

  RegistratorType * GetRegistratorType(void)
  {
    return m_Registrator;
  }

  /** Execute the application. */
  virtual void Execute();

protected:

  ApplicationBase();
  virtual ~ApplicationBase() {}

  /** Initialize the input parser. */
  virtual void InitializeParser() {}

  /*** Initialize the preprocessor */
  virtual void InitializePreprocessor() {}

  /*** Initialize the registrator  */
  virtual void InitializeRegistrator() {}

  ParserPointer       m_Parser;
  PreprocessorPointer m_Preprocessor;
  RegistratorPointer  m_Registrator;
  bool                m_OutDebug;
};
}   // namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "ApplicationBase.txx"
#endif

#endif

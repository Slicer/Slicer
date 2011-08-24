#ifndef __RegistrationMonitor_h
#define __RegistrationMonitor_h

#include <itkRealTimeClock.h>
#include <itkCommand.h>
#include <itkSingleValuedNonLinearOptimizer.h>

class ImageRegistrationViewer
  : public itk::Command
{
public:
  typedef ImageRegistrationViewer Self;
  typedef itk::Command            Superclass;
  typedef itk::SmartPointer<Self> Pointer;

  itkTypeMacro( ImageRegistrationViewer, Command );

  itkNewMacro( ImageRegistrationViewer );

  typedef itk::SingleValuedNonLinearOptimizer OptimizerType;

  itkSetMacro(DontShowParameters, bool);
  itkSetMacro(UpdateInterval, int);

  void Execute( itk::Object * caller, const itk::EventObject & event )
  {
    Execute( (const Object *)caller, event );
  }

  void Execute( const itk::Object * object, const itk::EventObject & event )
  {
    if( typeid( event ) == typeid( itk::StartEvent ) )
      {
      this->Reset();
      return;
      }

    if( typeid( event ) != typeid( itk::IterationEvent ) || object == NULL )
      {
      return;
      }

    const OptimizerType * opt = dynamic_cast<const OptimizerType *>(object);

    if( ++m_Iteration % m_UpdateInterval == 0 )
      {
#if ITK_VERSION_MAJOR < 4
      itk::RealTimeClock::TimeStampType t = m_Clock->GetTimeStamp();
#else
      itk::RealTimeClock::TimeStampType t = m_Clock->GetTimeInSeconds();
#endif
      if( !m_DontShowParameters )
        {
        std::cout << "   " << m_Iteration << " : "
                  << opt->GetCurrentPosition() << " = "
                  << opt->GetValue( opt->GetCurrentPosition() )
                  << "   (" << (t - m_LastTime) / m_UpdateInterval << "s)"
                  << std::endl;
        }
      else
        {
        std::cout << "   " << m_Iteration << " : "
                  << opt->GetValue( opt->GetCurrentPosition() )
                  << "   (" << (t - m_LastTime) / m_UpdateInterval << "s)"
                  << std::endl;
        }
      m_LastTime = t;
      }
  }

  void Update()
  {
    this->Execute( (const Object *)NULL, itk::IterationEvent() );
  }

  void Reset()
  {
#if ITK_VERSION_MAJOR < 4
    m_LastTime = m_Clock->GetTimeStamp();
#else
    m_LastTime = m_Clock->GetTimeInSeconds();
#endif
    this->m_Iteration = 0;
    this->m_UpdateInterval = 1;
    this->m_DontShowParameters = false;

  }

protected:

  itk::RealTimeClock::Pointer       m_Clock;
  itk::RealTimeClock::TimeStampType m_LastTime;

  int  m_Iteration;
  int  m_UpdateInterval;
  bool m_DontShowParameters;

  ImageRegistrationViewer()
  {
    m_Clock = itk::RealTimeClock::New();
#if ITK_VERSION_MAJOR < 4
    m_LastTime = m_Clock->GetTimeStamp();
#else
    m_LastTime = m_Clock->GetTimeInSeconds();
#endif
    m_Iteration = 0;
    m_UpdateInterval = 1;
    m_DontShowParameters = false;
  };
  ~ImageRegistrationViewer()
  {
  };

};

#endif

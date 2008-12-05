/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkPluginFilterWatcher.h,v $
  Language:  C++
  Date:      $Date: 2007/04/09 02:26:30 $
  Version:   $Revision: 1.1 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkPluginFilterWatcher_h
#define __itkPluginFilterWatcher_h

#include "itkSimpleFilterWatcher.h"
#include "ModuleProcessInformation.h"

namespace itk
{

/** \class PluginFilterWatcher
 * \brief Simple mechanism for monitoring the pipeline events of a
 * filter and reporting these events to std::cout. Formats reports
 * with xml.
 */
class PluginFilterWatcher: public SimpleFilterWatcher
{
public:
  PluginFilterWatcher(itk::ProcessObject* o,
                      const char *comment="",
                      ModuleProcessInformation *inf=0,
                      double fraction = 1.0,
                      double start = 0.0)
    : SimpleFilterWatcher(o, comment)
  {
    m_ProcessInformation = inf;
    m_Fraction = fraction;
    m_Start = start;
  };

protected:

/** Callback method to show the ProgressEvent */
virtual void ShowProgress()
{
  if (this->GetProcess())
    {
    this->SetSteps( this->GetSteps()+1 );
    if (!this->GetQuiet())
      {
      if (m_ProcessInformation)
        {
        strncpy(m_ProcessInformation->ProgressMessage,
                this->GetComment().c_str(), 1023);
        m_ProcessInformation->Progress = 
          (this->GetProcess()->GetProgress() * m_Fraction + m_Start);
        if (m_Fraction != 1.0)
          {
          m_ProcessInformation->StageProgress = this->GetProcess()->GetProgress();
          }

        this->GetTimeProbe().Stop();
        m_ProcessInformation->ElapsedTime
          = this->GetTimeProbe().GetMeanTime()
          * this->GetTimeProbe().GetNumberOfStops();
        this->GetTimeProbe().Start();

        if (m_ProcessInformation->Abort)
          {
          this->GetProcess()->AbortGenerateDataOn();
          m_ProcessInformation->Progress = 0;
          m_ProcessInformation->StageProgress = 0;
          }

        if (m_ProcessInformation->ProgressCallbackFunction
            && m_ProcessInformation->ProgressCallbackClientData)
          {
          (*(m_ProcessInformation->ProgressCallbackFunction))(m_ProcessInformation->ProgressCallbackClientData);
          }
        }
      else
        {
        std::cout << "<filter-progress>"
                  << (this->GetProcess()->GetProgress() * m_Fraction) + m_Start
                  << "</filter-progress>"
                  << std::endl;
        if (m_Fraction != 1.0)
          {
          std::cout << "<filter-stage-progress>"
                    << this->GetProcess()->GetProgress() 
                    << "</filter-stage-progress>"
                    << std::endl;
          }
        std::cout << std::flush;
        }
      }
    }
}

/** Callback method to show the StartEvent */
virtual void StartFilter()
{
  this->SetSteps(0);
  this->SetIterations(0);
  this->GetTimeProbe().Start();
  if (!this->GetQuiet())
    {
    if (m_ProcessInformation)
      {
      m_ProcessInformation->Progress = 0;
      m_ProcessInformation->StageProgress = 0;
      strncpy(m_ProcessInformation->ProgressMessage,
              this->GetComment().c_str(), 1023);
      
      if (m_ProcessInformation->ProgressCallbackFunction
          && m_ProcessInformation->ProgressCallbackClientData)
        {
        (*(m_ProcessInformation->ProgressCallbackFunction))(m_ProcessInformation->ProgressCallbackClientData);
        }
      }
    else
      {
      std::cout << "<filter-start>"
                << std::endl;
      std::cout << "<filter-name>"
                << (this->GetProcess()
                    ? this->GetProcess()->GetNameOfClass() : "None")
                << "</filter-name>"
                << std::endl;
      std::cout << "<filter-comment>"
                << " \"" << this->GetComment() << "\" "
                << "</filter-comment>"
                << std::endl;
      std::cout << "</filter-start>"
                << std::endl;
      std::cout << std::flush;
      }
    }
}

/** Callback method to show the EndEvent */
virtual void EndFilter()
{
  this->GetTimeProbe().Stop();
  if (!this->GetQuiet())
    {
    if (m_ProcessInformation)
      {
      m_ProcessInformation->Progress = 0;
      m_ProcessInformation->StageProgress = 0;

      m_ProcessInformation->ElapsedTime
        = this->GetTimeProbe().GetMeanTime()
        * this->GetTimeProbe().GetNumberOfStops();
      
      if (m_ProcessInformation->ProgressCallbackFunction
          && m_ProcessInformation->ProgressCallbackClientData)
        {
        (*(m_ProcessInformation->ProgressCallbackFunction))(m_ProcessInformation->ProgressCallbackClientData);
        }
      }
    else
      {
      std::cout << "<filter-end>"
                << std::endl;
      std::cout << "<filter-name>"
                << (this->GetProcess()
                    ? this->GetProcess()->GetNameOfClass() : "None")
                << "</filter-name>"
                << std::endl;
      std::cout << "<filter-time>"
                << this->GetTimeProbe().GetMeanTime()
                << "</filter-time>"
                << std::endl;
      std::cout << "</filter-end>";
      std::cout << std::flush;
      }
    }
}


  ModuleProcessInformation *m_ProcessInformation;
  double m_Fraction;
  double m_Start;
};

} // end namespace itk

#endif

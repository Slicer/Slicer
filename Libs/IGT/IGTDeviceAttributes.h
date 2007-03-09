
#ifndef IGTAURORATRACKER_H
#define IGTAURORATRACKER_H

#include <string>

/**
  * class IGTAuroraTracker
  */

class IGTAuroraTracker
{
public:

  // Constructors/Destructors
  //  


  /**
   * Empty Constructor
   */
  IGTAuroraTracker ( );

  /**
   * Empty Destructor
   */
  virtual ~IGTAuroraTracker ( );

  // Static Public attributes
  //  

  // Public attributes
  //  


  // Public attribute accessor methods
  //  


  // Public attribute accessor methods
  //  


protected:

  // Static Protected attributes
  //  

  // Protected attributes
  //  


  // Protected attribute accessor methods
  //  


  // Protected attribute accessor methods
  //  


private:

  // Static Private attributes
  //  

  // Private attributes
  //  


  // Private attribute accessor methods
  //  


  // Private attribute accessor methods
  //  



};

#endif // IGTAURORATRACKER_H


#ifndef IGTDEVICEATTRIBUTES_H
#define IGTDEVICEATTRIBUTES_H

#include <string>

/**
  * class IGTDeviceAttributes
  */

class IGTDeviceAttributes
{
public:

  // Constructors/Destructors
  //  


  /**
   * Empty Constructor
   */
  IGTDeviceAttributes ( );

  /**
   * Empty Destructor
   */
  virtual ~IGTDeviceAttributes ( );

  // Static Public attributes
  //  

  // Public attributes
  //  


  // Public attribute accessor methods
  //  


  // Public attribute accessor methods
  //  


protected:

  // Static Protected attributes
  //  

  // Protected attributes
  //  


  // Protected attribute accessor methods
  //  


  // Protected attribute accessor methods
  //  


private:

  // Static Private attributes
  //  

  // Private attributes
  //  

  int m_status;

  // Private attribute accessor methods
  //  


  // Private attribute accessor methods
  //  


  /**
   * Set the value of m_status
   * @param new_var the new value of m_status
   */
  void setStatus ( int new_var );

  /**
   * Get the value of m_status
   * @return the value of m_status
   */
  int getStatus ( );


  void initAttributes ( ) ;

};

#endif // IGTDEVICEATTRIBUTES_H

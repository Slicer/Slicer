
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


#ifndef IGTROBOTFRAMESTREAM_H
#define IGTROBOTFRAMESTREAM_H

#include <string>

/**
  * class IGTRobotFrameStream
  */

class IGTRobotFrameStream
{
public:

  // Constructors/Destructors
  //  


  /**
   * Empty Constructor
   */
  IGTRobotFrameStream ( );

  /**
   * Empty Destructor
   */
  virtual ~IGTRobotFrameStream ( );

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

  IGTRobotFrameAttributes m_RobotAttributeBuffer;

  // Private attribute accessor methods
  //  


  // Private attribute accessor methods
  //  


  /**
   * Set the value of m_RobotAttributeBuffer
   * @param new_var the new value of m_RobotAttributeBuffer
   */
  void setRobotAttributeBuffer ( IGTRobotFrameAttributes new_var );

  /**
   * Get the value of m_RobotAttributeBuffer
   * @return the value of m_RobotAttributeBuffer
   */
  IGTRobotFrameAttributes getRobotAttributeBuffer ( );


  void initAttributes ( ) ;

};

#endif // IGTROBOTFRAMESTREAM_H

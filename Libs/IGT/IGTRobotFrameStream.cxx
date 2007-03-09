#include "igtauroratracker.h"

// Constructors/Destructors
//  

IGTAuroraTracker::IGTAuroraTracker ( ) {
}

IGTAuroraTracker::~IGTAuroraTracker ( ) { }

//  
// Methods
//  


// Accessor methods
//  


// Other methods
//  



#include "igtrobotframestream.h"

// Constructors/Destructors
//  

IGTRobotFrameStream::IGTRobotFrameStream ( ) {
initAttributes();
}

IGTRobotFrameStream::~IGTRobotFrameStream ( ) { }

//  
// Methods
//  


// Accessor methods
//  


// Public static attribute accessor methods
//  


// Public attribute accessor methods
//  


// Protected static attribute accessor methods
//  


// Protected attribute accessor methods
//  


// Private static attribute accessor methods
//  


// Private attribute accessor methods
//  


/**
 * Set the value of m_RobotAttributeBuffer
 * @param new_var the new value of m_RobotAttributeBuffer
 */
void IGTRobotFrameStream::setRobotAttributeBuffer ( IGTRobotFrameAttributes new_var ) {
  m_RobotAttributeBuffer = new_var;
}

/**
 * Get the value of m_RobotAttributeBuffer
 * @return the value of m_RobotAttributeBuffer
 */
IGTRobotFrameAttributes IGTRobotFrameStream::getRobotAttributeBuffer ( ) {
  return m_RobotAttributeBuffer;
}

// Other methods
//  

void IGTRobotFrameStream::initAttributes ( ) {
}


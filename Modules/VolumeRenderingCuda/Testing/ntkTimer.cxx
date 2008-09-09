#include "ntkTimer.h"


#ifdef WIN32
 #include <windows.h>

#pragma comment(lib, "Winmm")

extern "C"
  static int gettimeofday
      (struct timeval* tp, void* tzp) {
    DWORD t;
    t = timeGetTime();
    tp->tv_sec = t / 1000;
    tp->tv_usec = 1000 * (t % 1000 );
    /* 0 indicates success. */
    return 0;
}
#else
 #include <sys/time.h>
#endif

#include <stdio.h>

ntkTimer::ntkTimer(){
  m_startTime=0;
  m_endTime=0;
}

ntkTimer::~ntkTimer(){

}

void ntkTimer::Start(){
  m_startTime=gethrtime();
}

void ntkTimer::End(){
  m_endTime=gethrtime();
}

void ntkTimer::Reset(){
  m_startTime=0;
  m_endTime=0;
}

long long ntkTimer::getElapsedTime(){
  return (m_endTime-m_startTime)/1000;
}

long long ntkTimer::getStartTime(){
  return m_startTime;
}

long long ntkTimer::getEndTime(){
  return m_endTime;
}

void ntkTimer::printElapsedTime(char* text=""){
  printf("%s Elapsed Time: %d ms\n", text, (int)((m_endTime-m_startTime)/1000));
}

void ntkTimer::printDetailedElapsedTime(char* text=""){
  printf("%s Elapsed Time: %lf ms\n", text, ((m_endTime-m_startTime)/1000.0));
}

void ntkTimer::printElapsedTime(long long startTime, long long endTime, char* text=""){
  printf("%s Elapsed Time: %d ms", text, (int)((endTime-startTime)/1000));
}

void ntkTimer::printDetailedElapsedTime(long long startTime, long long endTime, char* text=""){
  printf("%s Elapsed Time: %lf ms", text, ((endTime-startTime)/1000.0));
}

long long ntkTimer::gethrtime(){
  struct timeval t;
  gettimeofday(&t, NULL);
  return t.tv_usec+t.tv_sec*1000000;
}

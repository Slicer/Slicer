//$$ controlw.h                Control word class

#ifndef CONTROL_WORD_LIB
#define CONTROL_WORD_LIB 0

// for organising an int as a series of bits which indicate whether an
// option is on or off.

class ControlWord
{
protected:
   int cw;                                      // the control word
public:
   ControlWord() : cw(0) {}                     // do nothing
   ControlWord(int i) : cw(i) {}                // load an integer

      // select specific bits (for testing at least one set)
   ControlWord operator*(ControlWord i) const
      { return ControlWord(cw & i.cw); }
   void operator*=(ControlWord i)  { cw &= i.cw; }

      // set bits
   ControlWord operator+(ControlWord i) const
      { return ControlWord(cw | i.cw); }
   void operator+=(ControlWord i)  { cw |= i.cw; }

      // reset bits
   ControlWord operator-(ControlWord i) const
      { return ControlWord(cw - (cw & i.cw)); }
   void operator-=(ControlWord i) { cw -= (cw & i.cw); }

      // check if all of selected bits set or reset
   bool operator>=(ControlWord i) const { return (cw & i.cw) == i.cw; }
   bool operator<=(ControlWord i) const { return (cw & i.cw) == cw; }

      // flip selected bits
   ControlWord operator^(ControlWord i) const
      { return ControlWord(cw ^ i.cw); }
   ControlWord operator~() const { return ControlWord(~cw); }

      // convert to integer
   int operator+() const { return cw; }
   int operator!() const { return cw==0; }
   FREE_CHECK(ControlWord)
};


#endif

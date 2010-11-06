#ifndef RVGINTFLOAT_H
#define RVGINTFLOAT_H 1

/*
IntFloat is a mutable type that adjusts to storing int or float value
to match whatever the expresion it is used in resolves to. */

class IntFloat
{
  union Data
  {
    int i;
    float f;
  };

public:
  Data data;
  bool isFloat;

  //Constructors and assignment
  //(set current type based on given value)

  IntFloat () {
    data.i = 0; isFloat = false;
  }

  IntFloat (int i) {
    data.i = i; isFloat = false;
  }

  IntFloat (float f) {
    data.f = f; isFloat = true;
  }

  IntFloat& operator= (int i) {
    data.i = i; isFloat = false;
    return *this;
  }

  IntFloat& operator= (float f) {
    data.f = f; isFloat = true;
    return *this;
  }

  //Cast and comparison operators
  //(use appropriate variable based on current type)
  /*
  operator int () const {
    return (isFloat ? (int)data.f : data.i);
  }

  operator float () const {
    return (isFloat ? data.f : (float)data.i);
  }*/
  int toInt () const {
    return (isFloat ? (int)data.f : data.i);
  }

  float toFloat () const {
    return (isFloat ? data.f : (float)data.i);
  }

  bool operator == (int i) const {
    return (isFloat == false && data.i == i);
  }

  bool operator == (float f) const {
    return (isFloat == true && data.f == f);
  }

  bool operator == (const IntFloat &d) const {
    if (isFloat || d.isFloat) return (toFloat() == d.toFloat());
    else return (data.i == d.data.i);
  }

  bool operator != (const IntFloat &d) const {
    return !(this->operator==(d));
  }

  //Numerical operators +  -  /  *
  //(switch type to whatever the result evaluates to)

  IntFloat operator + (const IntFloat &d) {
    if (isFloat || d.isFloat) return IntFloat( toFloat() + d.toFloat());
    else return IntFloat( data.i + d.data.i );
  }

  IntFloat operator - (const IntFloat &d) {
    if (isFloat || d.isFloat) return IntFloat( toFloat() - d.toFloat() );
    else return IntFloat( data.i - d.data.i );
  }

  IntFloat operator * (const IntFloat &d) {
    if (isFloat || d.isFloat) return IntFloat( toFloat() * d.toFloat() );
    else return IntFloat( data.i * d.data.i );
  }

  IntFloat operator / (const IntFloat &d) {
    if (isFloat || d.isFloat) return IntFloat( toFloat() * d.toFloat() );
    else return IntFloat( data.i / d.data.i );
  }

  //Numerical assignment operators +=  -=  /=  *=
  //(switch type to whatever the result evaluates to)

  IntFloat& operator += (const IntFloat &d) {
    if (isFloat || d.isFloat) operator=( toFloat() + d.toFloat() );
    else operator=( data.i + d.data.i );
    return *this;
  }

  IntFloat& operator -= (const IntFloat &d) {
    if (isFloat || d.isFloat) operator=( toFloat() - d.toFloat() );
    else operator=( data.i - d.data.i );
    return *this;
  }

  IntFloat& operator *= (const IntFloat &d) {
    if (isFloat || d.isFloat) operator=( toFloat() * d.toFloat() );
    else operator=( data.i * d.data.i );
    return *this;
  }

  IntFloat& operator /= (const IntFloat &d) {
    if (isFloat || d.isFloat) operator=( toFloat() * d.toFloat() );
    else operator=( data.i / d.data.i );
    return *this;
  }
};


inline std::ostream& operator << (std::ostream &str, const IntFloat &d) {
  return (d.isFloat ? str << d.toFloat() : str << d.toInt());
}

#endif//RVGINTFLOAT_H

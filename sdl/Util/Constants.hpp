














#include <limits>
#include <cstddef>







// numeric limits class
template <class T>
class FloatLimits {
 public:
  static const T posInfinity;
  static const T negInfinity;
  static const T bad;
  static const T max;
};

template <class T>
const T FloatLimits<T>::posInfinity = std::numeric_limits<T>::infinity();

template <class T>
const T FloatLimits<T>::negInfinity = -FloatLimits<T>::posInfinity;

template <class T>
const T FloatLimits<T>::bad = std::numeric_limits<T>::quiet_NaN();

template <class T>
const T FloatLimits<T>::max = std::numeric_limits<T>::max();

template <class T>
class FloatConstants {
 public:
  static const T epsilon;
};

template <class T>

























































#endif

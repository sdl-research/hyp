










#include <vector>





namespace Util {









template <class T>

 private:











  Matrix(const Matrix<T>& other)


  ~Matrix() {}







































    container_.clear();
    rows_ = n;
    cols_ = m;
    container_.resize(n * m, default_value);
  }










};

template <class T>
std::ostream& operator<<(std::ostream& out, const Matrix<T>& m) {
  bool first = true;






      out << i << "," << j << ": " << m(i, j);
      first = false;
    }
  }
  return out;
}






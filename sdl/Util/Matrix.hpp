










#include <vector>





namespace Util {

// TODO: Could make it a Matrix class member.
template <class T>
class MatrixVisitor {
 public:
  virtual ~MatrixVisitor() {}
  virtual void visit(T&) = 0;
};

template <class T>

 private:




  typedef T value_type;






  Matrix(const Matrix<T>& other)


  ~Matrix() {}







































    container_.clear();
    rows_ = n;
    cols_ = m;
    container_.resize(n * m, default_value);
  }

  void accept(MatrixVisitor<T>& visitor) {


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






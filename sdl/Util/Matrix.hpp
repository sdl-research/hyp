// Copyright 2014-2015 SDL plc
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//     http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
/** \file

    2d matrix in contiguous memory

    \author mdreyer, graehl
*/

#ifndef SDL_UTIL_MATRIX_HPP
#define SDL_UTIL_MATRIX_HPP
#pragma once

#include <vector>
#include <algorithm>
#include <sdl/Util/Debug.hpp>
#include <sdl/Array.hpp>

namespace sdl {
namespace Util {

// TODO: Could make it a Matrix class member.
template <class T>
class MatrixVisitor {
 public:
  virtual ~MatrixVisitor() {}
  virtual void visit(T&) = 0;
};

template <class T>
struct Matrix {
 private:
  typedef std::vector<T> Array;

 public:
  typedef std::size_t size_type;
  typedef T value_type;

  typedef std::pair<size_type, size_type> Dimensions;

  Matrix(size_type n, size_type m, T const& default_value = T())
      : rows_(n), cols_(m), container_(n * m, default_value) {}

  // default copy, move, assign, etc

  friend inline void swap(Matrix & x1, Matrix & x2) {
    x1.swap(x2);
  }

  void swap(Matrix &o) {
    size_type t;
    t = o.rows_; o.rows_ = rows_; rows_ = t;
    t = o.cols_; o.cols_ = cols_; cols_ = t;
    container_.swap(o.container_);
  }


  void setJustDiagonal(T const& onDiag) {
    size_type const N = std::min(rows_, cols_);
    if (N) {
      size_type const stride = cols_ + 1;
      T* a = arrayBegin(container_);
      a[0] = onDiag;
      // e.g. square N*N: (stride=N+1)*(N-1) = N^2 - 1 = size - 1
      for (size_type i = stride, toofar = stride * N; i < toofar; i += stride) a[i] = onDiag;
    }
  }
  void setDiagonal(T const& onDiag, T const& offDiag = T()) {
    fill(offDiag);
    setJustDiagonal(onDiag);
  }
  void fill(T const& val) { std::fill(container_.begin(), container_.end(), val); }

  /**
     for repeated access to columns of the same row, it's faster to cache this pointer to (row,0).

     \return this->row(r)[col] == (*this)(r, col)
  */
  T* row(size_type row) { return &container_[cols_ * row]; }

  T const* row(size_type row) const { return const_cast<Matrix<T>*>(this)->row(row); }

  T& operator()(size_type row, size_type col) { return container_[cols_ * row + col]; }

  T const& operator()(size_type row, size_type col) const { return const_cast<Matrix<T>&>(*this)(row, col); }

  Dimensions size() const { return Dimensions(rows_, cols_); }

  size_type getNumRows() const { return rows_; }

  size_type getNumCols() const { return cols_; }

  size_type getEntryId(size_type row, size_type col) const { return cols_ * row + col; }

  void reset(size_type n, size_type m, T default_value = T()) {
    container_.clear();
    rows_ = n;
    cols_ = m;
    container_.resize(n * m, default_value);
  }

  void accept(MatrixVisitor<T>& visitor) {
    for (typename Array::iterator i = container_.begin(), e = container_.end(); i != e; ++i)
      visitor.visit(*i);
  }

 private:
  size_type rows_;
  size_type cols_;
  Array container_;
};

template <class T>
std::ostream& operator<<(std::ostream& out, Matrix<T> const& m) {
  bool first = true;
  typedef typename Matrix<T>::size_type size_type;
  for (size_type i = 0; i < m.getNumRows(); ++i) {
    for (size_type j = 0; j < m.getNumCols(); ++j) {
      if (!first) {
        out << '\n';
      }
      out << i << "," << j << ": " << m(i, j);
      first = false;
    }
  }
  return out;
}


}}

#endif

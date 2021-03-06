// -*- C++ -*-

#ifndef _LLAMA_COMPLEX_MATRIX_H_
#define _LLAMA_COMPLEX_MATRIX_H_

#include "Submatrix.h"
#include "Random.h"
#include "EigenDecomposition.h"
#include "f77fun.h"

namespace Llama {

  inline void 
  Matrix<Complex>::_copy (Complex *dest, const Index incd, const Complex *src, 
			  const Index incs, const Index size)
  {
    CBLAS_FUN(zcopy)(size, src, incs, dest, incd);
  }

  inline void 
  Matrix<Complex>::_axpy (Complex *y, const Index incy, const Complex a, const Complex *x, 
			  const Index incx, const Index size)
  {
    CBLAS_FUN(zaxpy)(size, &a, x, incx, y, incy);
  }

  inline void
  Matrix<Complex>::_scal (Complex *x, const Index inc, const Complex a, const Index size)
  {
    CBLAS_FUN(zscal)(size, &a, x, inc);
  }

  inline void
  Matrix<Complex>::_gemm (const Transpose_t lt, const Transpose_t rt, 
			  const Index m, const Index n, const Index k,
			  const Complex alpha, 
			  const Complex *a, const Index lda,
			  const Complex *b, const Index ldb,
			  const Complex beta,
			  Complex *c, const Index ldc) {

    CBLAS_FUN(zgemm)(COLMAJOR, lt, rt, m, n, k, &alpha, a, lda, b, ldb, 
		     &beta, c, ldc );

  };

  inline Matrix<Real> *
  real (const CAbstMatrix<Complex>& x)
  {
    Matrix<Real> *y = new Matrix<Real>(x.rows(), x.cols());
    for (Index k = 0; k < x.cols(); k++)
      for (Index j= 0; j < x.rows(); j++)
	(*y)(j,k) = real(x(j,k));
    return y;
  }

  inline Matrix<Real> *
  imag (const CAbstMatrix<Complex>& x)
  {
    Matrix<Real> *y = new Matrix<Real>(x.rows(), x.cols());
    for (Index k = 0; k < x.cols(); k++)
      for (Index j= 0; j < x.rows(); j++)
	(*y)(j,k) = imag(x(j,k));
    return y;
  }

  inline Matrix<Complex> *
  conj (const CAbstMatrix<Complex>& x)
  {
    Matrix<Complex> *y = new Matrix<Complex>(x);
    for (Index k = 0; k < x.size(); k++) (*y)(k) = conj((*y)(k));
    return y;
  }

  inline Matrix<Real> *
  abs (const CAbstMatrix<Complex>& x)
  {
    Matrix<Real> *t = new Matrix<Real>(x.rows(), x.cols());
    for (Index k = 0; k < x.cols(); k++)
      for (Index j = 0; j < x.rows(); j++) 
	(*t)(j,k) = abs(x(j,k));
    return t;
  }

  inline Matrix<Real> *
  arg (const CAbstMatrix<Complex>& x)
  {
    Matrix<Real> *t = new Matrix<Real>(x.rows(), x.cols());
    for (Index k = 0; k < x.cols(); k++)
      for (Index j = 0; j < x.rows(); j++) 
	(*t)(j,k) = arg(x(j,k));
    return t;
  }

  inline Real
  Matrix<Complex>::frobnorm (void) const
  {
    char norm = 'F';
    int m = this->rows(), n = this->cols();
    Real t = F77_FUN(zlange)(&norm, &m, &n, this->data(), &m, 0);
    return t;
  }

  inline Real
  Matrix<Complex>::norm1 (void) const
  {
    char norm = '1';
    int m = this->rows(), n = this->cols();
    Real t = F77_FUN(zlange)(&norm, &m, &n, this->data(), &m, 0);
    return t;
  }

  inline Real
  Matrix<Complex>::norminf (void) const
  {
    try {
      char norm = 'I';
      int m = this->rows(), n = this->cols();
      Real *work = 0, t;
      try {
	work = new Real[m];
      } catch (std::bad_alloc) {
	throw OutOfMemory();
      }
      t = F77_FUN(zlange)(&norm, &m, &n, this->data(), &m, work);
      delete[] work;
      return t;
    } catch (Exception e) {
      std::cerr << e;
      throw Exception("matrix infinity norm");
    }
  }

  inline Real
  Matrix<Complex>::norm (void) const
  {
    Real x = CBLAS_FUN(dznrm2)(this->size(), this->data(), 1);
    return x;
  }

  inline Real
  Matrix<Complex>::sumnorm (void) const
  {
    Real t = CBLAS_FUN(dzasum)(this->size(), this->data(), 1);
    return t;
  }

  /**
     Ordinary inner product with another vector.
  */
  inline Complex
  Matrix<Complex>::dot (const Matrix<Complex>& x) const
  {
    Complex d = 0;
    if (this->size() != x.size())
      throw DimError("complex dot product");
    CBLAS_FUN(zdotc_sub)(this->size(), this->data(), 1, x.data(), 1, &d);
    return d;
  }

  /**
     Find the roots of a single-variable polynomial of degree n,
     the coefficients of which are stored in the matrix x, which
     is in column-vector format and has length n+1. 
     The polynomial corresponding to a is 
     a(0) x^n + a(1) x^(n-1) + ... + a(n-2) x + a(n).
  */
  inline Matrix<Complex> *
  roots (const Matrix<Real>& a)
  {
    Matrix<Real> *c = companion(a);
    Matrix<Complex> *w = eigenvals(*c);
    delete c;
    return w;
  }

  inline Matrix<Complex>&
  Matrix<Complex>::operator = (Random& r)
  {
    for (Index k = 0; k < this->size(); k++)
      (*this)(k) = Complex(r(), r());
    return *this;
  }

  inline Submatrix<Complex>&
  Submatrix<Complex>::operator = (Random& r)
  {
    for (Index j = 0; j < this->rows(); j++) 
      for (Index k = 0; k < this->cols(); k++) 
	(*this)(j,k) = Complex(r(), r());

    return *this;
  }

}

#endif

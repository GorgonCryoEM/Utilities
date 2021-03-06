// -*- C++ -*-

#ifndef   _LLAMA_MAPPING_H_
#define   _LLAMA_MAPPING_H_

#include "Decls.h"
#include "General.h"
#include "Exception.h"
#include "Matrix.h"
#include "Range.h"
#include "Submatrix.h"
#include "SMClosure.h"
#include "MMClosure.h"
#include "AbstMapping.h"
#include "Diffeomorphism.h"
#include "machinu.h"

namespace Llama {

  /**
     Mapping class.  Implements a mapping from R^N x R^P -> R^N */
  template <class Scalar>
  class Mapping : public AbstMapping<Scalar>,
		  public Diffeomorphism<Scalar> {

  private:

    // Implementation
    Index _itmap;
    Matrix<Scalar> *_work;

    struct {			// control parameters for 
      Index njac, nchord;		// Newton-Raphson iterations
      Real tol, mina, safe;
    } _controls;

    // Iterate the map forward once
    void advance (Matrix<Scalar> *x) const {
      Diffeomorphism<Scalar>::operator() (_work, *x);
      for (Index k = 0; k < this->neqs(); k++) (*x)(k) = (*_work)(k);
    }

  public:

    // Default constructor
  
    Mapping (void) : Diffeomorphism<Scalar>("undefined", 0, 0, 0, 0) {
      _itmap = 0;
      _work = 0;
      _controls.njac   = 0;
      _controls.nchord = 0;
      _controls.tol    = 0;
      _controls.mina   = 0;
      _controls.safe   = 0;
    };

    // Basic constructor
    /**
       Construct a new mapping.  The arguments are:
       1) a string descriptive of the mapping,
       2) the dimension of the state space,
       3) the dimension of the parameter space,
       4) the routine which implements the mapping,
       5) (optional) a routine which calculates the Jacobian of the mapping,
       6) (optional) the number of times the diffeomorphism is to be iterated (default = 1).
    */
    Mapping (const std::string& what, Index N, Index P, 
	     typename Diffeomorphism<Scalar>::diffeo_fcn f, 
	     typename Diffeomorphism<Scalar>::diffeo_jac j = 0, 
	     Index iter = 1) 
      : Diffeomorphism<Scalar>(what, N+P, N, f, j), _itmap(iter) {

      // Allocate working memory
      _work = new Matrix<Scalar>(N);

      // Set default values of control parameters
      _controls.njac = 20;
      _controls.nchord = 10;
      _controls.tol = 1.0e-12;
      _controls.mina = 1.0e-9;
      _controls.safe = 10.0 * MACH_EPS;

    };

    /**
       Copy constructor
    */
    Mapping (const Mapping& m) 
      : Diffeomorphism<Scalar>(m), _itmap(m.itmap()) {
      try {
	_work = new Matrix<Scalar>(m.neqs());
      } catch (std::bad_alloc) {
	throw OutOfMemory("mapping copy");
      }
      _controls.njac   = m._controls.njac;
      _controls.nchord = m._controls.nchord;
      _controls.tol    = m._controls.tol;
      _controls.mina   = m._controls.mina;
      _controls.safe   = m._controls.safe;
    };

    /**
       Copy a Mapping object.
    */

    Mapping& operator = (const Mapping& x) {
      Diffeomorphism<Scalar>::operator = (x);
      _itmap = x.itmap();
      delete _work;
      try {
	_work = new Matrix<Scalar>(x.neqs());
      } catch (std::bad_alloc) {
	throw OutOfMemory("mapping copy");
      }
      _controls.njac   = x._controls.njac;
      _controls.nchord = x._controls.nchord;
      _controls.tol    = x._controls.tol;
      _controls.mina   = x._controls.mina;
      _controls.safe   = x._controls.safe;
      return *this;
    };

    // Destructor
    ~Mapping (void) {
      delete _work;
    };

    /**
       How many equations are there?
    */
    const Index neqs (void) const {
      return Diffeomorphism<Scalar>::neqs();
    };

    /**
       How many variables are there?
    */
    const Index nvar (void) const {
      return Diffeomorphism<Scalar>::nvar();
    };

    /**
       How many parameters are there?  */
    const Index npar (void) const {
      return this->Diffeomorphism<Scalar>::nvar() 
	- this->Diffeomorphism<Scalar>::neqs();
    };

    /**
       How many times is the mapping to be composed with itself?
    */
    virtual Index itmap (void) const {
      return this->_itmap;
    };

    /**
       Set the number of times is the mapping to be composed with
       itself.  
    */
    virtual void itmap (Index iters) {
      _itmap = iters;
    };

    /** 
	Evaluate the state-variable portion of the Jacobian only (one
	iteration)
    */
    void deriv (Matrix<Scalar> *df, const Matrix<Scalar>& x) const {
      try {
	typename Diffeomorphism<Scalar>::diffeo_jac jac = this->jacobian();
	Real *dfdx = &((*df)(0,0));
	for (Index j = 0; j < this->neqs(); j++) {
	  (*jac)(x.data(), dfdx, j);
	  dfdx += this->neqs();
	}
      } catch (Exception e) {
	std::cerr << e;
	throw Exception("mapping jacobian evaluation");
      }
    };

    /**
       Apply the mapping, overwriting the input with the output.
    */
    virtual void operator () (Matrix<Scalar> *x) const {
      try {
	if (x->size() != this->nvar()) 
	  throw DimError("mapping iteration");
	for (Index k = 0; k < this->itmap(); k++) advance(x);
	return;
      } catch (Exception e) {
	std::cerr << e;
	throw Exception("mapping iteration");
      }
    };

    /**
       Evaluate the mapping at point x. Store the result in y. It is
       assumed that x & y are of the appropriate size.  */
    virtual void operator () (Matrix<Scalar> *f, const Matrix<Scalar>& x) const {
      try {
	Matrix<Scalar> y = x;

	// Advance itmap - 1 times
	for (Index k = 1; k < this->itmap(); k++) advance(&y);

	// Final iteration
	Diffeomorphism<Scalar>::operator() (f, y);

      } catch (Exception e) {
	std::cerr << e;
	throw Exception("mapping evaluation");
      }
    };

    /**
       Evaluate the mapping at x.  Create a new vector to hold the
       result.  */
    virtual Matrix<Scalar> * operator () (const Matrix<Scalar>& x) const {
      try {
	Matrix<Scalar> *f = new Matrix<Scalar>(this->neqs());
	(*this)(f, x);
	return f;
      } catch (Exception e) {
	std::cerr << e;
	throw Exception("mapping evaluation");
      }
    };

    /**
       Evaluate the Jacobian of the mapping at x.  Store the result in matrix df.
    */
    virtual void operator () (Matrix<Scalar> *df, const Matrix<Scalar>& x, 
			      const Matrix<Index>& i) const {
      try {

	// If the mapping is the identity,
	// return the identity matrix.
	if (this->itmap() == 0) {

	  *df = 1;

	} else {

	  Diffeomorphism<Scalar>::operator () (df, x, i);

	  if (this->itmap() > 1) {
	    // copy vector x for iteration
	    Matrix<Real> y = x;

	    Index n = 0;		// count parameter indices
	    for (Index k = 0; k < i.size(); k++)  
	      if (i(k) >= this->neqs()) n++;


	    // store parameter indices in p & their positions in q
	    Matrix<Index> p(n), q(n); 
	    n = 0;
	    for (Index k = 0; k < i.size(); k++) {
	      if (i(k) >= this->neqs()) {
		p(n) = i(k);
		q(n++) = k;
	      }
	    }

	    Matrix<Real> dx(this->neqs(), this->neqs()), dp(this->neqs(), p.size());

	    // do the chain rule
	    for (Index k = 1; k < this->itmap(); k++) {

	      advance(&y);	// advance y
	      deriv(&dx, y);	// derivative w.r.t. state variables
				// derivative w.r.t. relevant parameters
	      Diffeomorphism<Scalar>::operator () (&dp, y, p); 
	      *df = dx * (*df);
	      (*df)(Range(), q) += dp;
      
	    }
	  }
	}
      } catch (Exception e) {
	std::cerr << e;
	throw Exception("mapping jacobian evaluation");
      }
    };

    /**
       Evaluate the Jacobian of the mapping.  Create a new matrix
       to hold the result.  
    */
    virtual Matrix<Scalar> * operator () (const Matrix<Scalar>& x, 
					  const Matrix<Index>& i) const {
      Matrix<Scalar> *df = new Matrix<Scalar>(this->neqs(), i.size());
      (*this)(df, x, i);
      return df;
    };

    /**
       Compute an itinerary of length n beginning at x.
    */
    virtual Matrix<Scalar> * itinerary (const Matrix<Scalar>& x0, Index n) const {
      try {

	if (x0.size() != this->nvar()) 
	  throw DimError("mapping itinerary");

	Matrix<Real> x = x0;

	Matrix<Real> *y = new Matrix<Real>(n, this->neqs());
	for (Index j = 0; j < this->neqs(); j++) (*y)(0, j) = x(j);

	for (Index k = 1; k < n; k++) {
	  (*this)(&x);
	  for (Index j = 0; j < this->neqs(); j++) (*y)(k, j) = x(j);
	}
  
	return y;

      } catch (Exception e) {
	std::cerr << e;
	throw Exception("mapping itinerary");
      }

    };

  private:

    int chord_fixed_point_iteration (Matrix<Scalar> *x, const Matrix<Index>& i, Real *a) 
    {
      try {

	Range eqns(0, this->neqs()-1);
	Matrix<Scalar> xp = *x, y = xp(eqns, 0), f(this->neqs());

	(*this)(&f, xp);		// evaluate the function
	f -= y;			// subtract the original point
	Real errf = f.maxnorm();	// compute the error

	Matrix<Scalar> df(this->neqs(),this->neqs());
	(*this)(&df, xp, i);	// evaluate the jacobian
	df -= 1;			// subtract the identity matrix

	// LU decomposition
	LUDecomposition<Scalar> T(df);

	// check to make sure the problem is well-conditioned
	if (T.rcondn() < _controls.safe)
	  throw SingularMatrix("mapping chord fixed-point iteration", T.rcondn());

	// Now do the chord iterations.
	for (Index k = 1; k <= _controls.nchord; k++) {

	  Real errfs = errf;

	  T.solve(&f);		// backsolve
	  f *= *a;		// adjust by relaxation parameter
	  xp(i, 0) -= f;		// update the point

	  y = xp(eqns, 0);	// compute the difference
	  (*this)(&f, xp);
	  f -= y;

	  if ((errf = f.maxnorm()) < _controls.tol) {

	    *x = xp;
	    return k;

	  } else if (errfs <= errf) {

	    *a /= 2.0;
	    if (*a < _controls.mina) return -1;

	  }
	  
	}

	*x = xp;
	return 0;

      } catch (Exception e) {
	std::cerr << e;
	throw Exception("chord-fixed-point iteration");
      }

    };
  
  public:

    /**
       Find a fixed point, using x as an initial guess.  Allow the
       variables indexed by iact to vary.  */
    virtual void fixed_point (Matrix<Scalar> *x, const Matrix<Index>& iact) {
      try {  

	if (iact.size() != this->neqs())
	  throw Exception("map fixed-point", 
			  "wrong number of active variables");

	Real a = 1.0;
	int status = 0;

	for (Index count = 0; (status >= 0) && (count < _controls.njac); count++)
	  status = chord_fixed_point_iteration (x, iact, &a);

	if (status == 0) 
	  throw Exception("map fixed-point", 
			  "excessively many steps");

	if (status < 0) 
	  throw Exception("map fixed-point", 
			  "excessively small steps");

      } catch (Exception e) {
	std::cerr << e;
	throw Exception("map fixed-point");
      }

    };

  };

}

#endif

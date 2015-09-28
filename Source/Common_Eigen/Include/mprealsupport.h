// This file is part of a joint effort between Eigen, a lightweight C++ template library
// for linear algebra, and MPFR C++, a C++ interface to MPFR library (http://www.holoborodko.com/pavel/)
//
// Copyright (C) 2010-2012 Pavel Holoborodko <pavel@holoborodko.com>
// Copyright (C) 2010 Konstantin Holoborodko <konstantin@holoborodko.com>
// Copyright (C) 2010 Gael Guennebaud <gael.guennebaud@inria.fr>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef EIGEN_MPREALSUPPORT_MODULE_H
#define EIGEN_MPREALSUPPORT_MODULE_H

#include <Eigen/Core>
#include <mpreal.h>

namespace Eigen {

/**
  * \defgroup MPRealSupport_Module MPFRC++ Support module
  * \code
  * #include <Eigen/MPRealSupport>
  * \endcode
  *
  * This module provides support for multi precision floating point numbers
  * via the <a href="http://www.holoborodko.com/pavel/mpfr">MPFR C++</a>
  * library which itself is built upon <a href="http://www.mpfr.org/">MPFR</a>/<a href="http://gmplib.org/">GMP</a>.
  *
  * You can find a copy of MPFR C++ that is known to be compatible in the unsupported/test/mpreal folder.
  *
  * Here is an example:
  *
\code
#include <iostream>
#include <Eigen/MPRealSupport>
#include <Eigen/LU>
using namespace mpfr;
using namespace Eigen;
int main()
{
  // set precision to 256 bits (double has only 53 bits)
  mpreal::set_default_prec(256);
  // Declare matrix and vector types with multi-precision scalar type
  typedef Matrix<mpreal,Dynamic,Dynamic>  MatrixXmp;
  typedef Matrix<mpreal,Dynamic,1>        VectorXmp;

  MatrixXmp A = MatrixXmp::Random(100,100);
  VectorXmp b = VectorXmp::Random(100);

  // Solve Ax=b using LU
  VectorXmp x = A.lu().solve(b);
  std::cout << "relative error: " << (A*x - b).norm() / b.norm() << std::endl;
  return 0;
}
\endcode
  *
  */

  template<> struct NumTraits<mpfr2::mpreal>
    : GenericNumTraits<mpfr2::mpreal>
  {
    enum {
      IsInteger = 0,
      IsSigned = 1,
      IsComplex = 0,
      RequireInitialization = 1,
      ReadCost = 10,
      AddCost = 10,
      MulCost = 40
    };

    typedef mpfr2::mpreal Real;
    typedef mpfr2::mpreal NonInteger;

    inline static Real highest   (long Precision = mpfr2::mpreal::get_default_prec())  { return  mpfr2::maxval(Precision); }
    inline static Real lowest    (long Precision = mpfr2::mpreal::get_default_prec())  { return -mpfr2::maxval(Precision); }

    // Constants
    inline static Real Pi       (long Precision = mpfr2::mpreal::get_default_prec())     {    return mpfr2::const_pi(Precision);        }
    inline static Real Euler    (long Precision = mpfr2::mpreal::get_default_prec())     {    return mpfr2::const_euler(Precision);     }
    inline static Real Log2     (long Precision = mpfr2::mpreal::get_default_prec())     {    return mpfr2::const_log2(Precision);      }
    inline static Real Catalan  (long Precision = mpfr2::mpreal::get_default_prec())     {    return mpfr2::const_catalan(Precision);   }

    inline static Real epsilon  (long Precision = mpfr2::mpreal::get_default_prec())     {    return mpfr2::machine_epsilon(Precision); }
    inline static Real epsilon  (const Real& x)                                         {    return mpfr2::machine_epsilon(x); }

    inline static Real dummy_precision()
    {
        unsigned int weak_prec = ((mpfr2::mpreal::get_default_prec()-1) * 90) / 100;
        return mpfr2::machine_epsilon(weak_prec);
    }
  };

  namespace internal {

  template<> inline mpfr2::mpreal random<mpfr2::mpreal>()
  {
    return mpfr2::random();
  }

  template<> inline mpfr2::mpreal random<mpfr2::mpreal>(const mpfr2::mpreal& a, const mpfr2::mpreal& b)
  {
    return a + (b-a) * random<mpfr2::mpreal>();
  }

  inline bool isMuchSmallerThan(const mpfr2::mpreal& a, const mpfr2::mpreal& b, const mpfr2::mpreal& eps)
  {
    return mpfr2::abs(a) <= mpfr2::abs(b) * eps;
  }

  inline bool isApprox(const mpfr2::mpreal& a, const mpfr2::mpreal& b, const mpfr2::mpreal& eps)
  {
    return mpfr2::isEqualFuzzy(a,b,eps);
  }

  inline bool isApproxOrLessThan(const mpfr2::mpreal& a, const mpfr2::mpreal& b, const mpfr2::mpreal& eps)
  {
    return a <= b || mpfr2::isEqualFuzzy(a,b,eps);
  }

  template<> inline long double cast<mpfr2::mpreal,long double>(const mpfr2::mpreal& x)
  { return x.toLDouble(); }

  template<> inline double cast<mpfr2::mpreal,double>(const mpfr2::mpreal& x)
  { return x.toDouble(); }

  template<> inline long cast<mpfr2::mpreal,long>(const mpfr2::mpreal& x)
  { return x.toLong(); }

  template<> inline int cast<mpfr2::mpreal,int>(const mpfr2::mpreal& x)
  { return int(x.toLong()); }

  // Specialize GEBP kernel and traits for mpreal (no need for peeling, nor complicated stuff)
  // This also permits to directly call mpfr's routines and avoid many temporaries produced by mpreal
    template<>
    class gebp_traits<mpfr2::mpreal, mpfr2::mpreal, false, false>
    {
    public:
      typedef mpfr2::mpreal ResScalar;
      enum {
        nr = 2, // must be 2 for proper packing...
        mr = 1,
        WorkSpaceFactor = nr,
        LhsProgress = 1,
        RhsProgress = 1
      };
    };

    template<typename Index, int mr, int nr, bool ConjugateLhs, bool ConjugateRhs>
    struct gebp_kernel<mpfr2::mpreal,mpfr2::mpreal,Index,mr,nr,ConjugateLhs,ConjugateRhs>
    {
      typedef mpfr2::mpreal mpreal;

      EIGEN_DONT_INLINE
      void operator()(mpreal* res, Index resStride, const mpreal* blockA, const mpreal* blockB, Index rows, Index depth, Index cols, mpreal alpha,
                      Index strideA=-1, Index strideB=-1, Index offsetA=0, Index offsetB=0, mpreal* /*unpackedB*/ = 0)
      {
        mpreal acc1, acc2, tmp;

        if(strideA==-1) strideA = depth;
        if(strideB==-1) strideB = depth;

        for(Index j=0; j<cols; j+=nr)
        {
          Index actual_nr = (std::min<Index>)(nr,cols-j);
          mpreal *C1 = res + j*resStride;
          mpreal *C2 = res + (j+1)*resStride;
          for(Index i=0; i<rows; i++)
          {
            mpreal *B = const_cast<mpreal*>(blockB) + j*strideB + offsetB*actual_nr;
            mpreal *A = const_cast<mpreal*>(blockA) + i*strideA + offsetA;
            acc1 = 0;
            acc2 = 0;
            for(Index k=0; k<depth; k++)
            {
              mpfr_mul(tmp.mpfr_ptr(), A[k].mpfr_ptr(), B[0].mpfr_ptr(), mpreal::get_default_rnd());
              mpfr_add(acc1.mpfr_ptr(), acc1.mpfr_ptr(), tmp.mpfr_ptr(),  mpreal::get_default_rnd());

              if(actual_nr==2) {
                mpfr_mul(tmp.mpfr_ptr(), A[k].mpfr_ptr(), B[1].mpfr_ptr(), mpreal::get_default_rnd());
                mpfr_add(acc2.mpfr_ptr(), acc2.mpfr_ptr(), tmp.mpfr_ptr(),  mpreal::get_default_rnd());
              }

              B+=actual_nr;
            }

            mpfr_mul(acc1.mpfr_ptr(), acc1.mpfr_ptr(), alpha.mpfr_ptr(), mpreal::get_default_rnd());
            mpfr_add(C1[i].mpfr_ptr(), C1[i].mpfr_ptr(), acc1.mpfr_ptr(),  mpreal::get_default_rnd());

            if(actual_nr==2) {
              mpfr_mul(acc2.mpfr_ptr(), acc2.mpfr_ptr(), alpha.mpfr_ptr(), mpreal::get_default_rnd());
              mpfr_add(C2[i].mpfr_ptr(), C2[i].mpfr_ptr(), acc2.mpfr_ptr(),  mpreal::get_default_rnd());
            }
          }
        }
      }
    };

  } // end namespace internal
}

#endif // EIGEN_MPREALSUPPORT_MODULE_H

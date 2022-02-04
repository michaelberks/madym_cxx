/**
*  @file    mdm_SequenceNames.h
*  @brief Header only class to generate sequence names
*
*  Original author MA Berks 24 Oct 2018
*  (c) Copyright QBI, University of Manchester 2018
*/

#ifndef MDM_EXPONENTIALS_HDR
#define MDM_EXPONENTIALS_HDR

#include <cmath>
#include <vector>
#include <madym/dce/mdm_AIF.h>

//! Header only class to provide method for generating sequence names from user options
class mdm_Exponentials {

public:

  //!Computes update to convolition of function T.exp(-t_i*T) with Ca(t_i) between t_i and t_i-1
  /*!
  \param T exponent parameter
  \param delta_t time difference t_i - t_i-1
  \param Ca1 value of Ca at t_i
  \param Ca0 value of Ca at t_i-1
  \param f previous value of convolved function to be updated
  */
  static void exp_conv(double T, double delta_t, double Ca1, double Ca0, double& f)
  {
    auto xi = delta_t * T;
    auto delta_a = (Ca1 - Ca0) / xi;
    auto E = std::exp(-xi);
    auto E0 = 1 - E;
    auto E1 = xi - E0;

    auto integral = Ca0 * E0 + delta_a * E1;
    f *= E;
    f += integral;
  }

  //!Computes trapezoidal integration of time-series C(t) at time-points t
  /*!
  \param C_t time series to integrate
  \param t times at which to integrate
  \return cumulutaive intergration at each time-point
  */
  static std::vector<double> trapz_integral(const std::vector<double> &C_t, std::vector<double> t)
  {
    auto n_t = t.size();
    std::vector<double>C_t_integral(n_t, 0);

    for (size_t i_t = 1; i_t < n_t; i_t++)
    {
      auto delta_t = t[i_t] - t[i_t - 1];
      auto C_t_mid = 0.5 * (C_t[i_t] + C_t[i_t - 1]);
      C_t_integral[i_t] = C_t_integral[i_t - 1] + delta_t * C_t_mid;
    }

    return C_t_integral;
  }

  //! Compute bi-exponential tissue concentration model time-series
  /*!
  Model equation: Cm(t) = Cp(t) * [ Fpos.exp(-t.Kpos) + Fneg.exp(-t.Kneg) ]

  \param F_pos (see model equation)
  \param F_neg (see model equation)
  \param K_pos (see model equation)
  \param K_neg (see model equation)
  \param Cp_t vascular input function time-series
  \param t times
  \param Cm_t modelled concentration time-series
  */
  static void biexponential(
    const double F_pos, const double F_neg, const double K_pos, const double K_neg, 
    const std::vector<double> &Cp_t, const std::vector<double> &t,
    std::vector<double> &Cm_t)
  {

    // Let's rewrite the convolution sum, using the exponential "trick" so 
    //we can compute everything in one forward loop
    double  Ft_pos = 0;
    double Ft_neg = 0;

    auto nTimes = t.size();

    for (size_t i_t = 1; i_t < nTimes; i_t++)
    {

      // Get current time, and time change
      auto delta_t = t[i_t] - t[i_t - 1];

      mdm_Exponentials::exp_conv(K_pos, delta_t, Cp_t[i_t], Cp_t[i_t - 1], Ft_pos);
      mdm_Exponentials::exp_conv(K_neg, delta_t, Cp_t[i_t], Cp_t[i_t - 1], Ft_neg);

      auto C_t = F_neg * Ft_neg / K_neg + F_pos * Ft_pos / K_pos;

      if (std::isnan(C_t))
        return;

      Cm_t[i_t] = C_t;

    }
  }

  //! Combine vascular inputs with a given mixing fraction
  /*!
  \param
  \return
  */
  static std::vector<double> mix_vifs(mdm_AIF& aif, const double f_a, const double tau_a, const double tau_v)
  {
    double f_v = 1 - f_a;

    //Get AIF and PIF, labelled in model equation as Ca_t and Cv_t
    //Resample AIF and get AIF times
    std::vector<double> Cp_t;
    if (!f_v)
    {
      aif.resample_AIF(tau_a);
      Cp_t = aif.AIF();
    }
    else if (!f_a)
    {
      aif.resample_PIF(tau_v, false, true);
      Cp_t = aif.PIF();
    }
    else
    {
      aif.resample_AIF(tau_a);
      aif.resample_PIF(tau_v, false, true);

      const auto& Ca_t = aif.AIF();
      const auto& Cv_t = aif.PIF();

      auto nTimes = Ca_t.size();
      Cp_t.resize(nTimes, 0);
      for (size_t i = 0; i < nTimes; i++)
        Cp_t[i] = f_a * Ca_t[i] + f_v * Cv_t[i];
    }
    return Cp_t;
  }

  static std::vector<double> make_biexponential_LLS_matrix(
    const std::vector<double>& Ctis_t, const std::vector<double>& Cp_t,
    const std::vector<double>& t)
  {
    auto  n_t = t.size();
    std::vector<double> A(n_t * 4);

    auto Cp_t_int = mdm_Exponentials::trapz_integral(Cp_t, t);
    auto Cp_t_int2 = mdm_Exponentials::trapz_integral(Cp_t_int, t);

    auto Ctis_t_int = mdm_Exponentials::trapz_integral(Ctis_t, t);
    auto Ctis_t_int2 = mdm_Exponentials::trapz_integral(Ctis_t_int, t);

    size_t curr_pt = 0;
    for (size_t i_row = 0; i_row < n_t; i_row++)
    {
      A[curr_pt++] = -Ctis_t_int2[i_row];
      A[curr_pt++] = -Ctis_t_int[i_row];
      A[curr_pt++] = Cp_t_int2[i_row];
      A[curr_pt++] = Cp_t_int[i_row];
    }

    return A;
  }
  
};

#endif //MDM_EXPONENTIALS_HDR
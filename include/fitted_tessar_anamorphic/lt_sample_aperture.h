//input: scene_[x,y,z] - point in scene, ap_[x,y] - point on aperture
//output: [x,y,dx,dy] point and direction on sensor
#ifndef DEBUG_LOG
#define DEBUG_LOG
#endif
float view[3] =
{
  scene_x,
  scene_y,
  scene_z + lens_outer_pupil_curvature_radius
};
normalise(view);
int error = 0;
if(1 || view[2] >= lens_field_of_view)
{
  const float eps = 1e-8;
  float sqr_err = 1e30, sqr_ap_err = 1e30;
  float prev_sqr_err = 1e32, prev_sqr_ap_err = 1e32;
  for(int k=0;k<100&&(sqr_err>eps||sqr_ap_err>eps)&&error==0;k++)
  {
    prev_sqr_err = sqr_err, prev_sqr_ap_err = sqr_ap_err;
    const float begin_x = x;
    const float begin_y = y;
    const float begin_dx = dx;
    const float begin_dy = dy;
    const float begin_lambda = lambda;
    const float pred_ap[2] = {
       + 86.4597 *begin_dx + 0.719396 *begin_x + 0.0979591 *begin_x*begin_lambda + 8.40417 *begin_dx*begin_lambda + 1.78167 *begin_y*begin_dx*begin_dy + 0.0257671 *begin_x*begin_y*begin_dy + 0.0365674 *lens_ipow(begin_x, 2)*begin_dx + 62.77 *lens_ipow(begin_dx, 3) + -0.0678272 *begin_x*lens_ipow(begin_lambda, 2) + 61.8597 *begin_dx*lens_ipow(begin_dy, 2) + 0.000121639 *lens_ipow(begin_x, 3) + 1.17733 *begin_x*lens_ipow(begin_dy, 2) + 2.99936 *begin_x*lens_ipow(begin_dx, 2) + -5.77614 *begin_dx*lens_ipow(begin_lambda, 2) + 0.000597495 *begin_x*lens_ipow(begin_y, 2)*begin_lambda + 0.0513483 *lens_ipow(begin_y, 2)*begin_dx*begin_lambda + -2.93686e-08 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 2) + -1.06328 *begin_y*lens_ipow(begin_dx, 3)*begin_dy + -0.000817827 *begin_x*lens_ipow(begin_y, 2)*lens_ipow(begin_lambda, 2) + 0.000144399 *lens_ipow(begin_x, 3)*lens_ipow(begin_dy, 2) + -3.17616e-06 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*begin_dx + -0.0703785 *lens_ipow(begin_y, 2)*begin_dx*lens_ipow(begin_lambda, 2) + -4.22067 *begin_x*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2)*begin_lambda + -291.864 *lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 2)*begin_lambda + 0.000444537 *begin_x*lens_ipow(begin_y, 2)*lens_ipow(begin_lambda, 4) + 0.0381449 *lens_ipow(begin_y, 2)*begin_dx*lens_ipow(begin_lambda, 4) + -138191 *lens_ipow(begin_dx, 11) + -2398.77 *begin_x*lens_ipow(begin_dx, 10),
       + 0.720977 *begin_y + 86.5823 *begin_dy + 8.22068 *begin_dy*begin_lambda + 0.0941827 *begin_y*begin_lambda + 1.19424 *begin_y*lens_ipow(begin_dx, 2) + 0.0319703 *lens_ipow(begin_y, 2)*begin_dy + 2.60607 *begin_y*lens_ipow(begin_dy, 2) + 1.75108 *begin_x*begin_dx*begin_dy + 0.0257636 *begin_x*begin_y*begin_dx + 60.72 *lens_ipow(begin_dx, 2)*begin_dy + -0.064435 *begin_y*lens_ipow(begin_lambda, 2) + -5.61868 *begin_dy*lens_ipow(begin_lambda, 2) + 0.00010636 *lens_ipow(begin_y, 3) + 52.0013 *lens_ipow(begin_dy, 3) + 0.0503633 *lens_ipow(begin_x, 2)*begin_dy*begin_lambda + 0.000582882 *lens_ipow(begin_x, 2)*begin_y*begin_lambda + -0.000793167 *lens_ipow(begin_x, 2)*begin_y*lens_ipow(begin_lambda, 2) + 3.17233e-05 *lens_ipow(begin_y, 4)*begin_dy + 325.173 *lens_ipow(begin_dy, 5) + 0.525617 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 3) + 21.4899 *begin_y*lens_ipow(begin_dy, 4) + -0.0692483 *lens_ipow(begin_x, 2)*begin_dy*lens_ipow(begin_lambda, 2) + 6.06448e-08 *lens_ipow(begin_y, 5) + 0.00599174 *lens_ipow(begin_y, 3)*lens_ipow(begin_dy, 2) + -71.8423 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 3)*begin_lambda + -1.48937 *begin_y*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2)*begin_lambda + 0.0373362 *lens_ipow(begin_x, 2)*begin_dy*lens_ipow(begin_lambda, 4) + 0.000422188 *lens_ipow(begin_x, 2)*begin_y*lens_ipow(begin_lambda, 4)
    };
    const float delta_ap[] = {ap_x - pred_ap[0], ap_y - pred_ap[1]};
    sqr_ap_err = delta_ap[0]*delta_ap[0]+delta_ap[1]*delta_ap[1];
    float dx1_domega0[2][2];
    dx1_domega0[0][0] =  + 86.4597  + 8.40417 *begin_lambda + 1.78167 *begin_y*begin_dy + 0.0365674 *lens_ipow(begin_x, 2) + 188.31 *lens_ipow(begin_dx, 2) + 61.8597 *lens_ipow(begin_dy, 2) + 5.99872 *begin_x*begin_dx + -5.77614 *lens_ipow(begin_lambda, 2) + 0.0513483 *lens_ipow(begin_y, 2)*begin_lambda + -3.18983 *begin_y*lens_ipow(begin_dx, 2)*begin_dy + -3.17616e-06 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2) + -0.0703785 *lens_ipow(begin_y, 2)*lens_ipow(begin_lambda, 2) + -8.44134 *begin_x*begin_dx*lens_ipow(begin_dy, 2)*begin_lambda + -875.592 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2)*begin_lambda + 0.0381449 *lens_ipow(begin_y, 2)*lens_ipow(begin_lambda, 4) + -1.5201e+06 *lens_ipow(begin_dx, 10) + -23987.7 *begin_x*lens_ipow(begin_dx, 9)+0.0f;
    dx1_domega0[0][1] =  + 1.78167 *begin_y*begin_dx + 0.0257671 *begin_x*begin_y + 123.719 *begin_dx*begin_dy + 2.35466 *begin_x*begin_dy + -1.06328 *begin_y*lens_ipow(begin_dx, 3) + 0.000288799 *lens_ipow(begin_x, 3)*begin_dy + -8.44134 *begin_x*lens_ipow(begin_dx, 2)*begin_dy*begin_lambda + -583.728 *lens_ipow(begin_dx, 3)*begin_dy*begin_lambda+0.0f;
    dx1_domega0[1][0] =  + 2.38848 *begin_y*begin_dx + 1.75108 *begin_x*begin_dy + 0.0257636 *begin_x*begin_y + 121.44 *begin_dx*begin_dy + -143.685 *begin_dx*lens_ipow(begin_dy, 3)*begin_lambda + -2.97875 *begin_y*begin_dx*lens_ipow(begin_dy, 2)*begin_lambda+0.0f;
    dx1_domega0[1][1] =  + 86.5823  + 8.22068 *begin_lambda + 0.0319703 *lens_ipow(begin_y, 2) + 5.21214 *begin_y*begin_dy + 1.75108 *begin_x*begin_dx + 60.72 *lens_ipow(begin_dx, 2) + -5.61868 *lens_ipow(begin_lambda, 2) + 156.004 *lens_ipow(begin_dy, 2) + 0.0503633 *lens_ipow(begin_x, 2)*begin_lambda + 3.17233e-05 *lens_ipow(begin_y, 4) + 1625.86 *lens_ipow(begin_dy, 4) + 1.57685 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 2) + 85.9595 *begin_y*lens_ipow(begin_dy, 3) + -0.0692483 *lens_ipow(begin_x, 2)*lens_ipow(begin_lambda, 2) + 0.0119835 *lens_ipow(begin_y, 3)*begin_dy + -215.527 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2)*begin_lambda + -2.97875 *begin_y*lens_ipow(begin_dx, 2)*begin_dy*begin_lambda + 0.0373362 *lens_ipow(begin_x, 2)*lens_ipow(begin_lambda, 4)+0.0f;
    float invApJ[2][2];
    const float invdetap = 1.0f/(dx1_domega0[0][0]*dx1_domega0[1][1] - dx1_domega0[0][1]*dx1_domega0[1][0]);
    invApJ[0][0] =  dx1_domega0[1][1]*invdetap;
    invApJ[1][1] =  dx1_domega0[0][0]*invdetap;
    invApJ[0][1] = -dx1_domega0[0][1]*invdetap;
    invApJ[1][0] = -dx1_domega0[1][0]*invdetap;
    for(int i=0;i<2;i++)
    {
      dx += invApJ[0][i]*delta_ap[i];
      dy += invApJ[1][i]*delta_ap[i];
    }
    out[0] =  + 147.842 *begin_dx + 0.549491 *begin_x + -0.103186 *begin_x*begin_lambda + -45.4974 *begin_dx*begin_lambda + 1.44752 *begin_y*begin_dx*begin_dy + 0.0262441 *begin_x*begin_y*begin_dy + 0.0663009 *begin_x*lens_ipow(begin_lambda, 2) + -14.243 *begin_dx*lens_ipow(begin_dy, 2) + 0.000188283 *begin_x*lens_ipow(begin_y, 2) + 0.000217267 *lens_ipow(begin_x, 3) + 0.540053 *begin_x*lens_ipow(begin_dy, 2) + 0.0157306 *lens_ipow(begin_y, 2)*begin_dx + 66.7136 *begin_dx*lens_ipow(begin_lambda, 2) + 18.5134 *begin_x*lens_ipow(begin_dx, 2)*begin_lambda + -34.6687 *begin_dx*lens_ipow(begin_lambda, 3) + 0.31381 *lens_ipow(begin_x, 2)*begin_dx*begin_lambda + -111.474 *lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 2) + -40.0336 *begin_x*lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 2) + 0.000149611 *lens_ipow(begin_x, 3)*lens_ipow(begin_dy, 2) + -0.620631 *lens_ipow(begin_x, 2)*begin_dx*lens_ipow(begin_lambda, 2) + 0.000182623 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 2) + 112.026 *lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 2) + 26.4292 *begin_x*lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 3) + 0.000279274 *begin_x*lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2)*begin_lambda + 0.39161 *lens_ipow(begin_x, 2)*begin_dx*lens_ipow(begin_lambda, 3) + -7.3669e-11 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 2) + 588.508 *lens_ipow(begin_dx, 7) + 151.827 *lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 5);
    out[1] =  + 0.28577 *begin_y + 101.636 *begin_dy + 2.52522 *begin_dy*begin_lambda + 1.07359 *begin_y*lens_ipow(begin_dx, 2) + 0.0157958 *lens_ipow(begin_y, 2)*begin_dy + 1.23895 *begin_x*begin_dx*begin_dy + 0.0289015 *begin_x*begin_y*begin_dx + 0.000167325 *lens_ipow(begin_x, 2)*begin_y + -1.64973 *begin_dy*lens_ipow(begin_lambda, 2) + 9.03013e-05 *lens_ipow(begin_y, 3) + 0.0109587 *lens_ipow(begin_x, 2)*begin_dy + -63.6032 *lens_ipow(begin_dy, 3) + 109.057 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 3) + 9.53403e-05 *lens_ipow(begin_y, 4)*begin_dy + 2293.25 *lens_ipow(begin_dy, 5) + -0.0144726 *begin_x*begin_y*begin_dx*lens_ipow(begin_dy, 2) + 1.9289 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 3) + 9.78084e-05 *lens_ipow(begin_x, 2)*begin_y*lens_ipow(begin_dx, 2) + 102.796 *begin_y*lens_ipow(begin_dy, 4) + 1.7794e-07 *lens_ipow(begin_y, 5) + 0.0189447 *lens_ipow(begin_y, 3)*lens_ipow(begin_dy, 2) + 451.978 *lens_ipow(begin_dx, 6)*begin_dy + -3.191e-11 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 3) + -1704.18 *begin_y*lens_ipow(begin_dy, 6)*lens_ipow(begin_lambda, 2) + -28.2403 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 5)*lens_ipow(begin_lambda, 2) + -37070.9 *lens_ipow(begin_dy, 7)*lens_ipow(begin_lambda, 2) + -0.174262 *lens_ipow(begin_y, 3)*lens_ipow(begin_dy, 4)*lens_ipow(begin_lambda, 2) + -3.04904e-08 *lens_ipow(begin_y, 6)*begin_dy*lens_ipow(begin_lambda, 2);
    out[2] =  + -1.22583 *begin_dx + -0.0115335 *begin_x + 3.72489e-05 *begin_x*begin_lambda + 0.388564 *begin_dx*begin_lambda + -0.00243346 *begin_y*begin_dx*begin_dy + -0.0001031 *begin_x*begin_y*begin_dy + -0.000207958 *lens_ipow(begin_x, 2)*begin_dx + 0.546634 *begin_dx*lens_ipow(begin_dy, 2) + -3.89927e-07 *begin_x*lens_ipow(begin_y, 2) + -6.55528e-07 *lens_ipow(begin_x, 3) + -0.00491666 *begin_x*lens_ipow(begin_dx, 2) + -4.33029e-05 *lens_ipow(begin_y, 2)*begin_dx + -0.540983 *begin_dx*lens_ipow(begin_lambda, 2) + -0.00182509 *begin_x*lens_ipow(begin_dx, 2)*begin_lambda + 0.265823 *begin_dx*lens_ipow(begin_lambda, 3) + 3.94963 *lens_ipow(begin_dx, 3)*begin_lambda + 0.131347 *begin_x*lens_ipow(begin_dx, 4) + 0.00174461 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 3) + -7.63304 *lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 2) + 0.0169351 *begin_y*lens_ipow(begin_dx, 3)*begin_dy + 3.21435 *lens_ipow(begin_dx, 5) + 0.00013855 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 3) + 6.7179e-05 *lens_ipow(begin_x, 2)*begin_dx*lens_ipow(begin_dy, 2) + 7.96049e-06 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 2) + 9.81199e-05 *begin_x*begin_y*lens_ipow(begin_dy, 3)*begin_lambda + -0.614038 *begin_dx*lens_ipow(begin_dy, 4)*begin_lambda + 12.9919 *lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 4) + -9.4279 *lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 5);
    out[3] =  + -0.012339 *begin_y + -0.907962 *begin_dy + 7.66706e-05 *begin_y*begin_lambda + -0.001447 *begin_y*lens_ipow(begin_dx, 2) + -3.1298e-05 *lens_ipow(begin_y, 2)*begin_dy + 0.00386463 *begin_y*lens_ipow(begin_dy, 2) + 0.0111068 *begin_x*begin_dx*begin_dy + -6.98903e-05 *begin_x*begin_y*begin_dx + 1.05098 *lens_ipow(begin_dx, 2)*begin_dy + -1.13115e-07 *lens_ipow(begin_x, 2)*begin_y + 1.60241e-05 *lens_ipow(begin_x, 2)*begin_dy + 0.592125 *lens_ipow(begin_dy, 3) + 5.34109e-07 *lens_ipow(begin_y, 3)*lens_ipow(begin_lambda, 2) + -8.83134e-07 *lens_ipow(begin_y, 3)*lens_ipow(begin_lambda, 4) + 0.00698877 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 5) + 9.70047e-05 *begin_x*lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 3)*begin_dy + 8.14901e-06 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2)*begin_dy*begin_lambda + 0.000197141 *lens_ipow(begin_x, 2)*begin_y*lens_ipow(begin_dy, 4)*begin_lambda + 0.72148 *begin_y*lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 2)*begin_lambda + 6.96393e-08 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 2)*begin_dx*begin_dy*begin_lambda + -0.0577875 *begin_x*begin_y*begin_dx*lens_ipow(begin_dy, 4)*begin_lambda + -12.0402 *begin_y*lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 4) + -0.00203494 *begin_x*lens_ipow(begin_y, 2)*begin_dx*lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 2) + -2.86475e-08 *lens_ipow(begin_x, 4)*begin_y*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + 13390.4 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 9) + 219.637 *begin_y*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 8) + -2.03286e-05 *begin_x*lens_ipow(begin_y, 3)*begin_dx*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 4) + 0.00812089 *begin_x*lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 2);
    float pred_out_cs[7] = {0.0f};
    lens_sphereToCs(out, out+2, pred_out_cs, pred_out_cs+3, - lens_outer_pupil_curvature_radius, lens_outer_pupil_curvature_radius);
    float view[3] =
    {
      scene_x - pred_out_cs[0],
      scene_y - pred_out_cs[1],
      scene_z - pred_out_cs[2]
    };
    normalise(view);
    float out_new[5];
    lens_csToSphere(pred_out_cs, view, out_new, out_new+2, - lens_outer_pupil_curvature_radius, lens_outer_pupil_curvature_radius);
    const float delta_out[] = {out_new[2] - out[2], out_new[3] - out[3]};
    sqr_err = delta_out[0]*delta_out[0]+delta_out[1]*delta_out[1];
    float domega2_dx0[2][2];
    domega2_dx0[0][0] =  + -0.0115335  + 3.72489e-05 *begin_lambda + -0.0001031 *begin_y*begin_dy + -0.000415915 *begin_x*begin_dx + -3.89927e-07 *lens_ipow(begin_y, 2) + -1.96658e-06 *lens_ipow(begin_x, 2) + -0.00491666 *lens_ipow(begin_dx, 2) + -0.00182509 *lens_ipow(begin_dx, 2)*begin_lambda + 0.131347 *lens_ipow(begin_dx, 4) + 0.00348922 *begin_x*lens_ipow(begin_dx, 3) + 0.000134358 *begin_x*begin_dx*lens_ipow(begin_dy, 2) + 2.38815e-05 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 2) + 9.81199e-05 *begin_y*lens_ipow(begin_dy, 3)*begin_lambda+0.0f;
    domega2_dx0[0][1] =  + -0.00243346 *begin_dx*begin_dy + -0.0001031 *begin_x*begin_dy + -7.79854e-07 *begin_x*begin_y + -8.66058e-05 *begin_y*begin_dx + 0.0169351 *lens_ipow(begin_dx, 3)*begin_dy + 0.0002771 *begin_y*lens_ipow(begin_dx, 3) + 9.81199e-05 *begin_x*lens_ipow(begin_dy, 3)*begin_lambda+0.0f;
    domega2_dx0[1][0] =  + 0.0111068 *begin_dx*begin_dy + -6.98903e-05 *begin_y*begin_dx + -2.2623e-07 *begin_x*begin_y + 3.20481e-05 *begin_x*begin_dy + 0.0139775 *begin_x*lens_ipow(begin_dy, 5) + 9.70047e-05 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 3)*begin_dy + 1.6298e-05 *begin_x*lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2)*begin_dy*begin_lambda + 0.000394283 *begin_x*begin_y*lens_ipow(begin_dy, 4)*begin_lambda + 2.08918e-07 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*begin_dx*begin_dy*begin_lambda + -0.0577875 *begin_y*begin_dx*lens_ipow(begin_dy, 4)*begin_lambda + -0.00203494 *lens_ipow(begin_y, 2)*begin_dx*lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 2) + -1.1459e-07 *lens_ipow(begin_x, 3)*begin_y*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + -2.03286e-05 *lens_ipow(begin_y, 3)*begin_dx*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 4) + 0.00812089 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 2)+0.0f;
    domega2_dx0[1][1] =  + -0.012339  + 7.66706e-05 *begin_lambda + -0.001447 *lens_ipow(begin_dx, 2) + -6.2596e-05 *begin_y*begin_dy + 0.00386463 *lens_ipow(begin_dy, 2) + -6.98903e-05 *begin_x*begin_dx + -1.13115e-07 *lens_ipow(begin_x, 2) + 1.60233e-06 *lens_ipow(begin_y, 2)*lens_ipow(begin_lambda, 2) + -2.6494e-06 *lens_ipow(begin_y, 2)*lens_ipow(begin_lambda, 4) + 0.000194009 *begin_x*begin_y*lens_ipow(begin_dx, 3)*begin_dy + 1.6298e-05 *lens_ipow(begin_x, 2)*begin_y*lens_ipow(begin_dx, 2)*begin_dy*begin_lambda + 0.000197141 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 4)*begin_lambda + 0.72148 *lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 2)*begin_lambda + 1.39279e-07 *lens_ipow(begin_x, 3)*begin_y*begin_dx*begin_dy*begin_lambda + -0.0577875 *begin_x*begin_dx*lens_ipow(begin_dy, 4)*begin_lambda + -12.0402 *lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 4) + -0.00406988 *begin_x*begin_y*begin_dx*lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 2) + -2.86475e-08 *lens_ipow(begin_x, 4)*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + 219.637 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 8) + -6.09857e-05 *begin_x*lens_ipow(begin_y, 2)*begin_dx*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 4) + 0.0162418 *begin_x*begin_y*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 2)+0.0f;
    float invJ[2][2];
    const float invdet = 1.0f/(domega2_dx0[0][0]*domega2_dx0[1][1] - domega2_dx0[0][1]*domega2_dx0[1][0]);
    invJ[0][0] =  domega2_dx0[1][1]*invdet;
    invJ[1][1] =  domega2_dx0[0][0]*invdet;
    invJ[0][1] = -domega2_dx0[0][1]*invdet;
    invJ[1][0] = -domega2_dx0[1][0]*invdet;
    for(int i=0;i<2;i++)
    {
      x += invJ[0][i]*delta_out[i];
      y += invJ[1][i]*delta_out[i];
    }
    if(sqr_err>prev_sqr_err) error |= 1;
    if(sqr_ap_err>prev_sqr_ap_err) error |= 2;
    if(out[0]!=out[0]) error |= 4;
    if(out[1]!=out[1]) error |= 8;
    DEBUG_LOG;
    // reset error code for first few iterations.
    if(k<10) error = 0;
  }
}
else
  error = 128;
if(out[0]*out[0]+out[1]*out[1] > lens_outer_pupil_radius*lens_outer_pupil_radius) error |= 16;
const float begin_x = x;
const float begin_y = y;
const float begin_dx = dx;
const float begin_dy = dy;
const float begin_lambda = lambda;
if(error==0)
  out[4] =  + 3.29506 *begin_lambda + -0.00137751 *begin_y*begin_dy + -0.00180612 *begin_x*begin_dx + -8.2307e-06 *lens_ipow(begin_y, 2) + -0.0686622 *lens_ipow(begin_dy, 2) + -1.01567e-05 *lens_ipow(begin_x, 2) + -0.0815586 *lens_ipow(begin_dx, 2) + -9.53966 *lens_ipow(begin_lambda, 2) + 14.6284 *lens_ipow(begin_lambda, 3) + -11.5867 *lens_ipow(begin_lambda, 4) + -2.46469 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2) + -9.39554e-05 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 2) + -0.0187356 *begin_y*lens_ipow(begin_dx, 2)*begin_dy + -0.0187029 *begin_x*begin_dx*lens_ipow(begin_dy, 2) + -0.00014993 *begin_x*begin_y*begin_dx*begin_dy + 3.74265 *lens_ipow(begin_lambda, 5) + -8.54763e-06 *lens_ipow(begin_y, 4)*lens_ipow(begin_dy, 2) + -4.59691e-11 *lens_ipow(begin_x, 6) + -46.0789 *lens_ipow(begin_dx, 6) + -1.04574e-08 *lens_ipow(begin_x, 5)*begin_dx + -1.50845 *begin_x*lens_ipow(begin_dx, 5) + -0.00104505 *lens_ipow(begin_y, 3)*lens_ipow(begin_dy, 3) + -0.0141085 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 4) + -4.231e-08 *lens_ipow(begin_y, 5)*begin_dy + -69.8346 *lens_ipow(begin_dy, 6) + -9.64471e-11 *lens_ipow(begin_y, 6) + -0.0792566 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 4) + -3.47393 *begin_y*lens_ipow(begin_dy, 5);
else
  out[4] = 0.0f;

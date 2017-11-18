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
       + 66.2069 *begin_dx + 0.5962 *begin_x + 0.250805 *begin_x*begin_lambda + 17.9457 *begin_dx*begin_lambda + -0.631637 *begin_y*begin_dx*begin_dy + -0.00169732 *begin_x*begin_y*begin_dy + -0.00274299 *lens_ipow(begin_x, 2)*begin_dx + -48.5219 *lens_ipow(begin_dx, 3) + -0.346547 *begin_x*lens_ipow(begin_lambda, 2) + -46.9272 *begin_dx*lens_ipow(begin_dy, 2) + -2.76025e-06 *lens_ipow(begin_x, 3) + -0.29773 *begin_x*lens_ipow(begin_dy, 2) + -0.945888 *begin_x*lens_ipow(begin_dx, 2) + -0.00117191 *lens_ipow(begin_y, 2)*begin_dx + -24.4136 *begin_dx*lens_ipow(begin_lambda, 2) + 11.7019 *begin_dx*lens_ipow(begin_lambda, 3) + 0.167874 *begin_x*lens_ipow(begin_lambda, 3) + -4.80601e-06 *begin_x*lens_ipow(begin_y, 2)*begin_lambda + -0.00107164 *begin_x*begin_y*begin_dy*begin_lambda + -0.00151481 *lens_ipow(begin_x, 2)*begin_dx*begin_lambda + -0.0170197 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 3) + 48.409 *lens_ipow(begin_dx, 5) + -0.00013336 *begin_x*lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2) + -0.0107591 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 3) + -0.0237075 *lens_ipow(begin_x, 2)*begin_dx*lens_ipow(begin_dy, 2)*begin_lambda + 121.445 *lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 2)*begin_lambda + -0.000262956 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 2)*begin_lambda + -0.000233004 *lens_ipow(begin_x, 3)*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2),
       + 0.623078 *begin_y + 67.8437 *begin_dy + 8.18959 *begin_dy*begin_lambda + 0.0989906 *begin_y*begin_lambda + -0.303511 *begin_y*lens_ipow(begin_dx, 2) + -0.00213596 *lens_ipow(begin_y, 2)*begin_dy + -0.858359 *begin_y*lens_ipow(begin_dy, 2) + -0.00237215 *begin_x*begin_y*begin_dx + -0.067194 *begin_y*lens_ipow(begin_lambda, 2) + -4.29705e-06 *lens_ipow(begin_x, 2)*begin_y + -5.64735 *begin_dy*lens_ipow(begin_lambda, 2) + -45.5101 *lens_ipow(begin_dy, 3) + -201.662 *lens_ipow(begin_dx, 2)*begin_dy*begin_lambda + -0.00104475 *lens_ipow(begin_y, 2)*begin_dy*begin_lambda + -2.35583 *begin_x*begin_dx*begin_dy*begin_lambda + 2.1099 *begin_x*begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + -0.0133921 *lens_ipow(begin_x, 2)*begin_dy*lens_ipow(begin_lambda, 2) + 259.61 *lens_ipow(begin_dx, 2)*begin_dy*lens_ipow(begin_lambda, 2) + -1.13034e-05 *lens_ipow(begin_y, 4)*begin_dy*begin_lambda + 41.9825 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 3)*begin_lambda + -94.6589 *lens_ipow(begin_dx, 2)*begin_dy*lens_ipow(begin_lambda, 3) + -0.00229265 *lens_ipow(begin_y, 3)*lens_ipow(begin_dy, 2)*begin_lambda + -2.62607e-08 *lens_ipow(begin_y, 5)*begin_lambda + 0.015556 *lens_ipow(begin_x, 2)*begin_dy*lens_ipow(begin_lambda, 3) + -8.02377 *begin_y*lens_ipow(begin_dy, 4)*begin_lambda + -0.22871 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 3)*begin_lambda + -4.33424 *begin_y*lens_ipow(begin_dy, 4)*lens_ipow(begin_lambda, 2) + -264.677 *lens_ipow(begin_dy, 5)*lens_ipow(begin_lambda, 2)
    };
    const float delta_ap[] = {ap_x - pred_ap[0], ap_y - pred_ap[1]};
    sqr_ap_err = delta_ap[0]*delta_ap[0]+delta_ap[1]*delta_ap[1];
    float dx1_domega0[2][2];
    dx1_domega0[0][0] =  + 66.2069  + 17.9457 *begin_lambda + -0.631637 *begin_y*begin_dy + -0.00274299 *lens_ipow(begin_x, 2) + -145.566 *lens_ipow(begin_dx, 2) + -46.9272 *lens_ipow(begin_dy, 2) + -1.89178 *begin_x*begin_dx + -0.00117191 *lens_ipow(begin_y, 2) + -24.4136 *lens_ipow(begin_lambda, 2) + 11.7019 *lens_ipow(begin_lambda, 3) + -0.00151481 *lens_ipow(begin_x, 2)*begin_lambda + -0.0510591 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 2) + 242.045 *lens_ipow(begin_dx, 4) + -0.000266721 *begin_x*lens_ipow(begin_y, 2)*begin_dx + -0.0322772 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2) + -0.0237075 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 2)*begin_lambda + 364.336 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2)*begin_lambda + -0.000525913 *lens_ipow(begin_x, 3)*begin_dx*begin_lambda+0.0f;
    dx1_domega0[0][1] =  + -0.631637 *begin_y*begin_dx + -0.00169732 *begin_x*begin_y + -93.8544 *begin_dx*begin_dy + -0.59546 *begin_x*begin_dy + -0.00107164 *begin_x*begin_y*begin_lambda + -0.047415 *lens_ipow(begin_x, 2)*begin_dx*begin_dy*begin_lambda + 242.891 *lens_ipow(begin_dx, 3)*begin_dy*begin_lambda + -0.000466008 *lens_ipow(begin_x, 3)*begin_dy*lens_ipow(begin_lambda, 2)+0.0f;
    dx1_domega0[1][0] =  + -0.607023 *begin_y*begin_dx + -0.00237215 *begin_x*begin_y + -403.324 *begin_dx*begin_dy*begin_lambda + -2.35583 *begin_x*begin_dy*begin_lambda + 2.1099 *begin_x*begin_dy*lens_ipow(begin_lambda, 2) + 519.221 *begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + 83.965 *begin_dx*lens_ipow(begin_dy, 3)*begin_lambda + -189.318 *begin_dx*begin_dy*lens_ipow(begin_lambda, 3)+0.0f;
    dx1_domega0[1][1] =  + 67.8437  + 8.18959 *begin_lambda + -0.00213596 *lens_ipow(begin_y, 2) + -1.71672 *begin_y*begin_dy + -5.64735 *lens_ipow(begin_lambda, 2) + -136.53 *lens_ipow(begin_dy, 2) + -201.662 *lens_ipow(begin_dx, 2)*begin_lambda + -0.00104475 *lens_ipow(begin_y, 2)*begin_lambda + -2.35583 *begin_x*begin_dx*begin_lambda + 2.1099 *begin_x*begin_dx*lens_ipow(begin_lambda, 2) + -0.0133921 *lens_ipow(begin_x, 2)*lens_ipow(begin_lambda, 2) + 259.61 *lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 2) + -1.13034e-05 *lens_ipow(begin_y, 4)*begin_lambda + 125.947 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2)*begin_lambda + -94.6589 *lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 3) + -0.0045853 *lens_ipow(begin_y, 3)*begin_dy*begin_lambda + 0.015556 *lens_ipow(begin_x, 2)*lens_ipow(begin_lambda, 3) + -32.0951 *begin_y*lens_ipow(begin_dy, 3)*begin_lambda + -0.686131 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 2)*begin_lambda + -17.3369 *begin_y*lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 2) + -1323.39 *lens_ipow(begin_dy, 4)*lens_ipow(begin_lambda, 2)+0.0f;
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
    out[0] =  + 100.12 *begin_dx + 0.592582 *begin_x + 0.0499465 *begin_x*begin_lambda + -0.811097 *begin_dx*begin_lambda + 0.0323359 *begin_x*begin_y*begin_dy + 0.0379048 *lens_ipow(begin_x, 2)*begin_dx + -42.7325 *lens_ipow(begin_dx, 3) + -0.0389817 *begin_x*lens_ipow(begin_lambda, 2) + 0.000142213 *begin_x*lens_ipow(begin_y, 2) + 0.000142435 *lens_ipow(begin_x, 3) + 1.67656 *begin_x*lens_ipow(begin_dy, 2) + 1.81407 *begin_x*lens_ipow(begin_dx, 2) + 0.00570726 *lens_ipow(begin_y, 2)*begin_dx + -209.372 *begin_dx*lens_ipow(begin_dy, 2)*begin_lambda + 0.677473 *begin_y*begin_dx*begin_dy*begin_lambda + -0.536195 *begin_y*begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + 299.926 *begin_dx*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + 0.0713542 *begin_x*begin_y*lens_ipow(begin_dy, 3)*begin_lambda + 0.00125855 *begin_x*lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 2)*begin_lambda + -3.55319 *begin_x*lens_ipow(begin_dx, 4)*begin_lambda + 222.317 *lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 2)*begin_lambda + -0.000266069 *lens_ipow(begin_y, 3)*begin_dx*begin_dy*begin_lambda + -5.3226e-06 *lens_ipow(begin_x, 3)*begin_y*begin_dy*begin_lambda + 0.00102282 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 2)*begin_lambda + -168.54 *begin_dx*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 4) + -3.93679e-10 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 4)*begin_lambda + 1.32918e-08 *lens_ipow(begin_x, 6)*begin_dx*begin_lambda + -2.70347e-08 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 4)*begin_dx*begin_lambda;
    out[1] =  + 0.588556 *begin_y + 99.6889 *begin_dy + 0.0557925 *begin_y*begin_lambda + 1.62675 *begin_y*lens_ipow(begin_dx, 2) + 0.045876 *lens_ipow(begin_y, 2)*begin_dy + 2.49363 *begin_y*lens_ipow(begin_dy, 2) + 0.153878 *begin_x*begin_dx*begin_dy + 0.0314574 *begin_x*begin_y*begin_dx + -42.0967 *lens_ipow(begin_dx, 2)*begin_dy + -0.0358604 *begin_y*lens_ipow(begin_lambda, 2) + 0.000141395 *lens_ipow(begin_x, 2)*begin_y + 0.000178882 *lens_ipow(begin_y, 3) + 0.00572629 *lens_ipow(begin_x, 2)*begin_dy + -1.35881 *begin_y*lens_ipow(begin_dy, 2)*begin_lambda + -135.315 *lens_ipow(begin_dy, 3)*begin_lambda + -0.0168528 *lens_ipow(begin_y, 2)*begin_dy*begin_lambda + -6.88134e-05 *lens_ipow(begin_y, 3)*begin_lambda + 120.172 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 3) + 9.10801e-06 *lens_ipow(begin_y, 4)*begin_dy + 0.0748529 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 3) + 106.566 *lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 2) + 0.000223543 *lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 2) + 0.00161417 *lens_ipow(begin_y, 3)*lens_ipow(begin_dy, 2) + 0.000235019 *lens_ipow(begin_x, 2)*begin_y*lens_ipow(begin_dy, 2)*begin_lambda + 0.681351 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 4)*begin_dy + -0.000143401 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2)*begin_dy + -9.81214e-11 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 3) + -56.6549 *lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 6);
    out[2] =  + -3.05455 *begin_dx + -0.0282279 *begin_x + -0.000260254 *begin_x*begin_lambda + 0.150251 *begin_dx*begin_lambda + -0.193971 *begin_y*begin_dx*begin_dy + -0.00214261 *begin_x*begin_y*begin_dy + -0.000343775 *lens_ipow(begin_x, 2)*begin_dx + 1.28863 *lens_ipow(begin_dx, 3) + -13.7923 *begin_dx*lens_ipow(begin_dy, 2) + -6.63026e-06 *begin_x*lens_ipow(begin_y, 2) + -0.144619 *begin_x*lens_ipow(begin_dy, 2) + -0.0045328 *begin_x*lens_ipow(begin_dx, 2) + -0.000631766 *lens_ipow(begin_y, 2)*begin_dx + -0.108421 *begin_dx*lens_ipow(begin_lambda, 2) + -7.70575e-06 *lens_ipow(begin_x, 3)*lens_ipow(begin_lambda, 2) + -0.119872 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 2) + 3.23657e-05 *lens_ipow(begin_x, 3)*lens_ipow(begin_lambda, 4) + 2457.14 *lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 4) + -0.00163849 *begin_x*lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2) + 0.274907 *lens_ipow(begin_x, 2)*begin_dx*lens_ipow(begin_dy, 4) + 49.9949 *begin_x*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 4) + -4.84099e-06 *lens_ipow(begin_x, 3)*begin_y*lens_ipow(begin_dy, 3) + 1.54348e-06 *lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 3)*begin_lambda + -2.83936e-05 *lens_ipow(begin_x, 3)*lens_ipow(begin_lambda, 5) + -543.956 *begin_x*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 6) + -3.42461 *lens_ipow(begin_x, 2)*begin_dx*lens_ipow(begin_dy, 6) + -25109 *lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 6) + -0.0503811 *lens_ipow(begin_x, 3)*lens_ipow(begin_dy, 8);
    out[3] =  + -0.0282638 *begin_y + -3.02557 *begin_dy + 0.0262125 *begin_dy*begin_lambda + -0.000273248 *begin_y*begin_lambda + 0.0476166 *begin_y*lens_ipow(begin_dx, 2) + -0.00028279 *lens_ipow(begin_y, 2)*begin_dy + 0.278569 *begin_x*begin_dx*begin_dy + 0.000913623 *begin_x*begin_y*begin_dx + 16.1714 *lens_ipow(begin_dx, 2)*begin_dy + 5.08174e-06 *lens_ipow(begin_x, 2)*begin_y + 0.00117665 *lens_ipow(begin_x, 2)*begin_dy + 5.85395 *lens_ipow(begin_dy, 3)*begin_lambda + -1.29969e-06 *lens_ipow(begin_y, 3)*begin_lambda + -0.391177 *begin_x*lens_ipow(begin_dx, 3)*begin_dy + 58.129 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 3) + -5.96864e-07 *begin_x*lens_ipow(begin_y, 3)*begin_dx + 1.32679 *begin_x*begin_dx*lens_ipow(begin_dy, 3) + 0.0182734 *begin_x*begin_y*begin_dx*lens_ipow(begin_dy, 2) + -3.19863e-09 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3) + -6.23406 *lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 2) + -2.7888e-05 *lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 2) + 0.786317 *begin_y*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2) + 0.00738819 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 3) + 1.77456e-05 *lens_ipow(begin_x, 3)*begin_dx*begin_dy + -23.2769 *lens_ipow(begin_dx, 4)*begin_dy + 0.000103722 *lens_ipow(begin_x, 2)*begin_y*lens_ipow(begin_dy, 2) + 2.39665e-06 *lens_ipow(begin_y, 3)*lens_ipow(begin_lambda, 5) + 2.99896 *lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 6);
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
    domega2_dx0[0][0] =  + -0.0282279  + -0.000260254 *begin_lambda + -0.00214261 *begin_y*begin_dy + -0.00068755 *begin_x*begin_dx + -6.63026e-06 *lens_ipow(begin_y, 2) + -0.144619 *lens_ipow(begin_dy, 2) + -0.0045328 *lens_ipow(begin_dx, 2) + -2.31173e-05 *lens_ipow(begin_x, 2)*lens_ipow(begin_lambda, 2) + 9.70971e-05 *lens_ipow(begin_x, 2)*lens_ipow(begin_lambda, 4) + -0.00163849 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2) + 0.549814 *begin_x*begin_dx*lens_ipow(begin_dy, 4) + 49.9949 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 4) + -1.4523e-05 *lens_ipow(begin_x, 2)*begin_y*lens_ipow(begin_dy, 3) + -8.51808e-05 *lens_ipow(begin_x, 2)*lens_ipow(begin_lambda, 5) + -543.956 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 6) + -6.84923 *begin_x*begin_dx*lens_ipow(begin_dy, 6) + -0.151143 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 8)+0.0f;
    domega2_dx0[0][1] =  + -0.193971 *begin_dx*begin_dy + -0.00214261 *begin_x*begin_dy + -1.32605e-05 *begin_x*begin_y + -0.00126353 *begin_y*begin_dx + -0.239743 *begin_y*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 2) + -0.00327698 *begin_x*begin_y*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2) + -4.84099e-06 *lens_ipow(begin_x, 3)*lens_ipow(begin_dy, 3) + 6.17393e-06 *lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 3)*begin_lambda+0.0f;
    domega2_dx0[1][0] =  + 0.278569 *begin_dx*begin_dy + 0.000913623 *begin_y*begin_dx + 1.01635e-05 *begin_x*begin_y + 0.0023533 *begin_x*begin_dy + -0.391177 *lens_ipow(begin_dx, 3)*begin_dy + -5.96864e-07 *lens_ipow(begin_y, 3)*begin_dx + 1.32679 *begin_dx*lens_ipow(begin_dy, 3) + 0.0182734 *begin_y*begin_dx*lens_ipow(begin_dy, 2) + -6.39725e-09 *begin_x*lens_ipow(begin_y, 3) + 0.0147764 *begin_x*lens_ipow(begin_dy, 3) + 5.32368e-05 *lens_ipow(begin_x, 2)*begin_dx*begin_dy + 0.000207444 *begin_x*begin_y*lens_ipow(begin_dy, 2)+0.0f;
    domega2_dx0[1][1] =  + -0.0282638  + -0.000273248 *begin_lambda + 0.0476166 *lens_ipow(begin_dx, 2) + -0.00056558 *begin_y*begin_dy + 0.000913623 *begin_x*begin_dx + 5.08174e-06 *lens_ipow(begin_x, 2) + -3.89906e-06 *lens_ipow(begin_y, 2)*begin_lambda + -1.79059e-06 *begin_x*lens_ipow(begin_y, 2)*begin_dx + 0.0182734 *begin_x*begin_dx*lens_ipow(begin_dy, 2) + -9.59588e-09 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2) + -8.36641e-05 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2) + 0.786317 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2) + 0.000103722 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 2) + 7.18995e-06 *lens_ipow(begin_y, 2)*lens_ipow(begin_lambda, 5)+0.0f;
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
  out[4] =  + 3.76112 *begin_lambda + 0.000259609 *begin_y*begin_dy + 0.000266756 *begin_x*begin_dx + 0.0168177 *lens_ipow(begin_dy, 2) + 0.0171709 *lens_ipow(begin_dx, 2) + -10.7968 *lens_ipow(begin_lambda, 2) + 16.4369 *lens_ipow(begin_lambda, 3) + -12.9412 *lens_ipow(begin_lambda, 4) + -7.32614 *lens_ipow(begin_dx, 4) + -0.267851 *begin_y*lens_ipow(begin_dy, 3) + -0.00124542 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2) + -0.268464 *begin_x*lens_ipow(begin_dx, 3) + -14.667 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2) + -2.35604e-05 *lens_ipow(begin_x, 2)*begin_y*begin_dy + -7.31341 *lens_ipow(begin_dy, 4) + -2.35063e-05 *begin_x*lens_ipow(begin_y, 2)*begin_dx + -1.1016e-07 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2) + -5.37091e-08 *lens_ipow(begin_y, 4) + -0.003764 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 2) + -0.268213 *begin_y*lens_ipow(begin_dx, 2)*begin_dy + -2.36431e-05 *lens_ipow(begin_x, 3)*begin_dx + -0.00375249 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 2) + -0.26844 *begin_x*begin_dx*lens_ipow(begin_dy, 2) + -5.39032e-08 *lens_ipow(begin_x, 4) + -0.00500763 *begin_x*begin_y*begin_dx*begin_dy + -0.00124868 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 2) + -2.35432e-05 *lens_ipow(begin_y, 3)*begin_dy + 4.15947 *lens_ipow(begin_lambda, 5);
else
  out[4] = 0.0f;

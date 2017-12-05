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
       + 46.3231 *begin_dx + 0.713762 *begin_x + -0.203224 *begin_x*begin_lambda + -8.49003 *begin_dx*begin_lambda + 1.48432 *begin_y*begin_dx*begin_dy + 0.136162 *begin_x*lens_ipow(begin_lambda, 2) + 19.351 *begin_dx*lens_ipow(begin_dy, 2) + 0.000222426 *lens_ipow(begin_x, 3) + 0.0178375 *lens_ipow(begin_y, 2)*begin_dx + 6.06598 *begin_dx*lens_ipow(begin_lambda, 2) + 6.93669 *begin_x*lens_ipow(begin_dx, 2)*begin_lambda + 78.5921 *lens_ipow(begin_dx, 3)*begin_lambda + 0.657095 *begin_x*lens_ipow(begin_dy, 2)*begin_lambda + 0.00080037 *begin_x*lens_ipow(begin_y, 2)*begin_lambda + 0.0599381 *begin_x*begin_y*begin_dy*begin_lambda + 0.152641 *lens_ipow(begin_x, 2)*begin_dx*begin_lambda + -78.7821 *lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 2) + -0.000697152 *begin_x*lens_ipow(begin_y, 2)*lens_ipow(begin_lambda, 2) + -6.68444 *begin_x*lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 2) + -0.14429 *lens_ipow(begin_x, 2)*begin_dx*lens_ipow(begin_lambda, 2) + -0.0622761 *begin_x*begin_y*begin_dy*lens_ipow(begin_lambda, 3) + -1.17139 *begin_x*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 5) + 241.074 *begin_y*lens_ipow(begin_dx, 5)*lens_ipow(begin_dy, 3) + 438829 *lens_ipow(begin_dx, 9)*lens_ipow(begin_dy, 2) + 41684.5 *begin_x*lens_ipow(begin_dx, 8)*lens_ipow(begin_dy, 2) + 14.6511 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 6)*lens_ipow(begin_dy, 2) + 57564.4 *lens_ipow(begin_dx, 5)*lens_ipow(begin_dy, 6) + 1344.32 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 7)*lens_ipow(begin_dy, 2),
       + 0.711668 *begin_y + 46.3848 *begin_dy + -8.46928 *begin_dy*begin_lambda + -0.193319 *begin_y*begin_lambda + 0.286513 *begin_y*lens_ipow(begin_dx, 2) + 1.47114 *begin_x*begin_dx*begin_dy + 0.0219261 *begin_x*begin_y*begin_dx + 18.7219 *lens_ipow(begin_dx, 2)*begin_dy + 0.128495 *begin_y*lens_ipow(begin_lambda, 2) + 0.000225772 *lens_ipow(begin_x, 2)*begin_y + 6.05942 *begin_dy*lens_ipow(begin_lambda, 2) + 0.000205517 *lens_ipow(begin_y, 3) + 0.0176433 *lens_ipow(begin_x, 2)*begin_dy + 6.17418 *begin_y*lens_ipow(begin_dy, 2)*begin_lambda + 63.0907 *lens_ipow(begin_dy, 3)*begin_lambda + 0.140338 *lens_ipow(begin_y, 2)*begin_dy*begin_lambda + -0.132736 *lens_ipow(begin_y, 2)*begin_dy*lens_ipow(begin_lambda, 2) + -64.8887 *lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 2) + -5.97842 *begin_y*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + 11183.1 *lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 5) + 396.191 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 7) + 42860.5 *lens_ipow(begin_dy, 9) + 0.0130309 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 3) + 6467.85 *begin_y*lens_ipow(begin_dy, 8) + 0.189225 *lens_ipow(begin_y, 4)*lens_ipow(begin_dy, 5) + 0.00117308 *lens_ipow(begin_y, 5)*lens_ipow(begin_dy, 4) + 12.2293 *lens_ipow(begin_y, 3)*lens_ipow(begin_dy, 6) + -24.246 *begin_x*begin_y*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 4)
    };
    const float delta_ap[] = {ap_x - pred_ap[0], ap_y - pred_ap[1]};
    sqr_ap_err = delta_ap[0]*delta_ap[0]+delta_ap[1]*delta_ap[1];
    float dx1_domega0[2][2];
    dx1_domega0[0][0] =  + 46.3231  + -8.49003 *begin_lambda + 1.48432 *begin_y*begin_dy + 19.351 *lens_ipow(begin_dy, 2) + 0.0178375 *lens_ipow(begin_y, 2) + 6.06598 *lens_ipow(begin_lambda, 2) + 13.8734 *begin_x*begin_dx*begin_lambda + 235.776 *lens_ipow(begin_dx, 2)*begin_lambda + 0.152641 *lens_ipow(begin_x, 2)*begin_lambda + -236.346 *lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 2) + -13.3689 *begin_x*begin_dx*lens_ipow(begin_lambda, 2) + -0.14429 *lens_ipow(begin_x, 2)*lens_ipow(begin_lambda, 2) + 1205.37 *begin_y*lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 3) + 3.94946e+06 *lens_ipow(begin_dx, 8)*lens_ipow(begin_dy, 2) + 333476 *begin_x*lens_ipow(begin_dx, 7)*lens_ipow(begin_dy, 2) + 87.9067 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 5)*lens_ipow(begin_dy, 2) + 287822 *lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 6) + 9410.21 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 6)*lens_ipow(begin_dy, 2)+0.0f;
    dx1_domega0[0][1] =  + 1.48432 *begin_y*begin_dx + 38.7021 *begin_dx*begin_dy + 1.31419 *begin_x*begin_dy*begin_lambda + 0.0599381 *begin_x*begin_y*begin_lambda + -0.0622761 *begin_x*begin_y*lens_ipow(begin_lambda, 3) + -2.34278 *begin_x*begin_dy*lens_ipow(begin_lambda, 5) + 723.221 *begin_y*lens_ipow(begin_dx, 5)*lens_ipow(begin_dy, 2) + 877658 *lens_ipow(begin_dx, 9)*begin_dy + 83369 *begin_x*lens_ipow(begin_dx, 8)*begin_dy + 29.3022 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 6)*begin_dy + 345386 *lens_ipow(begin_dx, 5)*lens_ipow(begin_dy, 5) + 2688.63 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 7)*begin_dy+0.0f;
    dx1_domega0[1][0] =  + 0.573025 *begin_y*begin_dx + 1.47114 *begin_x*begin_dy + 0.0219261 *begin_x*begin_y + 37.4438 *begin_dx*begin_dy + 44732.5 *lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 5) + 0.0260617 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*begin_dx*lens_ipow(begin_dy, 3) + -72.738 *begin_x*begin_y*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 4)+0.0f;
    dx1_domega0[1][1] =  + 46.3848  + -8.46928 *begin_lambda + 1.47114 *begin_x*begin_dx + 18.7219 *lens_ipow(begin_dx, 2) + 6.05942 *lens_ipow(begin_lambda, 2) + 0.0176433 *lens_ipow(begin_x, 2) + 12.3484 *begin_y*begin_dy*begin_lambda + 189.272 *lens_ipow(begin_dy, 2)*begin_lambda + 0.140338 *lens_ipow(begin_y, 2)*begin_lambda + -0.132736 *lens_ipow(begin_y, 2)*lens_ipow(begin_lambda, 2) + -194.666 *lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + -11.9568 *begin_y*begin_dy*lens_ipow(begin_lambda, 2) + 55915.7 *lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 4) + 2773.33 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 6) + 385744 *lens_ipow(begin_dy, 8) + 0.0390926 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2) + 51742.8 *begin_y*lens_ipow(begin_dy, 7) + 0.946123 *lens_ipow(begin_y, 4)*lens_ipow(begin_dy, 4) + 0.00469234 *lens_ipow(begin_y, 5)*lens_ipow(begin_dy, 3) + 73.3758 *lens_ipow(begin_y, 3)*lens_ipow(begin_dy, 5) + -96.9839 *begin_x*begin_y*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 3)+0.0f;
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
    out[0] =  + -8.23478 *begin_x + -23.6448 *begin_dx*begin_lambda + 0.209775 *lens_ipow(begin_x, 2)*begin_dx + -0.00567908 *begin_x*lens_ipow(begin_y, 2) + 7.19223 *begin_x*lens_ipow(begin_dy, 2) + -0.0100983 *lens_ipow(begin_x, 3)*begin_lambda + 0.302739 *begin_x*begin_y*begin_dy*begin_lambda + 1207.94 *begin_x*lens_ipow(begin_dx, 4) + 31.3281 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 3) + 10503.8 *lens_ipow(begin_dx, 5) + 0.350912 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 2) + 8722.78 *lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 2) + 1038.42 *begin_y*lens_ipow(begin_dx, 3)*begin_dy*begin_lambda + 3375.71 *begin_dx*lens_ipow(begin_dy, 4)*begin_lambda + -0.000607626 *lens_ipow(begin_x, 4)*begin_y*begin_dx*begin_dy + -1941.37 *begin_x*lens_ipow(begin_dx, 6) + 18.8515 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 2) + -8484.66 *begin_y*lens_ipow(begin_dx, 5)*begin_dy*begin_lambda + 14101.2 *begin_y*lens_ipow(begin_dx, 7)*begin_dy + -157.75 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 5)*lens_ipow(begin_lambda, 3) + 987.563 *begin_x*lens_ipow(begin_dx, 8)*begin_dy + 1.10767e-08 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 6)*begin_lambda + 56171.5 *begin_x*lens_ipow(begin_dx, 8)*lens_ipow(begin_dy, 2) + 1.62373e-08 *lens_ipow(begin_x, 7)*lens_ipow(begin_y, 2)*lens_ipow(begin_lambda, 2) + 12.4767 *lens_ipow(begin_y, 3)*begin_dx*lens_ipow(begin_dy, 7) + -3.06799 *lens_ipow(begin_x, 4)*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 4) + -958.675 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 9) + -6.1304 *begin_x*lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 3);
    out[1] =  + -8.37596 *begin_y + -14.0792 *begin_dy + 8.14436 *begin_y*lens_ipow(begin_dx, 2) + 0.642083 *lens_ipow(begin_y, 2)*begin_dy + 30.1401 *begin_y*lens_ipow(begin_dy, 2) + 4.61396 *begin_x*begin_dx*begin_dy + 0.213958 *begin_x*begin_y*begin_dx + -0.00395195 *lens_ipow(begin_x, 2)*begin_y + 294.206 *lens_ipow(begin_dy, 3) + -0.00520918 *lens_ipow(begin_y, 3)*begin_lambda + 9648.97 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 3) + 374.136 *begin_x*begin_dx*lens_ipow(begin_dy, 3) + 6.27376 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 3) + 2263.15 *lens_ipow(begin_dx, 4)*begin_dy + -1.22252e-05 *lens_ipow(begin_y, 6)*begin_dy + 41765.7 *lens_ipow(begin_dy, 7) + 1.13934 *lens_ipow(begin_x, 2)*begin_y*lens_ipow(begin_dy, 4) + 1751.59 *begin_y*lens_ipow(begin_dy, 6) + -1.31667 *begin_x*lens_ipow(begin_y, 2)*begin_dx*lens_ipow(begin_dy, 3) + 0.555329 *lens_ipow(begin_y, 4)*lens_ipow(begin_dy, 5) + -0.0200994 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2) + 10.3339 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 7)*begin_dy + -3.3773 *lens_ipow(begin_x, 3)*begin_y*lens_ipow(begin_dx, 5)*lens_ipow(begin_dy, 2) + -3.27086 *lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 5) + -2.40173 *lens_ipow(begin_y, 4)*lens_ipow(begin_dy, 7) + 1.06144e-10 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 7) + -0.000123388 *begin_x*lens_ipow(begin_y, 6)*begin_dx*lens_ipow(begin_dy, 3) + 0.379743 *begin_x*lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 3);
    out[2] =  + -0.103083 *begin_x + -0.601438 *begin_dx*begin_lambda + 0.234038 *begin_y*begin_dx*begin_dy + 0.00352225 *begin_x*begin_y*begin_dy + 5.38731 *begin_dx*lens_ipow(begin_dy, 2) + 4.57178e-05 *begin_x*lens_ipow(begin_y, 2) + 0.000216091 *lens_ipow(begin_x, 3) + 0.107377 *begin_x*lens_ipow(begin_dy, 2) + 0.852366 *begin_x*lens_ipow(begin_dx, 2)*begin_lambda + 0.0184197 *lens_ipow(begin_x, 2)*begin_dx*begin_lambda + 0.0040717 *lens_ipow(begin_y, 2)*begin_dx*begin_lambda + 17.8917 *lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 2) + -0.000423037 *lens_ipow(begin_x, 2)*begin_y*begin_dx*begin_dy + -0.0197977 *lens_ipow(begin_x, 2)*begin_dx*lens_ipow(begin_lambda, 3) + 71.8647 *lens_ipow(begin_dx, 5)*begin_lambda + 9.52493e-05 *lens_ipow(begin_x, 4)*lens_ipow(begin_dx, 3) + -0.00668964 *lens_ipow(begin_y, 3)*begin_dx*lens_ipow(begin_dy, 3) + 0.000421438 *lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 3) + -4.09222e-09 *begin_x*lens_ipow(begin_y, 6) + -1.25617 *begin_x*lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 4) + 455.971 *lens_ipow(begin_dx, 5)*lens_ipow(begin_dy, 2) + -0.000581608 *lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 3)*begin_lambda + -492.467 *lens_ipow(begin_dx, 5)*lens_ipow(begin_lambda, 4) + 132.875 *begin_x*lens_ipow(begin_dx, 6)*lens_ipow(begin_dy, 2) + 61.7322 *begin_x*lens_ipow(begin_dx, 8)*begin_lambda + 1828.09 *lens_ipow(begin_dx, 7)*lens_ipow(begin_lambda, 4) + -1.09217e-05 *lens_ipow(begin_y, 6)*begin_dx*lens_ipow(begin_dy, 4) + 0.00256169 *begin_x*lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 2);
    out[3] =  + -0.103601 *begin_y + -0.713373 *begin_dy*begin_lambda + 0.000378384 *lens_ipow(begin_x, 2)*begin_y + 0.000247393 *lens_ipow(begin_y, 3) + 0.129964 *begin_y*lens_ipow(begin_dx, 2)*begin_lambda + 0.00402109 *lens_ipow(begin_x, 2)*begin_dy*begin_lambda + 0.0165033 *lens_ipow(begin_y, 2)*begin_dy*begin_lambda + 0.00729411 *begin_x*begin_y*begin_dx*begin_lambda + 0.693063 *begin_x*begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + 0.409198 *begin_dy*lens_ipow(begin_lambda, 4) + 32.9633 *lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 2) + -1.96139e-07 *lens_ipow(begin_y, 5) + 18.4048 *lens_ipow(begin_dx, 2)*begin_dy*lens_ipow(begin_lambda, 2) + 2.80892 *begin_y*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + 0.563506 *begin_y*lens_ipow(begin_dx, 4)*begin_lambda + -0.0130565 *lens_ipow(begin_y, 2)*begin_dy*lens_ipow(begin_lambda, 3) + 7.74914e-07 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3)*begin_lambda + -3.11538 *begin_y*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 3) + -44.8493 *lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 4) + 0.00454339 *lens_ipow(begin_x, 2)*begin_dy*lens_ipow(begin_lambda, 4) + 0.00015627 *lens_ipow(begin_x, 2)*begin_y*lens_ipow(begin_lambda, 5) + 10.8648 *begin_x*begin_dx*lens_ipow(begin_dy, 5)*lens_ipow(begin_lambda, 2) + -16.3559 *begin_y*lens_ipow(begin_dy, 8) + 0.509567 *begin_x*begin_y*begin_dx*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 6) + -0.000484984 *lens_ipow(begin_y, 5)*lens_ipow(begin_dy, 6) + -0.0186708 *lens_ipow(begin_y, 4)*lens_ipow(begin_dy, 7) + -7.7512e-10 *lens_ipow(begin_x, 6)*lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 2) + 12.1926 *begin_y*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 6);
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
    domega2_dx0[0][0] =  + -0.103083  + 0.00352225 *begin_y*begin_dy + 4.57178e-05 *lens_ipow(begin_y, 2) + 0.000648272 *lens_ipow(begin_x, 2) + 0.107377 *lens_ipow(begin_dy, 2) + 0.852366 *lens_ipow(begin_dx, 2)*begin_lambda + 0.0368395 *begin_x*begin_dx*begin_lambda + -0.000846074 *begin_x*begin_y*begin_dx*begin_dy + -0.0395955 *begin_x*begin_dx*lens_ipow(begin_lambda, 3) + 0.000380997 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 3) + -4.09222e-09 *lens_ipow(begin_y, 6) + -1.25617 *lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 4) + 132.875 *lens_ipow(begin_dx, 6)*lens_ipow(begin_dy, 2) + 61.7322 *lens_ipow(begin_dx, 8)*begin_lambda + 0.00256169 *lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 2)+0.0f;
    domega2_dx0[0][1] =  + 0.234038 *begin_dx*begin_dy + 0.00352225 *begin_x*begin_dy + 9.14357e-05 *begin_x*begin_y + 0.00814339 *begin_y*begin_dx*begin_lambda + -0.000423037 *lens_ipow(begin_x, 2)*begin_dx*begin_dy + -0.0200689 *lens_ipow(begin_y, 2)*begin_dx*lens_ipow(begin_dy, 3) + 0.00168575 *lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 3) + -2.45533e-08 *begin_x*lens_ipow(begin_y, 5) + -0.00232643 *lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 3)*begin_lambda + -6.55305e-05 *lens_ipow(begin_y, 5)*begin_dx*lens_ipow(begin_dy, 4) + 0.0102467 *begin_x*lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 2)+0.0f;
    domega2_dx0[1][0] =  + 0.000756768 *begin_x*begin_y + 0.00804217 *begin_x*begin_dy*begin_lambda + 0.00729411 *begin_y*begin_dx*begin_lambda + 0.693063 *begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + 1.54983e-06 *begin_x*lens_ipow(begin_y, 3)*begin_lambda + 0.00908677 *begin_x*begin_dy*lens_ipow(begin_lambda, 4) + 0.00031254 *begin_x*begin_y*lens_ipow(begin_lambda, 5) + 10.8648 *begin_dx*lens_ipow(begin_dy, 5)*lens_ipow(begin_lambda, 2) + 0.509567 *begin_y*begin_dx*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 6) + -4.65072e-09 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 2)+0.0f;
    domega2_dx0[1][1] =  + -0.103601  + 0.000378384 *lens_ipow(begin_x, 2) + 0.000742179 *lens_ipow(begin_y, 2) + 0.129964 *lens_ipow(begin_dx, 2)*begin_lambda + 0.0330067 *begin_y*begin_dy*begin_lambda + 0.00729411 *begin_x*begin_dx*begin_lambda + -9.80694e-07 *lens_ipow(begin_y, 4) + 2.80892 *lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + 0.563506 *lens_ipow(begin_dx, 4)*begin_lambda + -0.026113 *begin_y*begin_dy*lens_ipow(begin_lambda, 3) + 2.32474e-06 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*begin_lambda + -3.11538 *lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 3) + 0.00015627 *lens_ipow(begin_x, 2)*lens_ipow(begin_lambda, 5) + -16.3559 *lens_ipow(begin_dy, 8) + 0.509567 *begin_x*begin_dx*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 6) + -0.00242492 *lens_ipow(begin_y, 4)*lens_ipow(begin_dy, 6) + -0.0746833 *lens_ipow(begin_y, 3)*lens_ipow(begin_dy, 7) + -2.32536e-09 *lens_ipow(begin_x, 6)*lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2) + 12.1926 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 6)+0.0f;
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
  out[4] =  + 0.196001  + 0.378559 *begin_lambda + 0.128762 *lens_ipow(begin_dy, 2) + -0.267516 *lens_ipow(begin_lambda, 2) + -3.43289 *lens_ipow(begin_dy, 4) + -5.30271e-06 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2) + -0.00124765 *begin_x*lens_ipow(begin_y, 2)*begin_dx*lens_ipow(begin_dy, 2) + -1.33218e-08 *lens_ipow(begin_x, 6) + -1.27157e-08 *lens_ipow(begin_y, 6) + -331.504 *lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 2) + -22.8135 *begin_y*lens_ipow(begin_dy, 7) + -0.435257 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 6) + 4657.45 *lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 4) + -0.644826 *begin_x*lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 4) + 4717.32 *lens_ipow(begin_dx, 6)*lens_ipow(begin_dy, 2) + 7.77492 *begin_x*lens_ipow(begin_dx, 5)*lens_ipow(begin_lambda, 2) + 129.97 *lens_ipow(begin_dy, 8)*begin_lambda + -44.8792 *begin_x*lens_ipow(begin_dx, 7)*begin_lambda + -2092.11 *lens_ipow(begin_dy, 10) + -3144.49 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 8) + -20701.6 *lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 6) + -42660.6 *lens_ipow(begin_dx, 6)*lens_ipow(begin_dy, 4) + -19041.8 *lens_ipow(begin_dx, 8)*lens_ipow(begin_dy, 2) + -1799.27 *lens_ipow(begin_dx, 10) + 31.9804 *begin_x*begin_y*lens_ipow(begin_dx, 5)*lens_ipow(begin_dy, 3)*begin_lambda + 0.0626751 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 7)*begin_lambda + 5.56252e-07 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 4)*begin_dx*lens_ipow(begin_dy, 2)*begin_lambda + -0.0101147 *begin_x*lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 5)*begin_dy*begin_lambda;
else
  out[4] = 0.0f;

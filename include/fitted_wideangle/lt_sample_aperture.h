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
       + 33.436 *begin_dx + 0.620268 *begin_x + -0.0997683 *begin_x*begin_lambda + -1.02423 *begin_dx*begin_lambda + 0.0779104 *begin_x*begin_y*begin_dy + 0.11032 *lens_ipow(begin_x, 2)*begin_dx + 22.3258 *lens_ipow(begin_dx, 3) + 0.066893 *begin_x*lens_ipow(begin_lambda, 2) + 28.7753 *begin_dx*lens_ipow(begin_dy, 2) + 0.00114721 *begin_x*lens_ipow(begin_y, 2) + 0.00104281 *lens_ipow(begin_x, 3) + 1.05782 *begin_x*lens_ipow(begin_dy, 2) + 3.30806 *begin_x*lens_ipow(begin_dx, 2) + 0.622831 *begin_dx*lens_ipow(begin_lambda, 3) + 0.252479 *lens_ipow(begin_y, 2)*begin_dx*begin_lambda + 15.3869 *begin_y*begin_dx*begin_dy*begin_lambda + 0.000265991 *lens_ipow(begin_x, 4)*begin_dx + 17.5869 *begin_x*lens_ipow(begin_dx, 4) + 0.849033 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 3) + -28.4185 *begin_y*begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + 147.745 *lens_ipow(begin_dx, 5) + 1.40136e-06 *lens_ipow(begin_x, 5) + -0.469428 *lens_ipow(begin_y, 2)*begin_dx*lens_ipow(begin_lambda, 2) + 0.020917 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 2) + 17.1989 *begin_y*begin_dx*begin_dy*lens_ipow(begin_lambda, 3) + 0.285723 *lens_ipow(begin_y, 2)*begin_dx*lens_ipow(begin_lambda, 3) + 4.60929e-13 *lens_ipow(begin_x, 7)*lens_ipow(begin_y, 4) + -3.19292e-09 *lens_ipow(begin_x, 6)*lens_ipow(begin_y, 3)*begin_dx*begin_dy,
       + 0.620382 *begin_y + 33.2697 *begin_dy + -0.0988428 *begin_y*begin_lambda + 1.03903 *begin_y*lens_ipow(begin_dx, 2) + 0.108851 *lens_ipow(begin_y, 2)*begin_dy + 3.26034 *begin_y*lens_ipow(begin_dy, 2) + 2.71107 *begin_x*begin_dx*begin_dy + 0.0773918 *begin_x*begin_y*begin_dx + 28.2928 *lens_ipow(begin_dx, 2)*begin_dy + 0.0661776 *begin_y*lens_ipow(begin_lambda, 2) + 0.00113968 *lens_ipow(begin_x, 2)*begin_y + -1.94549 *begin_dy*lens_ipow(begin_lambda, 2) + 0.00102739 *lens_ipow(begin_y, 3) + 0.0442643 *lens_ipow(begin_x, 2)*begin_dy + 21.7286 *lens_ipow(begin_dy, 3) + 1.83525 *begin_dy*lens_ipow(begin_lambda, 3) + 0.000304092 *lens_ipow(begin_y, 4)*begin_dy + 156.877 *lens_ipow(begin_dy, 5) + 0.920891 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 3) + 18.733 *begin_y*lens_ipow(begin_dy, 4) + 1.5905e-06 *lens_ipow(begin_y, 5) + 0.0233247 *lens_ipow(begin_y, 3)*lens_ipow(begin_dy, 2) + 9.47641e-06 *lens_ipow(begin_y, 5)*lens_ipow(begin_dx, 2) + 0.000414172 *lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 2)*begin_dy + 7.77217e-09 *begin_x*lens_ipow(begin_y, 7)*begin_dx*begin_lambda + 3.2448e-13 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 9) + 2.5744e-13 *lens_ipow(begin_x, 8)*lens_ipow(begin_y, 3) + 6.2086e-07 *begin_x*lens_ipow(begin_y, 6)*begin_dx*begin_dy*lens_ipow(begin_lambda, 2)
    };
    const float delta_ap[] = {ap_x - pred_ap[0], ap_y - pred_ap[1]};
    sqr_ap_err = delta_ap[0]*delta_ap[0]+delta_ap[1]*delta_ap[1];
    float dx1_domega0[2][2];
    dx1_domega0[0][0] =  + 33.436  + -1.02423 *begin_lambda + 0.11032 *lens_ipow(begin_x, 2) + 66.9774 *lens_ipow(begin_dx, 2) + 28.7753 *lens_ipow(begin_dy, 2) + 6.61613 *begin_x*begin_dx + 0.622831 *lens_ipow(begin_lambda, 3) + 0.252479 *lens_ipow(begin_y, 2)*begin_lambda + 15.3869 *begin_y*begin_dy*begin_lambda + 0.000265991 *lens_ipow(begin_x, 4) + 70.3475 *begin_x*lens_ipow(begin_dx, 3) + 2.5471 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 2) + -28.4185 *begin_y*begin_dy*lens_ipow(begin_lambda, 2) + 738.725 *lens_ipow(begin_dx, 4) + -0.469428 *lens_ipow(begin_y, 2)*lens_ipow(begin_lambda, 2) + 0.041834 *lens_ipow(begin_x, 3)*begin_dx + 17.1989 *begin_y*begin_dy*lens_ipow(begin_lambda, 3) + 0.285723 *lens_ipow(begin_y, 2)*lens_ipow(begin_lambda, 3) + -3.19292e-09 *lens_ipow(begin_x, 6)*lens_ipow(begin_y, 3)*begin_dy+0.0f;
    dx1_domega0[0][1] =  + 0.0779104 *begin_x*begin_y + 57.5505 *begin_dx*begin_dy + 2.11564 *begin_x*begin_dy + 15.3869 *begin_y*begin_dx*begin_lambda + -28.4185 *begin_y*begin_dx*lens_ipow(begin_lambda, 2) + 17.1989 *begin_y*begin_dx*lens_ipow(begin_lambda, 3) + -3.19292e-09 *lens_ipow(begin_x, 6)*lens_ipow(begin_y, 3)*begin_dx+0.0f;
    dx1_domega0[1][0] =  + 2.07807 *begin_y*begin_dx + 2.71107 *begin_x*begin_dy + 0.0773918 *begin_x*begin_y + 56.5856 *begin_dx*begin_dy + 1.89528e-05 *lens_ipow(begin_y, 5)*begin_dx + 0.000828343 *lens_ipow(begin_y, 4)*begin_dx*begin_dy + 7.77217e-09 *begin_x*lens_ipow(begin_y, 7)*begin_lambda + 6.2086e-07 *begin_x*lens_ipow(begin_y, 6)*begin_dy*lens_ipow(begin_lambda, 2)+0.0f;
    dx1_domega0[1][1] =  + 33.2697  + 0.108851 *lens_ipow(begin_y, 2) + 6.52068 *begin_y*begin_dy + 2.71107 *begin_x*begin_dx + 28.2928 *lens_ipow(begin_dx, 2) + -1.94549 *lens_ipow(begin_lambda, 2) + 0.0442643 *lens_ipow(begin_x, 2) + 65.1857 *lens_ipow(begin_dy, 2) + 1.83525 *lens_ipow(begin_lambda, 3) + 0.000304092 *lens_ipow(begin_y, 4) + 784.387 *lens_ipow(begin_dy, 4) + 2.76267 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 2) + 74.9321 *begin_y*lens_ipow(begin_dy, 3) + 0.0466494 *lens_ipow(begin_y, 3)*begin_dy + 0.000414172 *lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 2) + 6.2086e-07 *begin_x*lens_ipow(begin_y, 6)*begin_dx*lens_ipow(begin_lambda, 2)+0.0f;
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
    out[0] =  + 16.7222 *begin_dx + -1.7126 *begin_x + -0.407092 *begin_x*begin_lambda + -1.50065 *begin_dx*begin_lambda + 2.13945 *begin_y*begin_dx*begin_dy + 0.122543 *lens_ipow(begin_x, 2)*begin_dx + 25.686 *lens_ipow(begin_dx, 3) + 0.265989 *begin_x*lens_ipow(begin_lambda, 2) + 29.8535 *begin_dx*lens_ipow(begin_dy, 2) + 0.00320318 *begin_x*lens_ipow(begin_y, 2) + 0.00349548 *lens_ipow(begin_x, 3) + 1.05804 *begin_x*lens_ipow(begin_dy, 2) + 3.24433 *begin_x*lens_ipow(begin_dx, 2) + 0.0495496 *lens_ipow(begin_y, 2)*begin_dx + -1.2933 *begin_x*lens_ipow(begin_dx, 2)*begin_lambda + 0.30668 *begin_x*begin_y*begin_dy*begin_lambda + -0.83918 *begin_y*begin_dx*begin_dy*begin_lambda + -3.57407e-06 *lens_ipow(begin_x, 5) + 0.298459 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 3) + -0.388491 *begin_x*begin_y*begin_dy*lens_ipow(begin_lambda, 2) + 0.0189128 *begin_x*lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2)*begin_lambda + -0.000272989 *lens_ipow(begin_x, 4)*begin_dx*begin_lambda + 0.000109252 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 2) + -7.08814e-08 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 4) + 0.00565248 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*begin_dx*lens_ipow(begin_dy, 2) + 0.225298 *begin_x*begin_y*begin_dy*lens_ipow(begin_lambda, 5) + -0.112286 *begin_x*lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 4)*begin_dy + -7.93361e-10 *lens_ipow(begin_x, 7)*lens_ipow(begin_y, 2)*lens_ipow(begin_lambda, 2);
    out[1] =  + -1.72441 *begin_y + 15.9053 *begin_dy + -0.377417 *begin_y*begin_lambda + 0.96701 *begin_y*lens_ipow(begin_dx, 2) + 0.174206 *lens_ipow(begin_y, 2)*begin_dy + 5.43202 *begin_y*lens_ipow(begin_dy, 2) + 1.53423 *begin_x*begin_dx*begin_dy + 26.636 *lens_ipow(begin_dx, 2)*begin_dy + 0.252523 *begin_y*lens_ipow(begin_lambda, 2) + 0.00320864 *lens_ipow(begin_x, 2)*begin_y + 0.0034628 *lens_ipow(begin_y, 3) + 0.0508916 *lens_ipow(begin_x, 2)*begin_dy + 61.5968 *lens_ipow(begin_dy, 3) + -5.08006 *begin_y*lens_ipow(begin_dy, 2)*begin_lambda + -60.4737 *lens_ipow(begin_dy, 3)*begin_lambda + -0.0976647 *lens_ipow(begin_y, 2)*begin_dy*begin_lambda + 0.234352 *begin_x*begin_y*begin_dx*begin_lambda + 0.00415282 *lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 2) + -0.000238714 *lens_ipow(begin_y, 4)*begin_dy*begin_lambda + 0.36372 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2)*begin_dy*begin_lambda + -6.27167e-06 *lens_ipow(begin_y, 5)*begin_lambda + -0.807132 *begin_x*begin_y*begin_dx*lens_ipow(begin_lambda, 3) + -0.00640517 *lens_ipow(begin_x, 3)*begin_y*begin_dx*lens_ipow(begin_dy, 2) + 0.722321 *begin_x*begin_y*begin_dx*lens_ipow(begin_lambda, 4) + -5.45787e-08 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 5)*begin_lambda + -6.20086e-10 *lens_ipow(begin_x, 6)*lens_ipow(begin_y, 3) + 0.0654266 *lens_ipow(begin_x, 4)*lens_ipow(begin_dy, 5) + 0.0037559 *lens_ipow(begin_x, 4)*begin_y*lens_ipow(begin_dy, 4)*begin_lambda;
    out[2] =  + -0.334003 *begin_dx + -0.026849 *begin_x + 0.0120662 *begin_x*begin_lambda + -0.0196292 *begin_dx*begin_lambda + 0.49947 *lens_ipow(begin_dx, 3) + -0.00827016 *begin_x*lens_ipow(begin_lambda, 2) + 0.435908 *begin_dx*lens_ipow(begin_dy, 2) + -1.03771e-05 *begin_x*lens_ipow(begin_y, 2) + 0.00186831 *begin_x*lens_ipow(begin_dy, 2) + 0.015388 *begin_x*lens_ipow(begin_dx, 2) + 0.00014192 *lens_ipow(begin_y, 2)*begin_dx + 9.35478e-05 *lens_ipow(begin_x, 3)*begin_lambda + -0.000392059 *lens_ipow(begin_x, 2)*begin_dx*begin_lambda + 0.0477328 *begin_y*begin_dx*begin_dy*begin_lambda + -0.0481053 *begin_y*begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + -0.000148114 *lens_ipow(begin_x, 3)*lens_ipow(begin_lambda, 2) + -0.0221637 *begin_x*lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 2) + -4.44502e-08 *lens_ipow(begin_x, 5) + -0.00671039 *begin_x*begin_y*begin_dy*lens_ipow(begin_lambda, 2) + 0.0174403 *begin_x*begin_y*begin_dy*lens_ipow(begin_lambda, 3) + -1.96719e-06 *lens_ipow(begin_x, 3)*begin_y*begin_dy*begin_lambda + -2.82448e-06 *lens_ipow(begin_x, 4)*begin_dx*begin_lambda + 1.35905e-06 *begin_x*lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 2) + 8.77094e-05 *lens_ipow(begin_x, 3)*lens_ipow(begin_lambda, 4) + 4.46548e-05 *lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 3) + -0.0124094 *begin_x*begin_y*begin_dy*lens_ipow(begin_lambda, 4) + 0.00219052 *lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 3)*begin_dy*begin_lambda + -2.70974 *lens_ipow(begin_dx, 5)*lens_ipow(begin_lambda, 3);
    out[3] =  + -0.0266125 *begin_y + -0.334066 *begin_dy + -0.0170558 *begin_dy*begin_lambda + 0.0114614 *begin_y*begin_lambda + -0.00511334 *begin_y*lens_ipow(begin_dx, 2) + 0.0129436 *begin_y*lens_ipow(begin_dy, 2) + 0.00896579 *begin_x*begin_dx*begin_dy + 0.477093 *lens_ipow(begin_dx, 2)*begin_dy + -0.00789395 *begin_y*lens_ipow(begin_lambda, 2) + 3.90402e-05 *lens_ipow(begin_x, 2)*begin_y + -0.000338784 *lens_ipow(begin_x, 2)*begin_dy + 0.457993 *lens_ipow(begin_dy, 3) + -0.000567731 *lens_ipow(begin_y, 2)*begin_dy*begin_lambda + 5.55763e-05 *lens_ipow(begin_y, 3)*begin_lambda + -2.51e-06 *begin_x*lens_ipow(begin_y, 3)*begin_dx + -1.24306e-07 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3) + -1.92408e-06 *lens_ipow(begin_x, 3)*begin_y*begin_dx + -9.87323e-08 *lens_ipow(begin_x, 4)*begin_y + -2.58648e-06 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*begin_dy + -0.0150597 *begin_y*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + 0.00600884 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2)*begin_dy*begin_lambda + -0.000216745 *lens_ipow(begin_y, 3)*lens_ipow(begin_lambda, 3) + -2.56526e-10 *lens_ipow(begin_y, 7) + 0.000199 *lens_ipow(begin_y, 3)*lens_ipow(begin_lambda, 4) + -0.00148407 *lens_ipow(begin_x, 3)*begin_y*begin_dx*lens_ipow(begin_dy, 4) + -7.61229e-11 *lens_ipow(begin_y, 8)*begin_dy + 0.00038575 *lens_ipow(begin_x, 4)*lens_ipow(begin_dy, 5) + 0.0287351 *lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 2);
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
    domega2_dx0[0][0] =  + -0.026849  + 0.0120662 *begin_lambda + -0.00827016 *lens_ipow(begin_lambda, 2) + -1.03771e-05 *lens_ipow(begin_y, 2) + 0.00186831 *lens_ipow(begin_dy, 2) + 0.015388 *lens_ipow(begin_dx, 2) + 0.000280643 *lens_ipow(begin_x, 2)*begin_lambda + -0.000784118 *begin_x*begin_dx*begin_lambda + -0.000444342 *lens_ipow(begin_x, 2)*lens_ipow(begin_lambda, 2) + -0.0221637 *lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 2) + -2.22251e-07 *lens_ipow(begin_x, 4) + -0.00671039 *begin_y*begin_dy*lens_ipow(begin_lambda, 2) + 0.0174403 *begin_y*begin_dy*lens_ipow(begin_lambda, 3) + -5.90157e-06 *lens_ipow(begin_x, 2)*begin_y*begin_dy*begin_lambda + -1.12979e-05 *lens_ipow(begin_x, 3)*begin_dx*begin_lambda + 1.35905e-06 *lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 2) + 0.000263128 *lens_ipow(begin_x, 2)*lens_ipow(begin_lambda, 4) + -0.0124094 *begin_y*begin_dy*lens_ipow(begin_lambda, 4)+0.0f;
    domega2_dx0[0][1] =  + -2.07541e-05 *begin_x*begin_y + 0.00028384 *begin_y*begin_dx + 0.0477328 *begin_dx*begin_dy*begin_lambda + -0.0481053 *begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + -0.00671039 *begin_x*begin_dy*lens_ipow(begin_lambda, 2) + 0.0174403 *begin_x*begin_dy*lens_ipow(begin_lambda, 3) + -1.96719e-06 *lens_ipow(begin_x, 3)*begin_dy*begin_lambda + 5.43619e-06 *begin_x*lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 2) + 0.000178619 *lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 3) + -0.0124094 *begin_x*begin_dy*lens_ipow(begin_lambda, 4) + 0.00657155 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 3)*begin_dy*begin_lambda+0.0f;
    domega2_dx0[1][0] =  + 0.00896579 *begin_dx*begin_dy + 7.80803e-05 *begin_x*begin_y + -0.000677568 *begin_x*begin_dy + -2.51e-06 *lens_ipow(begin_y, 3)*begin_dx + -2.48612e-07 *begin_x*lens_ipow(begin_y, 3) + -5.77225e-06 *lens_ipow(begin_x, 2)*begin_y*begin_dx + -3.94929e-07 *lens_ipow(begin_x, 3)*begin_y + -5.17297e-06 *begin_x*lens_ipow(begin_y, 2)*begin_dy + -0.00445222 *lens_ipow(begin_x, 2)*begin_y*begin_dx*lens_ipow(begin_dy, 4) + 0.001543 *lens_ipow(begin_x, 3)*lens_ipow(begin_dy, 5)+0.0f;
    domega2_dx0[1][1] =  + -0.0266125  + 0.0114614 *begin_lambda + -0.00511334 *lens_ipow(begin_dx, 2) + 0.0129436 *lens_ipow(begin_dy, 2) + -0.00789395 *lens_ipow(begin_lambda, 2) + 3.90402e-05 *lens_ipow(begin_x, 2) + -0.00113546 *begin_y*begin_dy*begin_lambda + 0.000166729 *lens_ipow(begin_y, 2)*begin_lambda + -7.52999e-06 *begin_x*lens_ipow(begin_y, 2)*begin_dx + -3.72919e-07 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2) + -1.92408e-06 *lens_ipow(begin_x, 3)*begin_dx + -9.87323e-08 *lens_ipow(begin_x, 4) + -5.17297e-06 *lens_ipow(begin_x, 2)*begin_y*begin_dy + -0.0150597 *lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + 0.0120177 *begin_y*lens_ipow(begin_dx, 2)*begin_dy*begin_lambda + -0.000650235 *lens_ipow(begin_y, 2)*lens_ipow(begin_lambda, 3) + -1.79568e-09 *lens_ipow(begin_y, 6) + 0.000596999 *lens_ipow(begin_y, 2)*lens_ipow(begin_lambda, 4) + -0.00148407 *lens_ipow(begin_x, 3)*begin_dx*lens_ipow(begin_dy, 4) + -6.08983e-10 *lens_ipow(begin_y, 7)*begin_dy + 0.0862054 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 2)+0.0f;
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
  out[4] =  + 1.07795 *begin_lambda + -0.000434464 *begin_y*begin_dy + -1.13994e-05 *lens_ipow(begin_y, 2) + -1.80117 *lens_ipow(begin_lambda, 2) + 1.13999 *lens_ipow(begin_lambda, 3) + -0.000660056 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2) + -0.0815446 *begin_x*lens_ipow(begin_dx, 3) + -8.44238 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2) + -1.0738e-05 *lens_ipow(begin_x, 2)*begin_y*begin_dy + -0.00279317 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 2) + -0.150544 *begin_y*lens_ipow(begin_dx, 2)*begin_dy + -3.27969e-05 *lens_ipow(begin_x, 3)*begin_dx + -0.160615 *begin_x*begin_dx*lens_ipow(begin_dy, 2) + -2.25778e-07 *lens_ipow(begin_x, 4) + -0.0025113 *begin_x*begin_y*begin_dx*begin_dy + -0.000994204 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 2) + -2.06406e-05 *lens_ipow(begin_y, 4)*lens_ipow(begin_dy, 2) + -98.5938 *lens_ipow(begin_dx, 6) + -0.195942 *lens_ipow(begin_lambda, 6) + -2.20023 *begin_x*lens_ipow(begin_dx, 5) + -0.00302466 *lens_ipow(begin_y, 3)*lens_ipow(begin_dy, 3) + -87.6123 *lens_ipow(begin_dy, 6) + -0.179227 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 4) + -5.59084 *begin_y*lens_ipow(begin_dy, 5) + -0.318447 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 6) + -1415.1 *lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 4) + 394.675 *lens_ipow(begin_dx, 8) + 0.671785 *begin_x*begin_y*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 3);
else
  out[4] = 0.0f;

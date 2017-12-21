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
       + 47.9583 *begin_dx + 0.726604 *begin_x + 0.30773 *begin_x*begin_lambda + 10.0443 *begin_dx*begin_lambda + 0.51757 *begin_y*begin_dx*begin_dy + 0.0130025 *begin_x*begin_y*begin_dy + 0.0209696 *lens_ipow(begin_x, 2)*begin_dx + -0.428152 *begin_x*lens_ipow(begin_lambda, 2) + 0.0001599 *begin_x*lens_ipow(begin_y, 2) + 0.000146126 *lens_ipow(begin_x, 3) + 0.109933 *begin_x*lens_ipow(begin_dy, 2) + 0.60574 *begin_x*lens_ipow(begin_dx, 2) + 0.00896142 *lens_ipow(begin_y, 2)*begin_dx + -10.4332 *begin_dx*lens_ipow(begin_lambda, 2) + 0.208867 *begin_x*lens_ipow(begin_lambda, 3) + 2.55743e-05 *lens_ipow(begin_x, 4)*begin_dx + 3.82423 *begin_dx*lens_ipow(begin_lambda, 4) + 2.4372e-07 *lens_ipow(begin_x, 5) + -0.000238119 *lens_ipow(begin_x, 2)*begin_y*begin_dx*begin_dy + 0.000614863 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 2) + -3.80606e-05 *begin_x*lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 2) + 0.836959 *begin_x*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2)*begin_lambda + 2.60785e-08 *lens_ipow(begin_x, 5)*begin_lambda + 2.35096e-10 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 2) + -1.07059e-06 *lens_ipow(begin_x, 4)*begin_y*begin_dx*begin_dy*begin_lambda + -3.9565e-06 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3)*begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + 5.01276e-09 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 4)*lens_ipow(begin_lambda, 3) + -4.29584e-09 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 4)*lens_ipow(begin_lambda, 4),
       + 0.727662 *begin_y + 47.9805 *begin_dy + 9.94011 *begin_dy*begin_lambda + 0.303795 *begin_y*begin_lambda + 0.116675 *begin_y*lens_ipow(begin_dx, 2) + 0.0212751 *lens_ipow(begin_y, 2)*begin_dy + 0.606324 *begin_y*lens_ipow(begin_dy, 2) + 0.541493 *begin_x*begin_dx*begin_dy + 0.0133007 *begin_x*begin_y*begin_dx + 0.525397 *lens_ipow(begin_dx, 2)*begin_dy + -0.422576 *begin_y*lens_ipow(begin_lambda, 2) + 0.000155139 *lens_ipow(begin_x, 2)*begin_y + -10.2959 *begin_dy*lens_ipow(begin_lambda, 2) + 0.000145719 *lens_ipow(begin_y, 3) + 0.00910908 *lens_ipow(begin_x, 2)*begin_dy + 0.205574 *begin_y*lens_ipow(begin_lambda, 3) + -0.000530731 *lens_ipow(begin_y, 2)*begin_dy*begin_lambda + 9.11966e-06 *lens_ipow(begin_x, 2)*begin_y*begin_lambda + 0.00188791 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 2)*begin_dy + -0.000453148 *begin_x*lens_ipow(begin_y, 2)*begin_dx*begin_dy + 3.75232 *begin_dy*lens_ipow(begin_lambda, 4) + 7.66467e-08 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3) + 0.54031 *begin_y*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2) + 4.37653e-05 *lens_ipow(begin_y, 4)*begin_dy*begin_lambda + 0.00104185 *lens_ipow(begin_y, 3)*lens_ipow(begin_dy, 2)*begin_lambda + 4.47109e-07 *lens_ipow(begin_y, 5)*begin_lambda + 8.89044e-10 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 3)*lens_ipow(begin_lambda, 3) + -3.83891e-06 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 2)*begin_dx*begin_dy*lens_ipow(begin_lambda, 4)
    };
    const float delta_ap[] = {ap_x - pred_ap[0], ap_y - pred_ap[1]};
    sqr_ap_err = delta_ap[0]*delta_ap[0]+delta_ap[1]*delta_ap[1];
    float dx1_domega0[2][2];
    dx1_domega0[0][0] =  + 47.9583  + 10.0443 *begin_lambda + 0.51757 *begin_y*begin_dy + 0.0209696 *lens_ipow(begin_x, 2) + 1.21148 *begin_x*begin_dx + 0.00896142 *lens_ipow(begin_y, 2) + -10.4332 *lens_ipow(begin_lambda, 2) + 2.55743e-05 *lens_ipow(begin_x, 4) + 3.82423 *lens_ipow(begin_lambda, 4) + -0.000238119 *lens_ipow(begin_x, 2)*begin_y*begin_dy + 0.00122973 *lens_ipow(begin_x, 3)*begin_dx + 1.67392 *begin_x*begin_dx*lens_ipow(begin_dy, 2)*begin_lambda + -1.07059e-06 *lens_ipow(begin_x, 4)*begin_y*begin_dy*begin_lambda + -3.9565e-06 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3)*begin_dy*lens_ipow(begin_lambda, 2)+0.0f;
    dx1_domega0[0][1] =  + 0.51757 *begin_y*begin_dx + 0.0130025 *begin_x*begin_y + 0.219866 *begin_x*begin_dy + -0.000238119 *lens_ipow(begin_x, 2)*begin_y*begin_dx + -7.61212e-05 *begin_x*lens_ipow(begin_y, 2)*begin_dy + 1.67392 *begin_x*lens_ipow(begin_dx, 2)*begin_dy*begin_lambda + -1.07059e-06 *lens_ipow(begin_x, 4)*begin_y*begin_dx*begin_lambda + -3.9565e-06 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3)*begin_dx*lens_ipow(begin_lambda, 2)+0.0f;
    dx1_domega0[1][0] =  + 0.233349 *begin_y*begin_dx + 0.541493 *begin_x*begin_dy + 0.0133007 *begin_x*begin_y + 1.05079 *begin_dx*begin_dy + 0.00377582 *lens_ipow(begin_x, 2)*begin_dx*begin_dy + -0.000453148 *begin_x*lens_ipow(begin_y, 2)*begin_dy + 1.08062 *begin_y*begin_dx*lens_ipow(begin_dy, 2) + -3.83891e-06 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 2)*begin_dy*lens_ipow(begin_lambda, 4)+0.0f;
    dx1_domega0[1][1] =  + 47.9805  + 9.94011 *begin_lambda + 0.0212751 *lens_ipow(begin_y, 2) + 1.21265 *begin_y*begin_dy + 0.541493 *begin_x*begin_dx + 0.525397 *lens_ipow(begin_dx, 2) + -10.2959 *lens_ipow(begin_lambda, 2) + 0.00910908 *lens_ipow(begin_x, 2) + -0.000530731 *lens_ipow(begin_y, 2)*begin_lambda + 0.00188791 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 2) + -0.000453148 *begin_x*lens_ipow(begin_y, 2)*begin_dx + 3.75232 *lens_ipow(begin_lambda, 4) + 1.08062 *begin_y*lens_ipow(begin_dx, 2)*begin_dy + 4.37653e-05 *lens_ipow(begin_y, 4)*begin_lambda + 0.00208371 *lens_ipow(begin_y, 3)*begin_dy*begin_lambda + -3.83891e-06 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 2)*begin_dx*lens_ipow(begin_lambda, 4)+0.0f;
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
    out[0] =  + 61.6861 *begin_dx + 0.516318 *begin_x + 0.239174 *begin_x*begin_lambda + 6.09756 *begin_dx*begin_lambda + 0.0418018 *begin_y*begin_dx*begin_dy + 0.0291763 *begin_x*begin_y*begin_dy + 0.0384633 *lens_ipow(begin_x, 2)*begin_dx + -41.8684 *lens_ipow(begin_dx, 3) + -0.16516 *begin_x*lens_ipow(begin_lambda, 2) + -41.0878 *begin_dx*lens_ipow(begin_dy, 2) + 0.000319801 *begin_x*lens_ipow(begin_y, 2) + 0.000310337 *lens_ipow(begin_x, 3) + 0.431597 *begin_x*lens_ipow(begin_dy, 2) + 0.417681 *begin_x*lens_ipow(begin_dx, 2) + 0.0106198 *lens_ipow(begin_y, 2)*begin_dx + -4.03513 *begin_dx*lens_ipow(begin_lambda, 3) + 1.11768e-05 *begin_x*lens_ipow(begin_y, 2)*begin_lambda + -0.000382566 *lens_ipow(begin_x, 2)*begin_dx*begin_lambda + -8.637e-05 *lens_ipow(begin_x, 2)*begin_y*begin_dx*begin_dy*begin_lambda + 5.14981e-06 *lens_ipow(begin_x, 7)*lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 2) + 13819.6 *lens_ipow(begin_dx, 9)*lens_ipow(begin_lambda, 2) + 1.71189e-08 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 2)*begin_dy + 3.21537e-10 *lens_ipow(begin_x, 9)*lens_ipow(begin_lambda, 2) + 0.00130788 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 2) + 0.000150672 *lens_ipow(begin_x, 6)*lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 2) + 5.82064e-14 *lens_ipow(begin_x, 7)*lens_ipow(begin_y, 4) + -0.0568649 *lens_ipow(begin_x, 4)*lens_ipow(begin_dx, 5)*lens_ipow(begin_lambda, 2) + 6.75549e-08 *lens_ipow(begin_x, 8)*begin_dx*lens_ipow(begin_lambda, 2);
    out[1] =  + 0.453506 *begin_y + 59.1587 *begin_dy + 19.1364 *begin_dy*begin_lambda + 0.592232 *begin_y*begin_lambda + 0.411922 *begin_y*lens_ipow(begin_dx, 2) + 0.0392662 *lens_ipow(begin_y, 2)*begin_dy + 0.451829 *begin_y*lens_ipow(begin_dy, 2) + 0.0283685 *begin_x*begin_y*begin_dx + -42.1243 *lens_ipow(begin_dx, 2)*begin_dy + -0.817315 *begin_y*lens_ipow(begin_lambda, 2) + 0.000312315 *lens_ipow(begin_x, 2)*begin_y + -19.7228 *begin_dy*lens_ipow(begin_lambda, 2) + 0.000313434 *lens_ipow(begin_y, 3) + 0.0101854 *lens_ipow(begin_x, 2)*begin_dy + -41.478 *lens_ipow(begin_dy, 3) + 0.395356 *begin_y*lens_ipow(begin_lambda, 3) + 1.6101e-05 *lens_ipow(begin_y, 3)*begin_lambda + 8.49311e-06 *lens_ipow(begin_x, 2)*begin_y*begin_lambda + 7.11498 *begin_dy*lens_ipow(begin_lambda, 4) + -7.43062e-05 *lens_ipow(begin_y, 3)*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + -9.90812e-06 *lens_ipow(begin_y, 5)*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2) + 1.08674e-14 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 9) + 2.57069e-11 *lens_ipow(begin_y, 9)*lens_ipow(begin_dx, 2) + 2.33768e-09 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 2)*begin_dy + 1.52162e-14 *lens_ipow(begin_x, 6)*lens_ipow(begin_y, 5) + -0.313837 *lens_ipow(begin_x, 2)*begin_y*lens_ipow(begin_dy, 8) + 4.28086e-05 *begin_x*lens_ipow(begin_y, 5)*begin_dx*lens_ipow(begin_dy, 4) + 0.00203743 *begin_x*lens_ipow(begin_y, 4)*begin_dx*lens_ipow(begin_dy, 5);
    out[2] =  + -1.63943 *begin_dx + -0.0305953 *begin_x + 0.0323359 *begin_dx*begin_lambda + -0.0004223 *lens_ipow(begin_x, 2)*begin_dx + 1.74614e-06 *begin_x*lens_ipow(begin_y, 2) + 4.9289 *lens_ipow(begin_dx, 3)*begin_lambda + -0.0847201 *begin_x*lens_ipow(begin_dy, 2)*begin_lambda + -1.54502e-05 *begin_x*lens_ipow(begin_y, 2)*begin_lambda + -8.9172e-06 *lens_ipow(begin_x, 3)*begin_lambda + -0.00275435 *begin_x*begin_y*begin_dy*begin_lambda + -0.000390955 *lens_ipow(begin_x, 2)*begin_dx*begin_lambda + -0.000707983 *lens_ipow(begin_y, 2)*begin_dx*begin_lambda + -2.05796 *begin_dx*lens_ipow(begin_dy, 2)*begin_lambda + -0.0998601 *begin_y*begin_dx*begin_dy*begin_lambda + 0.0184584 *begin_x*lens_ipow(begin_dx, 4) + 0.0249779 *begin_y*begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + -8.31442 *lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 2) + 1.57657 *lens_ipow(begin_dx, 5) + 4.83602e-06 *lens_ipow(begin_x, 3)*lens_ipow(begin_lambda, 2) + 0.0421838 *begin_x*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + 0.00116668 *begin_x*begin_y*begin_dy*lens_ipow(begin_lambda, 2) + -0.020373 *begin_x*lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 3) + 4.0944 *lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 3) + -1.18558 *begin_x*begin_y*lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 3) + 20.8038 *begin_dx*lens_ipow(begin_dy, 8) + 2134.98 *lens_ipow(begin_dx, 5)*lens_ipow(begin_dy, 4) + 0.000186958 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 2) + 0.000963685 *begin_x*lens_ipow(begin_lambda, 9);
    out[3] =  + -0.030348 *begin_y + -1.6733 *begin_dy + 0.226598 *begin_dy*begin_lambda + -0.000739868 *begin_y*begin_lambda + 0.0043513 *begin_y*lens_ipow(begin_dx, 2) + -0.000633417 *lens_ipow(begin_y, 2)*begin_dy + -0.00400459 *begin_y*lens_ipow(begin_dy, 2) + 0.0669129 *begin_x*begin_dx*begin_dy + -9.59075e-05 *begin_x*begin_y*begin_dx + 2.91954 *lens_ipow(begin_dx, 2)*begin_dy + 0.000576135 *begin_y*lens_ipow(begin_lambda, 2) + -0.334116 *begin_dy*lens_ipow(begin_lambda, 2) + -3.05205e-06 *lens_ipow(begin_y, 3) + 0.000314298 *lens_ipow(begin_x, 2)*begin_dy + 0.885659 *lens_ipow(begin_dy, 3) + 7.50285e-05 *lens_ipow(begin_x, 2)*begin_dy*begin_lambda + -0.113294 *lens_ipow(begin_dy, 3)*begin_lambda + 0.176171 *begin_dy*lens_ipow(begin_lambda, 3) + -6.45139e-07 *lens_ipow(begin_y, 3)*begin_lambda + 4.94668e-05 *begin_x*begin_y*begin_dx*begin_lambda + 0.00640416 *begin_x*begin_dx*begin_dy*begin_lambda + -0.0369238 *begin_x*lens_ipow(begin_dx, 3)*begin_dy + 2.5536 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 3) + 1.94879 *lens_ipow(begin_dy, 5) + -0.000400921 *begin_x*begin_y*begin_dx*lens_ipow(begin_dy, 2) + 0.0245005 *begin_y*lens_ipow(begin_dy, 4) + 5.94929e-06 *lens_ipow(begin_x, 3)*begin_dx*begin_dy + 0.011227 *begin_y*lens_ipow(begin_dx, 4)*begin_lambda;
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
    domega2_dx0[0][0] =  + -0.0305953  + -0.000844601 *begin_x*begin_dx + 1.74614e-06 *lens_ipow(begin_y, 2) + -0.0847201 *lens_ipow(begin_dy, 2)*begin_lambda + -1.54502e-05 *lens_ipow(begin_y, 2)*begin_lambda + -2.67516e-05 *lens_ipow(begin_x, 2)*begin_lambda + -0.00275435 *begin_y*begin_dy*begin_lambda + -0.00078191 *begin_x*begin_dx*begin_lambda + 0.0184584 *lens_ipow(begin_dx, 4) + 1.45081e-05 *lens_ipow(begin_x, 2)*lens_ipow(begin_lambda, 2) + 0.0421838 *lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + 0.00116668 *begin_y*begin_dy*lens_ipow(begin_lambda, 2) + -0.020373 *lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 3) + -1.18558 *begin_y*lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 3) + 0.000373915 *begin_x*lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 2) + 0.000963685 *lens_ipow(begin_lambda, 9)+0.0f;
    domega2_dx0[0][1] =  + 3.49228e-06 *begin_x*begin_y + -3.09004e-05 *begin_x*begin_y*begin_lambda + -0.00275435 *begin_x*begin_dy*begin_lambda + -0.00141597 *begin_y*begin_dx*begin_lambda + -0.0998601 *begin_dx*begin_dy*begin_lambda + 0.0249779 *begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + 0.00116668 *begin_x*begin_dy*lens_ipow(begin_lambda, 2) + -1.18558 *begin_x*lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 3) + 0.000373915 *lens_ipow(begin_x, 2)*begin_y*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 2)+0.0f;
    domega2_dx0[1][0] =  + 0.0669129 *begin_dx*begin_dy + -9.59075e-05 *begin_y*begin_dx + 0.000628596 *begin_x*begin_dy + 0.000150057 *begin_x*begin_dy*begin_lambda + 4.94668e-05 *begin_y*begin_dx*begin_lambda + 0.00640416 *begin_dx*begin_dy*begin_lambda + -0.0369238 *lens_ipow(begin_dx, 3)*begin_dy + -0.000400921 *begin_y*begin_dx*lens_ipow(begin_dy, 2) + 1.78479e-05 *lens_ipow(begin_x, 2)*begin_dx*begin_dy+0.0f;
    domega2_dx0[1][1] =  + -0.030348  + -0.000739868 *begin_lambda + 0.0043513 *lens_ipow(begin_dx, 2) + -0.00126683 *begin_y*begin_dy + -0.00400459 *lens_ipow(begin_dy, 2) + -9.59075e-05 *begin_x*begin_dx + 0.000576135 *lens_ipow(begin_lambda, 2) + -9.15615e-06 *lens_ipow(begin_y, 2) + -1.93542e-06 *lens_ipow(begin_y, 2)*begin_lambda + 4.94668e-05 *begin_x*begin_dx*begin_lambda + -0.000400921 *begin_x*begin_dx*lens_ipow(begin_dy, 2) + 0.0245005 *lens_ipow(begin_dy, 4) + 0.011227 *lens_ipow(begin_dx, 4)*begin_lambda+0.0f;
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
  out[4] =  + 0.59399  + 0.836383 *begin_lambda + -0.000344805 *begin_x*begin_dx + -7.02536e-06 *lens_ipow(begin_x, 2) + -1.73936 *lens_ipow(begin_lambda, 2) + 1.70047 *lens_ipow(begin_lambda, 3) + -0.644121 *lens_ipow(begin_lambda, 4) + -0.150549 *lens_ipow(begin_dx, 4) + -0.449125 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2) + -1.08274e-05 *lens_ipow(begin_x, 2)*begin_y*begin_dy + -1.09547e-05 *begin_x*lens_ipow(begin_y, 2)*begin_dx + -4.56631e-07 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2) + -3.11249e-07 *lens_ipow(begin_y, 4) + -0.00046016 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 2) + -2.35642e-05 *lens_ipow(begin_y, 3)*begin_dy + -2.22792e-09 *lens_ipow(begin_x, 6) + -1.06372e-07 *lens_ipow(begin_x, 5)*begin_dx + -1.21435e-10 *lens_ipow(begin_y, 6) + -5.63631e-10 *lens_ipow(begin_x, 7)*begin_dx + -4.88522e-12 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 4) + 1.25574e-08 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 3)*begin_dx*begin_dy + -2.84961e-08 *lens_ipow(begin_x, 6)*lens_ipow(begin_dx, 2) + -3.7395e-13 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 6) + 2.03261e-06 *lens_ipow(begin_y, 4)*lens_ipow(begin_dy, 4)*begin_lambda + 7.20677e-05 *lens_ipow(begin_x, 4)*lens_ipow(begin_dx, 6) + -3.53471e-15 *lens_ipow(begin_x, 8)*lens_ipow(begin_y, 2) + -3.64078e-15 *lens_ipow(begin_x, 10) + 3.93018e-08 *lens_ipow(begin_x, 5)*begin_y*lens_ipow(begin_dx, 3)*begin_dy*begin_lambda;
else
  out[4] = 0.0f;

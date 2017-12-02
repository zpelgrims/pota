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
       + 1 *begin_x + 105 *begin_dx + 1.39872e-05 *begin_dx*begin_dy*begin_lambda + -0.000107611 *lens_ipow(begin_dx, 3)*begin_dy + -4.71532e-05 *begin_dx*begin_dy*lens_ipow(begin_lambda, 3) + -1.96683e-05 *lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 2) + -0.000327671 *lens_ipow(begin_dx, 5) + 4.96919e-06 *begin_y*begin_dx*lens_ipow(begin_dy, 2)*begin_lambda + 4.83911e-08 *begin_x*begin_y*begin_dx*begin_dy*begin_lambda + 1.29119e-10 *begin_x*lens_ipow(begin_y, 2)*begin_dy*begin_lambda + -5.54182e-10 *begin_x*lens_ipow(begin_y, 2)*begin_dx*begin_dy + -1.02833e-05 *begin_y*begin_dx*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + -7.79854e-05 *lens_ipow(begin_dx, 6) + 0.000706735 *lens_ipow(begin_dx, 5)*begin_lambda + 9.23689e-10 *lens_ipow(begin_x, 3)*begin_dx*begin_dy*begin_lambda + -1.85773e-09 *lens_ipow(begin_x, 3)*begin_dx*lens_ipow(begin_dy, 2) + 5.53975e-09 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 2)*begin_dy + -6.86699e-11 *begin_x*lens_ipow(begin_y, 3)*begin_dx*begin_dy + 2.4149e-06 *begin_x*begin_dx*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + -9.64278e-06 *begin_x*lens_ipow(begin_dx, 2)*begin_dy*lens_ipow(begin_lambda, 2),
       + 1 *begin_y + 105 *begin_dy + 1.15237e-08 *lens_ipow(begin_y, 2)*begin_dx*begin_dy + 3.1238e-05 *lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 3) + -5.0963e-09 *begin_x*begin_y*begin_dx*lens_ipow(begin_lambda, 2) + -2.98221e-11 *lens_ipow(begin_x, 4)*begin_dy + 5.41749e-09 *lens_ipow(begin_x, 3)*lens_ipow(begin_dy, 3) + -4.76726e-07 *begin_x*begin_y*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2) + -5.48269e-05 *lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 4) + 8.52346e-13 *lens_ipow(begin_x, 4)*begin_y*begin_dy + -2.7651e-08 *lens_ipow(begin_x, 2)*begin_dy*lens_ipow(begin_lambda, 3) + 2.37073e-05 *lens_ipow(begin_dx, 2)*begin_dy*lens_ipow(begin_lambda, 3) + -0.00440305 *begin_dx*lens_ipow(begin_dy, 5) + 7.67541e-07 *lens_ipow(begin_x, 2)*begin_dx*lens_ipow(begin_dy, 3) + -6.11788e-10 *lens_ipow(begin_x, 2)*begin_y*begin_dy*lens_ipow(begin_lambda, 2) + -5.42857e-11 *lens_ipow(begin_y, 3)*lens_ipow(begin_lambda, 3) + 9.81771e-09 *lens_ipow(begin_y, 3)*begin_dx*lens_ipow(begin_dy, 2) + -3.4467e-06 *begin_x*lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 2) + 9.00461e-11 *lens_ipow(begin_x, 4)*begin_dy*begin_lambda + -8.56575e-05 *begin_y*begin_dx*lens_ipow(begin_dy, 4)
    };
    const float delta_ap[] = {ap_x - pred_ap[0], ap_y - pred_ap[1]};
    sqr_ap_err = delta_ap[0]*delta_ap[0]+delta_ap[1]*delta_ap[1];
    float dx1_domega0[2][2];
    dx1_domega0[0][0] =  + 105  + 1.39872e-05 *begin_dy*begin_lambda + -0.000322832 *lens_ipow(begin_dx, 2)*begin_dy + -4.71532e-05 *begin_dy*lens_ipow(begin_lambda, 3) + -5.90048e-05 *lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 2) + -0.00163836 *lens_ipow(begin_dx, 4) + 4.96919e-06 *begin_y*lens_ipow(begin_dy, 2)*begin_lambda + 4.83911e-08 *begin_x*begin_y*begin_dy*begin_lambda + -5.54182e-10 *begin_x*lens_ipow(begin_y, 2)*begin_dy + -1.02833e-05 *begin_y*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + -0.000467913 *lens_ipow(begin_dx, 5) + 0.00353368 *lens_ipow(begin_dx, 4)*begin_lambda + 9.23689e-10 *lens_ipow(begin_x, 3)*begin_dy*begin_lambda + -1.85773e-09 *lens_ipow(begin_x, 3)*lens_ipow(begin_dy, 2) + 1.10795e-08 *lens_ipow(begin_x, 3)*begin_dx*begin_dy + -6.86699e-11 *begin_x*lens_ipow(begin_y, 3)*begin_dy + 2.4149e-06 *begin_x*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + -1.92856e-05 *begin_x*begin_dx*begin_dy*lens_ipow(begin_lambda, 2)+0.0f;
    dx1_domega0[0][1] =  + 1.39872e-05 *begin_dx*begin_lambda + -0.000107611 *lens_ipow(begin_dx, 3) + -4.71532e-05 *begin_dx*lens_ipow(begin_lambda, 3) + 9.93837e-06 *begin_y*begin_dx*begin_dy*begin_lambda + 4.83911e-08 *begin_x*begin_y*begin_dx*begin_lambda + 1.29119e-10 *begin_x*lens_ipow(begin_y, 2)*begin_lambda + -5.54182e-10 *begin_x*lens_ipow(begin_y, 2)*begin_dx + -2.05667e-05 *begin_y*begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + 9.23689e-10 *lens_ipow(begin_x, 3)*begin_dx*begin_lambda + -3.71545e-09 *lens_ipow(begin_x, 3)*begin_dx*begin_dy + 5.53975e-09 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 2) + -6.86699e-11 *begin_x*lens_ipow(begin_y, 3)*begin_dx + 4.82981e-06 *begin_x*begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + -9.64278e-06 *begin_x*lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 2)+0.0f;
    dx1_domega0[1][0] =  + 1.15237e-08 *lens_ipow(begin_y, 2)*begin_dy + -5.0963e-09 *begin_x*begin_y*lens_ipow(begin_lambda, 2) + -9.53452e-07 *begin_x*begin_y*begin_dx*lens_ipow(begin_dy, 2) + 4.74145e-05 *begin_dx*begin_dy*lens_ipow(begin_lambda, 3) + -0.00440305 *lens_ipow(begin_dy, 5) + 7.67541e-07 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 3) + 9.81771e-09 *lens_ipow(begin_y, 3)*lens_ipow(begin_dy, 2) + -8.56575e-05 *begin_y*lens_ipow(begin_dy, 4)+0.0f;
    dx1_domega0[1][1] =  + 105  + 1.15237e-08 *lens_ipow(begin_y, 2)*begin_dx + 6.2476e-05 *begin_dy*lens_ipow(begin_lambda, 3) + -2.98221e-11 *lens_ipow(begin_x, 4) + 1.62525e-08 *lens_ipow(begin_x, 3)*lens_ipow(begin_dy, 2) + -9.53452e-07 *begin_x*begin_y*lens_ipow(begin_dx, 2)*begin_dy + -0.000109654 *begin_dy*lens_ipow(begin_lambda, 4) + 8.52346e-13 *lens_ipow(begin_x, 4)*begin_y + -2.7651e-08 *lens_ipow(begin_x, 2)*lens_ipow(begin_lambda, 3) + 2.37073e-05 *lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 3) + -0.0220152 *begin_dx*lens_ipow(begin_dy, 4) + 2.30262e-06 *lens_ipow(begin_x, 2)*begin_dx*lens_ipow(begin_dy, 2) + -6.11788e-10 *lens_ipow(begin_x, 2)*begin_y*lens_ipow(begin_lambda, 2) + 1.96354e-08 *lens_ipow(begin_y, 3)*begin_dx*begin_dy + -1.03401e-05 *begin_x*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + 9.00461e-11 *lens_ipow(begin_x, 4)*begin_lambda + -0.00034263 *begin_y*begin_dx*lens_ipow(begin_dy, 3)+0.0f;
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
    out[0] =  + 0.996674 *begin_x + 118.309 *begin_dx + 1.39008 *begin_dx*begin_lambda + -1.11035 *begin_x*lens_ipow(begin_dx, 2) + -70.6182 *begin_dx*lens_ipow(begin_dy, 2) + -0.00484487 *lens_ipow(begin_y, 2)*begin_dx + -0.00435344 *lens_ipow(begin_x, 2)*begin_dx + -70.2281 *lens_ipow(begin_dx, 3) + -1.15047 *begin_y*begin_dx*begin_dy + -0.948755 *begin_dx*lens_ipow(begin_lambda, 2) + 32.7504 *begin_dx*lens_ipow(begin_dy, 4) + 74.0983 *lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 2) + 49.726 *lens_ipow(begin_dx, 5) + 0.396142 *begin_y*begin_dx*lens_ipow(begin_dy, 3) + 0.985293 *begin_y*lens_ipow(begin_dx, 3)*begin_dy + 0.00370317 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 3) + 0.67681 *begin_x*lens_ipow(begin_dx, 4) + 0.000120336 *lens_ipow(begin_x, 3)*lens_ipow(begin_dy, 2) + 1.88443e-06 *lens_ipow(begin_x, 3)*begin_y*begin_dy + 8.15759e-09 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 2);
    out[1] =  + 0.996669 *begin_y + 118.312 *begin_dy + 1.3768 *begin_dy*begin_lambda + -1.15112 *begin_x*begin_dx*begin_dy + -0.00435209 *lens_ipow(begin_y, 2)*begin_dy + -0.937299 *begin_dy*lens_ipow(begin_lambda, 2) + -70.6186 *lens_ipow(begin_dx, 2)*begin_dy + -0.00484509 *lens_ipow(begin_x, 2)*begin_dy + -70.1825 *lens_ipow(begin_dy, 3) + -1.10988 *begin_y*lens_ipow(begin_dy, 2) + 48.9144 *lens_ipow(begin_dy, 5) + 73.7254 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 3) + 33.6204 *lens_ipow(begin_dx, 4)*begin_dy + 0.665577 *begin_y*lens_ipow(begin_dy, 4) + 0.000121573 *lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 2) + 0.980325 *begin_x*begin_dx*lens_ipow(begin_dy, 3) + 0.419576 *begin_x*lens_ipow(begin_dx, 3)*begin_dy + 1.90829e-06 *begin_x*lens_ipow(begin_y, 3)*begin_dx + 0.00368531 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 3) + 8.37799e-09 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3);
    out[2] =  + -0.0261141 *begin_x + -2.08444 *begin_dx + 1.24794 *begin_dx*begin_lambda + 0.0113579 *begin_x*begin_lambda + 0.00293917 *begin_x*lens_ipow(begin_dx, 2) + -0.0350307 *begin_x*lens_ipow(begin_dy, 2) + -2.96306 *begin_dx*lens_ipow(begin_dy, 2) + -0.000222209 *lens_ipow(begin_y, 2)*begin_dx + -0.0156891 *begin_x*lens_ipow(begin_lambda, 2) + -8.89703e-06 *lens_ipow(begin_x, 2)*begin_dx + 0.0475137 *lens_ipow(begin_dx, 3) + -2.86083e-06 *begin_x*lens_ipow(begin_y, 2) + -0.000675006 *begin_x*begin_y*begin_dy + -0.0524915 *begin_y*begin_dx*begin_dy + -1.7205 *begin_dx*lens_ipow(begin_lambda, 2) + 0.834249 *begin_dx*lens_ipow(begin_lambda, 3) + 0.00762043 *begin_x*lens_ipow(begin_lambda, 3) + 0.937209 *lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 2) + 0.0259636 *begin_x*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2) + 0.000226608 *lens_ipow(begin_x, 2)*begin_dx*lens_ipow(begin_dy, 2);
    out[3] =  + -0.0257907 *begin_y + -2.08085 *begin_dy + 1.23271 *begin_dy*begin_lambda + 0.00900548 *begin_y*begin_lambda + 0.0777339 *begin_x*begin_dx*begin_dy + -1.70286 *begin_dy*lens_ipow(begin_lambda, 2) + 2.86738e-06 *lens_ipow(begin_x, 2)*begin_y + 3.05385 *lens_ipow(begin_dx, 2)*begin_dy + 0.00045011 *lens_ipow(begin_x, 2)*begin_dy + 0.0842326 *lens_ipow(begin_dy, 3) + -0.00936601 *begin_y*lens_ipow(begin_lambda, 2) + 0.00413083 *begin_y*lens_ipow(begin_dy, 2) + 0.000433741 *begin_x*begin_y*begin_dx + 0.0162663 *begin_y*lens_ipow(begin_dx, 2) + 0.826829 *begin_dy*lens_ipow(begin_lambda, 3) + 4.64832e-08 *lens_ipow(begin_y, 3)*begin_lambda + 0.00343486 *begin_y*lens_ipow(begin_lambda, 4) + -0.00443922 *begin_y*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2) + 5.2043e-05 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 2)*begin_dy + 5.44099e-05 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 3)*begin_lambda;
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
    domega2_dx0[0][0] =  + -0.0261141  + 0.0113579 *begin_lambda + 0.00293917 *lens_ipow(begin_dx, 2) + -0.0350307 *lens_ipow(begin_dy, 2) + -0.0156891 *lens_ipow(begin_lambda, 2) + -1.77941e-05 *begin_x*begin_dx + -2.86083e-06 *lens_ipow(begin_y, 2) + -0.000675006 *begin_y*begin_dy + 0.00762043 *lens_ipow(begin_lambda, 3) + 0.0259636 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2) + 0.000453216 *begin_x*begin_dx*lens_ipow(begin_dy, 2)+0.0f;
    domega2_dx0[0][1] =  + -0.000444418 *begin_y*begin_dx + -5.72166e-06 *begin_x*begin_y + -0.000675006 *begin_x*begin_dy + -0.0524915 *begin_dx*begin_dy+0.0f;
    domega2_dx0[1][0] =  + 0.0777339 *begin_dx*begin_dy + 5.73476e-06 *begin_x*begin_y + 0.00090022 *begin_x*begin_dy + 0.000433741 *begin_y*begin_dx + 0.000104086 *begin_x*lens_ipow(begin_dx, 2)*begin_dy+0.0f;
    domega2_dx0[1][1] =  + -0.0257907  + 0.00900548 *begin_lambda + 2.86738e-06 *lens_ipow(begin_x, 2) + -0.00936601 *lens_ipow(begin_lambda, 2) + 0.00413083 *lens_ipow(begin_dy, 2) + 0.000433741 *begin_x*begin_dx + 0.0162663 *lens_ipow(begin_dx, 2) + 1.39449e-07 *lens_ipow(begin_y, 2)*begin_lambda + 0.00343486 *lens_ipow(begin_lambda, 4) + -0.00443922 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2) + 0.00010882 *begin_y*lens_ipow(begin_dy, 3)*begin_lambda+0.0f;
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
  out[4] =  + 0.878548  + 0.175256 *begin_lambda + -0.242836 *lens_ipow(begin_lambda, 2) + -7.12396e-07 *lens_ipow(begin_x, 2) + -0.000114502 *begin_x*begin_dx + -0.000267376 *begin_y*begin_dy + -1.79673e-06 *lens_ipow(begin_y, 2) + -0.0103272 *lens_ipow(begin_dy, 2) + -0.00513843 *lens_ipow(begin_dx, 2) + 0.11827 *lens_ipow(begin_lambda, 3) + -3.53246e-09 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2) + -1.40969e-06 *lens_ipow(begin_x, 3)*begin_dx + -3.44642e-07 *lens_ipow(begin_x, 2)*begin_y*begin_dy + -0.136696 *lens_ipow(begin_dx, 4) + -3.40223e-07 *begin_x*lens_ipow(begin_y, 2)*begin_dx + -3.6618e-05 *begin_x*begin_y*begin_dx*begin_dy + -0.0069304 *begin_x*lens_ipow(begin_dx, 3) + 0.000352804 *begin_y*lens_ipow(begin_dy, 3) + -5.01847e-09 *lens_ipow(begin_x, 4) + -0.000148497 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 2);
else
  out[4] = 0.0f;

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
       + 24.0758 *begin_dx + 0.714404 *begin_x + 1.13639 *begin_y*begin_dx*begin_dy + 0.032411 *begin_x*begin_y*begin_dy + 0.0496792 *lens_ipow(begin_x, 2)*begin_dx + 9.19327 *lens_ipow(begin_dx, 3) + 10.149 *begin_dx*lens_ipow(begin_dy, 2) + 0.464028 *begin_x*lens_ipow(begin_dy, 2) + 1.51791 *begin_x*lens_ipow(begin_dx, 2) + 0.0192878 *lens_ipow(begin_y, 2)*begin_dx + -1.14405e-06 *lens_ipow(begin_x, 5) + -0.000677498 *lens_ipow(begin_x, 4)*lens_ipow(begin_dx, 3) + -5.64672e-08 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 4) + -2.12508e-07 *lens_ipow(begin_x, 5)*begin_y*begin_dy + -6.02066e-07 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 4)*begin_dx + -0.0098213 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 4) + -1.90383e-08 *lens_ipow(begin_y, 7)*begin_dx*begin_dy + -6.69381e-10 *lens_ipow(begin_y, 8)*begin_dx + -7.78368e-08 *lens_ipow(begin_x, 7)*lens_ipow(begin_dx, 2) + -7.13255e-11 *begin_x*lens_ipow(begin_y, 8) + -2.5276e-10 *lens_ipow(begin_x, 7)*lens_ipow(begin_y, 2) + 1.63176e-07 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 4)*lens_ipow(begin_dy, 2) + -4.59009e-09 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 4)*begin_dx*begin_lambda + 2.41334e-08 *lens_ipow(begin_x, 7)*lens_ipow(begin_dy, 2)*begin_lambda + -2.13711e-13 *lens_ipow(begin_x, 11) + -2.81141e-11 *lens_ipow(begin_x, 8)*lens_ipow(begin_y, 2)*begin_dx + -1.07931e-11 *lens_ipow(begin_x, 10)*begin_dx + -7.39003e-12 *begin_x*lens_ipow(begin_y, 9)*begin_dy,
       + 0.720082 *begin_y + 24.0428 *begin_dy + -0.014603 *begin_y*begin_lambda + 0.488201 *begin_y*lens_ipow(begin_dx, 2) + 0.0532851 *lens_ipow(begin_y, 2)*begin_dy + 1.6659 *begin_y*lens_ipow(begin_dy, 2) + 1.1315 *begin_x*begin_dx*begin_dy + 0.033149 *begin_x*begin_y*begin_dx + 10.3268 *lens_ipow(begin_dx, 2)*begin_dy + 0.0189151 *lens_ipow(begin_x, 2)*begin_dy + 10.648 *lens_ipow(begin_dy, 3) + 6.86709e-07 *lens_ipow(begin_y, 5)*begin_lambda + -1.18904e-08 *lens_ipow(begin_y, 7) + -1.59389e-07 *begin_x*lens_ipow(begin_y, 5)*begin_dx + -6.6993e-08 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 3) + -7.33692e-07 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 2)*begin_dy + 2.01469e-08 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 3)*begin_lambda + -5.45356e-11 *lens_ipow(begin_x, 8)*begin_y + -1.45339e-09 *lens_ipow(begin_y, 8)*begin_dy + 1.39319e-07 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 2) + -2.51749e-10 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 7) + 1.70727e-10 *lens_ipow(begin_x, 8)*begin_y*lens_ipow(begin_dx, 2) + -2.91946e-11 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 8)*begin_dy + -7.61611e-14 *lens_ipow(begin_y, 11) + 5.90072e-11 *lens_ipow(begin_y, 9)*lens_ipow(begin_dx, 2) + -1.60411e-12 *lens_ipow(begin_x, 10)*begin_dy + 8.55147e-10 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 6)*begin_dx*begin_dy + -0.000201456 *lens_ipow(begin_x, 5)*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 3)
    };
    const float delta_ap[] = {ap_x - pred_ap[0], ap_y - pred_ap[1]};
    sqr_ap_err = delta_ap[0]*delta_ap[0]+delta_ap[1]*delta_ap[1];
    float dx1_domega0[2][2];
    dx1_domega0[0][0] =  + 24.0758  + 1.13639 *begin_y*begin_dy + 0.0496792 *lens_ipow(begin_x, 2) + 27.5798 *lens_ipow(begin_dx, 2) + 10.149 *lens_ipow(begin_dy, 2) + 3.03582 *begin_x*begin_dx + 0.0192878 *lens_ipow(begin_y, 2) + -0.00203249 *lens_ipow(begin_x, 4)*lens_ipow(begin_dx, 2) + -6.02066e-07 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 4) + -0.0392852 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 3) + -1.90383e-08 *lens_ipow(begin_y, 7)*begin_dy + -6.69381e-10 *lens_ipow(begin_y, 8) + -1.55674e-07 *lens_ipow(begin_x, 7)*begin_dx + -4.59009e-09 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 4)*begin_lambda + -2.81141e-11 *lens_ipow(begin_x, 8)*lens_ipow(begin_y, 2) + -1.07931e-11 *lens_ipow(begin_x, 10)+0.0f;
    dx1_domega0[0][1] =  + 1.13639 *begin_y*begin_dx + 0.032411 *begin_x*begin_y + 20.2981 *begin_dx*begin_dy + 0.928057 *begin_x*begin_dy + -2.12508e-07 *lens_ipow(begin_x, 5)*begin_y + -1.90383e-08 *lens_ipow(begin_y, 7)*begin_dx + 3.26352e-07 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 4)*begin_dy + 4.82668e-08 *lens_ipow(begin_x, 7)*begin_dy*begin_lambda + -7.39003e-12 *begin_x*lens_ipow(begin_y, 9)+0.0f;
    dx1_domega0[1][0] =  + 0.976401 *begin_y*begin_dx + 1.1315 *begin_x*begin_dy + 0.033149 *begin_x*begin_y + 20.6537 *begin_dx*begin_dy + -1.59389e-07 *begin_x*lens_ipow(begin_y, 5) + 2.78637e-07 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 3)*begin_dx + 3.41454e-10 *lens_ipow(begin_x, 8)*begin_y*begin_dx + 1.18014e-10 *lens_ipow(begin_y, 9)*begin_dx + 8.55147e-10 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 6)*begin_dy + -0.000604368 *lens_ipow(begin_x, 5)*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 3)+0.0f;
    dx1_domega0[1][1] =  + 24.0428  + 0.0532851 *lens_ipow(begin_y, 2) + 3.3318 *begin_y*begin_dy + 1.1315 *begin_x*begin_dx + 10.3268 *lens_ipow(begin_dx, 2) + 0.0189151 *lens_ipow(begin_x, 2) + 31.944 *lens_ipow(begin_dy, 2) + -7.33692e-07 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 2) + -1.45339e-09 *lens_ipow(begin_y, 8) + -2.91946e-11 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 8) + -1.60411e-12 *lens_ipow(begin_x, 10) + 8.55147e-10 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 6)*begin_dx + -0.000604368 *lens_ipow(begin_x, 5)*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 2)+0.0f;
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
    out[0] =  + -1.10799 *begin_x + -0.0905798 *begin_x*begin_lambda + 40.196 *begin_dx*begin_lambda + 2.10515 *begin_y*begin_dx*begin_dy + 0.0754832 *begin_x*begin_y*begin_dy + 0.133226 *lens_ipow(begin_x, 2)*begin_dx + 26.791 *lens_ipow(begin_dx, 3) + 27.9088 *begin_dx*lens_ipow(begin_dy, 2) + 0.00105276 *begin_x*lens_ipow(begin_y, 2) + 0.00119667 *lens_ipow(begin_x, 3) + 1.23306 *begin_x*lens_ipow(begin_dy, 2) + 3.36803 *begin_x*lens_ipow(begin_dx, 2) + 0.0534469 *lens_ipow(begin_y, 2)*begin_dx + -61.0312 *begin_dx*lens_ipow(begin_lambda, 2) + 30.4395 *begin_dx*lens_ipow(begin_lambda, 3) + -0.00016579 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*begin_dx + -8.16049e-07 *lens_ipow(begin_x, 6)*begin_dx + -2.03688e-06 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 3)*begin_dy + -6.44038e-08 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 4) + -0.0262952 *lens_ipow(begin_x, 2)*begin_y*lens_ipow(begin_dx, 3)*begin_dy + -3.92523e-08 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 2) + -0.00581142 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 4) + -1.95829e-08 *lens_ipow(begin_x, 7) + -0.128373 *begin_x*lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 4) + -6.99074e-11 *begin_x*lens_ipow(begin_y, 8) + -2.8667e-09 *begin_x*lens_ipow(begin_y, 7)*begin_dy + 1.94447e-07 *lens_ipow(begin_x, 6)*begin_y*begin_dx*begin_dy + 1.37312e-09 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 2);
    out[1] =  + -1.00151 *begin_y + 8.2466 *begin_dy + 0.97099 *begin_dy*begin_lambda + -0.455304 *begin_y*begin_lambda + 0.995845 *begin_y*lens_ipow(begin_dx, 2) + 0.130079 *lens_ipow(begin_y, 2)*begin_dy + 3.34769 *begin_y*lens_ipow(begin_dy, 2) + 1.9089 *begin_x*begin_dx*begin_dy + 0.0699908 *begin_x*begin_y*begin_dx + 18.5642 *lens_ipow(begin_dx, 2)*begin_dy + 0.324668 *begin_y*lens_ipow(begin_lambda, 2) + 0.000856088 *lens_ipow(begin_x, 2)*begin_y + 0.00110582 *lens_ipow(begin_y, 3) + 0.05103 *lens_ipow(begin_x, 2)*begin_dy + 26.4204 *lens_ipow(begin_dy, 3) + -1.25836 *begin_x*lens_ipow(begin_dx, 3)*begin_dy + -0.000134239 *lens_ipow(begin_x, 3)*begin_y*begin_dx + 0.00190002 *lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 2) + -3.15418 *begin_y*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2) + -0.000167132 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*begin_dy + -1.79689e-08 *lens_ipow(begin_y, 7) + -7.19221e-07 *lens_ipow(begin_y, 6)*begin_dy + -1.1258e-08 *lens_ipow(begin_x, 6)*begin_y + -3.3697e-08 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 5) + -3.903e-08 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 3) + 0.143156 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 5) + 2.30271e-07 *begin_x*lens_ipow(begin_y, 6)*begin_dx*begin_dy*begin_lambda + 1.74713e-09 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 5)*lens_ipow(begin_dx, 2);
    out[2] =  + -0.24973 *begin_dx + -0.0790022 *begin_x + 0.00582542 *begin_x*begin_lambda + 0.00236612 *lens_ipow(begin_x, 2)*begin_dx + 3.81983e-05 *begin_x*lens_ipow(begin_y, 2) + 8.9246e-05 *lens_ipow(begin_x, 3) + 0.172699 *begin_x*lens_ipow(begin_dx, 2)*begin_lambda + 0.00165633 *lens_ipow(begin_y, 2)*begin_dx*begin_lambda + 5.81234e-06 *lens_ipow(begin_y, 4)*begin_dx + 6.72492 *lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 2) + -6.06714 *lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 3) + 0.00621329 *lens_ipow(begin_x, 2)*begin_dx*lens_ipow(begin_lambda, 3) + 0.000999368 *lens_ipow(begin_y, 3)*begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + -5.56523e-12 *lens_ipow(begin_x, 7)*lens_ipow(begin_y, 2) + 3.86556e-09 *begin_x*lens_ipow(begin_y, 6)*lens_ipow(begin_lambda, 2) + 5.03358e-05 *begin_x*lens_ipow(begin_y, 3)*begin_dy*lens_ipow(begin_lambda, 4) + 1.35499e-07 *begin_x*lens_ipow(begin_y, 5)*begin_dy*lens_ipow(begin_lambda, 2) + -3.11126e-05 *lens_ipow(begin_x, 4)*begin_dx*lens_ipow(begin_lambda, 4) + 6.72353e-09 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 4)*lens_ipow(begin_lambda, 2) + 171.022 *lens_ipow(begin_dx, 5)*lens_ipow(begin_dy, 4)*begin_lambda + -0.00458756 *lens_ipow(begin_y, 3)*begin_dx*lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 3) + -2.83521e-11 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 6)*begin_lambda + -2.96283e-11 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 4)*begin_lambda + -3.03637e-12 *lens_ipow(begin_x, 9)*begin_lambda + 3.03521e-08 *lens_ipow(begin_y, 6)*begin_dx*lens_ipow(begin_lambda, 4) + 8.32911e-05 *lens_ipow(begin_x, 4)*lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 4) + -1.12336e-11 *begin_x*lens_ipow(begin_y, 8)*lens_ipow(begin_lambda, 2) + 1.24652e-05 *begin_x*lens_ipow(begin_y, 4)*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 4);
    out[3] =  + -0.0812351 *begin_y + -0.280867 *begin_dy + 0.0429656 *begin_dy*begin_lambda + 0.0140781 *begin_y*begin_lambda + 0.00330655 *lens_ipow(begin_y, 2)*begin_dy + 0.106182 *begin_y*lens_ipow(begin_dy, 2) + 0.0529577 *begin_x*begin_dx*begin_dy + -0.00852811 *begin_y*lens_ipow(begin_lambda, 2) + 0.000112606 *lens_ipow(begin_x, 2)*begin_y + 8.92269e-05 *lens_ipow(begin_y, 3) + 0.00131885 *lens_ipow(begin_x, 2)*begin_dy + 1.11666 *lens_ipow(begin_dy, 3) + 1.72313 *lens_ipow(begin_dx, 2)*begin_dy*begin_lambda + 0.00206368 *begin_x*begin_y*begin_dx*begin_lambda + 0.0544768 *begin_x*begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + -0.0347859 *begin_x*lens_ipow(begin_dx, 3)*begin_dy + 0.114422 *begin_y*lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 2) + 0.00361735 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2)*begin_dy + -0.0311424 *begin_y*lens_ipow(begin_dy, 4) + 9.33907e-05 *lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 2) + -0.0805241 *begin_y*lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 3) + 1.877e-05 *lens_ipow(begin_x, 4)*lens_ipow(begin_dy, 3)*begin_lambda + 2.03187e-06 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 2)*begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + -1.02479e-12 *lens_ipow(begin_y, 9)*begin_lambda + -5.92894e-12 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 7)*begin_lambda + -2.83505e-15 *lens_ipow(begin_y, 11) + -3.67053e-14 *lens_ipow(begin_x, 8)*lens_ipow(begin_y, 3) + -4.77863e-14 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 7);
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
    domega2_dx0[0][0] =  + -0.0790022  + 0.00582542 *begin_lambda + 0.00473224 *begin_x*begin_dx + 3.81983e-05 *lens_ipow(begin_y, 2) + 0.000267738 *lens_ipow(begin_x, 2) + 0.172699 *lens_ipow(begin_dx, 2)*begin_lambda + 0.0124266 *begin_x*begin_dx*lens_ipow(begin_lambda, 3) + -3.89566e-11 *lens_ipow(begin_x, 6)*lens_ipow(begin_y, 2) + 3.86556e-09 *lens_ipow(begin_y, 6)*lens_ipow(begin_lambda, 2) + 5.03358e-05 *lens_ipow(begin_y, 3)*begin_dy*lens_ipow(begin_lambda, 4) + 1.35499e-07 *lens_ipow(begin_y, 5)*begin_dy*lens_ipow(begin_lambda, 2) + -0.000124451 *lens_ipow(begin_x, 3)*begin_dx*lens_ipow(begin_lambda, 4) + 2.01706e-08 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 4)*lens_ipow(begin_lambda, 2) + -8.50564e-11 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 6)*begin_lambda + -1.48141e-10 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 4)*begin_lambda + -2.73274e-11 *lens_ipow(begin_x, 8)*begin_lambda + 0.000333164 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 4) + -1.12336e-11 *lens_ipow(begin_y, 8)*lens_ipow(begin_lambda, 2) + 1.24652e-05 *lens_ipow(begin_y, 4)*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 4)+0.0f;
    domega2_dx0[0][1] =  + 7.63966e-05 *begin_x*begin_y + 0.00331265 *begin_y*begin_dx*begin_lambda + 2.32494e-05 *lens_ipow(begin_y, 3)*begin_dx + 0.0029981 *lens_ipow(begin_y, 2)*begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + -1.11305e-11 *lens_ipow(begin_x, 7)*begin_y + 2.31933e-08 *begin_x*lens_ipow(begin_y, 5)*lens_ipow(begin_lambda, 2) + 0.000151007 *begin_x*lens_ipow(begin_y, 2)*begin_dy*lens_ipow(begin_lambda, 4) + 6.77497e-07 *begin_x*lens_ipow(begin_y, 4)*begin_dy*lens_ipow(begin_lambda, 2) + 2.68941e-08 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 3)*lens_ipow(begin_lambda, 2) + -0.0137627 *lens_ipow(begin_y, 2)*begin_dx*lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 3) + -1.70113e-10 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 5)*begin_lambda + -1.18513e-10 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 3)*begin_lambda + 1.82113e-07 *lens_ipow(begin_y, 5)*begin_dx*lens_ipow(begin_lambda, 4) + -8.98691e-11 *begin_x*lens_ipow(begin_y, 7)*lens_ipow(begin_lambda, 2) + 4.9861e-05 *begin_x*lens_ipow(begin_y, 3)*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 4)+0.0f;
    domega2_dx0[1][0] =  + 0.0529577 *begin_dx*begin_dy + 0.000225211 *begin_x*begin_y + 0.0026377 *begin_x*begin_dy + 0.00206368 *begin_y*begin_dx*begin_lambda + 0.0544768 *begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + -0.0347859 *lens_ipow(begin_dx, 3)*begin_dy + 7.50802e-05 *lens_ipow(begin_x, 3)*lens_ipow(begin_dy, 3)*begin_lambda + 6.09562e-06 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + -1.18579e-11 *begin_x*lens_ipow(begin_y, 7)*begin_lambda + -2.93642e-13 *lens_ipow(begin_x, 7)*lens_ipow(begin_y, 3) + -1.91145e-13 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 7)+0.0f;
    domega2_dx0[1][1] =  + -0.0812351  + 0.0140781 *begin_lambda + 0.0066131 *begin_y*begin_dy + 0.106182 *lens_ipow(begin_dy, 2) + -0.00852811 *lens_ipow(begin_lambda, 2) + 0.000112606 *lens_ipow(begin_x, 2) + 0.000267681 *lens_ipow(begin_y, 2) + 0.00206368 *begin_x*begin_dx*begin_lambda + 0.114422 *lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 2) + 0.00723471 *begin_y*lens_ipow(begin_dx, 2)*begin_dy + -0.0311424 *lens_ipow(begin_dy, 4) + 0.000280172 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2) + -0.0805241 *lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 3) + 4.06375e-06 *lens_ipow(begin_x, 3)*begin_y*begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + -9.22309e-12 *lens_ipow(begin_y, 8)*begin_lambda + -4.15026e-11 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 6)*begin_lambda + -3.11856e-14 *lens_ipow(begin_y, 10) + -1.10116e-13 *lens_ipow(begin_x, 8)*lens_ipow(begin_y, 2) + -3.34504e-13 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 6)+0.0f;
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
  out[4] =  + 0.829835 *begin_lambda + -1.08215 *lens_ipow(begin_lambda, 2) + 0.503594 *lens_ipow(begin_lambda, 3) + 0.0145961 *begin_y*lens_ipow(begin_dy, 3) + -0.568749 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2) + -2.16067e-06 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2) + -3.82145e-05 *lens_ipow(begin_y, 3)*begin_dy + -1.76453e-06 *lens_ipow(begin_x, 4)*begin_lambda + -5.34716e-07 *lens_ipow(begin_y, 4)*begin_lambda + 0.00280576 *begin_x*begin_y*begin_dx*begin_dy*begin_lambda + -0.793914 *lens_ipow(begin_dx, 6) + -2.16535e-08 *lens_ipow(begin_y, 6) + -4.93127e-07 *lens_ipow(begin_x, 5)*begin_dx*begin_lambda + -4.54478e-09 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 4)*begin_dx + 1.46013e-10 *lens_ipow(begin_y, 8) + -2.45372e-11 *lens_ipow(begin_x, 8) + -0.0114921 *begin_x*begin_y*lens_ipow(begin_dx, 3)*begin_dy*lens_ipow(begin_lambda, 2) + -7.97349e-09 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 3)*begin_dy*begin_lambda + -0.0138635 *begin_x*begin_y*begin_dx*lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 3) + -3.07018e-07 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 3)*begin_dx*begin_dy*begin_lambda + 1.30754e-10 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 6)*begin_lambda + -3.74594e-13 *lens_ipow(begin_y, 10) + -5.66313e-05 *lens_ipow(begin_x, 3)*begin_dx*lens_ipow(begin_lambda, 6) + -1.20621e-12 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 6) + -6.28574e-08 *lens_ipow(begin_x, 6)*lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 2) + -2.57329e-13 *lens_ipow(begin_x, 8)*lens_ipow(begin_y, 2) + -1.16038e-12 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 8)*begin_lambda + -2.06616e-12 *lens_ipow(begin_x, 6)*lens_ipow(begin_y, 4)*begin_lambda;
else
  out[4] = 0.0f;

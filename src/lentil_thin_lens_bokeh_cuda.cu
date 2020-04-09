#include <vector>
#include <iostream>
#include <map>
#include "../../Eigen/Eigen/Core"
#include "../../Eigen/Eigen/LU"
#include <fstream>


#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"


#include <chrono>


std::string replace_first_occurence(std::string& s, const std::string& toReplace, const std::string& replaceWith) {
    std::size_t pos = s.find(toReplace);
    if (pos == std::string::npos) return s;
    return s.replace(pos, toReplace.length(), replaceWith);
}

void save_to_exr_rgba(std::vector<float> img, std::string filename, unsigned xres, unsigned yres) {
  EXRHeader header;
  InitEXRHeader(&header);

  EXRImage image;
  InitEXRImage(&image);
  image.num_channels = 4;
  image.width = xres;
  image.height = yres;

  std::vector<float> images[4];
  images[0].resize(xres * yres);
  images[1].resize(xres * yres);
  images[2].resize(xres * yres);
  images[3].resize(xres * yres);

  for (unsigned int i = 0; i < xres * yres; i++) {
    images[0][i] = img[4*i+0];
    images[1][i] = img[4*i+1];
    images[2][i] = img[4*i+2];
    images[3][i] = img[4*i+3];
  }

  float* image_ptr[4];
  image_ptr[0] = &(images[3].at(0)); // A
  image_ptr[1] = &(images[2].at(0)); // B
  image_ptr[2] = &(images[1].at(0)); // G
  image_ptr[3] = &(images[0].at(0)); // R

  image.images = (unsigned char**)image_ptr;
  header.num_channels = 4;
  header.channels = (EXRChannelInfo *)malloc(sizeof(EXRChannelInfo) * header.num_channels);
  strncpy(header.channels[0].name, "A", 255); header.channels[0].name[strlen("A")] = '\0';
  strncpy(header.channels[1].name, "B", 255); header.channels[1].name[strlen("B")] = '\0';
  strncpy(header.channels[2].name, "G", 255); header.channels[2].name[strlen("G")] = '\0';
  strncpy(header.channels[3].name, "R", 255); header.channels[3].name[strlen("R")] = '\0';

  header.pixel_types = (int *)malloc(sizeof(int) * header.num_channels); 
  header.requested_pixel_types = (int *)malloc(sizeof(int) * header.num_channels);
  for (int i = 0; i < header.num_channels; i++) {
    header.pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT; // pixel type of input image
    header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_HALF; // pixel type of output image to be stored in .EXR
  }
  
  header.compression_type = TINYEXR_COMPRESSIONTYPE_ZIP;

  const char* err;
  int ret = SaveEXRImageToFile(&image, &header, filename.c_str(), &err);
  if (ret != TINYEXR_SUCCESS) {
    std::cout << "[LENTIL BIDIRECTIONAL TL] Error when saving exr: " << err << std::endl;
  }
}


// xorshift fast random number generator
__device__ uint32_t xor128(void){
  static uint32_t x = 123456789, y = 362436069, z = 521288629, w = 88675123;
  uint32_t t = x ^ (x << 11);
  x = y; y = z; z = w;
  return w = (w ^ (w >> 19) ^ t ^ (t >> 8));
}


// sin approximation, not completely accurate but faster than std::sin
__device__ float fast_sin(float x){
    x = fmod(x + M_PI, M_PI * 2) - M_PI; // restrict x so that -M_PI < x < M_PI
    const float B = 4.0f / M_PI;
    const float C = -4.0f / (M_PI*M_PI);
    float y = B * x + C * x * std::abs(x);
    const float P = 0.225f;
    return P * (y * std::abs(y) - y) + y;
}


__device__ float fast_cos(float x){
    // conversion from sin to cos
    x += M_PI * 0.5;

    x = fmod(x + M_PI, M_PI * 2) - M_PI; // restrict x so that -M_PI < x < M_PI
    const float B = 4.0f / M_PI;
    const float C = -4.0f / (M_PI*M_PI);
    float y = B * x + C * x * std::abs(x);
    const float P = 0.225f;
    return P * (y * std::abs(y) - y) + y;
}


// Improved concentric mapping code by Dave Cline [peter shirley´s blog]
// maps points on the unit square onto the unit disk uniformly
__device__ void concentricDiskSample(float ox, float oy, Eigen::Vector2d &lens) {
    if (ox == 0.0 && oy == 0.0){
        lens(0) = 0.0;
        lens(1) = 0.0;
        return;
    }

    float phi, r;

    // switch coordinate space from [0, 1] to [-1, 1]
    const float a = 2.0 * ox - 1.0;
    const float b = 2.0 * oy - 1.0;

    if ((a * a) > (b * b)){
        r = a;
        phi = 0.78539816339 * (b / a);
    }
    else {
        r = b;
        phi = (1.57079632679) - ((0.78539816339) * (a / b));
    }


    bool fast_trigo = false;

    const float cos_phi = cosf(phi);
    const float sin_phi =  sinf(phi);
    lens(0) = r * cos_phi;
    lens(1) = r * sin_phi;
}


inline float clamp_min(float in, const float min) {
    if (in < min) in = min;
    return in;
}

inline float clamp(float in, const float min, const float max) {
    if (in < min) in = min;
    if (in > max) in = max;
    return in;
}

__device__ float thinlens_get_image_dist_focusdist(const float focal_length, const float focus_distance){
    return (-focal_length * -focus_distance) / (-focal_length + -focus_distance);
}


 __device__ float rand(Eigen::Vector2f vec){
    Eigen::Vector2f constant(12.9898,78.233);
    float dotproduct = vec.dot(constant);
    float tmp = sinf(dotproduct) * 43758.5453;
    return tmp - floor(tmp);
}


__global__ void trace_backwards(Eigen::Vector4d *image, Eigen::Vector4d *image_unredist, Eigen::Vector4d *image_redist,
                                      float *redist_weight_per_pixel, float *unredist_weight_per_pixel, float *zbuffer,
                                      const Eigen::Vector3d *sample_pos_cs, const float *focal_length, const float *aperture_radius, 
                                      const float *focus_distance, const float *sensor_width, const float *frame_aspect_ratio,
                                      const int *xres, const int *yres,
                                      const Eigen::Vector4d *sample, const int *samples, const float *inv_density) {

  
  const Eigen::Vector3d camera_space_sample_position_mb = *sample_pos_cs;
  const float image_dist_samplepos_mb = (-*focal_length * camera_space_sample_position_mb(2)) / (-*focal_length + camera_space_sample_position_mb(2));



  int i = threadIdx.x + blockIdx.x * blockDim.x;
  Eigen::Vector2f randseed1(float(i), float(i+123));
  int i2 = threadIdx.x + blockIdx.x * blockDim.x + 12345;
  Eigen::Vector2f randseed2(float(i2), float(i2+123));
  float r1 = rand(randseed1);
  float r2 = rand(randseed2);
  



  // either get uniformly distributed points on the unit disk or bokeh image
  Eigen::Vector2d unit_disk(0, 0);
  // float r1 = xor128() / 4294967296.0;
  // float r2 = xor128() / 4294967296.0;
  // printf("r1: %f, r2: %f", r1, r2);
  concentricDiskSample(r1, r2, unit_disk);
  
  // ray through center of lens
  Eigen::Vector3d dir_tobase = camera_space_sample_position_mb.normalized();
  float samplepos_image_intersection = std::abs(image_dist_samplepos_mb/dir_tobase(2));
  Eigen::Vector3d samplepos_image_point = dir_tobase * samplepos_image_intersection;

  // depth of field
  Eigen::Vector3d lens(unit_disk(0) * *aperture_radius, unit_disk(1) * *aperture_radius, 0.0);
  Eigen::Vector3d dir_from_lens_to_image_sample = samplepos_image_point - lens;
  dir_from_lens_to_image_sample.normalize();
  float focusdist_intersection = std::abs(thinlens_get_image_dist_focusdist(*focal_length, *focus_distance)/dir_from_lens_to_image_sample(2));
  

  Eigen::Vector3d focusdist_image_point = lens + dir_from_lens_to_image_sample*focusdist_intersection;
  
  // takes care of correct screenspace coordinate mapping
  Eigen::Vector2d sensor_position(focusdist_image_point(0) / focusdist_image_point(2),
                                  focusdist_image_point(1) / focusdist_image_point(2));
  sensor_position /= (*sensor_width*0.5)/-*focal_length;


  // optical vignetting
  Eigen::Vector3d dir_lens_to_P = camera_space_sample_position_mb - lens;
  dir_lens_to_P.normalize();


  // convert sensor position to pixel position
  float frame_aspect_ratio_tmp = *frame_aspect_ratio;
  // printf("frame aspect: %f", *frame_aspect_ratio);
  const float pixel_x = (( sensor_position(0) + 1.0) / 2.0) * *xres;
  const float pixel_y = ((-sensor_position(1) * frame_aspect_ratio_tmp + 1.0) / 2.0) * *yres;
  // printf("%f %f \n", pixel_x, pixel_y);

  // if outside of image
  if ((pixel_x >= *xres) || (pixel_x < 0) || (pixel_y >= *yres) || (pixel_y < 0)) return;

  // write sample to image
  unsigned pixelnumber = static_cast<int>(*xres * floor(pixel_y) + floor(pixel_x));

  
  Eigen::Vector4d rgba_energy = *sample / (double)(*samples);
  image_redist[pixelnumber] += rgba_energy * *inv_density;
  redist_weight_per_pixel[pixelnumber] += *inv_density / double(*samples);
  

  return;
}


int main() {
  
  // read the sampledata into vectors
  std::ifstream infile("/home/cactus/lentil/pota/tests/cuda/sampledata.txt");
  float sample_r, sample_g, sample_b, sample_a, depth, sample_pos_ws_x, sample_pos_ws_y, sample_pos_ws_z;
  std::vector<Eigen::Vector4d> sample_list;
  std::vector<Eigen::Vector3d> pos_ws_list;
  std::vector<float> depth_list;
  while (infile >> sample_r >> sample_g >> sample_b >> sample_a >> depth >> sample_pos_ws_x >> sample_pos_ws_y >> sample_pos_ws_z)
  {
      sample_list.push_back(Eigen::Vector4d(sample_r, sample_g, sample_b, sample_a));
      depth_list.push_back(depth);
      pos_ws_list.push_back(Eigen::Vector3d(sample_pos_ws_x, sample_pos_ws_y, sample_pos_ws_z));
  }



  int xres = 1920;
  int yres = 1080;
  int framenumber = 1;
  int aa_samples = 4;
  
  
  Eigen::Vector4d *image_device, *image_unredist_device, *image_redist_device;
  cudaMalloc((void **)&image_device, xres*yres*sizeof(Eigen::Vector4d));
  cudaMalloc((void **)&image_unredist_device, xres*yres*sizeof(Eigen::Vector4d));
  cudaMalloc((void **)&image_redist_device, xres*yres*sizeof(Eigen::Vector4d));

  float *redist_weight_per_pixel_device, *unredist_weight_per_pixel_device, *zbuffer_device;
  cudaMalloc((void **)&redist_weight_per_pixel_device, xres*yres*sizeof(float));
  cudaMalloc((void **)&unredist_weight_per_pixel_device, xres*yres*sizeof(float));
  cudaMalloc((void **)&zbuffer_device, xres*yres*sizeof(float));




  float sensor_width;
  float focal_length;
  float fstop;
  float focus_distance;
  float aperture_radius;
  float abb_spherical;
  float circle_to_square;
  float bokeh_anamorphic;
  std::string bidir_output_path;
  unsigned int bidir_sample_mult;

  sensor_width = 36.0;
  focal_length = 50.0;
  focal_length = clamp_min(focal_length, 0.01);
  fstop = 1.4;
  fstop = clamp_min(fstop, 0.01);
  focus_distance = 35.0;
  aperture_radius = (focal_length / (2.0 * fstop)) / 10.0;
  bidir_output_path = "/home/cactus/lentil/pota/tests/cuda/cuda.tl.<aov>.<frame>.exr";
  abb_spherical = 0.5;
  abb_spherical = clamp(abb_spherical, 0.001, 0.999);
  circle_to_square = 0.0;
  circle_to_square = clamp(circle_to_square, 0.01, 0.99);
  bokeh_anamorphic = 1.0;
  bokeh_anamorphic = clamp(bokeh_anamorphic, 0.01, 99999.0);
  bidir_sample_mult = 10;


  

  std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();




  const float d_xres = (float)xres;
  const float d_yres = (float)yres;
  const float frame_aspect_ratio = d_xres/d_yres;
  std::cout << "frame aspect ratio: " << frame_aspect_ratio << std::endl;




  int *xres_device, *yres_device, *samples_device;
  float *focal_length_device, *aperture_radius_device, *focus_distance_device, *sensor_width_device, *frame_aspect_ratio_device;

  cudaMalloc((void **)&xres_device, sizeof(int));
  cudaMalloc((void **)&yres_device, sizeof(int));
  cudaMalloc((void **)&samples_device, sizeof(int));

  cudaMalloc((void **)&focal_length_device, sizeof(float));
  cudaMalloc((void **)&aperture_radius_device, sizeof(float));
  cudaMalloc((void **)&focus_distance_device, sizeof(float));
  cudaMalloc((void **)&sensor_width_device, sizeof(float));
  cudaMalloc((void **)&frame_aspect_ratio_device, sizeof(float));

  cudaMemcpy(focal_length_device, &focal_length, sizeof(float), cudaMemcpyHostToDevice);
  cudaMemcpy(aperture_radius_device, &aperture_radius, sizeof(float), cudaMemcpyHostToDevice);
  cudaMemcpy(focus_distance_device, &focus_distance, sizeof(float), cudaMemcpyHostToDevice);
  cudaMemcpy(sensor_width_device, &sensor_width, sizeof(float), cudaMemcpyHostToDevice);	
  cudaMemcpy(frame_aspect_ratio_device, &frame_aspect_ratio, sizeof(float), cudaMemcpyHostToDevice);
  cudaMemcpy(xres_device, &xres, sizeof(int), cudaMemcpyHostToDevice);	
  cudaMemcpy(yres_device, &yres, sizeof(int), cudaMemcpyHostToDevice);


  for (int i=0; i<sample_list.size(); ++i) {

    // std::cout << "count: " << i << std::endl;


    Eigen::Vector4d sample = sample_list[i];
    
    const Eigen::Vector3d sample_pos_cs = pos_ws_list[i];
    float depth = depth_list[i];
    const float inv_density = 1.0/16.0;
    
    const Eigen::Vector4d sample_transmission = Eigen::Vector4d(0,0,0,0);
    bool transmitted_energy_in_sample = ((sample_transmission(0)) > 0.0);
    if (transmitted_energy_in_sample){
      sample(0) -= sample_transmission(0);
      sample(1) -= sample_transmission(1);
      sample(2) -= sample_transmission(2);
    }




      // Eigen::Matrix4d world_to_camera_matrix;
      // world_to_camera_matrix << 1.0, 0.0, 0.0, 0.0,
      //                           0.0, 1.0, 0.0, 0.0,
      //                           0.0, 0.0, 1.0, -75.0,
      //                           0.0, 0.0, 0.0, 1.0;

      const Eigen::Vector3d camera_space_sample_position_static = sample_pos_cs;
      
      const float image_dist_samplepos = (-focal_length * camera_space_sample_position_static(2)) / (-focal_length + camera_space_sample_position_static(2));
      const float image_dist_focusdist = (-focal_length * -focus_distance) / (-focal_length + -focus_distance);
      float circle_of_confusion = std::abs((aperture_radius * (image_dist_samplepos - image_dist_focusdist))/image_dist_samplepos); // coc diameter
      


      const float coc_squared_pixels = std::pow(circle_of_confusion * yres, 2) * bidir_sample_mult * 0.01; // pixel area as baseline for sample count
      // if (std::pow(circle_of_confusion * yres, 2) < std::pow(15, 2)) goto no_redist; // 15^2 px minimum coc
      int samples = std::ceil(coc_squared_pixels / (double)std::pow(aa_samples, 2)); // aa_sample independence
      samples = clamp(samples, 100, 1000000);


      
      

// TO-PARALLELIZE
      // unsigned total_samples_taken = 0;
      // unsigned count = 0;
      // while(count<samples && ++total_samples_taken < samples*10) {


    float *inv_density_device;
    Eigen::Vector4d *sample_device;
    Eigen::Vector3d *sample_pos_cs_device;
    cudaMalloc((void **)&inv_density_device, sizeof(float));
    cudaMalloc((void **)&sample_device, sizeof(Eigen::Vector4d));
    cudaMalloc((void **)&sample_pos_cs_device, sizeof(Eigen::Vector3d));

    cudaMemcpy(inv_density_device, &inv_density, sizeof(float), cudaMemcpyHostToDevice);	
    cudaMemcpy(sample_device, &sample, sizeof(Eigen::Vector4d), cudaMemcpyHostToDevice);	
    cudaMemcpy(sample_pos_cs_device, &sample_pos_cs, sizeof(Eigen::Vector3d), cudaMemcpyHostToDevice);
      
      
    trace_backwards<<<4, 512>>>(image_device, image_unredist_device, image_redist_device,
                    redist_weight_per_pixel_device, unredist_weight_per_pixel_device, zbuffer_device,
                    sample_pos_cs_device, focal_length_device, aperture_radius_device, 
                    focus_distance_device, sensor_width_device, frame_aspect_ratio_device,
                    xres_device, yres_device,
                    sample_device, samples_device, inv_density_device);
        
        // if (!success) continue;
        // ++count;
      // }
      
    

      
    cudaFree(inv_density_device);
    cudaFree(samples_device);
    cudaFree(sample_device);
    cudaFree(sample_pos_cs_device);
  }

  cudaDeviceSynchronize();

  cudaFree(xres_device);
  cudaFree(yres_device);
  cudaFree(focal_length_device);
  cudaFree(aperture_radius_device);
  cudaFree(focus_distance_device);
  cudaFree(sensor_width_device);
  cudaFree(frame_aspect_ratio_device);
  
    
  Eigen::Vector4d *image = new Eigen::Vector4d[xres*yres];
  Eigen::Vector4d *image_unredist = new Eigen::Vector4d[xres*yres];
  Eigen::Vector4d *image_redist = new Eigen::Vector4d[xres*yres];
  float *redist_weight_per_pixel = new float[xres*yres];
  float *unredist_weight_per_pixel = new float[xres*yres];
  float *zbuffer = new float[xres*yres];



  // cudaMemcpy(&image, image_device, xres*yres*sizeof(Eigen::Vector4d), cudaMemcpyDeviceToHost);
  // cudaMemcpy(&image_unredist, image_unredist_device, xres*yres*sizeof(Eigen::Vector4d), cudaMemcpyDeviceToHost);
  // cudaMemcpy(&image_redist, image_redist_device, xres*yres*sizeof(Eigen::Vector4d), cudaMemcpyDeviceToHost);
  // cudaMemcpy(&redist_weight_per_pixel, redist_weight_per_pixel_device, xres*yres*sizeof(float), cudaMemcpyDeviceToHost);
  // cudaMemcpy(&unredist_weight_per_pixel, unredist_weight_per_pixel_device, xres*yres*sizeof(float), cudaMemcpyDeviceToHost);
  // cudaMemcpy(&zbuffer, zbuffer_device, xres*yres*sizeof(float), cudaMemcpyDeviceToHost);
  cudaMemcpy(image, image_device, xres*yres*sizeof(Eigen::Vector4d), cudaMemcpyDeviceToHost);
  cudaMemcpy(image_unredist, image_unredist_device, xres*yres*sizeof(Eigen::Vector4d), cudaMemcpyDeviceToHost);
  cudaMemcpy(image_redist, image_redist_device, xres*yres*sizeof(Eigen::Vector4d), cudaMemcpyDeviceToHost);
  cudaMemcpy(redist_weight_per_pixel, redist_weight_per_pixel_device, xres*yres*sizeof(float), cudaMemcpyDeviceToHost);
  cudaMemcpy(unredist_weight_per_pixel, unredist_weight_per_pixel_device, xres*yres*sizeof(float), cudaMemcpyDeviceToHost);
  cudaMemcpy(zbuffer, zbuffer_device, xres*yres*sizeof(float), cudaMemcpyDeviceToHost);

  
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  std::cout << "Time difference (sec) = " << (std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()) /1000000.0 <<std::endl;
  





// CLOSE


  std::vector<float> imageexr(yres * xres * 4);
  int offset = -1;

  for(unsigned px = 0; px < xres * yres; px++){

    Eigen::Vector4d redist = image_redist[px] / ((redist_weight_per_pixel[px] == 0.0) ? 1.0 : redist_weight_per_pixel[px]);
    Eigen::Vector4d unredist = image_unredist[px] / ((unredist_weight_per_pixel[px] == 0.0) ? 1.0 : unredist_weight_per_pixel[px]);
    Eigen::Vector4d combined_redist_unredist = (unredist * (1.0-redist_weight_per_pixel[px])) + (redist * (redist_weight_per_pixel[px]));
    // if (image_redist[px](0) > 0.0) std::cout << image_redist[px] << std::endl;
    if (combined_redist_unredist(3) > 0.95) combined_redist_unredist /= combined_redist_unredist(3);

    imageexr[++offset] = combined_redist_unredist(0);
    imageexr[++offset] = combined_redist_unredist(1);
    imageexr[++offset] = combined_redist_unredist(2);
    imageexr[++offset] = combined_redist_unredist(3);
  
  }

  // replace <aov> and <frame>
  std::string path = bidir_output_path;
  std::string path_replaced_aov = replace_first_occurence(path, "<aov>", "RGBA");
  
  std::string frame_str = std::to_string(framenumber);
  std::string frame_padded = std::string(4 - frame_str.length(), '0') + frame_str;
  std::string path_replaced_framenumber = replace_first_occurence(path, "<frame>", frame_padded);

  // dump framebuffers to exrs
  save_to_exr_rgba(imageexr, path_replaced_framenumber, xres, yres);

  printf("written image!");

  cudaFree(image_device);
  cudaFree(image_unredist_device);
  cudaFree(image_redist_device);
  cudaFree(redist_weight_per_pixel_device);
  cudaFree(unredist_weight_per_pixel_device);
  cudaFree(zbuffer_device);


  return 0;
}
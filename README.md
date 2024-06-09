# FacialBRDFCapture
Personal implementation of disney's paper:  
2020, J Riviere et al., Single-Shot High-Quality Facial Geometry and Skin Appearance Capture  
https://www.youtube.com/watch?v=H_iiIl4EqHU

To apply the techniques discussed in the paper, a data preparation step is required to process the captured images. Although I implemented the data preparation step in a separate project, this repository will only include the main part of the paper. Instead, I will provide pre-processed data so that you can run the main algorithm directly. 


## Build Requirements
* Microsoft's Visual Studio 2019 or newer
* Google's ceres solver


This project is designed for Windows environments, but I believe that adapting it to other environments would require minimal effort, as it relies very little on Windows-specific dependencies, except for the macro DEFINE_ENUM_FLAG_OPERATORS which helps bitflag operations for enum.

The easiest method for installing Ceres Solver on Windows is through vcpkg:  
http://ceres-solver.org/installation.html


## Pipeline
![pipeline](https://github.com/phgphg777/FacialBRDFCapture/assets/57425078/4c57ec7c-2644-4d58-ab5b-a6d87be52ac3)

![스크린샷 2024-06-09 214222](https://github.com/phgphg777/FacialBRDFCapture/assets/57425078/ec6c0f89-ee03-47d0-9bb7-f225ec7780e9)



## Final output (zoom in)
![Example Image1](Data/example/output/1/predicted_diffuse_31.png)

![Example Image2](Data/example/output/1/predicted_roughness_31.png)

![Example Image3](Data/example/output/1/predicted_normal_31.png)

![Example Image3](Data/example/output/1/predicted_height_31.png)

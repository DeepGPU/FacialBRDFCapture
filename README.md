# FacialBRDFCapture
Personal implementation of disney's paper:  
'Single-Shot High-Quality Facial Geometry and Skin Appearance Capture'  
https://www.youtube.com/watch?v=H_iiIl4EqHU

To apply the techniques discussed in the paper, a data preparation step is required to process the captured images. Although I implemented the data preparation step in a separate project, this repository will only include the main part of the paper. Instead, I will provide pre-processed data so that you can run the main algorithm directly. 


## Build Requirements
* Microsoft's Visual Studio 2019 or newer
* Google's ceres solver


This project is designed for Windows environments, but I believe that adapting it to other environments would require minimal effort, as it relies very little on Windows-specific dependencies, except for the macro DEFINE_ENUM_FLAG_OPERATORS which helps bitflag operations for enum.

The easiest method for installing Ceres Solver on Windows is through vcpkg:  
http://ceres-solver.org/installation.html


## Pipeline

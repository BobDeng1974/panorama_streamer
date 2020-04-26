FASTVIDEO SDK for GPU Image Processing - Copyright 2012-2014 Fastvideo
version 0.7.6.9
www.fastcompression.com
Compiled for Windows-7/8 [x86/x64]

Distribution of this software without any permission from Fastvideo is NOT allowed. NO warranty and responsibility is provided by the authors for the consequences of using it. 

Installation
Unzip all files into working folder and run console applications from command line. It's necessary to have NVIDIA GPU (Fermi or Kepler) installed to run the software on GPU. Please use fastvideo_samples.sln from root folder to load all sample projects for MSVS 2012. You can get all parameters for each console application by running the software without any parameter.

Dongle
By default the software is running in demo mode with built-in watermarks on the final images. To run the software in release mode (without watermarks) one have to connect a dongle and install the driver.

Software and Hardware Requirements
 - OS Windows-7/8 (64-bit)
 - NVIDIA GPU 400, 500, 600 or 700 series with Compute Capability >= 2.0, NVIDIA drivers 337.88 or later
 - CUDA-6.0 for Windows-7/8 (64-bit)

For the latest NVIDIA drivers visit http://www.nvidia.com/Download/index.aspx
The latest Senselock dongle drivers one can get here: http://senselock.ru/files/senselock_windows_2.52.1.0.rar

This software uses code of FFmpeg, licensed under the LGPL v2.1 and its source can be downloaded from ffmpeg.org. We do not own FFmpeg, and you can find the relevant owners at ffmpeg.org.

This software uses code of C++ Template Image Processing Toolkit (CImg Library) from http://cimg.sourceforge.net. CImg is a free, open-source library distributed under the CeCILL-C (close to the GNU LGPL) license.

REFERENCES
 - What is JPEG: http://en.wikipedia.org/wiki/JPEG
 - JPEG standard: http://www.w3.org/Graphics/JPEG/itu-t81.pdf
 - FFMPEG library: http://www.ffmpeg.org
 - Full Image Processing Pipeline on the GPU: http://on-demand.gputechconf.com/gtc/2014/presentations/S4151-full-gpu-image-processing-pipeline-camera-apps.pdf
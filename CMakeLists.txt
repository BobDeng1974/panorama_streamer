cmake_minimum_required(VERSION 3.0)

project(PanoOne)

#
# Find includes in corresponding build directories
#
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_INSTALL_PREFIX C:/Program\ Files/360\ Vision CACHE FILEPATH "" FORCE)

#
# Qt
# 
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5Quick REQUIRED)
#
# QtOpenGL
#
find_package(Qt5OpenGL REQUIRED)

#
# OpenGL
#
find_package(OpenGL REQUIRED)
if(NOT OPENGL_FOUND)
    message(ERROR "OPENGL not found!")
endif(NOT OPENGL_FOUND)

#
# Ximea Camera
#
set(XIMEA_API_DIR ${CMAKE_SOURCE_DIR}/External/Libs/XIMEA CACHE FILEPATH "")
set(XIMEA_LIB_DIR ${XIMEA_API_DIR}/x64 CACHE FILEPATH "")
find_library(XIMEA_LIBS m3apiX64 ${XIMEA_LIB_DIR})
message(${XIMEA_LIB_DIR})

set(CMAKE_CONFIGURATION_TYPES 
	Debug 
	Release CACHE STRING "" FORCE)

#
# ffmpeg
#
set(FFMPEG_API_DIR ${CMAKE_SOURCE_DIR}/External/Libs/ffmpeg CACHE FILEPATH "")
set(FFMPEG_INC_DIR ${FFMPEG_API_DIR}/include  CACHE FILEPATH "")
set(FFMPEG_LIB_DIR ${FFMPEG_API_DIR}/windows/lib/x64 CACHE FILEPATH "")
set(FFMPEG_BIN_DIR ${FFMPEG_API_DIR}/windows/bin/x64 CACHE FILEPATH "")
find_library(FFMPEG_AVCODEC_LIBS avcodec ${FFMPEG_LIB_DIR})
find_library(FFMPEG_AVFORMAT_LIBS avformat ${FFMPEG_LIB_DIR})
find_library(FFMPEG_AVUTIL_LIBS avutil ${FFMPEG_LIB_DIR})
find_library(FFMPEG_AVDEVICE_LIBS avdevice ${FFMPEG_LIB_DIR})
find_library(FFMPEG_SWSCALE_LIBS swscale ${FFMPEG_LIB_DIR})
find_library(FFMPEG_SWRESAMPLE_LIBS swresample ${FFMPEG_LIB_DIR})

#
# GL Extensions
#
set(GLEW_API_DIR ${CMAKE_SOURCE_DIR}/External/Libs/OpenGL CACHE FILEPATH "")
set(GLEW_INC_DIR ${GLEW_API_DIR}/inc CACHE FILEPATH "")
set(GLEW_LIB_DIR ${GLEW_API_DIR}/lib/win/x64 CACHE FILEPATH "")
find_library(GLEW_LIBS glew64 ${GLEW_LIB_DIR})

set(CMAKE_CONFIGURATION_TYPES 
	Debug 
	Release CACHE STRING "" FORCE)

#
# Oculus VR SDK dependency 
#
set(OCULUS_1_8_0 ON)
if(OCULUS_1_8_0)
message("Oculus SDK version 1.8.0")
set(OVR_API_DIR ${CMAKE_SOURCE_DIR}/External/Libs/OculusSDK/1.8.0/LibOVR CACHE FILEPATH "")
set(OVR_INC_DIR ${OVR_API_DIR}/Include CACHE FILEPATH "")
set(OVR_LIB_DIR ${OVR_API_DIR}/Lib/Windows/x64/Release/VS2013 CACHE FILEPATH "")

set(OVRKERNEL_API_DIR ${CMAKE_SOURCE_DIR}/External/Libs/OculusSDK/1.8.0/LibOVRKernel CACHE FILEPATH "")
set(OVRKERNEL_INC_DIR ${OVRKERNEL_API_DIR}/Src CACHE FILEPATH "")
else()
message("Oculus SDK version 0.8.0")
set(OVR_API_DIR ${CMAKE_SOURCE_DIR}/External/Libs/OculusSDK/LibOVR CACHE FILEPATH "")
set(OVR_INC_DIR ${OVR_API_DIR}/Include CACHE FILEPATH "")
set(OVR_LIB_DIR ${OVR_API_DIR}/Lib/Windows/x64/VS2013 CACHE FILEPATH "")
endif()
find_library(OVR_LIBS LibOVR ${OVR_LIB_DIR})

#
# Eigen Library dependency
#
set(EIGEN_INC_DIR ${CMAKE_SOURCE_DIR}/External/Libs/Eigen CACHE FILEPATH "")

#
# More Qt
#
get_target_property(QtCore_location Qt5::Core LOCATION)

#
# Disable some warnings in Windows
#
if(MSVC)
	add_definitions(/DWIN32)
	add_definitions(/MT)
    add_definitions(/W1)
	add_definitions(/D_SCL_SECURE_NO_WARNINGS)
	add_definitions(/D__STDC_LIMIT_MACROS)
	add_definitions(/wd4251)
	add_definitions(/wd4244 /wd4800 /wd4146 /wd4355 /wd4530)
	add_definitions(-DUSE_SHADER_CODE)
	if(OCULUS_1_8_0)
		add_definitions(-DOCULUS_1_8_0)
	endif()
	set(CMAKE_EXE_LINKER_FLAGS
      "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:msvcrt.lib")
	  set(CMAKE_EXE_LINKER_FLAGS
      "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE")
	  set(CMAKE_CXX_FLAGS_DEBUG "/Zi /MTd /Od" CACHE STRING "Flags used by the C++ Debug" FORCE)
	set(CMAKE_CXX_FLAGS_RELEASE "/MT" CACHE STRING "Flags used by C++ Release" FORCE)
endif()

#
# Arrange project files in folders/filters
#
set_property(GLOBAL PROPERTY USE_FOLDERS On) 

# The Qt5Widgets_INCLUDES also includes the include directories for
# dependencies QtCore and QtGui

include_directories(
	${Qt5Widgets_INCLUDES}
	${Qt5Gui_PRIVATE_INCLUDE_DIRS}
	${XIMEA_API_DIR}
	${FFMPEG_INC_DIR}
	${GLEW_INC_DIR}
	${OVR_INC_DIR}
	${OVRKERNEL_INC_DIR}
	${EIGEN_INC_DIR}
   	./ 
	./Include/
	./Capture/
	./Process/
	./Graphics/
	./View/
	./Common/
)

# We need add -DQT_WIDGETS_LIB when using QtWidgets in Qt 5.
add_definitions(${Qt5Widgets_DEFINITIONS} -DENABLE_LOG)

#
# Executables fail to build with Qt 5 in the default configuration
#
set(CMAKE_CXX_FLAGS "${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

# This line includes the current Dir of CMake. The xxx.ui file is
# converted to a ui_xxx.h file by uic.exe. The header ui_xxx.h file
# is located in the build directory. To make this header visible to
# MSVS the build path needs to be included.
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(MAIN_FILES
	./main.cpp
)
set(CAPTURE_H_FILES
	./Capture/AudioThread.h
	./Capture/Capture.h
	./Capture/CameraView.h
	./Capture/CaptureAudio.h
	./Capture/CaptureDevices.h
	./Capture/CaptureDShow.h
	./Capture/CaptureImageFile.h
	./Capture/CaptureThread.h
	./Capture/CaptureXimea.h
	./Capture/VideoCapture.h
	./Capture/AdvThread.h
)
set(CAPTURE_CXX_FILES	
	./Capture/AudioThread.cpp
	./Capture/CameraView.cpp
	./Capture/Capture.cpp
	./Capture/CaptureAudio.cpp
	./Capture/CaptureDevices.cpp
	./Capture/CaptureDShow.cpp
	./Capture/CaptureImageFile.cpp
	./Capture/CaptureThread.cpp
	./Capture/CaptureXimea.cpp
	./Capture/VideoCapture.cpp
	./Capture/AdvThread.cpp
)
set(PROCESS_H_FILES
	./Process/D360Parser.h
	./Process/D360Process.h
	./Process/D360Stitcher.h
	./Process/OculusViewer.h
	./Process/PtsParser.h
	./Process/StreamProcess.h
	./Process/CalibProcess.h
	)
set(PROCESS_CXX_FILES	
	./Process/D360Parser.cpp
	./Process/D360Process.cpp
	./Process/D360Stitcher.cpp
	./Process/OculusViewer.cpp
	./Process/PtsParser.cpp
	./Process/StreamProcess.cpp
	./Process/CalibProcess.cpp
)
set(GRAPHICS_H_FILES
	./Graphics/GLSLProgram.h
	./Graphics/GLSLColorCvt.h
	./Graphics/GLSLGainCompensation.h
	./Graphics/GLSLComposite.h
	./Graphics/GLSLLiveViewer.h
	./Graphics/GLSLUnwarp.h
	./Graphics/GLSLFeathering.h
	./Graphics/GLSLMultibandBlending.h
	./Graphics/GLSLBoundary.h
	./Graphics/GLSLGaussianBlur.h
	./Graphics/GLSLBoxBlur.h
	./Graphics/GLSLLaplacian.h
	./Graphics/GLSLBlending.h
	./Graphics/GLSLResize.h
	./Graphics/GLSLPyramidSum.h
	./Graphics/GLSLFinalPanorama.h
	./Graphics/3DMath.h
	./Graphics/GLSLPanoramaPlacement.h
	./Graphics/GLSLPanoramaInteract.h
)
set(GRAPHICS_CXX_FILES
	./Graphics/GLSLProgram.cpp
	./Graphics/GLSLColorCvt.cpp
	./Graphics/GLSLGainCompensation.cpp
	./Graphics/GLSLComposite.cpp
	./Graphics/GLSLLiveViewer.cpp
	./Graphics/GLSLUnwarp.cpp
	./Graphics/GLSLFeathering.cpp
	./Graphics/GLSLMultibandBlending.cpp
	./Graphics/GLSLBoundary.cpp
	./Graphics/GLSLGaussianBlur.cpp
	./Graphics/GLSLBoxBlur.cpp
	./Graphics/GLSLLaplacian.cpp
	./Graphics/GLSLBlending.cpp
	./Graphics/GLSLResize.cpp
	./Graphics/GLSLPyramidSum.cpp
	./Graphics/GLSLFinalPanorama.cpp
	./Graphics/3DMath.cpp
	./Graphics/GLSLPanoramaPlacement.cpp
	./Graphics/GLSLPanoramaInteract.cpp
)
set(VIEW_H_FILES
	./View/MCQmlCameraView.h
	./View/QmlInteractiveView.h
	./View/QmlMainWindow.h
	./View/QmlRecentDialog.h
)
set(VIEW_CXX_FILES	
	./View/MCQmlCameraView.cpp
	./View/QmlInteractiveView.cpp
	./View/QmlMainWindow.cpp
	./View/QmlRecentDialog.cpp
)
set(COMMON_H_FILES
	./Common/BaseFfmpeg.hpp
	./Common/common.h
	./Common/FfmpegUtils.hpp
#	./Common/GlobalAnimSettings.h
	./Common/PanoQueue.h
	./Common/QTHandler.h
	./Common/SharedImageBuffer.h
	./Common/StreamFfmpeg.hpp
	./Common/TLogger.h
	./Common/PanoLog.h
	./Common/SlotInfo.h
)
set(COMMON_CXX_FILES	
	./Common/BaseFfmpeg.cpp
	./Common/common.cpp
#	./Common/GlobalAnimSettings.cpp
	./Common/PanoQueue.cpp
	./Common/QTHandler.cpp
	./Common/SharedImageBuffer.cpp
	./Common/StreamFfmpeg.cpp
	./Common/TLogger.cpp
	./Common/PanoLog.cpp
	./Common/SlotInfo.cpp
)

#
# QML files
#

set(QML_FILES 
	main.qml
	qml/Header.qml
	qml/Toolbox.qml
	qml/Recent.qml
	qml/RecentGriditem.qml
	qml/RecentListitem.qml
	qml/Status.qml	
)
set(QML_FILES_COMPONENTS
	qml/components/Aboutbox.qml
	qml/components/Anaglyphbox.qml
	qml/components/Exposurebox.qml
	qml/components/Helpbox.qml
	qml/components/Interactivebox.qml
	qml/components/Livebox.qml
	qml/components/Savebox.qml
	qml/components/Settingsbox.qml
	qml/components/Snapshotbox.qml
	qml/components/Sphericalbox.qml
	qml/components/Topbottombox.qml
	qml/components/LiveSettingbox.qml
	qml/components/LiveAudioitem.qml
	qml/components/LiveCamListitem.qml	
	qml/components/VideoSettingbox.qml
	qml/components/VideoListitem.qml
	qml/components/VideoEmptyitem.qml
	qml/components/ImageSettingbox.qml
	qml/components/ImageListitem.qml
	qml/components/Spliter.qml
	qml/components/FloatingWindow.qml
	qml/components/FloatingStitchWindow.qml
	qml/components/MCCameraView.qml
	qml/components/MCStitchCameraView.qml
	qml/components/SphericalView.qml
	qml/components/LiveView.qml
    qml/components/MCVideoWindow.qml
	qml/components/MCStitchVideoWindow.qml
	qml/components/MdiArea.qml
	qml/components/ToolWindowPositions.js
	qml/components/ExposureDialog.qml
	qml/components/ToolWindow.qml
	qml/components/FloatingInteractiveWindow.qml
	qml/components/MCInteractCameraView.qml
	qml/components/MCInteractVideoWindow.qml
	qml/components/CameraPreview.qml
	qml/components/CameraSettingbox.qml
	qml/components/VideoGlobalSettings.qml
	qml/components/LiveTempCamSetting.qml
     	qml/components/ImageTempCamSetting.qml
        qml/components/GroupCtrl.qml
	qml/components/ControlPoint.qml
	qml/components/SeamLabel.qml
	qml/components/Notification.qml
	qml/components/NotificationHistory.qml
	qml/components/NotificationItem.qml
	qml/components/ExposureCtrl.qml
	qml/components/Line.qml
	qml/components/AdvertisementCtrl.qml
)
set(QML_FILES_CONTROLS
	qml/controls/FlatCombo.qml
	qml/controls/FlatText.qml
)

link_directories(${XIMEA_LIB_DIR})

source_group( "Source Files" FILES ${MAIN_FILES})

source_group( "Header Files\\Capture" FILES ${CAPTURE_H_FILES} )
source_group( "Source Files\\Capture" FILES ${CAPTURE_CXX_FILES} )

source_group( "Header Files\\Process" FILES ${PROCESS_H_FILES} )
source_group( "Source Files\\Process" FILES ${PROCESS_CXX_FILES} )

source_group( "Header Files\\Graphics" FILES ${GRAPHICS_H_FILES} )
source_group( "Source Files\\Graphics" FILES ${GRAPHICS_CXX_FILES} )

source_group( "Header Files\\View" FILES ${VIEW_H_FILES} )
source_group( "Source Files\\View" FILES ${VIEW_CXX_FILES} )

source_group( "Header Files\\Common" FILES ${COMMON_H_FILES} )
source_group( "Source Files\\Common" FILES ${COMMON_CXX_FILES} )

source_group( "QML Files" FILES ${QML_FILES} ${QML_FILES_COMPONENTS} ${QML_FILES_CONTROLS} )

#
# This line is necessary to cal uic.exe and creates the ui_xxx.h
# files from the xxx.ui files. The header is created the first time
# the project is compiled
#
#
# Add Qt Resources
#
#qt5_add_resources(UI_RESOURCES_RCC ${CMAKE_SOURCE_DIR}/UI/PanoOne.qrc)
#source_group("Resource Files" FILES ${CMAKE_SOURCE_DIR}/UI/PanoOne.qrc)

qt5_add_resources(UI_RESOURCES_RCC ${CMAKE_SOURCE_DIR}/qml.qrc)
source_group("Resource Files" FILES ${CMAKE_SOURCE_DIR}/qml.qrc)
#cuda_add_executable(PanoOne 
add_executable(PanoOne 
				${MAIN_FILES} 
				${CAPTURE_H_FILES} ${CAPTURE_CXX_FILES} 
				${PROCESS_H_FILES} ${PROCESS_CXX_FILES} 
				${GRAPHICS_H_FILES} ${GRAPHICS_CXX_FILES} 
				${VIEW_H_FILES} ${VIEW_CXX_FILES} 
				${COMMON_H_FILES} ${COMMON_CXX_FILES}
				${QML_FILES} ${QML_FILES_COMPONENTS}  ${QML_FILES_CONTROLS} 
				${FORMS_HEADERS} ${HEADERS_MOC} ${UI_RESOURCES_RCC}
)

target_link_libraries(${PROJECT_NAME} 
	Qt5::Widgets Qt5::OpenGL Qt5::Network Qt5::Qml Qt5::Quick
	${XIMEA_LIBS} 
	${OPENGL_LIBRARIES}
	${FFMPEG_AVCODEC_LIBS}
	${FFMPEG_AVFORMAT_LIBS}
	${FFMPEG_AVUTIL_LIBS}
	${FFMPEG_AVDEVICE_LIBS}
	${FFMPEG_SWSCALE_LIBS}
	${FFMPEG_SWRESAMPLE_LIBS}
	${GLEW_LIBS}
	${OVR_LIBS}
	${WIN_VFW}
	)
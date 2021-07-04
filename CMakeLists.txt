#设置cmake版本号
cmake_minimum_required(VERSION 3.4.0)

#设置工程名称
project(DesktopDynamicWallpaperForQT)

#设置工程包含当前目录，非必须
set(CMAKE_INCLUDE_CURRENT_DIR ON)

#打开全局moc,设置自动生成moc文件，一定要设置
set(CMAKE_AUTOMOC ON)
#打开全局uic，非必须
set(CMAKE_AUTOUIC ON)
#打开全局rcc，非必须，如需打开，注意修改33行的qrc文件名
#set(CMAKE_AUTORCC ON)
add_definitions ( -D_MBCS )

#查找需要的Qt库文件，最好每一个库都要写，Qt也会根据依赖关系自动添加
find_package(Qt5Widgets)
find_package(Qt5Core)
find_package(Qt5Gui)
find_package(Qt5Multimedia)

#查找当前文件夹中的所有源代码文件，也可以通过Set命令将所有文件设置为一个变量
FILE(GLOB SRC_FILES "./src/*.cpp")
#查找设置当前文件夹中所有的头文件
FILE(GLOB HEAD_FILES "./include/*.h")
#查找设置当前文件夹中所有的ui文件
FILE(GLOB UI_FILES "./include/*.ui")

#通过Ui文件生成对应的头文件，一定要添加
qt5_wrap_ui(WRAP_FILES ${UI_FILES})

#添加资源文件，非必须，一旦采用，注意修改相应的qrc文件名
#set(RCC_FILES rcc.qrc)

#将ui文件和生成文件整理在一个文件夹中，非必须
source_group("Ui" FILES ${UI_FILES} ${WRAP_FILES} )
#include
include_directories(
"./ffmpeg/include"
)
#lib
link_directories(
"./ffmpeg/lib"
)
#创建工程文件
add_executable(${PROJECT_NAME} WIN32 ${SRC_FILES} ${HEAD_FILES}  ${WRAP_FILES})


#添加Qt5依赖项
target_link_libraries(${PROJECT_NAME} 
Qt5::Widgets 
Qt5::Core 
Qt5::Gui
Qt5::Multimedia
avutil avcodec avfilter  avdevice avformat postproc swresample swscale
)
#copy dll
FILE(GLOB LIB_FILES "./ffmpeg/bin/*.*")

# copy dll
file(COPY ${LIB_FILES} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/out/build/x64-Release)
file(COPY ${LIB_FILES} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/out/build/x64-Debug)
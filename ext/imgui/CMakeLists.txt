cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project ("ImGUI")


set(IMGUI_SOURCES )

# OpenGL Glew files
set(IMGUI_SRC
	"imconfig.h"
	"imgui.cpp"
	"imgui.h"
	"imgui_demo.cpp"
	"imgui_draw.cpp"
	"imgui_impl_glfw.cpp"
	"imgui_impl_glfw.h"
	"imgui_impl_opengl3.cpp"
	"imgui_impl_opengl3.h"
	"imgui_impl_opengl3_loader.h"
	"imgui_internal.h"
	"imgui_tables.cpp"
	"imgui_widgets.cpp"
	"imstb_rectpack.h"
	"imstb_textedit.h"
	"imstb_truetype.h"
	"misc/cpp/imgui_stdlib.h"
	"misc/cpp/imgui_stdlib.cpp"
)
source_group("ImGUI" FILES ${IMGUI_SRC})
list(APPEND IMGUI_SOURCES ${IMGUI_SRC})

add_library(ImGUI STATIC ${IMGUI_SOURCES})
target_link_libraries(ImGUI glfw)
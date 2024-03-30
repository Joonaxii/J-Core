cmake_minimum_required (VERSION 2.4...3.8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_WARN_DEPRECATED OFF)
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)

project ("J-Core")

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_WARNINGS OFF CACHE BOOL "" FORCE)

set(JCORE_GUI_SOURCES )

add_subdirectory("ext/glm")
add_subdirectory("ext/imgui")
add_subdirectory("ext/spdlog")
add_subdirectory("ext/json")
add_subdirectory("ext/zlib")
add_subdirectory("ext/glfw-3.3.8")

add_compile_definitions(GLEW_STATIC)
IF (WIN32)
	add_compile_definitions(WIN32_LEAN_AND_MEAN)
	add_compile_definitions(NOMINMAX)
ENDIF(WIN32)

target_compile_options(
    spdlog PRIVATE
    /external:W0
)

# OpenGL Glew files
set(GLEW_SRC
	"ext/include/GL/wglew.h"
	"ext/include/GL/glew.h"
	"ext/src/glew.c"
)
source_group("J-Core/OpenGL/Glew" FILES ${GLEW_SRC})
list(APPEND JCORE_GUI_SOURCES ${GLEW_SRC})

set(JCORE_MAIN_SRC
	"include/J-Core/EntryPoint.h"
	"include/J-Core/Application.h"
	"src/J-Core/Application.cpp"
	
	"include/J-Core/ThreadManager.h"
	"src/J-Core/ThreadManager.cpp"
	
	"include/J-Core/Log.h"
	"src/J-Core/Log.cpp"
)
source_group("J-Core" FILES ${JCORE_MAIN_SRC})
list(APPEND JCORE_GUI_SOURCES ${JCORE_MAIN_SRC})

set(JCORE_MAIN_SRC
	"include/J-Core/Rendering/Window.h"
	"src/J-Core/Rendering/Window.cpp"
	
	"include/J-Core/Rendering/SpritePacking.h"
	"include/J-Core/Rendering/Atlas.h"
	"src/J-Core/Rendering/Atlas.cpp"
	
	"include/J-Core/Rendering/Texture.h"
	"src/J-Core/Rendering/Texture.cpp"
	
	"include/J-Core/Rendering/DynamicBatch.h"
	
	"include/J-Core/Rendering/Sprite.h"
	"src/J-Core/Rendering/Sprite.cpp"
	
	"include/J-Core/Rendering/Animation.h"
	"src/J-Core/Rendering/Animation.cpp"
	
	"include/J-Core/Rendering/Renderer.h"
	"src/J-Core/Rendering/Renderer.cpp"
	
	"include/J-Core/Rendering/SortingLayer.h"
	"include/J-Core/Rendering/Renderable.h"
	"src/J-Core/Rendering/Renderable.cpp"
	"include/J-Core/Rendering/ICamera.h"	
	
	"include/J-Core/Rendering/Shader.h"
	"src/J-Core/Rendering/Shader.cpp"
)
source_group("J-Core/Rendering" FILES ${JCORE_MAIN_SRC})
list(APPEND JCORE_GUI_SOURCES ${JCORE_MAIN_SRC})

set(JCORE_MAIN_SRC
	"include/J-Core/Rendering/Buffers/BufferLayout.h"
	"include/J-Core/Rendering/Buffers/IndexBuffer.h"
	"src/J-Core/Rendering/Buffers/IndexBuffer.cpp"
	"include/J-Core/Rendering/Buffers/VertexArray.h"
	"src/J-Core/Rendering/Buffers/VertexArray.cpp"
	"include/J-Core/Rendering/Buffers/VertexBuffer.h"
	"src/J-Core/Rendering/Buffers/VertexBuffer.cpp"
	
	"include/J-Core/Rendering/Buffers/FrameBuffer.h"
	"src/J-Core/Rendering/Buffers/FrameBuffer.cpp"
	
	"include/J-Core/Rendering/Buffers/FrameBufferPool.h"
	"src/J-Core/Rendering/Buffers/FrameBufferPool.cpp"
)
source_group("J-Core/Rendering/Buffers" FILES ${JCORE_MAIN_SRC})
list(APPEND JCORE_GUI_SOURCES ${JCORE_MAIN_SRC})

set(JCORE_MAIN_SRC
	"include/J-Core/IO/Stream.h"
	
	"include/J-Core/IO/FileStream.h"
	"src/J-Core/IO/FileStream.cpp"
	
	"include/J-Core/IO/MemoryStream.h"
	"src/J-Core/IO/MemoryStream.cpp"
	
	"include/J-Core/IO/BitStream.h"
	"src/J-Core/IO/BitStream.cpp"
	
	"include/J-Core/IO/Image.h"
	"src/J-Core/IO/Image.cpp"
	
	"include/J-Core/IO/ImageUtils.h"
	"src/J-Core/IO/ImageUtils.cpp"
	
	"include/J-Core/IO/Audio.h"
	"src/J-Core/IO/Audio.cpp"
	
	"include/J-Core/IO/AudioUtils.h"
	"src/J-Core/IO/AudioUtils.cpp"
	
	"include/J-Core/IO/IOUtils.h"
	"src/J-Core/IO/IOUtils.cpp"
	
	"include/J-Core/IO/ZLib.h"
	"src/J-Core/IO/ZLib.cpp"
	
	"include/J-Core/IO/Base64.h"
)
source_group("J-Core/IO" FILES ${JCORE_MAIN_SRC})
list(APPEND JCORE_GUI_SOURCES ${JCORE_MAIN_SRC})

set(JCORE_MAIN_SRC
	"include/J-Core/Util/EnumUtils.h"
	"include/J-Core/Util/DataUtils.h"
	"include/J-Core/Util/IComparable.h"
	"include/J-Core/Util/Span.h"
	"include/J-Core/Util/HexStr.h"
	
	"src/J-Core/Util/StringUtils.cpp"
	"include/J-Core/Util/StringUtils.h"
	
	"src/J-Core/Util/Bitset.cpp"
	"include/J-Core/Util/Bitset.h"
	
	"include/J-Core/Util/PoolAllocator.h"
	"include/J-Core/Util/AlignmentAllocator.h"
	"include/J-Core/Util/Stack.h"
	
	"src/J-Core/Util/DataFormatUtils.cpp"
	"include/J-Core/Util/DataFormatUtils.h"
)
source_group("J-Core/Util" FILES ${JCORE_MAIN_SRC})
list(APPEND JCORE_GUI_SOURCES ${JCORE_MAIN_SRC})

set(JCORE_MAIN_SRC
	"include/J-Core/Gui/ImGui.h"
	"include/J-Core/Gui/IGuiDrawable.h"
	"include/J-Core/Gui/IGuiExtras.h"
	"src/J-Core/Gui/IGuiExtras.cpp"
)
source_group("J-Core/Gui" FILES ${JCORE_MAIN_SRC})
list(APPEND JCORE_GUI_SOURCES ${JCORE_MAIN_SRC})

set(JCORE_MAIN_SRC
	"include/J-Core/Math/Color32.h"
	"src/J-Core/Math/Color32.cpp"
	
	"include/J-Core/Math/Color24.h"
	"src/J-Core/Math/Color24.cpp"
	
	"include/J-Core/Math/Color555.h"
	"src/J-Core/Math/Color555.cpp"
	
	"include/J-Core/Math/Color565.h"
	"src/J-Core/Math/Color565.cpp"
	
	"include/J-Core/Math/Color4444.h"
	"src/J-Core/Math/Color4444.cpp"
	
	"include/J-Core/Math/Rect.h"
	"include/J-Core/Math/Vertex.h"
	
	"include/J-Core/Math/Matrix4f.h"
	"src/J-Core/Math/Matrix4f.cpp"
	
	"include/J-Core/Math/Math.h"
	"src/J-Core/Math/Math.cpp"
)
source_group("J-Core/Math" FILES ${JCORE_MAIN_SRC})
list(APPEND JCORE_GUI_SOURCES ${JCORE_MAIN_SRC})

find_package(OpenGL REQUIRED)
add_library(J-Core STATIC ${JCORE_GUI_SOURCES})
target_link_libraries(J-Core OpenGL::GL glfw ImGUI glm zlibstatic spdlog nlohmann_json)


target_compile_options(
    J-Core PUBLIC
    /external:W0
)

target_include_directories(J-Core PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/ext/zlib)

include_directories("include")
include_directories("ext/zlib/")
include_directories("ext/glm/glm")
include_directories("ext/imgui")
include_directories("ext/json/include")
include_directories("ext/include")
include_directories("ext/spdlog/include")

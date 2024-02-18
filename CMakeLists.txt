cmake_minimum_required(VERSION 3.10)

project(PSXMV VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ./out)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(glfw-3.3.9)

# assimp
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "Assimp build tests" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "Assimp install" FORCE)
set(ASSIMP_INSTALL_PDB OFF CACHE BOOL "Assimp install PDB" FORCE)

add_subdirectory(assimp-master)

add_executable(PSXMV 
src/main.cpp
src/mesh.cpp
src/model.cpp
src/imgui_widgets.cpp
src/imgui_tables.cpp
src/imgui_stdlib.cpp
src/imgui_impl_opengl3.cpp
src/imgui_impl_glfw.cpp
src/imgui_draw.cpp
src/imgui_demo.cpp
src/imgui.cpp
src/glad.c
)


target_include_directories(PSXMV PRIVATE
				"${PROJECT_SOURCE_DIR}/inc" 
				"${PROJECT_SOURCE_DIR}/inc/glad"
				"${PROJECT_SOURCE_DIR}/inc/glfw"
				"${PROJECT_SOURCE_DIR}/inc/glm"
				"${PROJECT_SOURCE_DIR}/inc/KHR"
				"${PROJECT_SOURCE_DIR}/inc/stbi"
				"${PROJECT_SOURCE_DIR}/inc/imgui"
				"${PROJECT_SOURCE_DIR}/inc/assimp"
				"${PROJECT_SOURCE_DIR}/assimp-master/build/include"
 )

target_link_libraries(PSXMV glfw)
target_link_libraries(PSXMV assimp)


#puts our models n such into build
add_custom_target(copy_resources ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/out/resources
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources
        COMMENT "Copying resources into build directory")

add_dependencies(${PROJECT_NAME} copy_resources)




 

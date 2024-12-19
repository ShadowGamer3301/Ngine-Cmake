file(GLOB src "source/*.cxx")
add_library(NgineCore SHARED ${src})

#Windows build settings
if(TARGET_PLATFORM_WINDOWS)
	#Mark project as DLL file
	target_compile_definitions(NgineCore PRIVATE "_WINDLL")
	target_link_libraries(NgineCore PRIVATE "glfw3.lib" "d3d11.lib" "dxgi.lib" "d3dcompiler.lib" Loguru)
endif()

target_include_directories(NgineCore PUBLIC "include" "../loguru")
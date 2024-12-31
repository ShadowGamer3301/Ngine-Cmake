file(GLOB src "source/*.cxx")
add_library(NgineCore SHARED ${src})

#Windows build settings
if(TARGET_PLATFORM_WINDOWS)
	#Mark project as DLL file
	target_compile_definitions(NgineCore PRIVATE "_WINDLL")
	target_link_libraries(NgineCore PRIVATE "glfw3.lib" "d3d11.lib" "dxgi.lib" "d3dcompiler.lib" Loguru)
elseif(TARGET_PLATFORM_LINUX)
	target_link_libraries(NgineCore PRIVATE "libvulkan.so" "libglfw.so" Loguru)
elseif(TARGET_PLATFORM_XBOX)
	target_compile_definitions(NgineCore PRIVATE "_WINDLL")
	target_link_libraries(NgineCore PRIVATE Loguru "dxgi.lib" "d3d11.lib" "d3dcompiler.lib" "User32.lib")
endif()

target_include_directories(NgineCore PUBLIC "include" "../loguru")
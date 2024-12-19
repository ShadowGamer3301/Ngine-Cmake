file(GLOB src "source/*.cxx")
add_executable(NgineRuntime ${src})

#Windows build settings
if(TARGET_PLATFORM_WINDOWS)
	target_link_libraries(NgineRuntime PRIVATE NgineCore)
endif()

target_include_directories(NgineCore PRIVATE "../NgineCore/include")
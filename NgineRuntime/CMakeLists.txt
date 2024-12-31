file(GLOB src "source/*.cxx")

add_executable(NgineRuntime ${src})

target_link_libraries(NgineRuntime PRIVATE NgineCore Loguru)
target_include_directories(NgineRuntime PRIVATE "../NgineCore/include" "../loguru" "../tga")

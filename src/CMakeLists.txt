add_executable(halcyonc
    halcyon.cpp
    parse.cpp
    syntax.cpp
    symbol.cpp
    arena.cpp)
target_compile_features(halcyonc PRIVATE cxx_std_23)

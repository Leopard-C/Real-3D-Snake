-- 3D snake

target("3d_snake")
    set_kind("binary")

    -- std=C+=11
    set_languages("c99", "cxx11")

    -- include
    add_includedirs("src/server/jsoncpp")

    -- source file
    add_files("src/*/*.cpp")
    add_files("src/server/jsoncpp/*.cpp")
    add_files("src/server/http-parser/*.c")
    add_files("src/server/multipart-parser-c/*.c")
    add_files("src/*.cpp")

    -- build dir
    set_objectdir("build/objs")
    set_targetdir(".")

    -- link flags
    add_links("pthread", "wiringPi", "asound")

    add_mflags("-O3")
    


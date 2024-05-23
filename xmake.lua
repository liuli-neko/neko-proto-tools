set_project("cc-proto")
add_rules("mode.debug", "mode.release", "mode.valgrind")
set_version("1.0.0")

add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})


add_requires("rapidjson", "spdlog")

if is_mode("debug") then
    add_defines("CS_CCPROTO_LOG_CONTEXT")
end 

if is_plat("linux") then
    add_cxxflags("-fcoroutines")
end

target("test_co")
    set_languages("c++20")
    set_kind("binary")
    add_packages("spdlog")
    add_files("tests/test_co.cpp")
target_end()

target("rpc_base")
    set_kind("static")
    add_defines("CS_RPC_STATIC")
    add_defines("CS_PROTO_STATIC")
    add_defines("ILIAS_COROUTINE_LIFETIME_CHECK")
    add_includedirs("./modules/Ilias/include")
    set_languages("c++20")
    add_packages("spdlog")
    add_files("src/cc_proto_base.cpp")
    add_files("src/cc_rpc_base.cpp")
target_end()

add_requires("gtest")
target("test_proto")
    set_kind("binary")
    add_defines("CS_PROTO_STATIC")
    set_languages("c++17")
    add_packages("rapidjson", "gtest", "spdlog")
    add_tests("proto")
    add_files("src/cc_proto_base.cpp")
    add_files("tests/test_proto.cpp")
target_end()

target("test_proto1")
    set_kind("binary")
    add_defines("CS_PROTO_STATIC")
    set_languages("c++20")
    add_packages("rapidjson", "gtest", "spdlog")
    add_tests("proto")
    set_group("serializer")
    add_files("src/cc_proto_base.cpp")
    add_files("tests/test_proto1.cpp")
target_end()

target("test_rpc")
    set_kind("binary")
    set_languages("c++20")
    add_includedirs("./modules/Ilias/include")
    add_defines("CS_PROTO_STATIC", "CS_RPC_STATIC")
    add_defines("ILIAS_COROUTINE_LIFETIME_CHECK")
    add_cxxflags("-Wstrict-aliasing")
    add_packages("rapidjson", "gtest", "spdlog")
    add_deps("rpc_base")
    add_tests("proto")
    set_group("rpc")
    add_files("tests/test_rpc.cpp")
target_end()

target("test_dump")
    set_kind("binary")
    set_languages("c++20")
    add_includedirs("./modules/Ilias/include")
    add_defines("CS_PROTO_STATIC", "CS_RPC_STATIC")
    add_defines("ILIAS_COROUTINE_LIFETIME_CHECK")
    add_cxxflags("-Wstrict-aliasing")
    add_packages("rapidjson", "gtest", "spdlog")
    add_deps("rpc_base")
    add_tests("proto")
    set_group("so")
    add_files("tests/test_dump_object.cpp")
target_end()

-- target("test_rpc_ui")
--     add_rules("qt.widgetapp")
--     set_languages("c++20")
--     add_includedirs("./modules/Ilias/include")
--     add_files("tests/test_rpc_ui.cpp")    
--     add_files("src/cc_proto_base.cpp")
--     add_files("src/cc_rpc_base.cpp")
--     add_defines("CS_PROTO_STATIC", "CS_RPC_STATIC")
--     add_defines("ILIAS_COROUTINE_LIFETIME_CHECK")
--     add_packages("rapidjson", "spdlog")
-- target_end()


add_languages("cxx20")
add_requires("cpp-httplib", {configs = {ssl = true}})
add_requires("nlohmann_json")
add_requires("onetbb")
--add_requires("vcpkg::mysql-connector-cpp", {alias = "mysql-connector"})
--add_requires("ormpp", {configs = {mysql = true}})
--(//.*?$)|(/\*[\s\S]*?\*/)

target("BackEnd")
    set_kind("binary")
    set_encodings("utf-8")

    add_includedirs("backend")
    add_files("backend/**.cpp")

    add_packages("cpp-httplib","nlohmann_json","onetbb")

target_end()
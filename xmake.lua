add_languages("cxx20")
add_requires("cpp-httplib", {configs = {ssl = true}})
add_requires("nlohmann_json")
add_requires("ormpp")

target("BackEnd")
    set_kind("binary")

    add_includedirs("backend")
    add_files("backend/**.cpp")

    add_packages("cpp-httplib","nlohmann_json","ormpp")

target_end()
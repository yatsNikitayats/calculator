# Каждый плагин собирается в отдельную dll. MODULE значит, что dll грузится
# только во время работы, а calc.exe от неё не зависит.
set(CALC_PLUGINS funcsin funccos funcln funcsqrt funcdeg funcmax)

foreach(plugin IN LISTS CALC_PLUGINS)
    add_library(${plugin} MODULE ${plugin}.cpp)
    target_link_libraries(${plugin} PRIVATE calc_plugin_api)
    set_target_properties(${plugin} PROPERTIES
        PREFIX ""                                  # funcsin.dll, а не libfuncsin.dll (MinGW)
        LIBRARY_OUTPUT_DIRECTORY ${CALC_OUTPUT_DIR}/plugins)
    calc_set_warnings(${plugin})
endforeach()

# Отдаём список плагинов наверх, корневому CMakeLists.txt, для add_dependencies.
set(CALC_PLUGINS ${CALC_PLUGINS} PARENT_SCOPE)

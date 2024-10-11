cmake_minimum_required(VERSION 3.5)
idf_component_register(
                    SRCS softap_example_main.c tcp_server.c uart1_driver.c
                    INCLUDE_DIRS ".")
component_compile_options(-fsigned-char)
set_property(TARGET ${COMPONENT_TARGET} PROPERTY C_STANDARD 99)
set_property(TARGET ${COMPONENT_TARGET} PROPERTY CXX_STANDARD)
set_source_files_properties(uart1_driver.c tcp_server.c softap_example_main.c PROPERTIES HEADER_FILE_ONLY TRUE)

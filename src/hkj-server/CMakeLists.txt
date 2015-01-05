project(HKJ-FTP)

include_directories(../../include)

aux_source_directory(. SRC_LIST)

message("server: {SRC_LIST}")
add_executable(hkj-server ${SRC_LIST})

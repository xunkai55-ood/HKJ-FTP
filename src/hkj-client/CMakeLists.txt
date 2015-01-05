project(HKJ-FTP)

include_directories(../../include)

aux_source_directory(. SRC_LIST)

message("client: {SRC_LIST}")
add_executable(hkj-client ${SRC_LIST})
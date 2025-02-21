
# 将源码加载到DIR_SRCS
aux_source_directory(. DIR_SRCS)

# 生成库文件，默认静态库
# 中间加"SHARED"为动态库
add_library(dir1 STATIC ${DIR_SRCS})
# 声明该组件的源文件（.c 文件）
file(GLOB SRC_FILES "multi_button.c")  # 包含所选的 .c 文件


# 配置头文件目录
set(INCLUDE_DIRS ".")								# 当前目录下的头文件
# set(INCLUDE_DIRS "include")						# include 文件夹下的头文件
# set(INCLUDE_DIRS "." "${CMAKE_SOURCE_DIR}/main")  # 当前目录 + 上级 main 目录下的头文件


# 配置组件的依赖（idf库）
set(PRIV_REQUIRES "driver")



# 注册组件
idf_component_register(	SRCS ${SRC_FILES}					# 源文件列表
						INCLUDE_DIRS ${INCLUDE_DIRS}		# 头文件目录
						PRIV_REQUIRES ${PRIV_REQUIRES}		# 该组件的依赖
)

####### MCU #######

MCU                 ?= STC8G1K17
MCU_IRAM            ?= 256
# 1024, 2048, 3072, 4096, 8192
MCU_XRAM            ?= 1024
# 8192, 16384, 32768, 65536
MCU_CODE_SIZE       ?= 16384

##### Project #####

PROJECT             ?= app
# The path for generated files
BUILD_DIR           = build

TOOCHAIN_PREFIX     ?= sdcc-

# C source folders
USER_CDIRS          := user
# C source single files
USER_CFILES         := 
USER_INCLUDES       := user


LIB_FLAGS           := __CONF_MCU_MODEL=MCU_MODEL_STC8G1K17

include ./rules.mk

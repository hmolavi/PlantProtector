idf_component_register(SRCS "wifi.c" 
                            "internet_check.c"
                            "app_main.c"
                    INCLUDE_DIRS "." 
                                 "../components/cmd_nvs" 
                                 "../components/cmd_system" 
                                 "../components/vfs"
                                 ../components/fatfs
                    REQUIRES console nvs_flash "fatfs")


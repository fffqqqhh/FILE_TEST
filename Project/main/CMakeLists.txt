idf_component_register(SRCS "main.c"
							"user_uart.c" "user_adc.c" "user_i2c.c"
							"user_lvgl.c" "user_gui.c" "user_icons.c"
                            "user_oper_data.c"
							"user_letter_shell.c"
							"user_button.c"
							# "user_SW3516.c"
							"user_SW3566.c"
							"user_IP2730.c" "user_HUSB238A.c" 
                            "user_new_source.c"
                            # "user_source.c"
							
						INCLUDE_DIRS ".")

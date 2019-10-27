set(COMPONENT_SRCS "hello_world_main.c"
                   "gpio_discharge.c"
                   "gpio_A1.c"
                   "gpio_A2.c"
                   "gpio_B1.c"
                   "gpio_B2.c"
                   "gpio_B3.c"
                   "spi_slave_peri.c"
                   "uart_lmm.c"
                   "spi_master.c"
                   "cmd_controller.c"
                   "system_param.c"
                   "i2c_server.c")
set(COMPONENT_ADD_INCLUDEDIRS "")

register_component()
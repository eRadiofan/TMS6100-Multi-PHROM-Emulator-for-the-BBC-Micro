This project uses a STM32G051F8P6 MCU but the code could be migrated to a new project configured for STM32G031F8P6 or STM32C051F8P6 using the same pin names, pin configuration and clock configuration.

Once a new project is generated with STM32CubeMX, you can delete main.c and stm32g0xx_it.c since the code for these sections now resides elsewhere.

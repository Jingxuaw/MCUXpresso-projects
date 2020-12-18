LPC15xx I2CM bus master example using polling

Example description:
--------------------
This example shows how to configure I2C as a bus master in polling mode using
the I2CM driver.
This example uses 7-bit addressing to periodically read temperature data from a temperature
sensor on the Motor Control board. After I2C is setup, the I2CM master receive and transmit
functions are called through the i2cm_15xx driver routines. The temperature data is outputted 
through the Debug UART port.

Special connection requirements:
--------------------------------
The Motor Control board is required with the LPCXpresso board to use this
example.

Build procedures:
-----------------
Visit the  LPCOpen quickstart guides to get started building LPCOpen projects.
[Link: http://www.lpcware.com/content/project/lpcopen-platform-nxp-lpc-microcontrollers/lpcopen-v200-quickstart-guides]


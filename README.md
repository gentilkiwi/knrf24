A little project I made to drive some `NRF24` (up to 4), via SPI.  
The code is minimalist and is only able to saturate Bluetooth data channels (2402 - 2480 MHz), but can be adapted to whatever your need, code it!  
It uses a Raspberry Pi Pico as MCU to drive `NRF24` module(s).


## Hardware

I made a board for the lulz: https://oshwlab.com/gentilkiwi/knrf24  
But you have to add NRF24 modules (1 is ~OK, 2 is OK, 4 is better).

### NRF24 modules

Ideally, you would like to use official `NRF24` modules, search for: 'NRF24L01+ PA' (LNA not mandatory).  
Counterfeits based on `Si24R1` are OK.


## Firmware

Firmware is built by GitHub Action (see artefacts)

```
> picotool info --all knrf24.uf2

Program Information
 name:              knrf24
 version:           0.1
 web site:          https://github.com/gentilkiwi/knrf24
 description:       knrf24 test on Raspberry Pi Pico
 binary start:      0x10000000
 binary end:        0x10002360

Fixed Pin Information
 0:                 #0 CE
 1:                 #0 CS
 2:                 SPI0 SCK
 3:                 SPI0 TX
 4:                 SPI0 RX
 5:                 #0 IRQ
 6:                 #0 LED
 7:                 #1 LED
 8:                 #1 CE
 9:                 #1 CS
 10:                #1 IRQ
 11:                #2 CE
 12:                #2 CS
 13:                #2 IRQ
 14:                #2 LED
 15:                #3 LED
 16:                #3 IRQ
 17:                #3 CS
 18:                #3 CE
 19:                User LED 3
 20:                User LED 2
 21:                User LED 1
 22:                User LED 0
 26:                User switch

Build Information
 sdk version:       2.2.0
 pico_board:        pico
 boot2_name:        boot2_w25q080
 build date:        Jan  4 2026
 build attributes:  Release
 ```


## Hardware references

- `NRF24L01` - https://www.nordicsemi.com/Products/nRF24-series
- `Si24R1` - http://www.csm-ic.com/html/ProductView.asp?ID=249


## Author

Benjamin DELPY 🥝 gentilkiwi, you can contact me on Twitter ( @gentilkiwi ) or by mail ( benjamin [at] gentilkiwi.com )  
This is a personal development, please respect its philosophy and don't use it for bad things!


## License

CC BY 4.0 licence - https://creativecommons.org/licenses/by/4.0/

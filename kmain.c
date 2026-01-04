#include <stdio.h>
#include "pico/stdlib.h"
#include "knrf24.h"

#define SPI_PORT    spi0
#define SPI_SPEED   (4 * 1000 * 1000)
#define PIN_SCK     2
#define PIN_MOSI    3
#define PIN_MISO    4

#define PIN_ULED0   22
#define PIN_ULED1   21
#define PIN_ULED2   20
#define PIN_ULED3   19

#define PIN_SW1     26

const uint USER_LEDS[] = {PIN_ULED0, PIN_ULED1, PIN_ULED2, PIN_ULED3,};

KNRF24_DEVICE KNRF24_DEVICES[] = {
    {.comm = {.spi = SPI_PORT, .cs =  1 , .ce =  0, .irq =  5}, .led =  6, .isPresent = false},
    {.comm = {.spi = SPI_PORT, .cs =  9 , .ce =  8, .irq = 10}, .led =  7, .isPresent = false},
    {.comm = {.spi = SPI_PORT, .cs = 12 , .ce = 11, .irq = 13}, .led = 14, .isPresent = false},
    {.comm = {.spi = SPI_PORT, .cs = 17 , .ce = 18, .irq = 16}, .led = 15, .isPresent = false},
};

const uint8_t NRF_BROADCAST_ADDR[5] = {0xff, 0xff, 0xff, 0xff, 0xff};
const uint8_t NRF_PAYLOAD[32] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

void InitNRF(KNRF24_DEVICE *pDevice, uint *sleepAmount)
{
    knrf24_Write_SingleRegister(pDevice, KNRF24_REG_CONFIG, KNRF24_REG_CONFIG_PWR_UP | KNRF24_REG_CONFIG_MASK_RX_DR | KNRF24_REG_CONFIG_MASK_TX_DS | KNRF24_REG_CONFIG_MASK_MAX_RT);
    if(knrf24_Read_SingleRegister(pDevice, KNRF24_REG_CONFIG) == (KNRF24_REG_CONFIG_PWR_UP | KNRF24_REG_CONFIG_MASK_RX_DR | KNRF24_REG_CONFIG_MASK_TX_DS | KNRF24_REG_CONFIG_MASK_MAX_RT))
    {
        pDevice->isPresent = true;
        knrf24_led_on(pDevice);
        *sleepAmount -= 5;

        busy_wait_us_32(1500);

        knrf24_Write_SingleRegister(pDevice, KNRF24_REG_EN_AA, 0);
        knrf24_Write_SingleRegister(pDevice, KNRF24_REG_SETUP_RETR, (0 << KNRF24_REG_SETUP_RETR_ARD_shift) | (0 << KNRF24_REG_SETUP_RETR_ARC_shift));
        knrf24_Write_SingleRegister(pDevice, KNRF24_REG_RF_SETUP, KNRF24_REG_RF_SETUP_PLL_LOCK | KNRF24_REG_RF_SETUP_RF_DR_2Mbps | KNRF24_REG_RF_SETUP_RF_PWR_0dbm);

        knrf24_Write_MultipleRegister(pDevice, KNRF24_REG_TX_ADDR, NRF_BROADCAST_ADDR, sizeof(NRF_BROADCAST_ADDR));
        knrf24_Write_TX_Payload(pDevice, NRF_PAYLOAD, sizeof(NRF_PAYLOAD));
        knrf24_chip_enable(pDevice);

        busy_wait_us_32(10);
        knrf24_chip_disable(pDevice);    

        busy_wait_ms(1);
    }
}

#define SLEEP_US_BASE  25

static const uint8_t Bluetooth_2[] = {
    /*0,  */2,  4,  6,  8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62, 64, 66, 68, 70, 72, 74, 76, 78, 80,
    79, 77, 75, 73, 71, 69, 67, 65, 63, 61, 59, 57, 55, 53, 51, 49, 47, 45, 43, 41, 39, 37, 35, 33, 31, 29, 27, 25, 23, 21, 19, 17, 15, 13, 11,  9,  7,  5,  3,/*  1,*/
};

int main()
{
    uint i, s = SLEEP_US_BASE, c = 0;

    stdio_init_all();

    gpio_init(PIN_SW1);
    gpio_pull_up(PIN_SW1);
    gpio_set_irq_enabled(PIN_SW1, GPIO_IRQ_EDGE_FALL, true);

    for(i = 0; i < count_of(USER_LEDS); i++)
    {
        gpio_init(USER_LEDS[i]);
        gpio_set_dir(USER_LEDS[i], GPIO_OUT);
        gpio_put(USER_LEDS[i], false);
    }

    knrf24_init_global_spi(SPI_PORT, SPI_SPEED, PIN_SCK, PIN_MOSI, PIN_MISO);
    
    for(i = 0; i < count_of(KNRF24_DEVICES); i++)
    {
        knrf24_init_device(KNRF24_DEVICES + i);
    }
    
    for(i = 0; i < count_of(KNRF24_DEVICES); i++)
    {
        InitNRF(KNRF24_DEVICES + i, &s);
    }

    while (true)
    {
        for(i = 0; i < count_of(KNRF24_DEVICES); i++)
        {
            if(KNRF24_DEVICES[i].isPresent)
            {
                knrf24_Write_SingleRegister(KNRF24_DEVICES + i, KNRF24_REG_RF_CH, Bluetooth_2[c]);
                knrf24_DirectCommand(KNRF24_DEVICES + i, KNRF24_INS_REUSE_TX_PL);
                knrf24_chip_enable(KNRF24_DEVICES + i);

                if (++c == sizeof(Bluetooth_2))
                {
                    c = 0;
                }
            }
        }
        
        busy_wait_us_32(s);

        for(i = 0; i < count_of(KNRF24_DEVICES); i++)
        {
            if(KNRF24_DEVICES[i].isPresent)
            {
                knrf24_chip_disable(KNRF24_DEVICES + i);
            }
        }
    }

    return 0;
}

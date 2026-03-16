#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "knrf24.h"

#define SPI_PORT    spi0
#define SPI_SPEED   (4 * 1000 * 1000)
#define PIN_SCK     2
#define PIN_MOSI    3
#define PIN_MISO    4
// NRF24 device #0
#define PIN_DEVICE_0_CS     1
#define PIN_DEVICE_0_CE     0
#define PIN_DEVICE_0_IRQ    5
#define PIN_DEVICE_0_LED    6
// NRF24 device #1
#define PIN_DEVICE_1_CS     9
#define PIN_DEVICE_1_CE     8
#define PIN_DEVICE_1_IRQ    10
#define PIN_DEVICE_1_LED    7
// NRF24 device #2
#define PIN_DEVICE_2_CS     12
#define PIN_DEVICE_2_CE     11
#define PIN_DEVICE_2_IRQ    13
#define PIN_DEVICE_2_LED    14
// NRF24 device #3
#define PIN_DEVICE_3_CS     17
#define PIN_DEVICE_3_CE     18
#define PIN_DEVICE_3_IRQ    16
#define PIN_DEVICE_3_LED    15
// User leds
#define PIN_ULED_0          22
#define PIN_ULED_1          21
#define PIN_ULED_2          20
#define PIN_ULED_3          19
// User switch
#define PIN_SW1     26

const uint USER_LEDS[] = {PIN_ULED_0, PIN_ULED_1, PIN_ULED_2, PIN_ULED_3,};

KNRF24_DEVICE KNRF24_DEVICES[] = {
    {.comm = {.spi = SPI_PORT, .cs = PIN_DEVICE_0_CS , .ce = PIN_DEVICE_0_CE, .irq = PIN_DEVICE_0_IRQ}, .led = PIN_DEVICE_0_LED, .isPresent = false},
    {.comm = {.spi = SPI_PORT, .cs = PIN_DEVICE_1_CS , .ce = PIN_DEVICE_1_CE, .irq = PIN_DEVICE_1_IRQ}, .led = PIN_DEVICE_1_LED, .isPresent = false},
    {.comm = {.spi = SPI_PORT, .cs = PIN_DEVICE_2_CS , .ce = PIN_DEVICE_2_CE, .irq = PIN_DEVICE_2_IRQ}, .led = PIN_DEVICE_2_LED, .isPresent = false},
    {.comm = {.spi = SPI_PORT, .cs = PIN_DEVICE_3_CS , .ce = PIN_DEVICE_3_CE, .irq = PIN_DEVICE_3_IRQ}, .led = PIN_DEVICE_3_LED, .isPresent = false},
};

bi_decl(bi_3pins_with_func(PIN_SCK, PIN_MOSI, PIN_MISO, GPIO_FUNC_SPI));
bi_decl(bi_4pins_with_names(PIN_DEVICE_0_CS, "#0 CS", PIN_DEVICE_0_CE, "#0 CE", PIN_DEVICE_0_IRQ, "#0 IRQ", PIN_DEVICE_0_LED, "#0 LED"));
bi_decl(bi_4pins_with_names(PIN_DEVICE_1_CS, "#1 CS", PIN_DEVICE_1_CE, "#1 CE", PIN_DEVICE_1_IRQ, "#1 IRQ", PIN_DEVICE_1_LED, "#1 LED"));
bi_decl(bi_4pins_with_names(PIN_DEVICE_2_CS, "#2 CS", PIN_DEVICE_2_CE, "#2 CE", PIN_DEVICE_2_IRQ, "#2 IRQ", PIN_DEVICE_2_LED, "#2 LED"));
bi_decl(bi_4pins_with_names(PIN_DEVICE_3_CS, "#3 CS", PIN_DEVICE_3_CE, "#3 CE", PIN_DEVICE_3_IRQ, "#3 IRQ", PIN_DEVICE_3_LED, "#3 LED"));
bi_decl(bi_4pins_with_names(PIN_ULED_0, "User LED 0", PIN_ULED_1, "User LED 1", PIN_ULED_2, "User LED 2", PIN_ULED_3, "User LED 3"));
bi_decl(bi_1pin_with_name(PIN_SW1, "User switch"));

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

static const uint8_t Bluetooth[] = {
    /*0,  */2,  4,  6,  8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62, 64, 66, 68, 70, 72, 74, 76, 78, 80,
    79, 77, 75, 73, 71, 69, 67, 65, 63, 61, 59, 57, 55, 53, 51, 49, 47, 45, 43, 41, 39, 37, 35, 33, 31, 29, 27, 25, 23, 21, 19, 17, 15, 13, 11,  9,  7,  5,  3,/*  1,*/
};
static const uint8_t Unifying[] = {/*0, */5, 8, 11, 14, 17, 20, 23, 26, 29, 32, 35, 38, 41, 44, 47, 50, 53, 56, 59, 62, 65, 68, 71, 74}; // 0 can be here to keep internal Unifying indexes

volatile bool g_irq_SW1;
void __isr __time_critical_func(IRQ_Callback)(uint gpio, uint32_t event_mask)
{
    if((gpio == PIN_SW1) && (event_mask & GPIO_IRQ_EDGE_FALL))
    {
        g_irq_SW1 = true;
    }
}

int main()
{
    uint i, s = SLEEP_US_BASE, c = 0;

    stdio_init_all();

    gpio_init(PIN_SW1);
    gpio_pull_up(PIN_SW1);

    //gpio_set_irq_enabled(PIN_SW1, GPIO_IRQ_EDGE_FALL, true);
    //gpio_set_irq_callback(IRQ_Callback);
    //irq_set_enabled(IO_IRQ_BANK0, true);

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
                knrf24_Write_SingleRegister(KNRF24_DEVICES + i, KNRF24_REG_RF_CH, Bluetooth[c]);
                knrf24_DirectCommand(KNRF24_DEVICES + i, KNRF24_INS_REUSE_TX_PL);
                knrf24_chip_enable(KNRF24_DEVICES + i);

                if (++c == sizeof(Bluetooth))
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

#pragma once
#include <stdio.h>
#include "hardware/spi.h"
#include "hardware/gpio.h"

#define KNRF24_INS_NOP              0b11111111

#define KNRF24_INS_R_REGISTER_ins   0b00000000
#define KNRF24_INS_R_REGISTER_mask  0b00011111

#define KNRF24_INS_W_REGISTER_ins   0b00100000
#define KNRF24_INS_W_REGISTER_mask  0b00011111

#define KNRF24_INS_R_RX_PAYLOAD     0b01100001
#define KNRF24_INS_W_TX_PAYLOAD     0b10100000

#define KNRF24_INS_REUSE_TX_PL      0b11100011

#define KNRF24_REG_CONFIG                   (0x00)
#define KNRF24_REG_EN_AA                    (0x01)
#define KNRF24_REG_EN_RXADDR                (0x02)
#define KNRF24_REG_SETUP_AW                 (0x03)
#define KNRF24_REG_SETUP_RETR               (0x04)
#define KNRF24_REG_RF_CH                    (0x05)
#define KNRF24_REG_RF_SETUP                 (0x06)
#define KNRF24_REG_STATUS                   (0x07)
#define KNRF24_REG_OBSERVE_TX               (0x08)
#define KNRF24_REG_CD                       (0x09)

#define KNRF24_REG_RX_ADDR_P0               (0x0a)
#define KNRF24_REG_RX_ADDR_P1               (0x0b)
#define KNRF24_REG_RX_ADDR_P2               (0x0c)
#define KNRF24_REG_RX_ADDR_P3               (0x0d)
#define KNRF24_REG_RX_ADDR_P4               (0x0e)
#define KNRF24_REG_RX_ADDR_P5               (0x0f)
#define KNRF24_REG_TX_ADDR                  (0x10)
#define KNRF24_REG_RX_PW_P0                 (0x11)
#define KNRF24_REG_RX_PW_P1                 (0x12)
#define KNRF24_REG_RX_PW_P2                 (0x13)
#define KNRF24_REG_RX_PW_P3                 (0x14)
#define KNRF24_REG_RX_PW_P4                 (0x15)
#define KNRF24_REG_RX_PW_P5                 (0x16)
#define KNRF24_REG_FIFO_STATUS              (0x17)



#define KNRF24_REG_CONFIG_MASK_RX_DR        (1 << 6)
#define KNRF24_REG_CONFIG_MASK_TX_DS        (1 << 5)
#define KNRF24_REG_CONFIG_MASK_MAX_RT       (1 << 4)
#define KNRF24_REG_CONFIG_EN_CRC            (1 << 3)
#define KNRF24_REG_CONFIG_CO                (1 << 2)
#define KNRF24_REG_CONFIG_PWR_UP            (1 << 1)
#define KNRF24_REG_CONFIG_PRIM_RX           (1 << 0)

#define KNRF24_REG_SETUP_RETR_ARD_shift     (4)
#define KNRF24_REG_SETUP_RETR_ARC_shift     (0)

#define KNRF24_REG_RF_SETUP_PLL_LOCK        (1 << 4)
#define KNRF24_REG_RF_SETUP_RF_DR_1Mbps     (0 << 3)
#define KNRF24_REG_RF_SETUP_RF_DR_2Mbps     (1 << 3)
#define KNRF24_REG_RF_SETUP_RF_PWR_18dbm    (0b00 << 1)
#define KNRF24_REG_RF_SETUP_RF_PWR_12dbm    (0b01 << 1)
#define KNRF24_REG_RF_SETUP_RF_PWR_6dbm     (0b10 << 1)
#define KNRF24_REG_RF_SETUP_RF_PWR_0dbm     (0b11 << 1)
#define KNRF24_REG_RF_SETUP_LNA_HCURR       (1 << 0)

typedef struct _KNRF24_DEVICE_COMM_PINS {
    spi_inst_t *spi;
    uint cs;
    uint ce;
    uint irq;
} KNRF24_DEVICE_COMM_PINS, *PKNRF24_DEVICE_COMM_PINS;

typedef struct _KNRF24_DEVICE {
    const KNRF24_DEVICE_COMM_PINS comm;
    const uint led;
    bool isPresent;
} KNRF24_DEVICE, *PKNRF24_DEVICE;

#define knrf24_spi_acquire(pDevice)                                     gpio_put((pDevice)->comm.cs, false);
#define knrf24_spi_release(pDevice)                                     gpio_put((pDevice)->comm.cs, true);

#define knrf24_chip_enable(pDevice)                                     gpio_put((pDevice)->comm.ce, true);
#define knrf24_chip_disable(pDevice)                                    gpio_put((pDevice)->comm.ce, false);

#define knrf24_led_on(pDevice)                                          gpio_put((pDevice)->led, true);
#define knrf24_led_off(pDevice)                                         gpio_put((pDevice)->led, false);

#define knrf24_GetStatus(pDevice)                                       knrf24_DirectCommand(pDevice, KNRF24_INS_NOP)
#define knrf24_Read_SingleRegister(pDevice, Register)                   knrf24_Read_SingleRegister_internal(pDevice, KNRF24_INS_R_REGISTER_ins | (Register & KNRF24_INS_R_REGISTER_mask))
#define knrf24_Write_SingleRegister(pDevice, Register, Value)           knrf24_Write_SingleRegister_internal(pDevice, KNRF24_INS_W_REGISTER_ins | (Register & KNRF24_INS_W_REGISTER_mask), Value)

#define knrf24_Write_MultipleRegister(pDevice, Register, Value, Size)   knrf24_Write_MultipleRegister_internal(pDevice, KNRF24_INS_W_REGISTER_ins | (Register & KNRF24_INS_W_REGISTER_mask), Value, Size)
#define knrf24_Write_TX_Payload(pDevice, Value, Size)                   knrf24_Write_MultipleRegister_internal(pDevice, KNRF24_INS_W_TX_PAYLOAD, Value, Size)

void knrf24_init_global_spi(spi_inst_t *spi, uint baudrate, uint sck, uint mosi, uint miso);
void knrf24_init_device(KNRF24_DEVICE *pDevice);

uint8_t knrf24_DirectCommand(const KNRF24_DEVICE *pDevice, const uint8_t Command);

uint8_t knrf24_Read_SingleRegister_internal(const KNRF24_DEVICE *pDevice, const uint8_t Register_Prepared);
void knrf24_Write_SingleRegister_internal(const KNRF24_DEVICE *pDevice, const uint8_t Register_Prepared, const uint8_t Value);
void knrf24_Write_MultipleRegister_internal(const KNRF24_DEVICE *pDevice, const uint8_t Register_Prepared, const uint8_t *pValue, const uint8_t size);

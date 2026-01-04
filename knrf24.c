#include "knrf24.h"

void knrf24_init_global_spi(spi_inst_t *spi, uint baudrate, uint sck, uint mosi, uint miso)
{
    spi_init(spi, baudrate);
    spi_set_format(spi, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(sck,  GPIO_FUNC_SPI);
    gpio_set_function(mosi, GPIO_FUNC_SPI);
    gpio_set_function(miso, GPIO_FUNC_SPI);
}

void knrf24_init_device_gpio(const KNRF24_DEVICE *pDevice)
{
    gpio_init(pDevice->comm.cs);
    gpio_set_dir(pDevice->comm.cs, GPIO_OUT);
    knrf24_spi_release(pDevice);

    gpio_init(pDevice->comm.ce);
    gpio_set_dir(pDevice->comm.ce, GPIO_OUT);
    knrf24_chip_disable(pDevice);

    gpio_init(pDevice->comm.irq);
    gpio_set_irq_enabled(pDevice->comm.irq, GPIO_IRQ_EDGE_FALL, true);

    gpio_init(pDevice->led);
    gpio_set_dir(pDevice->led, GPIO_OUT);
    knrf24_led_off(pDevice);
}

void knrf24_init_device(KNRF24_DEVICE *pDevice)
{
    pDevice->isPresent = false;
    knrf24_init_device_gpio(pDevice);
}

uint8_t knrf24_DirectCommand(const KNRF24_DEVICE *pDevice, const uint8_t Command)
{
    uint8_t ret;

    knrf24_spi_acquire(pDevice);
    spi_write_read_blocking(pDevice->comm.spi, &Command, &ret, sizeof(ret));
    knrf24_spi_release(pDevice);

    return ret;
}

uint8_t knrf24_Read_SingleRegister_internal(const KNRF24_DEVICE *pDevice, const uint8_t Register_Prepared)
{
    uint8_t buffer[2] = {Register_Prepared, KNRF24_INS_NOP};

    knrf24_spi_acquire(pDevice);
    spi_write_read_blocking(pDevice->comm.spi, buffer, buffer, sizeof(buffer));
    knrf24_spi_release(pDevice);

    return buffer[1];
}

void knrf24_Write_SingleRegister_internal(const KNRF24_DEVICE *pDevice, const uint8_t Register_Prepared, const uint8_t Value)
{
    uint8_t buffer[2] = {Register_Prepared, Value};

    knrf24_spi_acquire(pDevice);
    spi_write_blocking(pDevice->comm.spi, buffer, sizeof(buffer));
    knrf24_spi_release(pDevice);
}

void knrf24_Write_MultipleRegister_internal(const KNRF24_DEVICE *pDevice, const uint8_t Register_Prepared, const uint8_t *pValue, const uint8_t size)
{
    knrf24_spi_acquire(pDevice);
    spi_write_blocking(pDevice->comm.spi, &Register_Prepared, sizeof(Register_Prepared));
    spi_write_blocking(pDevice->comm.spi, pValue, size);
    knrf24_spi_release(pDevice);
}
#include "hal/hitl/hal_usb_cdc.h"
#include "hal/hitl/usb/usbd_conf.h"
#include "hal/hitl/usb/usbd_desc.h"
#include "hal/hitl/usb/usbd_cdc_if.h"
#include "usbd_core.h"
#include "usbd_cdc.h"

/* Exposed as extern so usbd_cdc_if.c can call USBD_CDC_SetRxBuffer etc. */
USBD_HandleTypeDef hUsbDeviceFS;

static volatile uint8_t  s_rx_buf[512];
static volatile uint16_t s_rx_head;
static volatile uint16_t s_rx_tail;

void hal_usb_cdc_init(void)
{
    USBD_Init(&hUsbDeviceFS, &CDC_Desc, DEVICE_FS);
    USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC);
    USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS);
    USBD_Start(&hUsbDeviceFS);
}

int hal_usb_cdc_send(const uint8_t *buf, size_t len)
{
    USBD_CDC_HandleTypeDef *hcdc = hUsbDeviceFS.pClassData;
    if (hcdc == NULL || hcdc->TxState != 0U)
        return -1;
    USBD_CDC_SetTxBuffer(&hUsbDeviceFS, (uint8_t *)buf, (uint16_t)len);
    return (USBD_CDC_TransmitPacket(&hUsbDeviceFS) == USBD_OK) ? (int)len : -1;
}

int hal_usb_cdc_recv(uint8_t *buf, size_t len)
{
    size_t n = 0;
    while (n < len && s_rx_head != s_rx_tail) {
        buf[n++] = s_rx_buf[s_rx_tail];
        s_rx_tail = (s_rx_tail + 1U) % sizeof(s_rx_buf);
    }
    return (int)n;
}

void hal_usb_cdc_on_rx(const uint8_t *buf, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++) {
        uint16_t next = (s_rx_head + 1U) % (uint16_t)sizeof(s_rx_buf);
        if (next != s_rx_tail) {
            s_rx_buf[s_rx_head] = buf[i];
            s_rx_head = next;
        }
    }
}

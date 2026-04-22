#include "hal/hitl/usb/usbd_cdc_if.h"
#include "hal/hitl/hal_usb_cdc.h"
#include "usbd_cdc.h"

#define APP_RX_DATA_SIZE  512U

static uint8_t s_rx_buf[APP_RX_DATA_SIZE];

/* hUsbDeviceFS is owned by hal_usb_cdc.c */
extern USBD_HandleTypeDef hUsbDeviceFS;

static int8_t CDC_Init_FS(void)
{
    USBD_CDC_SetRxBuffer(&hUsbDeviceFS, s_rx_buf);
    return USBD_OK;
}

static int8_t CDC_DeInit_FS(void)
{
    return USBD_OK;
}

static int8_t CDC_Control_FS(uint8_t cmd, uint8_t *pbuf, uint16_t length)
{
    (void)cmd;
    (void)pbuf;
    (void)length;
    return USBD_OK;
}

static int8_t CDC_Receive_FS(uint8_t *Buf, uint32_t *Len)
{
    hal_usb_cdc_on_rx(Buf, (uint16_t)*Len);
    USBD_CDC_SetRxBuffer(&hUsbDeviceFS, s_rx_buf);
    USBD_CDC_ReceivePacket(&hUsbDeviceFS);
    return USBD_OK;
}

static int8_t CDC_TransmitCplt_FS(uint8_t *Buf, uint32_t *Len, uint8_t epnum)
{
    (void)Buf;
    (void)Len;
    (void)epnum;
    return USBD_OK;
}

USBD_CDC_ItfTypeDef USBD_Interface_fops_FS = {
    CDC_Init_FS,
    CDC_DeInit_FS,
    CDC_Control_FS,
    CDC_Receive_FS,
    CDC_TransmitCplt_FS,
};

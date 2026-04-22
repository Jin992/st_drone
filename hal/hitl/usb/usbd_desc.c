#include "hal/hitl/usb/usbd_desc.h"
#include "hal/hitl/usb/usbd_conf.h"
#include "usbd_def.h"

#define USBD_VID              0x0483U   /* ST Microelectronics */
#define USBD_PID              0x5740U   /* CDC VCP */
#define USBD_LANGID           0x0409U   /* English */
#define USBD_MANUFACTURER     "STMicro"
#define USBD_PRODUCT          "STM32 FC HITL"
#define USBD_SERIALNUMBER     "000000000001"

static uint8_t s_desc_buf[USBD_MAX_STR_DESC_SIZ];

/* Build a USB string descriptor from a plain ASCII string. */
static uint8_t *make_str_desc(const char *str, uint16_t *length)
{
    uint16_t n = 0;
    const char *p = str;
    while (*p++) n++;
    s_desc_buf[0] = (uint8_t)(2U + 2U * n);
    s_desc_buf[1] = USB_DESC_TYPE_STRING;
    for (uint16_t i = 0; i < n; i++) {
        s_desc_buf[2U + 2U * i]      = (uint8_t)str[i];
        s_desc_buf[2U + 2U * i + 1U] = 0x00U;
    }
    *length = s_desc_buf[0];
    return s_desc_buf;
}

/* ── Descriptor callbacks ────────────────────────────────────────────────── */

static uint8_t s_device_desc[18] = {
    18,                     /* bLength */
    USB_DESC_TYPE_DEVICE,   /* bDescriptorType */
    0x00, 0x02,             /* bcdUSB = 2.00 */
    0x02,                   /* bDeviceClass: CDC */
    0x00,                   /* bDeviceSubClass */
    0x00,                   /* bDeviceProtocol */
    0x40,                   /* bMaxPacketSize0 = 64 */
    (uint8_t)(USBD_VID),
    (uint8_t)(USBD_VID >> 8),
    (uint8_t)(USBD_PID),
    (uint8_t)(USBD_PID >> 8),
    0x00, 0x02,             /* bcdDevice = 2.00 */
    0x01,                   /* iManufacturer */
    0x02,                   /* iProduct */
    0x03,                   /* iSerialNumber */
    0x01,                   /* bNumConfigurations */
};

static uint8_t *GetDeviceDesc(USBD_SpeedTypeDef speed, uint16_t *length)
{
    (void)speed;
    *length = sizeof(s_device_desc);
    return s_device_desc;
}

static uint8_t s_langid_desc[4] = {
    4, USB_DESC_TYPE_STRING,
    (uint8_t)(USBD_LANGID), (uint8_t)(USBD_LANGID >> 8),
};

static uint8_t *GetLangIDDesc(USBD_SpeedTypeDef speed, uint16_t *length)
{
    (void)speed;
    *length = sizeof(s_langid_desc);
    return s_langid_desc;
}

static uint8_t *GetManufacturerDesc(USBD_SpeedTypeDef speed, uint16_t *length)
{
    (void)speed;
    return make_str_desc(USBD_MANUFACTURER, length);
}

static uint8_t *GetProductDesc(USBD_SpeedTypeDef speed, uint16_t *length)
{
    (void)speed;
    return make_str_desc(USBD_PRODUCT, length);
}

static uint8_t *GetSerialDesc(USBD_SpeedTypeDef speed, uint16_t *length)
{
    (void)speed;
    return make_str_desc(USBD_SERIALNUMBER, length);
}

static uint8_t *GetConfigDesc(USBD_SpeedTypeDef speed, uint16_t *length)
{
    (void)speed;
    return make_str_desc("CDC Config", length);
}

static uint8_t *GetInterfaceDesc(USBD_SpeedTypeDef speed, uint16_t *length)
{
    (void)speed;
    return make_str_desc("CDC Interface", length);
}

USBD_DescriptorsTypeDef CDC_Desc = {
    GetDeviceDesc,
    GetLangIDDesc,
    GetManufacturerDesc,
    GetProductDesc,
    GetSerialDesc,
    GetConfigDesc,
    GetInterfaceDesc,
};

/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _USB_HOST_CONFIG_H_
#define _USB_HOST_CONFIG_H_

/* Host Controller Enable */
/*!
 * @brief The host KHCI instance count,  which indicates whether the KHCI is enabled or disabled.
 *        - If 0, host KHCI driver is disable.
 *        - If greater than 0, host KHCI driver is enable.
 */
#define USB_HOST_CONFIG_KHCI (1U)

/*!
 * @brief The host EHCI instance count, which indicates whether the EHCI is enabled or disabled.
 *        - If 0, host EHCI driver is disable.
 *        - If greater than 0, host EHCI driver is enable.
 */
#define USB_HOST_CONFIG_EHCI (1U)

/*!
 * @brief host ohci instance count, meantime it indicates ohci enable or disable.
 *        - if 0, host ohci driver is disable.
 *        - if greater than 0, host ohci driver is enable.
 */
#define USB_HOST_CONFIG_OHCI (0U)

/*!
 * @brief host ip3516hs instance count, meantime it indicates ohci enable or disable.
 *        - if 0, host ip3516hs driver is disable.
 *        - if greater than 0, host ip3516hs driver is enable.
 */
#define USB_HOST_CONFIG_IP3516HS (0U)

/* Common configuration macros for all controllers */

/*!
 * @brief The host driver instance maximum count.
 * For example: 2 - one for KHCI, one for EHCI.
 */
#define USB_HOST_CONFIG_MAX_HOST \
    (USB_HOST_CONFIG_KHCI + USB_HOST_CONFIG_EHCI + USB_HOST_CONFIG_OHCI + USB_HOST_CONFIG_IP3516HS)

/*!
 * @brief The host pipe maximum count.
 * The pipe is the host driver resource for device endpoint; One endpoint needs one pipe.
 */
#define USB_HOST_CONFIG_MAX_PIPES (16U)

/*!
 * @brief The host transfer maximum count.
 * The transfer is the host driver resource for data transmission mission; One transmission mission need one transfer.
 */
#define USB_HOST_CONFIG_MAX_TRANSFERS (16U)

/*!
 * @brief The maximum endpoint for one interface.
 * The maximum end pointer descriptor number that one interface descriptor contains.
 */
#define USB_HOST_CONFIG_INTERFACE_MAX_EP (4U)

/*!
 * @brief The maximum interface for one configuration.
 * The maximum interface descriptor number that one configuration descriptor can contain.
 */
#define USB_HOST_CONFIG_CONFIGURATION_MAX_INTERFACE (5U)

/*!
 * @brief The maximum power for one device.
 * The maximum power the host can provide for one device.
 */
#define USB_HOST_CONFIG_MAX_POWER (250U)

/*!
 * @brief The maximum retries for enumeration.
 * Retry time when the enumeration fails.
 */
#define USB_HOST_CONFIG_ENUMERATION_MAX_RETRIES (3U)

/*!
 * @brief The maximum retries for enumeration setup stall.
 * The maximum times for one transfer can stall.
 */
#define USB_HOST_CONFIG_ENUMERATION_MAX_STALL_RETRIES (1U)

/*!
 * @brief The maximum NAK count for one transaction.
 * When NAK count reach to the value, the transaction fail.
 */
#define USB_HOST_CONFIG_MAX_NAK (3000U)

/*! @brief Whether the transfer buffer is cache-enabled or not. */
#define USB_HOST_CONFIG_BUFFER_PROPERTY_CACHEABLE (0U)

/*! @brief If 1, enable USB compliance test codes. If 0, disable the USB compliance test codes. */
#define USB_HOST_CONFIG_COMPLIANCE_TEST (0U)

/*! @brief if 1, class driver clear stall automatically; if 0, class driver don't clear stall. */
#define USB_HOST_CONFIG_CLASS_AUTO_CLEAR_STALL (1U)

/*! @brief if 1, enable the low power; if 0, disbale the low power. */
#define USB_HOST_CONFIG_LOW_POWER_MODE (1U)

/* KHCI configuration */
#if ((defined USB_HOST_CONFIG_KHCI) && (USB_HOST_CONFIG_KHCI))

/*!
 * @brief KHCI DMA align fix buffer size.
 */
#define USB_HOST_CONFIG_KHCI_DMA_ALIGN_BUFFER (64U)

#endif

/* EHCI configuration */
#if ((defined USB_HOST_CONFIG_EHCI) && (USB_HOST_CONFIG_EHCI))

/*!
 * @brief EHCI periodic frame list size.
 * The value can be 1024, 512, 256, 128, 64, 32, 16, or 8.
 */
#define USB_HOST_CONFIG_EHCI_FRAME_LIST_SIZE (1024U)

/*!
 * @brief EHCI QH maximum count.
 */
#define USB_HOST_CONFIG_EHCI_MAX_QH (8U)

/*!
 * @brief EHCI QTD maximum count.
 */
#define USB_HOST_CONFIG_EHCI_MAX_QTD (8U)

/*!
 * @brief EHCI ITD maximum count.
 */
#define USB_HOST_CONFIG_EHCI_MAX_ITD (16U)

/*!
 * @brief EHCI SITD maximum count.
 */
#define USB_HOST_CONFIG_EHCI_MAX_SITD (16U)

#endif

/*!
 * @brief The host HUB class instance count, which indicates whether the HUB class is enabled or disabled.
 *        - If 0, host HUB class driver is disable.
 *        - If greater than 0, host HUB class driver is enable.
 */
#define USB_HOST_CONFIG_HUB (1U)

/*!
 * @brief The host HID class instance count, which indicates whether the HID class is enabled or disabled.
 *        - If 0, host HID class driver is disable.
 *        - If greater than 0, host HID class driver is enable.
 */
#define USB_HOST_CONFIG_HID (1U)
/*!
 * @brief The host MSD class instance count, which indicates whether the MSD class is enabled or disabled.
 *        - If 0, host MSD class driver is disable.
 *        - If greater than 0, host MSD class driver is enable.
 */
#define USB_HOST_CONFIG_MSD (1U)

/*!
 * @brief The host CDC class instance count, which indicates whether the CDC class is enabled or disabled.
 *        - If 0, host CDC class driver is disable.
 *        - If greater than 0, host CDC class driver is enable.
 */
#define USB_HOST_CONFIG_CDC (1U)

#define USB_HOST_CONFIG_CDC_RNDIS (1U)
/*!
 * @brief The host Audio class instance count, which indicates whether the Audio class is enabled or disabled.
 *        - If 0, host AUDIO class driver is disable.
 *        - If greater than 0, host AUDIO class driver is enable.
 */
#define USB_HOST_CONFIG_AUDIO (1U)

/*!
 * @brief The host PHDC class instance count, which indicates whether the PHDC class is enabled or disabled.
 *        - If 0, host PHDC class driver is disable.
 *        - If greater than 0, host PHDC class driver is enable.
 */
#define USB_HOST_CONFIG_PHDC (1U)

/*!
 * @brief host printer class instance count, meantime it indicates printer class enable or disable.
 *        - if 0, host printer class driver is disable.
 *        - if greater than 0, host printer class driver is enable.
 */
#define USB_HOST_CONFIG_PRINTER (1U)
#define USB_HOST_CONFIG_VIDEO (1U)
#endif /* _USB_HOST_CONFIG_H_ */
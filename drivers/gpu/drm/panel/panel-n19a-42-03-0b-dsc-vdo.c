// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2023 MediaTek Inc.
 */

#include <linux/backlight.h>
#include <linux/delay.h>
#include <drm/drm_panel.h>
#include <drm/drm_mipi_dsi.h>
#include <drm/drm_connector.h>
#include <drm/drm_device.h>
#include <drm/drm_modes.h>

#include <linux/gpio/consumer.h>
#include <linux/regulator/consumer.h>

#include <video/mipi_display.h>
#include <video/of_videomode.h>
#include <video/videomode.h>

#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/of_graph.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
/*N19A code for HQ-358314 by p-xielihui at 2023/12/20 start*/
#include <linux/hqsysfs.h>
/*N19A code for HQ-358314 by p-xielihui at 2023/12/20 end*/

/*N19A code for HQ-348461 by p-xielihui at 2024/1/11 start*/
#include <uapi/drm/mi_disp.h>
/*N19A code for HQ-348461 by p-xielihui at 2024/1/11 end*/

/*N19A code for HQ-348450 by p-xielihui at 2024/2/4 start*/
#include "../mediatek/mediatek_v2/mi_notifier_odm.h"
/*N19A code for HQ-348450 by p-xielihui at 2024/2/4 end*/

#define CONFIG_MTK_PANEL_EXT
#if defined(CONFIG_MTK_PANEL_EXT)
#include "../mediatek/mediatek_v2/mtk_panel_ext.h"
#include "../mediatek/mediatek_v2/mtk_drm_graphics_base.h"
#endif

#define REGFLAG_DELAY           0xFFFC
#define REGFLAG_UDELAY          0xFFFB
#define REGFLAG_END_OF_TABLE    0xFFFD
#define REGFLAG_RESET_LOW       0xFFFE
#define REGFLAG_RESET_HIGH      0xFFFF

#define FRAME_WIDTH                 1080
#define FRAME_HEIGHT                2460

#define PHYSICAL_WIDTH              69336
#define PHYSICAL_HEIGHT             157932

/*N19A code for HQ-369993 by p-luocong1 at 2024/1/31 start*/
#define DATA_RATE                   764
#define HSA                         16
#define HBP                         14
#define HFP                         172
#define VSA                         10
#define VBP                         20

/*N19A code for HQ-348462 by p-luocong1 at 2024/1/3 start*/
/*Parameter setting for mode 0 Start*/
#define MODE_0_FPS                  60
#define MODE_0_VFP                  1275
/*Parameter setting for mode 0 End*/

/*Parameter setting for mode 1 Start*/
#define MODE_1_FPS                  90
/*N19A code for HQ-364252 by p-luocong1 at 2024/1/6 start*/
#define MODE_1_VFP                  18
/*N19A code for HQ-364252 by p-luocong1 at 2024/1/6 end*/
/*Parameter setting for mode 1 End*/
/*N19A code for HQ-348462 by p-luocong1 at 2024/1/3 end*/

/*N19A code for HQ-366831 by p-luocong1 at 2024/1/15 start*/
/*Parameter setting for mode 2 Start*/
#define MODE_2_FPS                  48
#define MODE_2_VFP                  2215
/*Parameter setting for mode 2 End*/

/*Parameter setting for mode 3 Start*/
#define MODE_3_FPS                  36
#define MODE_3_VFP                  3765
/*Parameter setting for mode 3 End*/
/*N19A code for HQ-366831 by p-luocong1 at 2024/1/15 end*/
/*N19A code for HQ-369993 by p-luocong1 at 2024/1/31 end*/

/*N19A code for HQ-348461 by p-xielihui at 2024/1/11 start*/
#define CABC_CTRL_REG               0x55
/*N19A code for HQ-348461 by p-xielihui at 2024/1/11 end*/

/*N19A code for HQ-348394 by p-xielihui at 2024/1/20 start*/
#define HBM_MAP_MAX_BRIGHTNESS      4095
#define NORMAL_MAX_BRIGHTNESS       2047
#define REG_MAX_BRIGHTNESS          2047
#define REG_HBM_BRIGHTNESS          1735
#define REG_NORMAL_BRIGHTNESS       1420
/*N19A code for HQ-348394 by p-xielihui at 2024/1/20 end*/

/*N19A code for HQ-369142 by p-xielihui at 2024/1/29 start*/
static u32 panel_id = 0xFF;
/*N19A code for HQ-369142 by p-xielihui at 2024/1/29 end*/
/*N19A code for HQ-372911 by p-luocong1 at 2024/3/4 start*/
/*N19A code for HQ- by p-xielihui at 2024/5/7 start*/
/*4096 is a value that will not be used*/
static unsigned int last_bl = 4096;
/*N19A code for HQ- by p-xielihui at 2024/5/7 end*/
static bool need_enable_dimming_flag = false;

#define OPEN_DIMMING                0x2C
#define CLOSE_DIMMING               0x24
/*N19A code for HQ-372911 by p-luocong1 at 2024/3/4 end*/

#define LFR_EN                      1
/* DSC RELATED */

#define DSC_ENABLE                  1
#define DSC_VER                     17
#define DSC_SLICE_MODE              1
#define DSC_RGB_SWAP                0
#define DSC_DSC_CFG                 34
#define DSC_RCT_ON                  1
#define DSC_BIT_PER_CHANNEL         8
#define DSC_DSC_LINE_BUF_DEPTH      9
#define DSC_BP_ENABLE               1
#define DSC_BIT_PER_PIXEL           128
#define DSC_PIC_HEIGHT              2460
#define DSC_PIC_WIDTH               1080
#define DSC_SLICE_HEIGHT            20
#define DSC_SLICE_WIDTH             540
#define DSC_CHUNK_SIZE              540
#define DSC_XMIT_DELAY              512
#define DSC_DEC_DELAY               526
#define DSC_SCALE_VALUE             32
#define DSC_INCREMENT_INTERVAL      488
#define DSC_DECREMENT_INTERVAL      7
#define DSC_LINE_BPG_OFFSET         12
#define DSC_NFL_BPG_OFFSET          1294
#define DSC_SLICE_BPG_OFFSET        1302
#define DSC_INITIAL_OFFSET          6144
#define DSC_FINAL_OFFSET            7072
#define DSC_FLATNESS_MINQP          3
#define DSC_FLATNESS_MAXQP          12
#define DSC_RC_MODEL_SIZE           8192
#define DSC_RC_EDGE_FACTOR          6
#define DSC_RC_QUANT_INCR_LIMIT0    11
#define DSC_RC_QUANT_INCR_LIMIT1    11
#define DSC_RC_TGT_OFFSET_HI        3
#define DSC_RC_TGT_OFFSET_LO        3

struct panel {
	struct device *dev;
	struct drm_panel panel;
	struct backlight_device *backlight;
	struct gpio_desc *reset_gpio;
	struct gpio_desc *avee_gpio;
	struct gpio_desc *avdd_gpio;
	bool prepared;
	bool enabled;
	struct mutex panel_lock;
	const char *panel_info;

	/*N19A code for HQ-348461 by p-xielihui at 2024/1/11 start*/
	int cabc_status;
	/*N19A code for HQ-348461 by p-xielihui at 2024/1/11 end*/

	int error;
};

/*N19A code for HQ-348462 by p-luocong1 at 2024/1/3 start*/
static int current_fps = 60;
/*N19A code for HQ-348462 by p-luocong1 at 2024/1/3 end*/
static u32 bdg_support;
/*N19A code for HQ-353621 by p-xielihui at 2023/12/26 start*/
static const char *panel_name = "panel_name=dsi_n19a_42_03_0b_dsc_vdo";
static struct drm_panel *this_panel = NULL;
/*N19A code for HQ-353621 by p-xielihui at 2023/12/26 end*/
/* N19A code for HQ-348470 by p-xielihui at 2024/1/2 start */
static atomic_t gesture_status = ATOMIC_INIT(0);
/* N19A code for HQ-348470 by p-xielihui at 2024/1/2 end */
/* N19A code for HQ-364908 by p-huangyunbiao at 2024/1/8 start */
static atomic_t proximity_status = ATOMIC_INIT(0);
/* N19A code for HQ-364908 by p-huangyunbiao at 2024/1/8 end */

/*N19A code for HQ-348450 by p-xielihui at 2024/2/4 start*/
struct drm_notifier_data g_notify_data1;
static int blank;
/*N19A code for HQ-348450 by p-xielihui at 2024/2/4 end*/

/* N19A code for HQ-369421 by p-huangyunbiao at 2024/2/2 start */
static bool panel_check_proximity_mode(void);
/* N19A code for HQ-369421 by p-huangyunbiao at 2024/2/2 end */

#define lcm_dcs_write_seq(ctx, seq...)                                     \
	({                                                                     \
		const u8 d[] = {seq};                                          \
		BUILD_BUG_ON_MSG(ARRAY_SIZE(d) > 64,                           \
				 "DCS sequence too big for stack");            \
		lcm_dcs_write(ctx, d, ARRAY_SIZE(d));                      \
	})

/*N19A code for HQ-365072 by p-luocong1 at 2024/2/1 start*/
#define lcm_dcs_write_seq_static(ctx, seq...)                              \
	({                                                                     \
		static const u8 d[] = {seq};                                   \
		lcm_dcs_write(ctx, d, ARRAY_SIZE(d));                      \
		if (ARRAY_SIZE(d) > 2)                                    \
			udelay(100);                                      \
	})
/*N19A code for HQ-365072 by p-luocong1 at 2024/2/1 end*/

static inline struct panel *panel_to_lcm(struct drm_panel *panel)
{
	return container_of(panel, struct panel, panel);
}

#ifdef PANEL_SUPPORT_READBACK
static int lcm_dcs_read(struct panel *ctx, u8 cmd, void *data, size_t len)
{
	struct mipi_dsi_device *dsi = to_mipi_dsi_device(ctx->dev);
	int ret = 0;

	if (ctx->error < 0)
		return 0;

	ret = mipi_dsi_dcs_read(dsi, cmd, data, len);
	if (ret < 0) {
		dev_info(ctx->dev, "error %d reading dcs seq:(%#x)\n", ret, cmd);
		ctx->error = ret;
	}

	return ret;
}

static void lcm_panel_get_data(struct panel *ctx)
{
	u8 buffer[3] = {0};
	static int ret = 0;

	if (ret == 0) {
		ret = lcm_dcs_read(ctx, 0x0A, buffer, 1);
		dev_info(ctx->dev, "return %d data(0x%08x) to dsi engine\n",
			 ret, buffer[0] | (buffer[1] << 8));
	}
}

#endif

static void lcm_dcs_write(struct panel *ctx, const void *data, size_t len)
{
	struct mipi_dsi_device *dsi = to_mipi_dsi_device(ctx->dev);
	int ret = 0;
	char *addr;

	if (ctx->error < 0)
		return;

	addr = (char *)data;
	if ((int)*addr < 0xB0)
		ret = mipi_dsi_dcs_write_buffer(dsi, data, len);
	else
		ret = mipi_dsi_generic_write(dsi, data, len);
	if (ret < 0) {
		dev_info(ctx->dev, "error %zd writing seq: %ph\n", ret, data);
		ctx->error = ret;
	}
}
/* N19A code for HQ-369421 by p-huangyunbiao at 2024/2/2 start */
void (*fts_fwresume_work_callback)(void) = NULL;
EXPORT_SYMBOL_GPL(fts_fwresume_work_callback);
/* N19A code for HQ-369421 by p-huangyunbiao at 2024/2/2 end */
static void lcm_panel_init(struct panel *ctx)
{
	gpiod_set_value(ctx->reset_gpio, 1);
	mdelay(5);
	gpiod_set_value(ctx->reset_gpio, 0);
	mdelay(1);
	gpiod_set_value(ctx->reset_gpio, 1);
	mdelay(12);

/* N19A code for HQ-369421 by p-huangyunbiao at 2024/2/2 start */
	if (!panel_check_proximity_mode()) {
		dev_info(ctx->dev, "[FT8725][%s]proximity_status = 0[%d]\n", __func__, __LINE__);
		if (fts_fwresume_work_callback != NULL)
			fts_fwresume_work_callback();
	}
/* N19A code for HQ-369421 by p-huangyunbiao at 2024/2/2 end */

	/*N19A code for HQ-353621 by p-xielihui at 2023/12/26 start*/
	mutex_lock(&ctx->panel_lock);
	/*N19A code for HQ-353621 by p-xielihui at 2023/12/26 end*/
	// optimization
	lcm_dcs_write_seq_static(ctx, 0x00, 0x00);
	lcm_dcs_write_seq_static(ctx, 0xFF, 0x87, 0x25, 0x01);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x80);
	lcm_dcs_write_seq_static(ctx, 0xFF, 0x87, 0x25);

	/*N19A code for HQ-359946 by p-luocong1 at 2023/12/21 start*/
	lcm_dcs_write_seq_static(ctx, 0x00, 0x81);
	lcm_dcs_write_seq_static(ctx, 0xA4, 0x23);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x90);
	lcm_dcs_write_seq_static(ctx, 0xE9, 0x50);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x85);
	lcm_dcs_write_seq_static(ctx, 0xB4, 0x5F);

	/*N19A code for HQ-378290 by p-luocong1 at 2024/3/21 start*/
	lcm_dcs_write_seq_static(ctx, 0x00, 0xB0);
	lcm_dcs_write_seq_static(ctx, 0xB4, 0x00, 0x14, 0x02, 0x00, 0x01, 0xE8, 0x00, 0x07, 0x05, 0x0E, 0x05, 0x16, 0x10, 0xF0);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xB0);
	lcm_dcs_write_seq_static(ctx, 0xCA, 0x03, 0x03, 0x0B, 0x0F, 0xFF, 0x03);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x80);
	lcm_dcs_write_seq_static(ctx, 0xCA, 0xE6, 0xD1, 0xC0, 0xB4, 0xAA, 0xA1, 0x9A, 0x94, 0x8F, 0x8A, 0x86, 0x82);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x90);
	lcm_dcs_write_seq_static(ctx, 0xCA, 0xFE, 0xFF, 0x66, 0xF5, 0xFF, 0xAD, 0xFA, 0xFF, 0xD6);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xA0);
	lcm_dcs_write_seq_static(ctx, 0xCA, 0x06, 0x06, 0x06);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x82);
	lcm_dcs_write_seq_static(ctx, 0xCE, 0x27, 0x27);
	/*N19A code for HQ-378290 by p-luocong1 at 2024/3/21 end*/

	lcm_dcs_write_seq_static(ctx, 0x00, 0x83);
	lcm_dcs_write_seq_static(ctx, 0xC0, 0x18, 0x00, 0x10);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x93);
	lcm_dcs_write_seq_static(ctx, 0xC0, 0x18, 0x00, 0x10);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xA3);
	lcm_dcs_write_seq_static(ctx, 0xC0, 0x18, 0x00, 0x10);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xB0);
	lcm_dcs_write_seq_static(ctx, 0xC0, 0x00, 0xB8, 0x00, 0x18, 0x10);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x73);
	lcm_dcs_write_seq_static(ctx, 0xC0, 0x18, 0x00, 0x10);

	lcm_dcs_write_seq_static(ctx, 0x00, 0xA4);
	lcm_dcs_write_seq_static(ctx, 0xC1, 0x6C);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x85);
	lcm_dcs_write_seq_static(ctx, 0xCE, 0xE0);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x9A);
	lcm_dcs_write_seq_static(ctx, 0xCE, 0x0C, 0x1C, 0x03);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xB2);
	lcm_dcs_write_seq_static(ctx, 0xCF, 0xD9, 0xDD);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xB7);
	lcm_dcs_write_seq_static(ctx, 0xCF, 0x4D, 0x51);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xE4);
	lcm_dcs_write_seq_static(ctx, 0xCF, 0x0A, 0x18, 0x0A, 0x17, 0x0A, 0x17, 0x0A, 0x17, 0x0A, 0x17, 0x0A, 0x17);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x0E);
	lcm_dcs_write_seq_static(ctx, 0xF3, 0x80, 0xFF);
	/*N19A code for HQ-359946 by p-luocong1 at 2023/12/21 end*/
	lcm_dcs_write_seq_static(ctx, 0x00, 0x00);
	lcm_dcs_write_seq_static(ctx, 0xFF, 0xFF, 0xFF, 0xFF);
	/*N19A code for HQ-375453 by p-luocong1 at 2024/3/21 start*/
	/*N19A code for HQ-348450 by p-xielihui at 2024/2/4 start*/
	lcm_dcs_write_seq_static(ctx, 0x51, 0x00, 0x0A);
	/*N19A code for HQ-348450 by p-xielihui at 2024/2/4 end*/
	/*N19A code for HQ-372911 by p-luocong1 at 2024/3/4 start*/
	lcm_dcs_write_seq_static(ctx, 0x53, 0x00);
	/*N19A code for HQ-372911 by p-luocong1 at 2024/3/4 end*/
	/*N19A code for HQ-375453 by p-luocong1 at 2024/3/21 end*/
	/*N19A code for HQ-373389 by p-luocong1 at 2024/2/28 start*/
	lcm_dcs_write_seq_static(ctx, 0x5E, 0x00, 0x0A);
	/*N19A code for HQ-373389 by p-luocong1 at 2024/2/28 end*/
	lcm_dcs_write_seq_static(ctx, 0x55, 0x00);
	lcm_dcs_write_seq_static(ctx, 0x35, 0x00);

	lcm_dcs_write_seq_static(ctx, 0X11);
	lcm_dcs_write_seq_static(ctx, 0X11);
	/*N19A code for HQ-367483 by p-luocong1 at 2024/1/30 start*/
	mdelay(90);
	/*N19A code for HQ-367483 by p-luocong1 at 2024/1/30 end*/
	lcm_dcs_write_seq_static(ctx, 0x29);
	mdelay(10);
	/*N19A code for HQ-353621 by p-xielihui at 2023/12/26 start*/
	mutex_unlock(&ctx->panel_lock);
	/*N19A code for HQ-353621 by p-xielihui at 2023/12/26 end*/

}


/*N19A code for HQ-375453 by p-luocong1 at 2024/3/21 start*/
static void lcm_panel_init_common(struct panel *ctx)
{
	gpiod_set_value(ctx->reset_gpio, 1);
	mdelay(5);
	gpiod_set_value(ctx->reset_gpio, 0);
	mdelay(1);
	gpiod_set_value(ctx->reset_gpio, 1);
	mdelay(12);

	mutex_lock(&ctx->panel_lock);
	// optimization
	lcm_dcs_write_seq_static(ctx, 0x00, 0x00);
	lcm_dcs_write_seq_static(ctx, 0xFF, 0x87, 0x25, 0x01);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x80);
	lcm_dcs_write_seq_static(ctx, 0xFF, 0x87, 0x25);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xA3);
	lcm_dcs_write_seq_static(ctx, 0xB3, 0x09, 0x9C, 0x00, 0x18);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x80);
	lcm_dcs_write_seq_static(ctx, 0xC0, 0x00, 0x56, 0x00, 0x18, 0x00, 0x10);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x90);
	lcm_dcs_write_seq_static(ctx, 0xC0, 0x00, 0x56, 0x00, 0x18, 0x00, 0x10);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xA0);
	lcm_dcs_write_seq_static(ctx, 0xC0, 0x00, 0x96, 0x00, 0x18, 0x00, 0x10);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xB0);
	lcm_dcs_write_seq_static(ctx, 0xC0, 0x00, 0xB8, 0x00, 0x18, 0x10);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xC1);
	lcm_dcs_write_seq_static(ctx, 0xC0, 0x00, 0x94, 0x00, 0x75, 0x00, 0x63, 0x00, 0xB0);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x70);
	lcm_dcs_write_seq_static(ctx, 0xC0, 0x00, 0x9B, 0x00, 0x18, 0x00, 0x10);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xA3);
	lcm_dcs_write_seq_static(ctx, 0xC1, 0x00, 0x6C, 0x00, 0x32, 0x00, 0x02);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xB7);
	lcm_dcs_write_seq_static(ctx, 0xC1, 0x00, 0x48);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x7B);
	lcm_dcs_write_seq_static(ctx, 0xCE, 0xFF, 0xFF);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x80);
	lcm_dcs_write_seq_static(ctx, 0xCE, 0x01, 0x81, 0xFF, 0xFF, 0x00, 0xE0, 0x00, 0x98, 0x00, 0xC0, 0x00, 0xC8, 0x00, 0xD0, 0x00, 0xD0);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x90);
	lcm_dcs_write_seq_static(ctx, 0xCE, 0x00, 0xA7, 0x10, 0x43, 0x00, 0xA7, 0x80, 0xFF, 0xFF, 0x00, 0x0C, 0x1C, 0x03, 0x0F, 0x12);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xA0);
	lcm_dcs_write_seq_static(ctx, 0xCE, 0x10, 0x00, 0x00);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xB0);
	lcm_dcs_write_seq_static(ctx, 0xCE, 0x22, 0x00, 0x00);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xD1);
	lcm_dcs_write_seq_static(ctx, 0xCE, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xE1);
	lcm_dcs_write_seq_static(ctx, 0xCE, 0x05, 0x02, 0xB6, 0x02, 0xB6, 0x02, 0xB6, 0x00, 0x00, 0x00, 0x00);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xF1);
	lcm_dcs_write_seq_static(ctx, 0xCE, 0x20, 0x16, 0x15, 0x00, 0xD9, 0x00, 0xEA, 0x01, 0x1A);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xB0);
	lcm_dcs_write_seq_static(ctx, 0xCF, 0x00, 0x00, 0xD9, 0xDD);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xB5);
	lcm_dcs_write_seq_static(ctx, 0xCF, 0x05, 0x05, 0x4D, 0x51);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xC0);
	lcm_dcs_write_seq_static(ctx, 0xCF, 0x09, 0x09, 0x97, 0x9B);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xC5);
	lcm_dcs_write_seq_static(ctx, 0xCF, 0x09, 0x09, 0x9D, 0xA1);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x60);
	lcm_dcs_write_seq_static(ctx, 0xCF, 0x00, 0x00, 0xCC, 0xD0, 0x05, 0x05, 0x44, 0x48);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x70);
	lcm_dcs_write_seq_static(ctx, 0xCF, 0x00, 0x00, 0xBC, 0xC0, 0x05, 0x05, 0x6C, 0x70);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xD1);
	lcm_dcs_write_seq_static(ctx, 0xC1, 0x0B, 0x60, 0x0F, 0xDC, 0x1B, 0x15, 0x05, 0xAF, 0x07, 0xE5, 0x0D, 0x81);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xE1);
	lcm_dcs_write_seq_static(ctx, 0xC1, 0x0F, 0xDC);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xE4);
	lcm_dcs_write_seq_static(ctx, 0xCF, 0x0A, 0x18, 0x0A, 0x17, 0x0A, 0x17, 0x0A, 0x17, 0x0A, 0x17, 0x0A, 0x17);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x80);
	lcm_dcs_write_seq_static(ctx, 0xC1, 0x00, 0x00);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x90);
	lcm_dcs_write_seq_static(ctx, 0xC1, 0x03);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x93);
	lcm_dcs_write_seq_static(ctx, 0xC1, 0x82);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xF5);
	lcm_dcs_write_seq_static(ctx, 0xCF, 0x01);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xF6);
	lcm_dcs_write_seq_static(ctx, 0xCF, 0x5A);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xF1);
	lcm_dcs_write_seq_static(ctx, 0xCF, 0x5A);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xCC);
	lcm_dcs_write_seq_static(ctx, 0xC1, 0x18);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x91);
	lcm_dcs_write_seq_static(ctx, 0xC4, 0x08);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x80);
	lcm_dcs_write_seq_static(ctx, 0xC5, 0x87, 0x59);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x87);
	lcm_dcs_write_seq_static(ctx, 0xC5, 0x08, 0x08);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x84);
	lcm_dcs_write_seq_static(ctx, 0xC5, 0x66);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x9E);
	lcm_dcs_write_seq_static(ctx, 0xC5, 0x87);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x87);
	lcm_dcs_write_seq_static(ctx, 0xC4, 0x08, 0x08);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x80);
	lcm_dcs_write_seq_static(ctx, 0xC2, 0x84, 0x00, 0x02, 0x8B, 0x83, 0x00, 0x02, 0x8B);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xA0);
	lcm_dcs_write_seq_static(ctx, 0xC2, 0x82, 0x04, 0x00, 0x02, 0x8B, 0x81, 0x04, 0x00, 0x02, 0x8B, 0x80, 0x04, 0x00, 0x02, 0x8B);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xB0);
	lcm_dcs_write_seq_static(ctx, 0xC2, 0x01, 0x04, 0x00, 0x02, 0x8B, 0x02, 0x04, 0x00, 0x02, 0x8B, 0x03, 0x04, 0x00, 0x02, 0x8B);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xC0);
	lcm_dcs_write_seq_static(ctx, 0xC2, 0x04, 0x04, 0x00, 0x02, 0x8B, 0x05, 0x04, 0x00, 0x02, 0x8B);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xE0);
	lcm_dcs_write_seq_static(ctx, 0xC2, 0x77, 0x77, 0x77, 0x77);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xE8);
	lcm_dcs_write_seq_static(ctx, 0xC2, 0x18, 0x99, 0x6B, 0x75, 0x00, 0x00, 0x00, 0x00);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xD0);
	lcm_dcs_write_seq_static(ctx, 0xC3, 0x35, 0x0A, 0x00, 0x00, 0x35, 0x0A, 0x00, 0x00, 0x35, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xE0);
	lcm_dcs_write_seq_static(ctx, 0xC3, 0x35, 0x0A, 0x00, 0x00, 0x35, 0x0A, 0x00, 0x00, 0x35, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x80);
	lcm_dcs_write_seq_static(ctx, 0xCB, 0x00, 0x05, 0x00, 0x00, 0x05, 0x05, 0x00, 0x05, 0x0A, 0xC0, 0xC5, 0x00, 0x0F, 0x00, 0x00, 0x00);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x90);
	lcm_dcs_write_seq_static(ctx, 0xCB, 0x00, 0x04, 0x00, 0x00, 0x04, 0x04, 0x00, 0x04, 0x08, 0x00, 0x04, 0x00, 0x0C, 0x00, 0x00, 0x00);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xA0);
	lcm_dcs_write_seq_static(ctx, 0xCB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x00);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xB0);
	lcm_dcs_write_seq_static(ctx, 0xCB, 0x10, 0x51, 0x84, 0xC0);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xC0);
	lcm_dcs_write_seq_static(ctx, 0xCB, 0x10, 0x51, 0x84, 0xC0);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xD5);
	lcm_dcs_write_seq_static(ctx, 0xCB, 0x81, 0x00, 0x81, 0x81, 0x00, 0x81, 0x81, 0x00, 0x81, 0x81, 0x00);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xE0);
	lcm_dcs_write_seq_static(ctx, 0xCB, 0x81, 0x81, 0x00, 0x81, 0x81, 0x00, 0x81, 0x81, 0x00, 0x81, 0x81, 0x00, 0x81);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x80);
	lcm_dcs_write_seq_static(ctx, 0xCC, 0x03, 0x07, 0x09, 0x0B, 0x0D, 0x24, 0x24, 0x26, 0x2C, 0x22, 0x2C, 0x25, 0x2C, 0x1B, 0x1A, 0x19);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x90);
	lcm_dcs_write_seq_static(ctx, 0xCC, 0x2C, 0x2C, 0x2C, 0x18, 0x17, 0x16, 0x2C, 0x2C);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x80);
	lcm_dcs_write_seq_static(ctx, 0xCD, 0x02, 0x06, 0x08, 0x0A, 0x0C, 0x24, 0x24, 0x26, 0x2C, 0x22, 0x2C, 0x25, 0x2C, 0x1B, 0x1A, 0x19);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x90);
	lcm_dcs_write_seq_static(ctx, 0xCD, 0x2C, 0x2C, 0x2C, 0x18, 0x17, 0x16, 0x2C, 0x2C);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xA0);
	lcm_dcs_write_seq_static(ctx, 0xCC, 0x02, 0x08, 0x06, 0x0C, 0x0A, 0x22, 0x24, 0x26, 0x2C, 0x24, 0x2C, 0x25, 0x2C, 0x1B, 0x1A, 0x19);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xB0);
	lcm_dcs_write_seq_static(ctx, 0xCC, 0x2C, 0x2C, 0x2C, 0x18, 0x17, 0x16, 0x2C, 0x2C);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xA0);
	lcm_dcs_write_seq_static(ctx, 0xCD, 0x03, 0x09, 0x07, 0x0D, 0x0B, 0x22, 0x24, 0x26, 0x2C, 0x24, 0x2C, 0x25, 0x2C, 0x1B, 0x1A, 0x19);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xB0);
	lcm_dcs_write_seq_static(ctx, 0xCD, 0x2C, 0x2C, 0x2C, 0x18, 0x17, 0x16, 0x2C, 0x2C);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x86);
	lcm_dcs_write_seq_static(ctx, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x15, 0x15, 0x15, 0x03);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x76);
	lcm_dcs_write_seq_static(ctx, 0xC0, 0x01, 0x01, 0x01, 0x01, 0x21, 0x21, 0x21, 0x08);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xA3);
	lcm_dcs_write_seq_static(ctx, 0xCE, 0x00, 0x00, 0x00, 0x00, 0x15, 0x03);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xB3);
	lcm_dcs_write_seq_static(ctx, 0xCE, 0x00, 0x00, 0x00, 0x00, 0x15, 0x03);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x93);
	lcm_dcs_write_seq_static(ctx, 0xC5, 0x37);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x97);
	lcm_dcs_write_seq_static(ctx, 0xC5, 0x37);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x9A);
	lcm_dcs_write_seq_static(ctx, 0xC5, 0x19);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x9C);
	lcm_dcs_write_seq_static(ctx, 0xC5, 0x19);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xB6);
	lcm_dcs_write_seq_static(ctx, 0xC5, 0x2D, 0x2D, 0x0F, 0x0F, 0x2D, 0x2D, 0x0F, 0x0F);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x99);
	lcm_dcs_write_seq_static(ctx, 0xCF, 0x50);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x9C);
	lcm_dcs_write_seq_static(ctx, 0xF5, 0x00);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x9E);
	lcm_dcs_write_seq_static(ctx, 0xF5, 0x00);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xB0);
	lcm_dcs_write_seq_static(ctx, 0xC5, 0x10, 0x4A, 0x01, 0x1F, 0x4A, 0x00);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x00);
	lcm_dcs_write_seq_static(ctx, 0xD8, 0x2B, 0x2B);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x82);
	lcm_dcs_write_seq_static(ctx, 0xA7, 0x20, 0x00);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x8D);
	lcm_dcs_write_seq_static(ctx, 0xA7, 0x02);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x8F);
	lcm_dcs_write_seq_static(ctx, 0xA7, 0x02);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x9A);
	lcm_dcs_write_seq_static(ctx, 0xC4, 0x11, 0x08);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x93);
	lcm_dcs_write_seq_static(ctx, 0xE9, 0xFF, 0xFF, 0xB0);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x80);
	lcm_dcs_write_seq_static(ctx, 0xA4, 0xC9);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xB1);
	lcm_dcs_write_seq_static(ctx, 0xF5, 0x1F);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x80);
	lcm_dcs_write_seq_static(ctx, 0xB3, 0x22);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xB0);
	lcm_dcs_write_seq_static(ctx, 0xB3, 0x00);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x83);
	lcm_dcs_write_seq_static(ctx, 0xB0, 0x63);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xA0);
	lcm_dcs_write_seq_static(ctx, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1D, 0x01);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x84);
	lcm_dcs_write_seq_static(ctx, 0xA4, 0x02);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x81);
	lcm_dcs_write_seq_static(ctx, 0xA6, 0x04);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x82);
	lcm_dcs_write_seq_static(ctx, 0xF5, 0x01);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x93);
	lcm_dcs_write_seq_static(ctx, 0xF5, 0x01);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x9B);
	lcm_dcs_write_seq_static(ctx, 0xF5, 0x49);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x9D);
	lcm_dcs_write_seq_static(ctx, 0xF5, 0x49);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xBE);
	lcm_dcs_write_seq_static(ctx, 0xC5, 0xF0, 0xF0);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xE8);
	lcm_dcs_write_seq_static(ctx, 0xC0, 0x40);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x80);
	lcm_dcs_write_seq_static(ctx, 0xA7, 0x03);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xA0);
	lcm_dcs_write_seq_static(ctx, 0xC3, 0x00);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xCC);
	lcm_dcs_write_seq_static(ctx, 0xC0, 0x13);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xE0);
	lcm_dcs_write_seq_static(ctx, 0xCF, 0x34);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x92);
	lcm_dcs_write_seq_static(ctx, 0xC5, 0x00);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xA0);
	lcm_dcs_write_seq_static(ctx, 0xC5, 0x40);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x98);
	lcm_dcs_write_seq_static(ctx, 0xC5, 0x24);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xA1);
	lcm_dcs_write_seq_static(ctx, 0xC5, 0x40);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x9D);
	lcm_dcs_write_seq_static(ctx, 0xC5, 0x24);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x94);
	lcm_dcs_write_seq_static(ctx, 0xC5, 0x04);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x86);
	lcm_dcs_write_seq_static(ctx, 0xA4, 0x51);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x82);
	lcm_dcs_write_seq_static(ctx, 0xCE, 0x27, 0x27);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xC0);
	lcm_dcs_write_seq_static(ctx, 0xC5, 0x0F);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x90);
	lcm_dcs_write_seq_static(ctx, 0xA7, 0x1D);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x81);
	lcm_dcs_write_seq_static(ctx, 0xA4, 0x23);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x90);
	lcm_dcs_write_seq_static(ctx, 0xE9, 0x50);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xFC);
	lcm_dcs_write_seq_static(ctx, 0xC0, 0x80, 0x18);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x87);
	lcm_dcs_write_seq_static(ctx, 0xF5, 0x00);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xE0);
	lcm_dcs_write_seq_static(ctx, 0xCE, 0x88);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x0E);
	lcm_dcs_write_seq_static(ctx, 0xF3, 0x80, 0xFF);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xBE);
	lcm_dcs_write_seq_static(ctx, 0xC5, 0xC0, 0xC0);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x85);
	lcm_dcs_write_seq_static(ctx, 0xB4, 0x5F);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xB0);
	lcm_dcs_write_seq_static(ctx, 0xB4, 0x00, 0x14, 0x02, 0x00, 0x01, 0xE8, 0x00, 0x07, 0x05, 0x0E, 0x05, 0x16, 0x10, 0xF0);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xB0);
	lcm_dcs_write_seq_static(ctx, 0xCA, 0x03, 0x03, 0x0B, 0x0F, 0xFF, 0x03);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x80);
	lcm_dcs_write_seq_static(ctx, 0xCA, 0xE6, 0xD1, 0xC0, 0xB4, 0xAA, 0xA1, 0x9A, 0x94, 0x8F, 0x8A, 0x86, 0x82);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x90);
	lcm_dcs_write_seq_static(ctx, 0xCA, 0xFE, 0xFF, 0x66, 0xF5, 0xFF, 0xAD, 0xFA, 0xFF, 0xD6);
	lcm_dcs_write_seq_static(ctx, 0x00, 0xA0);
	lcm_dcs_write_seq_static(ctx, 0xCA, 0x06, 0x06, 0x06);
	lcm_dcs_write_seq_static(ctx, 0x00, 0x00);
	lcm_dcs_write_seq_static(ctx, 0xFF, 0xFF, 0xFF, 0xFF);
	lcm_dcs_write_seq_static(ctx, 0x35, 0x00);
	lcm_dcs_write_seq_static(ctx, 0x51, 0x00, 0x0A);
	lcm_dcs_write_seq_static(ctx, 0x51, 0x00, 0x0A);
	/*N19A code for HQ-372911 by p-luocong1 at 2024/3/4 start*/
	lcm_dcs_write_seq_static(ctx, 0x53, 0x00);
	/*N19A code for HQ-372911 by p-luocong1 at 2024/3/4 end*/
	lcm_dcs_write_seq_static(ctx, 0x5E, 0x00, 0x0A);
	lcm_dcs_write_seq_static(ctx, 0x55, 0x00);
	lcm_dcs_write_seq_static(ctx, 0x35, 0x00);
	lcm_dcs_write_seq_static(ctx, 0x11);
	lcm_dcs_write_seq_static(ctx, 0x11);
	mdelay(90);
	lcm_dcs_write_seq_static(ctx, 0x29);
	mdelay(10);
	mutex_unlock(&ctx->panel_lock);

}
/*N19A code for HQ-375453 by p-luocong1 at 2024/3/21 end*/

static int panel_disable(struct drm_panel *panel)
{
	struct panel *ctx = panel_to_lcm(panel);

	pr_info("[kernel/lcm]%s enter\n", __func__);

	if (!ctx->enabled)
		return 0;

	if (ctx->backlight) {
		ctx->backlight->props.power = FB_BLANK_POWERDOWN;
		backlight_update_status(ctx->backlight);
	}

	ctx->enabled = false;

	return 0;
}

/* N19A code for HQ-348470 by p-xielihui at 2024/1/2 start */
void panel_gesture_mode_set_by_ft8725(bool gst_mode)
{
	atomic_set(&gesture_status, gst_mode);
}
EXPORT_SYMBOL_GPL(panel_gesture_mode_set_by_ft8725);

static bool panel_check_gesture_mode(void)
{
	bool ret = false;

	pr_info("[kernel/lcm]%s enter\n", __func__);
	ret = atomic_read(&gesture_status);

	return ret;
}
/* N19A code for HQ-348470 by p-xielihui at 2024/1/2 end */

/* N19A code for HQ-364908 by p-huangyunbiao at 2024/1/8 start */
void panel_proximity_mode_set_by_ft8725(bool prx_mode)
{
	atomic_set(&proximity_status, prx_mode);
}
EXPORT_SYMBOL_GPL(panel_proximity_mode_set_by_ft8725);

static bool panel_check_proximity_mode(void)
{
	bool ret = false;

	pr_info("[kernel/lcm]%s enter\n", __func__);
	ret = atomic_read(&proximity_status);

	return ret;
}
/* N19A code for HQ-364908 by p-huangyunbiao at 2024/1/8 end */

static int panel_unprepare(struct drm_panel *panel)
{
	struct panel *ctx = panel_to_lcm(panel);

	pr_info("[kernel/lcm]%s enter\n", __func__);

	if (!ctx->prepared)
		return 0;

	/*N19A code for HQ-348450 by p-xielihui at 2024/2/1 start*/
	blank = DRM_BLANK_POWERDOWN;
	g_notify_data1.data = &blank;
	drm_notifier_call_chain(DRM_EVENT_BLANK, &g_notify_data1);
	pr_err("[XMFP] : %s ++++ blank = DRM_BLANK_POWERDOWN ++++", __func__);
	/*N19A code for HQ-348450 by p-xielihui at 2024/2/1 end*/

/* N19A code for HQ-369421 by p-huangyunbiao at 2024/2/2 start */
	if (!panel_check_proximity_mode()) {
		/*N19A code for HQ-353621 by p-xielihui at 2023/12/26 start*/
		mutex_lock(&ctx->panel_lock);
		/*N19A code for HQ-353621 by p-xielihui at 2023/12/26 end*/
		lcm_dcs_write_seq_static(ctx, 0x00, 0x00);
		lcm_dcs_write_seq_static(ctx, 0xFF, 0x87, 0X25, 0X01);
		lcm_dcs_write_seq_static(ctx, 0x00, 0x80);
		lcm_dcs_write_seq_static(ctx, 0xFF, 0x87, 0x25);

		lcm_dcs_write_seq_static(ctx, 0x28);
		/*N19A code for HQ-348460 by p-luocong1 at 2024/1/5 start*/
		msleep(40);
		/*N19A code for HQ-348460 by p-luocong1 at 2024/1/5 end*/
		lcm_dcs_write_seq_static(ctx, 0x10);
		msleep(150);

		lcm_dcs_write_seq_static(ctx, 0x00, 0x00);
		lcm_dcs_write_seq_static(ctx, 0xF7, 0x5A, 0xA5, 0x95, 0x27);
		lcm_dcs_write_seq_static(ctx, 0x00, 0x00);
		lcm_dcs_write_seq_static(ctx, 0xFF, 0xFF, 0XFF,0xFF);
		msleep(10);
		/*N19A code for HQ-353621 by p-xielihui at 2023/12/26 start*/
		mutex_unlock(&ctx->panel_lock);
		/*N19A code for HQ-353621 by p-xielihui at 2023/12/26 end*/
	} else {
		mutex_lock(&ctx->panel_lock);
		lcm_dcs_write_seq_static(ctx, 0x28);
		mutex_unlock(&ctx->panel_lock);
		msleep(10);
	}
/* N19A code for HQ-369421 by p-huangyunbiao at 2024/2/2 end */

/* N19A code for HQ-348470 by p-xielihui at 2024/1/2 start */
/* N19A code for HQ-364908 by p-huangyunbiao at 2024/1/8 start */
	if(panel_check_gesture_mode() || panel_check_proximity_mode()){
		pr_info("[kernel/lcm]%s gesture mode or proximity mode on, gesture_status = %d proximity_status = %d\n",
					__func__, gesture_status, proximity_status);
	} else {
/* N19A code for HQ-364908 by p-huangyunbiao at 2024/1/8 end */
		gpiod_set_value(ctx->avee_gpio, 0);
		mdelay(1);

		gpiod_set_value(ctx->avdd_gpio, 0);
	}
/* N19A code for HQ-348470 by p-xielihui at 2024/1/2 end */

	ctx->error = 0;
	ctx->prepared = false;
	return 0;
}

static int panel_prepare(struct drm_panel *panel)
{
	struct panel *ctx = panel_to_lcm(panel);
	int ret = 0;

	pr_info("[kernel/lcm]%s enter\n", __func__);

	if (ctx->prepared)
		return 0;

	gpiod_set_value(ctx->avdd_gpio, 1);
	mdelay(3);

	gpiod_set_value(ctx->avee_gpio, 1);
	mdelay(3);

	/*N19A code for HQ-369142 by p-xielihui at 2024/1/29 start*/
	switch(panel_id){
		case FT8725_HUAXING_COMMON:
			lcm_panel_init_common(ctx);
			break;
		case FT8725_HUAXING_PRIVATE:
			lcm_panel_init(ctx);
			break;
		default:
			lcm_panel_init(ctx);
			break;
	}
	/*N19A code for HQ-369142 by p-xielihui at 2024/1/29 end*/

	ret = ctx->error;
	if (ret < 0)
		panel_unprepare(panel);

	ctx->prepared = true;
#if defined(CONFIG_MTK_PANEL_EXT)
	mtk_panel_tch_rst(panel);
#endif

#ifdef PANEL_SUPPORT_READBACK
	lcm_panel_get_data(ctx);
#endif

	/*N19A code for HQ-348450 by p-xielihui at 2024/2/1 start*/
	blank = DRM_BLANK_UNBLANK;
	g_notify_data1.data = &blank;
	drm_notifier_call_chain(DRM_EVENT_BLANK, &g_notify_data1);
	pr_err("[XMFP] : %s ++++ blank = DRM_BLANK_UNBLANK ++++", __func__);
	/*N19A code for HQ-348450 by p-xielihui at 2024/2/1 end*/

	return ret;
}

static int panel_enable(struct drm_panel *panel)
{
	struct panel *ctx = panel_to_lcm(panel);

	pr_info("[kernel/lcm]%s enter\n", __func__);

	if (ctx->enabled)
		return 0;

	if (ctx->backlight) {
		ctx->backlight->props.power = FB_BLANK_UNBLANK;
		backlight_update_status(ctx->backlight);
	}

	ctx->enabled = true;

	return 0;
}

static const struct drm_display_mode default_mode = {
	.clock = (FRAME_WIDTH + HFP + HSA + HBP) *
		(FRAME_HEIGHT + MODE_0_VFP + VSA + VBP) * MODE_0_FPS / 1000,
	.hdisplay = FRAME_WIDTH,
	.hsync_start = FRAME_WIDTH + HFP,
	.hsync_end = FRAME_WIDTH + HFP + HSA,
	.htotal = FRAME_WIDTH + HFP + HSA + HBP,
	.vdisplay = FRAME_HEIGHT,
	.vsync_start = FRAME_HEIGHT + MODE_0_VFP,
	.vsync_end = FRAME_HEIGHT + MODE_0_VFP + VSA,
	.vtotal = FRAME_HEIGHT + MODE_0_VFP + VSA + VBP,
};

/*N19A code for HQ-348462 by p-luocong1 at 2024/1/3 start*/
static const struct drm_display_mode performance_mode_1 = {
	.clock = (FRAME_WIDTH + HFP + HSA + HBP) *
		(FRAME_HEIGHT + MODE_1_VFP + VSA + VBP) * MODE_1_FPS / 1000,
	.hdisplay = FRAME_WIDTH,
	.hsync_start = FRAME_WIDTH + HFP,
	.hsync_end = FRAME_WIDTH + HFP + HSA,
	.htotal = FRAME_WIDTH + HFP + HSA + HBP,
	.vdisplay = FRAME_HEIGHT,
	.vsync_start = FRAME_HEIGHT + MODE_1_VFP,
	.vsync_end = FRAME_HEIGHT + MODE_1_VFP + VSA,
	.vtotal = FRAME_HEIGHT + MODE_1_VFP + VSA + VBP,
};
/*N19A code for HQ-348462 by p-luocong1 at 2024/1/3 end*/

/*N19A code for HQ-366831 by p-luocong1 at 2024/1/15 start*/
static const struct drm_display_mode performance_mode_2 = {
	.clock = (FRAME_WIDTH + HFP + HSA + HBP) *
		(FRAME_HEIGHT + MODE_2_VFP + VSA + VBP) * MODE_2_FPS / 1000,
	.hdisplay = FRAME_WIDTH,
	.hsync_start = FRAME_WIDTH + HFP,
	.hsync_end = FRAME_WIDTH + HFP + HSA,
	.htotal = FRAME_WIDTH + HFP + HSA + HBP,
	.vdisplay = FRAME_HEIGHT,
	.vsync_start = FRAME_HEIGHT + MODE_2_VFP,
	.vsync_end = FRAME_HEIGHT + MODE_2_VFP + VSA,
	.vtotal = FRAME_HEIGHT + MODE_2_VFP + VSA + VBP,
};

static const struct drm_display_mode performance_mode_3 = {
	.clock = (FRAME_WIDTH + HFP + HSA + HBP) *
		(FRAME_HEIGHT + MODE_3_VFP + VSA + VBP) * MODE_3_FPS / 1000,
	.hdisplay = FRAME_WIDTH,
	.hsync_start = FRAME_WIDTH + HFP,
	.hsync_end = FRAME_WIDTH + HFP + HSA,
	.htotal = FRAME_WIDTH + HFP + HSA + HBP,
	.vdisplay = FRAME_HEIGHT,
	.vsync_start = FRAME_HEIGHT + MODE_3_VFP,
	.vsync_end = FRAME_HEIGHT + MODE_3_VFP + VSA,
	.vtotal = FRAME_HEIGHT + MODE_3_VFP + VSA + VBP,
};
/*N19A code for HQ-366831 by p-luocong1 at 2024/1/15 end*/

#if defined(CONFIG_MTK_PANEL_EXT)

static struct mtk_panel_params ext_params = {
	/*N19A code for HQ-374761 by p-luocong1 at 2024/3/12 start*/
	.vfp_low_power = MODE_3_VFP,//36hz
	/*N19A code for HQ-374761 by p-luocong1 at 2024/3/12 end*/
	/*N19A code for HQ-348361 by p-xielihui at 2024/1/19 start*/
	.cust_esd_check = 1,
	.esd_check_enable = 1,
	/*N19A code for HQ-348361 by p-xielihui at 2024/1/19 end*/
	.lcm_esd_check_table[0] = {
		.cmd = 0x0A,
		.count = 1,
		.para_list[0] = 0x9C,
	},
	.physical_width_um = PHYSICAL_WIDTH,
	.physical_height_um = PHYSICAL_HEIGHT,
	.dsc_params = {
		.bdg_dsc_enable = DSC_ENABLE,
		.ver                   =  DSC_VER,
		.slice_mode            =  DSC_SLICE_MODE,
		.rgb_swap              =  DSC_RGB_SWAP,
		.dsc_cfg               =  DSC_DSC_CFG,
		.rct_on                =  DSC_RCT_ON,
		.bit_per_channel       =  DSC_BIT_PER_CHANNEL,
		.dsc_line_buf_depth    =  DSC_DSC_LINE_BUF_DEPTH,
		.bp_enable             =  DSC_BP_ENABLE,
		.bit_per_pixel         =  DSC_BIT_PER_PIXEL,
		.pic_height            =  FRAME_HEIGHT,
		.pic_width             =  FRAME_WIDTH,
		.slice_height          =  DSC_SLICE_HEIGHT,
		.slice_width           =  DSC_SLICE_WIDTH,
		.chunk_size            =  DSC_CHUNK_SIZE,
		.xmit_delay            =  DSC_XMIT_DELAY,
		.dec_delay             =  DSC_DEC_DELAY,
		.scale_value           =  DSC_SCALE_VALUE,
		.increment_interval    =  DSC_INCREMENT_INTERVAL,
		.decrement_interval    =  DSC_DECREMENT_INTERVAL,
		.line_bpg_offset       =  DSC_LINE_BPG_OFFSET,
		.nfl_bpg_offset        =  DSC_NFL_BPG_OFFSET,
		.slice_bpg_offset      =  DSC_SLICE_BPG_OFFSET,
		.initial_offset        =  DSC_INITIAL_OFFSET,
		.final_offset          =  DSC_FINAL_OFFSET,
		.flatness_minqp        =  DSC_FLATNESS_MINQP,
		.flatness_maxqp        =  DSC_FLATNESS_MAXQP,
		.rc_model_size         =  DSC_RC_MODEL_SIZE,
		.rc_edge_factor        =  DSC_RC_EDGE_FACTOR,
		.rc_quant_incr_limit0  =  DSC_RC_QUANT_INCR_LIMIT0,
		.rc_quant_incr_limit1  =  DSC_RC_QUANT_INCR_LIMIT1,
		.rc_tgt_offset_hi      =  DSC_RC_TGT_OFFSET_HI,
		.rc_tgt_offset_lo      =  DSC_RC_TGT_OFFSET_LO,
	},
	.data_rate = DATA_RATE,
	.bdg_ssc_enable = 0,
	.ssc_enable = 0,
	.dyn_fps = {
		.switch_en = 1,
		/*N19A code for HQ-353622 by p-luocong1 at 2024/1/18 start*/
		/* N19A code for HQ-368485 by p-luocong1 at 2024/1/29 start */
		.vact_timing_fps = MODE_1_FPS,
		/* N19A code for HQ-368485 by p-luocong1 at 2024/1/29 end */
	},
	/* dsi_hbp is hbp_wc, cal hbp according to hbp_wc, ref:4997538 */
	.dyn = {
		.switch_en = 1,
		/*N19A code for HQ-348462 by p-luocong1 at 2024/1/3 start*/
		.data_rate = DATA_RATE,
		.vfp = MODE_0_VFP,
		/*N19A code for HQ-348462 by p-luocong1 at 2024/1/3 end*/
		/*N19A code for HQ-353622 by p-luocong1 at 2024/1/18 end*/
	},
	.phy_timcon = {
		.clk_hs_post = 0x24,
	},
};

/*N19A code for HQ-348462 by p-luocong1 at 2024/1/3 start*/
static struct mtk_panel_params ext_params_mode_1 = {
	/*N19A code for HQ-374761 by p-luocong1 at 2024/3/12 start*/
	.vfp_low_power = MODE_3_VFP,//36hz
	/*N19A code for HQ-374761 by p-luocong1 at 2024/3/12 end*/
	/*N19A code for HQ-348361 by p-xielihui at 2024/1/19 start*/
	.cust_esd_check = 1,
	.esd_check_enable = 1,
	/*N19A code for HQ-348361 by p-xielihui at 2024/1/19 end*/
	.lcm_esd_check_table[0] = {
		.cmd = 0x0A,
		.count = 1,
		.para_list[0] = 0x9C,
	},
	.physical_width_um = PHYSICAL_WIDTH,
	.physical_height_um = PHYSICAL_HEIGHT,
	.dsc_params = {
		.bdg_dsc_enable = DSC_ENABLE,
		.ver                   =  DSC_VER,
		.slice_mode            =  DSC_SLICE_MODE,
		.rgb_swap              =  DSC_RGB_SWAP,
		.dsc_cfg               =  DSC_DSC_CFG,
		.rct_on                =  DSC_RCT_ON,
		.bit_per_channel       =  DSC_BIT_PER_CHANNEL,
		.dsc_line_buf_depth    =  DSC_DSC_LINE_BUF_DEPTH,
		.bp_enable             =  DSC_BP_ENABLE,
		.bit_per_pixel         =  DSC_BIT_PER_PIXEL,
		.pic_height            =  FRAME_HEIGHT,
		.pic_width             =  FRAME_WIDTH,
		.slice_height          =  DSC_SLICE_HEIGHT,
		.slice_width           =  DSC_SLICE_WIDTH,
		.chunk_size            =  DSC_CHUNK_SIZE,
		.xmit_delay            =  DSC_XMIT_DELAY,
		.dec_delay             =  DSC_DEC_DELAY,
		.scale_value           =  DSC_SCALE_VALUE,
		.increment_interval    =  DSC_INCREMENT_INTERVAL,
		.decrement_interval    =  DSC_DECREMENT_INTERVAL,
		.line_bpg_offset       =  DSC_LINE_BPG_OFFSET,
		.nfl_bpg_offset        =  DSC_NFL_BPG_OFFSET,
		.slice_bpg_offset      =  DSC_SLICE_BPG_OFFSET,
		.initial_offset        =  DSC_INITIAL_OFFSET,
		.final_offset          =  DSC_FINAL_OFFSET,
		.flatness_minqp        =  DSC_FLATNESS_MINQP,
		.flatness_maxqp        =  DSC_FLATNESS_MAXQP,
		.rc_model_size         =  DSC_RC_MODEL_SIZE,
		.rc_edge_factor        =  DSC_RC_EDGE_FACTOR,
		.rc_quant_incr_limit0  =  DSC_RC_QUANT_INCR_LIMIT0,
		.rc_quant_incr_limit1  =  DSC_RC_QUANT_INCR_LIMIT1,
		.rc_tgt_offset_hi      =  DSC_RC_TGT_OFFSET_HI,
		.rc_tgt_offset_lo      =  DSC_RC_TGT_OFFSET_LO,
	},
	.data_rate = DATA_RATE,
	.bdg_ssc_enable = 0,
	.ssc_enable = 0,
	.dyn_fps = {
		.switch_en = 1,
		/*N19A code for HQ-353622 by p-luocong1 at 2024/1/18 start*/
		.vact_timing_fps = MODE_1_FPS,
	},
	/* dsi_hbp is hbp_wc, cal hbp according to hbp_wc, ref:4997538 */
	.dyn = {
		.switch_en = 1,
		.data_rate = DATA_RATE,
		/*N19A code for HQ-366831 by p-luocong1 at 2024/1/15 start*/
		.vfp = MODE_1_VFP,
		/*N19A code for HQ-366831 by p-luocong1 at 2024/1/15 end*/
		/*N19A code for HQ-353622 by p-luocong1 at 2024/1/18 end*/
	},
	.phy_timcon = {
		.clk_hs_post = 0x24,
	},
};
/*N19A code for HQ-348462 by p-luocong1 at 2024/1/3 end*/

/*N19A code for HQ-366831 by p-luocong1 at 2024/1/15 start*/
static struct mtk_panel_params ext_params_mode_2 = {
	/*N19A code for HQ-374761 by p-luocong1 at 2024/3/12 start*/
	.vfp_low_power = MODE_3_VFP,//36hz
	/*N19A code for HQ-374761 by p-luocong1 at 2024/3/12 end*/
	/*N19A code for HQ-348361 by p-xielihui at 2024/1/19 start*/
	.cust_esd_check = 1,
	.esd_check_enable = 1,
	/*N19A code for HQ-348361 by p-xielihui at 2024/1/19 end*/
	.lcm_esd_check_table[0] = {
		.cmd = 0x0A,
		.count = 1,
		.para_list[0] = 0x9C,
	},
	.physical_width_um = PHYSICAL_WIDTH,
	.physical_height_um = PHYSICAL_HEIGHT,
	.dsc_params = {
		.bdg_dsc_enable = DSC_ENABLE,
		.ver                   =  DSC_VER,
		.slice_mode            =  DSC_SLICE_MODE,
		.rgb_swap              =  DSC_RGB_SWAP,
		.dsc_cfg               =  DSC_DSC_CFG,
		.rct_on                =  DSC_RCT_ON,
		.bit_per_channel       =  DSC_BIT_PER_CHANNEL,
		.dsc_line_buf_depth    =  DSC_DSC_LINE_BUF_DEPTH,
		.bp_enable             =  DSC_BP_ENABLE,
		.bit_per_pixel         =  DSC_BIT_PER_PIXEL,
		.pic_height            =  FRAME_HEIGHT,
		.pic_width             =  FRAME_WIDTH,
		.slice_height          =  DSC_SLICE_HEIGHT,
		.slice_width           =  DSC_SLICE_WIDTH,
		.chunk_size            =  DSC_CHUNK_SIZE,
		.xmit_delay            =  DSC_XMIT_DELAY,
		.dec_delay             =  DSC_DEC_DELAY,
		.scale_value           =  DSC_SCALE_VALUE,
		.increment_interval    =  DSC_INCREMENT_INTERVAL,
		.decrement_interval    =  DSC_DECREMENT_INTERVAL,
		.line_bpg_offset       =  DSC_LINE_BPG_OFFSET,
		.nfl_bpg_offset        =  DSC_NFL_BPG_OFFSET,
		.slice_bpg_offset      =  DSC_SLICE_BPG_OFFSET,
		.initial_offset        =  DSC_INITIAL_OFFSET,
		.final_offset          =  DSC_FINAL_OFFSET,
		.flatness_minqp        =  DSC_FLATNESS_MINQP,
		.flatness_maxqp        =  DSC_FLATNESS_MAXQP,
		.rc_model_size         =  DSC_RC_MODEL_SIZE,
		.rc_edge_factor        =  DSC_RC_EDGE_FACTOR,
		.rc_quant_incr_limit0  =  DSC_RC_QUANT_INCR_LIMIT0,
		.rc_quant_incr_limit1  =  DSC_RC_QUANT_INCR_LIMIT1,
		.rc_tgt_offset_hi      =  DSC_RC_TGT_OFFSET_HI,
		.rc_tgt_offset_lo      =  DSC_RC_TGT_OFFSET_LO,
	},
	.data_rate = DATA_RATE,
	.bdg_ssc_enable = 0,
	.ssc_enable = 0,
	.dyn_fps = {
		.switch_en = 1,
		/*N19A code for HQ-353622 by p-luocong1 at 2024/1/18 start*/
		/* N19A code for HQ-368485 by p-luocong1 at 2024/1/29 start */
		.vact_timing_fps = MODE_1_FPS,
		/* N19A code for HQ-368485 by p-luocong1 at 2024/1/29 end */
	},
	/* dsi_hbp is hbp_wc, cal hbp according to hbp_wc, ref:4997538 */
	.dyn = {
		.switch_en = 1,
		.data_rate = DATA_RATE,
		.vfp = MODE_2_VFP,
		/*N19A code for HQ-353622 by p-luocong1 at 2024/1/18 end*/
	},
	.phy_timcon = {
		.clk_hs_post = 0x24,
	},
};

static struct mtk_panel_params ext_params_mode_3 = {
	/*N19A code for HQ-348361 by p-xielihui at 2024/1/19 start*/
	.cust_esd_check = 1,
	.esd_check_enable = 1,
	/*N19A code for HQ-348361 by p-xielihui at 2024/1/19 end*/
	.lcm_esd_check_table[0] = {
		.cmd = 0x0A,
		.count = 1,
		.para_list[0] = 0x9C,
	},
	.physical_width_um = PHYSICAL_WIDTH,
	.physical_height_um = PHYSICAL_HEIGHT,
	.dsc_params = {
		.bdg_dsc_enable = DSC_ENABLE,
		.ver                   =  DSC_VER,
		.slice_mode            =  DSC_SLICE_MODE,
		.rgb_swap              =  DSC_RGB_SWAP,
		.dsc_cfg               =  DSC_DSC_CFG,
		.rct_on                =  DSC_RCT_ON,
		.bit_per_channel       =  DSC_BIT_PER_CHANNEL,
		.dsc_line_buf_depth    =  DSC_DSC_LINE_BUF_DEPTH,
		.bp_enable             =  DSC_BP_ENABLE,
		.bit_per_pixel         =  DSC_BIT_PER_PIXEL,
		.pic_height            =  FRAME_HEIGHT,
		.pic_width             =  FRAME_WIDTH,
		.slice_height          =  DSC_SLICE_HEIGHT,
		.slice_width           =  DSC_SLICE_WIDTH,
		.chunk_size            =  DSC_CHUNK_SIZE,
		.xmit_delay            =  DSC_XMIT_DELAY,
		.dec_delay             =  DSC_DEC_DELAY,
		.scale_value           =  DSC_SCALE_VALUE,
		.increment_interval    =  DSC_INCREMENT_INTERVAL,
		.decrement_interval    =  DSC_DECREMENT_INTERVAL,
		.line_bpg_offset       =  DSC_LINE_BPG_OFFSET,
		.nfl_bpg_offset        =  DSC_NFL_BPG_OFFSET,
		.slice_bpg_offset      =  DSC_SLICE_BPG_OFFSET,
		.initial_offset        =  DSC_INITIAL_OFFSET,
		.final_offset          =  DSC_FINAL_OFFSET,
		.flatness_minqp        =  DSC_FLATNESS_MINQP,
		.flatness_maxqp        =  DSC_FLATNESS_MAXQP,
		.rc_model_size         =  DSC_RC_MODEL_SIZE,
		.rc_edge_factor        =  DSC_RC_EDGE_FACTOR,
		.rc_quant_incr_limit0  =  DSC_RC_QUANT_INCR_LIMIT0,
		.rc_quant_incr_limit1  =  DSC_RC_QUANT_INCR_LIMIT1,
		.rc_tgt_offset_hi      =  DSC_RC_TGT_OFFSET_HI,
		.rc_tgt_offset_lo      =  DSC_RC_TGT_OFFSET_LO,
	},
	.data_rate = DATA_RATE,
	.bdg_ssc_enable = 0,
	.ssc_enable = 0,
	.dyn_fps = {
		.switch_en = 1,
		/*N19A code for HQ-353622 by p-luocong1 at 2024/1/18 start*/
		/* N19A code for HQ-368485 by p-luocong1 at 2024/1/29 start */
		.vact_timing_fps = MODE_1_FPS,
		/* N19A code for HQ-368485 by p-luocong1 at 2024/1/29 end */
	},
	/* dsi_hbp is hbp_wc, cal hbp according to hbp_wc, ref:4997538 */
	.dyn = {
		.switch_en = 1,
		.data_rate = DATA_RATE,
		.vfp = MODE_3_VFP,
		/*N19A code for HQ-353622 by p-luocong1 at 2024/1/18 end*/
	},
	.phy_timcon = {
		.clk_hs_post = 0x24,
	},
};
/*N19A code for HQ-366831 by p-luocong1 at 2024/1/15 end*/

static int panel_setbacklight_cmdq(void *dsi, dcs_write_gce cb,
	void *handle, unsigned int level)
{
	/*N19A code for HQ-353621 by p-xielihui at 2023/12/26 start*/
	struct panel *ctx = panel_to_lcm(this_panel);
	/*N19A code for HQ-353621 by p-xielihui at 2023/12/26 end*/
	/*N19A code for HQ-354076 by p-xielihui at 2023/12/19 start*/
	char bl_tb0[] = {0x51, 0xFF, 0x0E};
	/*N19A code for HQ-372911 by p-luocong1 at 2024/3/4 start*/
	char bl_en0[] = {0x53, 0x24};
	/*N19A code for HQ-372911 by p-luocong1 at 2024/3/4 end*/
	/*N19A code for HQ-381364 by p-xielihui at 2024/4/10 start*/
	unsigned int level_in = level;
	/*N19A code for HQ-381364 by p-xielihui at 2024/4/10 end*/

	/*N19A code for HQ-348394 by p-xielihui at 2024/1/20 start*/
	if (level > HBM_MAP_MAX_BRIGHTNESS) {
		pr_err("[kernel/lcm]%s err level;", __func__);
		return -EPERM;
	} else if (level > NORMAL_MAX_BRIGHTNESS) {
		pr_info("[kernel/lcm]%s hbm_on, level = %d", __func__, level);
		level = (level - REG_MAX_BRIGHTNESS) * (REG_HBM_BRIGHTNESS - REG_NORMAL_BRIGHTNESS)
			 / (HBM_MAP_MAX_BRIGHTNESS - REG_MAX_BRIGHTNESS) + REG_NORMAL_BRIGHTNESS;
	} else {
		pr_info("[kernel/lcm]%s hbm_off, level = %d", __func__, level);
		level = level * REG_NORMAL_BRIGHTNESS / REG_MAX_BRIGHTNESS;
	}
	/*N19A code for HQ-348394 by p-xielihui at 2024/1/20 end*/

	bl_tb0[1] = level >> 3 & 0xFF;
	bl_tb0[2] = level << 1 & 0x0E;

	pr_info("[kernel/lcm]%s backlight = %d, bl_tb0[1] = 0x%x, bl_tb0[2] = 0x%x\n",
				__func__, level, bl_tb0[1], bl_tb0[2]);

	if (!cb){
		pr_err("[kernel/lcm] cb error");
		return -1;
	}
	/*N19A code for HQ-354076 by p-xielihui at 2023/12/19 end*/

	/*N19A code for HQ-353621 by p-xielihui at 2023/12/26 start*/
	mutex_lock(&ctx->panel_lock);
	/*N19A code for HQ-353621 by p-xielihui at 2023/12/26 end*/
	/*N19A code for HQ-372911 by p-luocong1 at 2024/3/4 start*/
	/*N19A code for HQ-381364 by p-xielihui at 2024/4/10 start*/
	if (level_in == 0 || last_bl == 0 || last_bl == level_in) {
		bl_en0[1] = CLOSE_DIMMING;
		need_enable_dimming_flag = true;
		cb(dsi, handle, bl_en0, ARRAY_SIZE(bl_en0));
	} else if (level > 0 && need_enable_dimming_flag) {
		bl_en0[1] = OPEN_DIMMING;
		need_enable_dimming_flag = false;
		cb(dsi, handle, bl_en0, ARRAY_SIZE(bl_en0));
	}

	last_bl = level_in;
	/*N19A code for HQ-381363 by p-xielihui at 2024/4/10 end*/
	/*N19A code for HQ-372911 by p-luocong1 at 2024/3/4 end*/
	cb(dsi, handle, bl_tb0, ARRAY_SIZE(bl_tb0));
	/*N19A code for HQ-353621 by p-xielihui at 2023/12/26 start*/
	mutex_unlock(&ctx->panel_lock);
	/*N19A code for HQ-353621 by p-xielihui at 2023/12/26 end*/
	return 0;
}

static struct drm_display_mode *get_mode_by_id(struct drm_connector *connector,
	unsigned int mode)
{
	struct drm_display_mode *m;
	unsigned int i = 0;

	pr_info("[kernel/lcm]%s enter\n", __func__);

	list_for_each_entry(m, &connector->modes, head) {
		if (i == mode)
			return m;
		i++;
	}
	pr_info("[kernel/lcm]%s get mode fail\n", __func__);
	return NULL;
}

static int mtk_panel_ext_param_set(struct drm_panel *panel,
			 struct drm_connector *connector, unsigned int mode)
{
	struct mtk_panel_ext *ext = find_panel_ext(panel);
	int ret = 0;
	struct drm_display_mode *m = get_mode_by_id(connector, mode);

	if (m == NULL) {
		pr_info("[kernel/lcm]%s:%d invalid display_mode\n", __func__, __LINE__);
		return -1;
	}
	/*N19A code for HQ-366831 by p-luocong1 at 2024/1/15 start*/
	if (drm_mode_vrefresh(m) == MODE_0_FPS)
		ext->params = &ext_params;
	else if (drm_mode_vrefresh(m) == MODE_1_FPS)
		ext->params = &ext_params_mode_1;
	else if (drm_mode_vrefresh(m) == MODE_2_FPS)
		ext->params = &ext_params_mode_2;
	else if (drm_mode_vrefresh(m) == MODE_3_FPS)
		ext->params = &ext_params_mode_3;
	else
		ret = 1;
	/*N19A code for HQ-366831 by p-luocong1 at 2024/1/15 end*/

	if (!ret)
		current_fps = drm_mode_vrefresh(m);
	return ret;
}

static int panel_ext_reset(struct drm_panel *panel, int on)
{
	struct panel *ctx = panel_to_lcm(panel);

	pr_info("[kernel/lcm]%s enter\n", __func__);

	ctx->reset_gpio =
		devm_gpiod_get(ctx->dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->reset_gpio)) {
		dev_info(ctx->dev, "%s: cannot get reset_gpio %ld\n",
			__func__, PTR_ERR(ctx->reset_gpio));
		return PTR_ERR(ctx->reset_gpio);
	}
	gpiod_set_value(ctx->reset_gpio, on);
	devm_gpiod_put(ctx->dev, ctx->reset_gpio);

	return 0;
}

static int panel_ata_check(struct drm_panel *panel)
{
#ifdef BDG_PORTING_DBG
	struct lcm *ctx = panel_to_lcm(panel);
	struct mipi_dsi_device *dsi = to_mipi_dsi_device(ctx->dev);
	unsigned char data[3] = {0x00, 0x00, 0x00};
	unsigned char id[3] = {0x6e, 0x48, 0x00};
	int ret = 0;

	ret = mipi_dsi_dcs_read(dsi, 0x4, data, 3);
	if (ret < 0) {
		pr_info("[kernel/lcm]%s error\n", __func__);
		return 0;
	}

	pr_info("ATA read data %x %x %x\n", data[0], data[1], data[2]);

	if (data[0] == id[0])
		return 1;

	pr_info("ATA expect read data is %x %x %x\n",
			id[0], id[1], id[2]);
#endif
	return 1;
}

/*N19A code for HQ-353621 by p-xielihui at 2023/12/26 start*/
static int panel_get_panel_info(struct drm_panel *panel, char *buf)
{
	int count = 0;
	struct panel *ctx;

	if (!panel) {
		pr_err(": panel is NULL\n", __func__);
		return -EAGAIN;
	}

	ctx = panel_to_lcm(panel);
	count = snprintf(buf, PAGE_SIZE, "%s\n", ctx->panel_info);

	return count;
}

static int panel_normal_hbm_control(struct drm_panel *panel, uint32_t level)
{
	struct panel *ctx;

	if (!panel) {
		pr_err("invalid params\n");
		return -EAGAIN;
	}

	ctx = panel_to_lcm(panel);

	mutex_lock(&ctx->panel_lock);
	if (level == 1) {
		lcm_dcs_write_seq_static(ctx, 0x51, 0x0F, 0xFF);
	} else if (level == 0) {
		lcm_dcs_write_seq_static(ctx, 0x51, 0x07, 0xFF);
	}
	mutex_unlock(&ctx->panel_lock);

	return 0;
}

static int lcm_setbacklight_control(struct drm_panel *panel, unsigned int level)
{
	struct panel *ctx;

	if (!panel) {
		pr_err("invalid params\n");
		return -EAGAIN;
	}

	ctx = panel_to_lcm(panel);

	pr_info("lcm_setbacklight_control backlight %d\n", level);

	return 0;
}

static bool get_lcm_initialized(struct drm_panel *panel)
{
	bool ret = false;

	struct panel *ctx;

	if (!panel) {
		pr_err("invalid params\n");
		return -EAGAIN;
	}

	ctx = panel_to_lcm(panel);
	ret = ctx->prepared;
	return ret;
}
/*N19A code for HQ-353621 by p-xielihui at 2023/12/26 end*/

/*N19A code for HQ-348461 by p-xielihui at 2024/1/11 start*/
static int panel_set_cabc_mode(void *dsi, dcs_write_gce cb,
	void *handle, u32 mode)
{
	struct panel *ctx = panel_to_lcm(this_panel);
	char cabc_state[] = {CABC_CTRL_REG, mode};

	pr_info("[kernel/lcm]%s enter, set cabc_mode: %d\n", __func__, mode);

	/*N19A code for HQ-371527 by p-xielihui at 2024/2/22 start*/
	if (!ctx) {
		pr_err("invalid params\n");
		return -ENODEV;
	}

	if (!cb){
		pr_err("[kernel/lcm] cb error");
		return -ENODEV;
	}

	switch (mode)
	{
		case DDIC_CABC_UI_ON:
		case DDIC_CABC_STILL_ON:
		case DDIC_CABC_MOVIE_ON:
			cabc_state[1] = mode & 0xFF;
			ctx->cabc_status = mode;
			break;
		default:
			cabc_state[1] = 0x00;
			ctx->cabc_status = DDIC_CABC_OFF;
			break;
	}
	/*N19A code for HQ-371527 by p-xielihui at 2024/2/22 end*/

	mutex_lock(&ctx->panel_lock);
	cb(dsi, handle, cabc_state, ARRAY_SIZE(cabc_state));
	mutex_unlock(&ctx->panel_lock);

	pr_info("[kernel/lcm]%s exit, final cabc_status is: %d\n", __func__, ctx->cabc_status);

	return 0;
}

static int panel_get_cabc_mode(struct drm_panel *panel, int *mode)
{
	struct panel *ctx;

	if (!panel) {
		pr_err("invalid params\n");
		return -EAGAIN;
	}

	ctx = panel_to_lcm(panel);
	*mode = ctx->cabc_status;

	pr_info("[kernel/lcm]%s enter, cabc_mode: %d\n", __func__, ctx->cabc_status);

	return 0;
}
/*N19A code for HQ-348461 by p-xielihui at 2024/1/11 end*/

static struct mtk_panel_funcs ext_funcs = {
	.set_backlight_cmdq = panel_setbacklight_cmdq,
	.reset = panel_ext_reset,
	.ext_param_set = mtk_panel_ext_param_set,
	.ata_check = panel_ata_check,
	/*N19A code for HQ-353621 by p-xielihui at 2023/12/26 start*/
	.get_panel_info = panel_get_panel_info,
	.normal_hbm_control = panel_normal_hbm_control,
	.setbacklight_control = lcm_setbacklight_control,
	.get_panel_initialized = get_lcm_initialized,
	/*N19A code for HQ-353621 by p-xielihui at 2023/12/26 end*/
	/*N19A code for HQ-348461 by p-xielihui at 2024/1/11 start*/
	.set_cabc_mode = panel_set_cabc_mode,
	.get_cabc_mode = panel_get_cabc_mode,
	/*N19A code for HQ-348461 by p-xielihui at 2024/1/11 end*/
};
#endif

/*N19A code for HQ-366831 by p-luocong1 at 2024/1/15 start*/
static int panel_get_modes(struct drm_panel *panel,
			struct drm_connector *connector)
{
	struct drm_display_mode *mode;
	struct drm_display_mode *mode_1;
	struct drm_display_mode *mode_2;
	struct drm_display_mode *mode_3;
	static int mode_count = 0;

	pr_info("[kernel/lcm]%s enter\n", __func__);

	mode = drm_mode_duplicate(connector->dev, &default_mode);
	if (!mode) {
		dev_info(connector->dev->dev, "failed to add mode %ux%ux@%u\n",
			default_mode.hdisplay, default_mode.vdisplay,
			drm_mode_vrefresh(&default_mode));
		return -ENOMEM;
	}
	drm_mode_set_name(mode);
	mode->type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED;
	drm_mode_probed_add(connector, mode);
	mode_count++;

	mode_1 = drm_mode_duplicate(connector->dev, &performance_mode_1);
	if (!mode_1) {
		dev_info(connector->dev->dev, "failed to add mode %ux%ux@%u\n",
			 performance_mode_1.hdisplay,
			 performance_mode_1.vdisplay,
			 drm_mode_vrefresh(&performance_mode_1));
		return -ENOMEM;
	}
	drm_mode_set_name(mode_1);
	mode_1->type = DRM_MODE_TYPE_DRIVER;
	drm_mode_probed_add(connector, mode_1);
	mode_count++;

	mode_2 = drm_mode_duplicate(connector->dev, &performance_mode_2);
	if (!mode_2) {
		dev_info(connector->dev->dev, "failed to add mode %ux%ux@%u\n",
			 performance_mode_2.hdisplay,
			 performance_mode_2.vdisplay,
			 drm_mode_vrefresh(&performance_mode_2));
		return -ENOMEM;
	}
	drm_mode_set_name(mode_2);
	mode_2->type = DRM_MODE_TYPE_DRIVER;
	drm_mode_probed_add(connector, mode_2);
	mode_count++;

	mode_3 = drm_mode_duplicate(connector->dev, &performance_mode_3);
	if (!mode_3) {
		dev_info(connector->dev->dev, "failed to add mode %ux%ux@%u\n",
			 performance_mode_3.hdisplay,
			 performance_mode_3.vdisplay,
			 drm_mode_vrefresh(&performance_mode_3));
		return -ENOMEM;
	}
	drm_mode_set_name(mode_3);
	mode_3->type = DRM_MODE_TYPE_DRIVER;
	drm_mode_probed_add(connector, mode_3);
	mode_count++;

	connector->display_info.width_mm = 64;
	connector->display_info.height_mm = 129;

	return mode_count;
}
/*N19A code for HQ-366831 by p-luocong1 at 2024/1/15 end*/

static const struct drm_panel_funcs panel_drm_funcs = {
	.disable = panel_disable,
	.unprepare = panel_unprepare,
	.prepare = panel_prepare,
	.enable = panel_enable,
	.get_modes = panel_get_modes,
};

static void check_is_bdg_support(struct device *dev)
{
	unsigned int ret = 0;

	ret = of_property_read_u32(dev->of_node, "bdg-support", &bdg_support);
	if (!ret && bdg_support == 1) {
		pr_info("kernel/lcm]%s, bdg support 1", __func__);
	} else {
		pr_info("kernel/lcm]%s, bdg support 0", __func__);
		bdg_support = 0;
	}
}

/*N19A code for HQ-348450 by p-xielihui at 2024/2/4 start*/
extern int panel_event;
static ssize_t panel_event_show(struct device *device,
			struct device_attribute *attr,
			char *buf)
{
	ssize_t ret = 0;
	struct drm_connector *connector = dev_get_drvdata(device);
	if (!connector) {
		pr_info("%s-%d connector is NULL \r\n",__func__, __LINE__);
		return ret;
	}
	return snprintf(buf, PAGE_SIZE, "%d\n", panel_event);
}

static DEVICE_ATTR_RO(panel_event);

static struct attribute *panel2_attrs[] = {
	&dev_attr_panel_event.attr,
        NULL,
};

static const struct attribute_group panel2_attr_group = {
        .attrs = panel2_attrs,
};
/*N19A code for HQ-348450 by p-xielihui at 2024/2/4 end*/

/*N19A code for HQ-369142 by p-xielihui at 2024/1/29 start*/
extern unsigned int mi_get_panel_id(void);
/*N19A code for HQ-369142 by p-xielihui at 2024/1/29 end*/
static int panel_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct panel *ctx;
	int ret = 0;

	/*N19A code for HQ-369142 by p-xielihui at 2024/1/29 start*/
	panel_id = mi_get_panel_id();
	if ((panel_id == FT8725_HUAXING_PRIVATE) || (panel_id == FT8725_HUAXING_COMMON))
		pr_info("[%s]: It is panel_n19a_42_03_0b, panel_id = 0x%x\n", __func__, panel_id);
	else {
		pr_info("[%s]: It is not panel_n19a_42_03_0b, panel_id = 0x%x\n", __func__, panel_id);
		return -ENODEV;
	}
	/*N19A code for HQ-369142 by p-xielihui at 2024/1/29 end*/

	ctx = devm_kzalloc(dev, sizeof(struct panel), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	mipi_dsi_set_drvdata(dsi, ctx);

	ctx->dev = dev;
	dsi->lanes = 4;
	dsi->format = MIPI_DSI_FMT_RGB888;
	/*N19A code for HQ-358349 by p-xielihui at 2023/12/15 start*/
	dsi->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_SYNC_PULSE;
	/*N19A code for HQ-358349 by p-xielihui at 2023/12/15 end*/

	ctx->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->reset_gpio)) {
		dev_info(dev, "cannot get reset-gpios %ld\n",
			PTR_ERR(ctx->reset_gpio));
		return PTR_ERR(ctx->reset_gpio);
	}
	devm_gpiod_put(dev, ctx->reset_gpio);

	ctx->avee_gpio = devm_gpiod_get(dev, "avee", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->avee_gpio)) {
		dev_info(dev, "cannot get avee_gpios %ld\n",
			PTR_ERR(ctx->avee_gpio));
		return PTR_ERR(ctx->avee_gpio);
	}
	devm_gpiod_put(dev, ctx->avee_gpio);

	ctx->avdd_gpio = devm_gpiod_get(dev, "avdd", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->avdd_gpio)) {
		dev_info(dev, "cannot get avdd_gpios %ld\n",
			PTR_ERR(ctx->avdd_gpio));
		return PTR_ERR(ctx->avdd_gpio);
	}
	devm_gpiod_put(dev, ctx->avdd_gpio);

	ctx->prepared = true;
	ctx->enabled = true;

	/*N19A code for HQ-353621 by p-xielihui at 2023/12/26 start*/
	ctx->panel_info = panel_name;
	/*N19A code for HQ-353621 by p-xielihui at 2023/12/26 end*/

	drm_panel_init(&ctx->panel, dev, &panel_drm_funcs, DRM_MODE_CONNECTOR_DSI);

	drm_panel_add(&ctx->panel);

	ret = mipi_dsi_attach(dsi);
	if (ret < 0)
		drm_panel_remove(&ctx->panel);

	check_is_bdg_support(dev);
#if defined(CONFIG_MTK_PANEL_EXT)
	mtk_panel_tch_handle_reg(&ctx->panel);
	ret = mtk_panel_ext_create(dev, &ext_params, &ext_funcs, &ctx->panel);
	if (ret < 0)
		return ret;
#endif
	/*N19A code for HQ-353621 by p-xielihui at 2023/12/26 start*/
	mutex_init(&ctx->panel_lock);
	this_panel = &ctx->panel;
	/*N19A code for HQ-353621 by p-xielihui at 2023/12/26 end*/
	/*N19A code for HQ-358314 by p-xielihui at 2023/12/20 start*/
	hq_regiser_hw_info(HWID_LCM, "incell, vendor:42, IC:03");
	/*N19A code for HQ-358314 by p-xielihui at 2023/12/20 end*/

	/*N19A code for HQ-348461 by p-xielihui at 2024/1/11 start*/
	ctx->cabc_status = DDIC_CABC_OFF;
	/*N19A code for HQ-348461 by p-xielihui at 2024/1/11 end*/

	/*N19A code for HQ-348450 by p-xielihui at 2024/2/4 start*/
	ret = sysfs_create_group(&dev->kobj, &panel2_attr_group);
	if (ret)
		return ret;
	/*N19A code for HQ-348450 by p-xielihui at 2024/2/4 end*/

	pr_info("[kernel/lcm] %s exit\n", __func__);

	return ret;
}

static int panel_remove(struct mipi_dsi_device *dsi)
{
	struct panel *ctx = mipi_dsi_get_drvdata(dsi);
#if defined(CONFIG_MTK_PANEL_EXT)
	struct mtk_panel_ctx *ext_ctx = find_panel_ctx(&ctx->panel);
#endif
	mipi_dsi_detach(dsi);
	drm_panel_remove(&ctx->panel);
#if defined(CONFIG_MTK_PANEL_EXT)
	mtk_panel_detach(ext_ctx);
	mtk_panel_remove(ext_ctx);
#endif
	return 0;
}

static const struct of_device_id panel_of_match[] = {
	{
		.compatible = "n19a-42-03-0b-dsc-vdo,lcm",
	},
	{} };

MODULE_DEVICE_TABLE(of, panel_of_match);

static struct mipi_dsi_driver panel_driver = {
	.probe = panel_probe,
	.remove = panel_remove,
	.driver = {
		.name = "panel-n19a-42-03-0b-dsc-vdo",
		.owner = THIS_MODULE,
		.of_match_table = panel_of_match,
	},
};

module_mipi_dsi_driver(panel_driver);

MODULE_DESCRIPTION("huaxing ft8725 vdo 90hz 6382 Panel Driver");
MODULE_LICENSE("GPL v2");
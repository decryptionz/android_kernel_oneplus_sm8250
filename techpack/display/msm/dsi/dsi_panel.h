/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2016-2020, The Linux Foundation. All rights reserved.
 */

#ifndef _DSI_PANEL_H_
#define _DSI_PANEL_H_

#include <linux/of_device.h>
#include <linux/types.h>
#include <linux/bitops.h>
#include <linux/errno.h>
#include <linux/backlight.h>
#include <drm/drm_panel.h>
#include <drm/msm_drm.h>

#include "dsi_defs.h"
#include "dsi_ctrl_hw.h"
#include "dsi_clk.h"
#include "dsi_pwr.h"
#include "dsi_parser.h"
#include "msm_drv.h"

#define MAX_BL_LEVEL 4096
#define MAX_BL_SCALE_LEVEL 1024
#define MAX_SV_BL_SCALE_LEVEL 65535
#define DSI_CMD_PPS_SIZE 135

#define DSI_MODE_MAX 32

#define EVT2_113MHZ_OSC 0x99
#define PVT_113MHZ_OSC 0x10
#define PVT_113MHZ_OSC_XTALK 0x11

#define GAMMA_READ_SUCCESS 1
#define GAMMA_READ_ERROR 0

extern u32 mode_fps;
extern int gamma_read_flag;
extern int tp_1v8_power;
extern char dimming_gamma_60hz[30];
extern char dimming_gamma_120hz[15];

enum dsi_gamma_cmd_set_type {
	DSI_GAMMA_CMD_SET_SWITCH_60HZ = 0,
	DSI_GAMMA_CMD_SET_SWITCH_90HZ,
	DSI_GAMMA_CMD_SET_MAX
};
/*
 * Defining custom dsi msg flag,
 * continued from drm_mipi_dsi.h
 * Override to use async transfer
 */
#define MIPI_DSI_MSG_ASYNC_OVERRIDE BIT(4)

enum dsi_panel_rotation {
	DSI_PANEL_ROTATE_NONE = 0,
	DSI_PANEL_ROTATE_HV_FLIP,
	DSI_PANEL_ROTATE_H_FLIP,
	DSI_PANEL_ROTATE_V_FLIP
};

enum dsi_backlight_type {
	DSI_BACKLIGHT_PWM = 0,
	DSI_BACKLIGHT_WLED,
	DSI_BACKLIGHT_DCS,
	DSI_BACKLIGHT_EXTERNAL,
	DSI_BACKLIGHT_UNKNOWN,
	DSI_BACKLIGHT_MAX,
};

enum bl_update_flag {
	BL_UPDATE_DELAY_UNTIL_FIRST_FRAME,
	BL_UPDATE_NONE,
};

enum {
	MODE_GPIO_NOT_VALID = 0,
	MODE_SEL_DUAL_PORT,
	MODE_SEL_SINGLE_PORT,
	MODE_GPIO_HIGH,
	MODE_GPIO_LOW,
};

enum dsi_dms_mode {
	DSI_DMS_MODE_DISABLED = 0,
	DSI_DMS_MODE_RES_SWITCH_IMMEDIATE,
};

enum dsi_panel_physical_type {
	DSI_DISPLAY_PANEL_TYPE_LCD = 0,
	DSI_DISPLAY_PANEL_TYPE_OLED,
	DSI_DISPLAY_PANEL_TYPE_MAX,
};

struct dsi_dfps_capabilities {
	enum dsi_dfps_type type;
	u32 min_refresh_rate;
	u32 max_refresh_rate;
	u32 *dfps_list;
	u32 dfps_list_len;
	bool dfps_support;
};

struct dsi_dyn_clk_caps {
	bool dyn_clk_support;
	u32 *bit_clk_list;
	u32 bit_clk_list_len;
	enum dsi_dyn_clk_feature_type type;
	bool maintain_const_fps;
};

struct dsi_pinctrl_info {
	struct pinctrl *pinctrl;
	struct pinctrl_state *active;
	struct pinctrl_state *suspend;
};

struct dsi_panel_phy_props {
	u32 panel_width_mm;
	u32 panel_height_mm;
	enum dsi_panel_rotation rotation;
};

struct dsi_backlight_config {
	enum dsi_backlight_type type;
	enum bl_update_flag bl_update;

	u32 bl_min_level;
	u32 bl_max_level;
	u32 brightness_max_level;
	u32 bl_level;
	u32 bl_scale;
	u32 bl_scale_sv;
	bool bl_inverted_dbv;

	int en_gpio;
	/* PWM params */
	struct pwm_device *pwm_bl;
	bool pwm_enabled;
	u32 pwm_period_usecs;

	bool bl_high2bit;
	u32 bl_def_val;

	/* WLED params */
	struct led_trigger *wled;
	struct backlight_device *raw_bd;
};

struct dsi_reset_seq {
	u32 level;
	u32 sleep_ms;
};

struct dsi_panel_reset_config {
	struct dsi_reset_seq *sequence;
	u32 count;

	int reset_gpio;
	int disp_en_gpio;
	int lcd_mode_sel_gpio;
	u32 mode_sel_state;
};

enum esd_check_status_mode {
	ESD_MODE_REG_READ,
	ESD_MODE_SW_BTA,
	ESD_MODE_PANEL_TE,
	ESD_MODE_SW_SIM_SUCCESS,
	ESD_MODE_SW_SIM_FAILURE,
	ESD_MODE_MAX
};

struct drm_panel_esd_config {
	bool esd_enabled;

	enum esd_check_status_mode status_mode;
	struct dsi_panel_cmd_set status_cmd;
	u32 *status_cmds_rlen;
	u32 *status_valid_params;
	u32 *status_value;
	u8 *return_buf;
	u8 *status_buf;
	u32 groups;
};

struct dsi_panel {
	const char *name;
	const char *type;
	struct device_node *panel_of_node;
	struct mipi_dsi_device mipi_device;

	struct mutex panel_lock;
	struct drm_panel drm_panel;
	struct mipi_dsi_host *host;
	struct device *parent;

	struct dsi_host_common_cfg host_config;
	struct dsi_video_engine_cfg video_config;
	struct dsi_cmd_engine_cfg cmd_config;
	enum dsi_op_mode panel_mode;
	bool panel_mode_switch_enabled;

	struct dsi_dfps_capabilities dfps_caps;
	struct dsi_dyn_clk_caps dyn_clk_caps;
	struct dsi_panel_phy_props phy_props;

	struct dsi_display_mode *cur_mode;
	u32 num_timing_nodes;
	u32 num_display_modes;

	struct dsi_regulator_info power_info;
	struct dsi_backlight_config bl_config;
	struct dsi_panel_reset_config reset_config;
	struct dsi_pinctrl_info pinctrl;
	struct drm_panel_hdr_properties hdr_props;
	struct drm_panel_esd_config esd_config;

	struct dsi_parser_utils utils;

	char buf_id[32];
	int panel_year;
	int panel_mon;
	int panel_day;
	int panel_hour;
	int panel_min;
	int panel_sec;
	int panel_msec;
	int panel_msec_int;
	int panel_msec_rem;
	int panel_year_index;
	int panel_mon_index;
	int panel_day_index;
	int panel_hour_index;
	int panel_min_index;
	int panel_sec_index;
	int panel_msec_high_index;
	int panel_msec_low_index;
	u64 panel_serial_number;
	int panel_code_info;
	int panel_stage_info;
	int panel_production_info;
	int panel_ic_v;
	char buf_select[10];
	int panel_tool;
	int ddic_x;
	int ddic_y;
	int ddic_check_info;

	int hbm_backlight;
	int hbm_brightness;
	int hbm_mode;
	bool is_hbm_enabled;
	int op_force_screenfp;
	bool dim_status;

	int aod_mode;
	int aod_status;
	int aod_curr_mode;
	int aod_disable;

	int seed_lp_mode;
	int acl_mode;
	int acl_cmd_index;
	int acl_mode_index;
	int srgb_mode;
	int dci_p3_mode;
	int night_mode;
	int oneplus_mode;
	int adaption_mode;
	int naive_display_p3_mode;
	int naive_display_wide_color_mode;
	int naive_display_srgb_color_mode;
	int naive_display_loading_effect_mode;
	int naive_display_customer_srgb_mode;
	int naive_display_customer_p3_mode;
	int mca_setting_mode;
	struct delayed_work gamma_read_work;

	int status_value;
	int panel_mismatch_check;
	int panel_mismatch;
	bool panel_switch_status;
	bool need_power_on_backlight;

	int poc;
	int vddr_gpio;
	int vddd_gpio;
	int tp1v8_gpio;
	int err_flag_gpio;
	bool is_err_flag_irq_enabled;
	bool err_flag_status;

	bool lp11_init;
	bool ulps_feature_enabled;
	bool ulps_suspend_enabled;
	bool allow_phy_power_off;
	bool reset_gpio_always_on;
	atomic_t esd_recovery_pending;

	bool panel_initialized;
	bool te_using_watchdog_timer;
	u32 qsync_min_fps;

	char dsc_pps_cmd[DSI_CMD_PPS_SIZE];
	enum dsi_dms_mode dms_mode;

	bool sync_broadcast_en;
	int power_mode;

	int panel_test_gpio;
#if defined(CONFIG_PXLW_IRIS)
	bool is_secondary;
#endif
	enum dsi_panel_physical_type panel_type;
};

static inline bool dsi_panel_ulps_feature_enabled(struct dsi_panel *panel)
{
	return panel->ulps_feature_enabled;
}

static inline bool dsi_panel_initialized(struct dsi_panel *panel)
{
	return panel->panel_initialized;
}

static inline void dsi_panel_acquire_panel_lock(struct dsi_panel *panel)
{
	mutex_lock(&panel->panel_lock);
}

static inline void dsi_panel_release_panel_lock(struct dsi_panel *panel)
{
	mutex_unlock(&panel->panel_lock);
}

static inline bool dsi_panel_is_type_oled(struct dsi_panel *panel)
{
	return (panel->panel_type == DSI_DISPLAY_PANEL_TYPE_OLED);
}

struct dsi_panel *dsi_panel_get(struct device *parent,
				struct device_node *of_node,
				struct device_node *parser_node,
				const char *type,
				int topology_override);

int dsi_panel_trigger_esd_attack(struct dsi_panel *panel);

void dsi_panel_put(struct dsi_panel *panel);

int dsi_panel_drv_init(struct dsi_panel *panel, struct mipi_dsi_host *host);

int dsi_panel_drv_deinit(struct dsi_panel *panel);

int dsi_panel_get_mode_count(struct dsi_panel *panel);

void dsi_panel_put_mode(struct dsi_display_mode *mode);

int dsi_panel_get_mode(struct dsi_panel *panel,
		       u32 index,
		       struct dsi_display_mode *mode,
		       int topology_override);

int dsi_panel_validate_mode(struct dsi_panel *panel,
			    struct dsi_display_mode *mode);

int dsi_panel_get_host_cfg_for_mode(struct dsi_panel *panel,
				    struct dsi_display_mode *mode,
				    struct dsi_host_config *config);

int dsi_panel_get_phy_props(struct dsi_panel *panel,
			    struct dsi_panel_phy_props *phy_props);
int dsi_panel_get_dfps_caps(struct dsi_panel *panel,
			    struct dsi_dfps_capabilities *dfps_caps);

int dsi_panel_pre_prepare(struct dsi_panel *panel);

int dsi_panel_set_lp1(struct dsi_panel *panel);

int dsi_panel_set_lp2(struct dsi_panel *panel);

int dsi_panel_set_nolp(struct dsi_panel *panel);

int dsi_panel_prepare(struct dsi_panel *panel);

int dsi_panel_enable(struct dsi_panel *panel);

int dsi_panel_post_enable(struct dsi_panel *panel);

int dsi_panel_pre_disable(struct dsi_panel *panel);

int dsi_panel_disable(struct dsi_panel *panel);

int dsi_panel_unprepare(struct dsi_panel *panel);

int dsi_panel_post_unprepare(struct dsi_panel *panel);

int dsi_panel_set_backlight(struct dsi_panel *panel, u32 bl_lvl);

int dsi_panel_update_pps(struct dsi_panel *panel);

int dsi_panel_send_qsync_on_dcs(struct dsi_panel *panel,
		int ctrl_idx);
int dsi_panel_send_qsync_off_dcs(struct dsi_panel *panel,
		int ctrl_idx);

int dsi_panel_send_roi_dcs(struct dsi_panel *panel, int ctrl_idx,
		struct dsi_rect *roi);

int dsi_panel_pre_mode_switch_to_video(struct dsi_panel *panel);
int dsi_panel_pre_mode_switch_to_cmd(struct dsi_panel *panel);
int dsi_panel_mode_switch_to_cmd(struct dsi_panel *panel);
int dsi_panel_mode_switch_to_vid(struct dsi_panel *panel);

int dsi_panel_switch(struct dsi_panel *panel);

int dsi_panel_post_switch(struct dsi_panel *panel);

void dsi_dsc_pclk_param_calc(struct msm_display_dsc_info *dsc, int intf_width);

void dsi_panel_bl_handoff(struct dsi_panel *panel);

struct dsi_panel *dsi_panel_ext_bridge_get(struct device *parent,
				struct device_node *of_node,
				int topology_override);

int dsi_panel_parse_esd_reg_read_configs(struct dsi_panel *panel);

void dsi_panel_ext_bridge_put(struct dsi_panel *panel);
int dsi_panel_set_hbm_mode(struct dsi_panel *panel, int level);
int dsi_panel_set_acl_mode(struct dsi_panel *panel, int level);
int dsi_panel_set_hbm_brightness(struct dsi_panel *panel, int level);
int dsi_panel_op_set_hbm_mode(struct dsi_panel *panel, int level);
void dsi_panel_calc_dsi_transfer_time(struct dsi_host_common_cfg *config,
		struct dsi_display_mode *mode, u32 frame_threshold_us);
extern int drm_panel_notifier_call_chain(struct drm_panel *panel,
	unsigned long val, void *v);
int dsi_panel_set_aod_mode(struct dsi_panel *panel, int level);
int dsi_panel_set_dci_p3_mode(struct dsi_panel *panel, int level);
int dsi_panel_set_night_mode(struct dsi_panel *panel, int level);
int dsi_panel_set_native_display_p3_mode(struct dsi_panel *panel, int level);
int dsi_panel_set_native_display_wide_color_mode(struct dsi_panel *panel, int level);
int dsi_panel_set_native_display_srgb_color_mode(struct dsi_panel *panel, int level);
int dsi_panel_set_native_loading_effect_mode(struct dsi_panel *panel, int level);
int dsi_panel_gamma_read_address_setting(struct dsi_panel *panel, u16 read_number);
int dsi_panel_tx_cmd_set(struct dsi_panel *panel, enum dsi_cmd_set_type type);
int dsi_panel_parse_gamma_cmd_sets(void);
int dsi_panel_tx_gamma_cmd_set(struct dsi_panel *panel, enum dsi_gamma_cmd_set_type type);
extern int mipi_dsi_dcs_write_c1(struct mipi_dsi_device *dsi, u16 read_number);
int dsi_panel_update_cmd_sets_sub(struct dsi_panel_cmd_set *cmd,
					enum dsi_cmd_set_type type, const char *data, unsigned int length);
int dsi_panel_send_dsi_panel_command(struct dsi_panel *panel);
int dsi_panel_update_dsi_seed_command(struct dsi_cmd_desc *cmds,
					enum dsi_cmd_set_type type, const char *data);
int dsi_panel_send_dsi_seed_command(struct dsi_panel *panel);
int dsi_panel_set_customer_srgb_mode(struct dsi_panel *panel, int level);
int dsi_panel_set_customer_p3_mode(struct dsi_panel *panel, int level);
int dsi_panel_set_seed_lp_mode(struct dsi_panel *panel, int seed_lp_level);
int dsi_panel_set_mca_setting_mode(struct dsi_panel *panel, int mca_setting_mode);
void dsi_panel_update_gamma_change_write(void);
int dsi_panel_dimming_gamma_write(struct dsi_panel *panel);
#endif /* _DSI_PANEL_H_ */

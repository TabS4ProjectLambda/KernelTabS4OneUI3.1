/*
 *  jack_wcd-mbhc-v2_sysfs_cb.c
 *
 *  Copyright (c) Samsung Electronics
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include <linux/input.h>
#include <sound/jack.h>
#include <sound/soc.h>
#include <sound/samsung/sec_audio_sysfs.h>
#include "jack_wcd-mbhc-v2_sysfs_cb.h"
#ifdef CONFIG_SND_SOC_WCD_MBHC_CCIC_ADAPTOR_JACK_DET
#include "../codecs/wcd-mbhc-v2-usbc.h"
#else
#include "../codecs/wcd-mbhc-v2.h"
#endif
#include "../codecs/wcd934x/wcd934x.h"
#include "../codecs/wcd934x/wcd934x-mbhc.h"

static struct snd_soc_codec *wcd_codec;

static int get_jack_status(void)
{
	struct snd_soc_codec *codec = wcd_codec;
	struct wcd934x_mbhc *wcd934x_mbhc = tavil_soc_get_mbhc(codec);
	struct wcd_mbhc *mbhc = &wcd934x_mbhc->wcd_mbhc;
	int value = 0;

	if ((mbhc->hph_status == SND_JACK_HEADSET) ||
	    (mbhc->hph_status == SND_JACK_HEADPHONE))
		value = 1;

	dev_info(codec->dev, "%s: %d\n", __func__, value);

	return value;
}

static int get_key_status(void)
{
	struct snd_soc_codec *codec = wcd_codec;
	struct wcd934x_mbhc *wcd934x_mbhc = tavil_soc_get_mbhc(codec);
	struct wcd_mbhc *mbhc = &wcd934x_mbhc->wcd_mbhc;
	int value = -1;

	if (mbhc->buttons_pressed)
		value = mbhc->mbhc_cb->map_btn_code_to_num(mbhc->codec);

	dev_info(codec->dev, "%s: %d\n", __func__, value);

	return value;
}

void register_mbhc_jack_cb(struct snd_soc_codec *codec)
{
	wcd_codec = codec;

	audio_register_jack_state_cb(get_jack_status);
	audio_register_key_state_cb(get_key_status);
#ifndef CONFIG_SND_SOC_WCD_MBHC
	audio_register_mic_adc_cb(get_key_status);
#endif
}
EXPORT_SYMBOL_GPL(register_mbhc_jack_cb);


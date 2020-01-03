/*******************************************************************************
	Copyright SmartAction Tech. 2015.
	All Rights Reserved.
	
	File: sa_sound_dapm.h

	Description:

	TIME LIST:
	CREATE By Ringsd   2015/12/11 12:10:25

*******************************************************************************/

#ifndef _sa_sound_dapm_h_
#define _sa_sound_dapm_h_

#ifdef __cplusplus
extern "C" {
#endif

const char* event_to_ename(int event);

#define DEFINE_POWER_CTRL(name)                                                 \
                                                                                \
static int name##_power_flag = 0;                                               \
                                                                                \
static int name##_power_get(struct snd_kcontrol *kcontrol,                      \
		struct snd_ctl_elem_value *ucontrol)                                    \
{                                                                               \
	ucontrol->value.integer.value[0] = name##_power_flag;                       \
                                                                                \
	return 0;                                                                   \
}                                                                               \
                                                                                \
static int name##_power_put(struct snd_kcontrol *kcontrol,                      \
		struct snd_ctl_elem_value *ucontrol)                                    \
{                                                                               \
	name##_power_flag = ucontrol->value.integer.value[0];                       \
	return 0;                                                                   \
}                                                                               \
                                                                                \
static int name##_power_connected(struct snd_soc_dapm_widget *source,           \
			 struct snd_soc_dapm_widget *sink)                                  \
{                                                                               \
    return name##_power_flag;                                                   \
}                                                                               \


#define DEFINE_MUTE_CTRL(name)                                                  \
                                                                                \
static int name##_mute_flag = 0;                                                \
                                                                                \
static int name##_mute_get(struct snd_kcontrol *kcontrol,                       \
		struct snd_ctl_elem_value *ucontrol)                                    \
{                                                                               \
	ucontrol->value.integer.value[0] = name##_mute_flag;                        \
	return 0;                                                                   \
}                                                                               \
                                                                                \
static int name##_mute_put(struct snd_kcontrol *kcontrol,                       \
		struct snd_ctl_elem_value *ucontrol)                                    \
{                                                                               \
	name##_mute_flag = ucontrol->value.integer.value[0];                        \
    sa_sound_dapm_mute(#name, name##_mute_flag);                                \
	return 0;                                                                   \
}                                                                               \
                                                                                \
static int name##_mute_connected(struct snd_soc_dapm_widget *source,            \
			 struct snd_soc_dapm_widget *sink)                                  \
{                                                                               \
    return name##_mute_flag;                                                    \
}                                                                               \

#ifdef __cplusplus
}
#endif

#endif

/*******************************************************************************
	END OF FILE
*******************************************************************************/

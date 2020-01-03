/*
 * linux/drivers/pwm/jz-pwm/pwm-dev.c
 *
 * simple PWM based LED or beeper control
 * Platform driver support for Ingenic X1000 SoC.
 *
 * Copyright 2016, <qiuwei.wang@ingenic.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/of_platform.h>
#include <linux/fb.h>
#include <linux/leds.h>
#include <linux/err.h>
#include <linux/pwm.h>
#include <linux/leds_pwm.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <mach/jz_pwm.h>


struct jz_pwm_data {
	struct jz_pwm_classdev cdev;
	struct pwm_device *pwm;
	struct work_struct work;

	unsigned int period;
	unsigned int duty;
	bool can_sleep;
};

struct jz_pwm_priv {
	int num_devs;
	struct jz_pwm_data devs[0];
};


static void _jz_pwm_set(struct jz_pwm_data *pwm_dat)
{
	unsigned int new_duty = pwm_dat->duty;

	pwm_config(pwm_dat->pwm, new_duty, pwm_dat->period);

	if (new_duty == 0)
		pwm_disable(pwm_dat->pwm);
	else
		pwm_enable(pwm_dat->pwm);
}

static void jz_pwm_dutyratio_set(struct jz_pwm_classdev *cdev, unsigned int dutyratio)
{
	struct jz_pwm_data *pwm_dat = \
			container_of(cdev, struct jz_pwm_data, cdev);
	unsigned int max = cdev->max_dutyratio;

	pwm_dat->period = cdev->period_ns;
	pwm_dat->duty = dutyratio * pwm_dat->period / max;

	if(pwm_dat->can_sleep)
		schedule_work(&pwm_dat->work);
	else
		_jz_pwm_set(pwm_dat);
}

static void jz_pwm_active_level_set(struct jz_pwm_classdev *cdev)
{
	struct jz_pwm_data *pwm_dat = \
			container_of(cdev, struct jz_pwm_data, cdev);

	pwm_dat->pwm->active_level = cdev->active_level;
}

static void jz_pwm_update(struct jz_pwm_classdev *cdev)
{
	struct jz_pwm_data *pwm_dat = \
			container_of(cdev, struct jz_pwm_data, cdev);

	cdev->period_ns = pwm_dat->period;
}

static void jz_pwm_enable(struct jz_pwm_classdev *cdev)
{
	struct jz_pwm_data *pwm_dat = \
			container_of(cdev, struct jz_pwm_data, cdev);

	if(cdev->enable)
		pwm_enable(pwm_dat->pwm);
	else
		pwm_disable(pwm_dat->pwm);
}

static void jz_pwm_dev_work(struct work_struct *work)
{
	struct jz_pwm_data *pwm_dat = \
			container_of(work, struct jz_pwm_data, work);

	_jz_pwm_set(pwm_dat);
}

static inline size_t sizeof_jz_pwm_priv(int num_leds)
{
	return sizeof(struct jz_pwm_priv) +
		      (sizeof(struct jz_pwm_data) * num_leds);
}

static struct jz_pwm_priv *jz_pwm_dev_creat_of(struct platform_device *pdev)
{
	struct device_node *node = pdev->dev.of_node;
	struct device_node *child;
	struct jz_pwm_priv *priv;
	int count, ret;

	/* count device number, so we know how much to allocate */
	count = of_get_child_count(node);
	if(!count)
		return NULL;

	priv = devm_kzalloc(&pdev->dev, sizeof_jz_pwm_priv(count), GFP_KERNEL);
	if(!priv)
		return NULL;

	for_each_child_of_node(node, child) {
		struct jz_pwm_data *pwm_dat = &priv->devs[priv->num_devs];

		pwm_dat->cdev.name = of_get_property(child, "label", NULL) ? : child->name;
		pwm_dat->pwm = devm_of_pwm_get(&pdev->dev, child, NULL);
		if(IS_ERR(pwm_dat->pwm)) {
			dev_err(&pdev->dev, "unable to request PWM for %s\n",
			pwm_dat->cdev.name);
			goto err;
		}

		/* Get the period from PWM core when n */
		pwm_dat->period = pwm_get_period(pwm_dat->pwm);
		pwm_dat->cdev.active_level_set = jz_pwm_active_level_set;
		pwm_dat->cdev.dutyratio_set = jz_pwm_dutyratio_set;
		pwm_dat->cdev.pwm_update    = jz_pwm_update;
		pwm_dat->cdev.pwm_enable    = jz_pwm_enable;
		pwm_dat->cdev.dutyratio     = PWM_OFF;
		pwm_dat->cdev.enable        = 0;
		pwm_dat->cdev.flags        |= JZ_PWM_DEV_CORE_SUSPENDRESUME;
		pwm_dat->can_sleep = pwm_can_sleep(pwm_dat->pwm);
		if(pwm_dat->can_sleep)
			INIT_WORK(&pwm_dat->work, jz_pwm_dev_work);

		ret = jz_pwm_classdev_register(&pdev->dev, &pwm_dat->cdev);
		if(ret < 0) {
			dev_err(&pdev->dev, "failed to register for %s\n",
			pwm_dat->cdev.name);
			of_node_put(child);
			goto err;
		}

		priv->num_devs++;
	}

	return priv;
err:
	while (priv->num_devs--)
		jz_pwm_classdev_unregister(&priv->devs[priv->num_devs].cdev);

	return NULL;
}

static int jz_pwm_dev_probe(struct platform_device *pdev)
{
	struct jz_pwm_platform_data *pdata = pdev->dev.platform_data;
	struct jz_pwm_priv *priv;
	int i, ret = 0;

	if(pdata && pdata->num_devs) {
		priv = devm_kzalloc(&pdev->dev,
							sizeof_jz_pwm_priv(pdata->num_devs),
							GFP_KERNEL);
		if(!priv)
			return -ENOMEM;

		for(i = 0; i < pdata->num_devs; i++) {
			struct jz_pwm_dev *cur_dev = &pdata->devs[i];
			struct jz_pwm_data *pwm_dat = &priv->devs[i];

			pwm_dat->pwm = devm_pwm_get(&pdev->dev, cur_dev->name);
			if(IS_ERR(pwm_dat->pwm)) {
				/* devm_pwm_get failed, trying legacy API */
				pwm_dat->pwm = pwm_request(cur_dev->pwm_id, cur_dev->name);
				if(IS_ERR(pwm_dat->pwm)) {
					ret = PTR_ERR(pwm_dat->pwm);
					dev_err(&pdev->dev,
							"unable to request PWM for %s\n",
							cur_dev->name);
					goto err;
				}
			}

			pwm_dat->cdev.name          = cur_dev->name;
			pwm_dat->cdev.active_level_set = jz_pwm_active_level_set;
			pwm_dat->cdev.dutyratio_set = jz_pwm_dutyratio_set;
			pwm_dat->cdev.pwm_update    = jz_pwm_update;
			pwm_dat->cdev.pwm_enable    = jz_pwm_enable;
			pwm_dat->cdev.dutyratio     = cur_dev->dutyratio;
			pwm_dat->cdev.max_dutyratio = cur_dev->max_dutyratio;
			pwm_dat->cdev.period_ns     = cur_dev->period_ns;
			pwm_dat->cdev.enable        = 0;
			pwm_dat->cdev.active_level  = !cur_dev->active_low;
			pwm_dat->cdev.flags        |= JZ_PWM_DEV_CORE_SUSPENDRESUME;

			pwm_dat->period     = cur_dev->period_ns;
			pwm_dat->can_sleep  = pwm_can_sleep(pwm_dat->pwm);
			if(pwm_dat->can_sleep)
				INIT_WORK(&pwm_dat->work, jz_pwm_dev_work);

			ret = jz_pwm_classdev_register(&pdev->dev, &pwm_dat->cdev);
			if(ret < 0) {
				dev_err(&pdev->dev, "unable to register classdev for %s\n", cur_dev->name);
				goto err;
			}

			pwm_dat->pwm->active_level = !cur_dev->active_low;

			if(pwm_dat->cdev.dutyratio)
				pwm_dat->duty = pwm_dat->cdev.dutyratio * pwm_dat->cdev.period_ns / pwm_dat->cdev.max_dutyratio;

			_jz_pwm_set(pwm_dat);
		}
		priv->num_devs = pdata->num_devs;
	} else {
		priv = jz_pwm_dev_creat_of(pdev);
		if(!priv)
			return -ENODEV;
	}

	platform_set_drvdata(pdev, priv);

	return 0;

err:
	while(i--)
		jz_pwm_classdev_unregister(&priv->devs[i].cdev);

	return ret;
}

static int jz_pwm_dev_remove(struct platform_device *pdev)
{
	struct jz_pwm_priv *priv = platform_get_drvdata(pdev);
	int i;

	for(i = 0; i < priv->num_devs; i++) {
		jz_pwm_classdev_unregister(&priv->devs[i].cdev);
		if(priv->devs[i].can_sleep)
			cancel_work_sync(&priv->devs[i].work);
	}

	return 0;
}

static const struct of_device_id of_jz_pwm_dev_match[] = {
	{ .compatible = "pwm-devs", },
	{},
};
MODULE_DEVICE_TABLE(of, of_jz_pwm_dev_match);

static struct platform_driver jz_pwm_dev_driver = {
	.probe     = jz_pwm_dev_probe,
	.remove    = jz_pwm_dev_remove,
	.driver    = {
		.name  = "jz-pwm-dev",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(of_jz_pwm_dev_match),
	},
};
module_platform_driver(jz_pwm_dev_driver);

MODULE_AUTHOR("Qiuwei Wang <qiuwei.wang@ingenic.com>");
MODULE_DESCRIPTION("JZ PWM DEVS driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:jz-pwm-dev");

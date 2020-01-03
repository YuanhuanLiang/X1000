#if defined (CONFIG_USE_OCV)
static void axp_charging_monitor(struct work_struct *work)
{
	struct axp_charger *charger;
	uint8_t val;

	charger = container_of(work, struct axp_charger, work.work);
	axp_charger_update_state(charger);
	axp_charger_update(charger);
	/* 更新电池信息 */
#if defined (CONFIG_KP_AXP20)
	/* 更新电池开路电压 */
	axp_reads(charger->master,AXP_OCV_BUFFER0,2,v);
	charger->ocv = ((v[0] << 4) + (v[1] & 0x0f)) * 11 /10 ;
#endif
#if defined (CONFIG_KP_AXP19)
	/* 更新电池开路电压及其对应百分比 */
	val = axp_ocv_restcap(charger);
#endif
	
#if defined (CONFIG_KP_AXP20)	
	/* usb 限流 */
	axp_usb_curlim(USBCURLIM);
#endif
	
	if(charge_index >= TIMER4){
		charger->disvbat = 0;
		charger->disibat = 0;
		charge_index = 0;
	}		
	if(charger->ext_valid == 1)
		charge_index ++;
	else 
		charge_index = 0;
	
	schedule_delayed_work(&charger->work, charger->interval);
}

static void axp_battery_infoinit(struct axp_charger *charger)
{
}
static int axp_suspend(struct platform_device *dev, pm_message_t state)
{
	struct axp_charger *charger = platform_get_drvdata(dev);
	if(charger->bat_det == 1)
		cancel_delayed_work_sync(&charger->work);

	/* 清irq */
	axp_clear_irq(charger);

	/* 关闭irq */
#if defined (CONFIG_KP_USEIRQ)
	//axp_unregister_notifier(charger->master, &charger->nb, AXP_NOTIFIER_ON);
#endif
	
	/* 设置休眠充电电流 */
#if defined (CONFIG_AXP_CHGCURCHG)
	axp_set_chargecurrent(charger->chgsuscur);
#endif

	/* 设置阈值时间为127分钟 */
	axp_write(charger->master, AXP_TIMER_CTL, 0x80);
	axp_write(charger->master, AXP_TIMER_CTL, 0x7F);

	return 0;
}

static int axp_resume(struct platform_device *dev)
{
	struct axp_charger *charger = platform_get_drvdata(dev);

	uint8_t val;
	/* 打开irq */
#if defined (CONFIG_KP_USEIRQ)
	//axp_register_notifier(charger->master, &charger->nb, AXP_NOTIFIER_ON );
#endif

	axp_charger_update_state(charger);
	axp_charger_update(charger);
	if(charger->bat_det == 1){
		
	} else {
		charger->rest_cap = 100;
		charger->ocv = 0;
		power_supply_changed(&charger->batt);
	}
#if defined (CONFIG_KP_AXP20)
	/* 休眠低电关机接口 */
	axp_read(charger->master, AXP_DATA_BUFFERA,&val);
	printk("[AXP]suspend power off val = 0x%x\n",val);
	if(val){
		axp_close(charger);
		axp_set_bits(charger->master,AXP_INTSTS5,0x03);
	}
	axp_write(charger->master, AXP_DATA_BUFFERA,0x00);
#endif

	/* 设置开机充电电流 */
#if defined (CONFIG_AXP_CHGCURCHG)
	axp_set_chargecurrent(charger->chgcur);
#endif

	if(charger->bat_det == 1)
		schedule_delayed_work(&charger->work, charger->interval);

	return 0;
}
#endif

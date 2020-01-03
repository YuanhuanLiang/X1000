/*******************************************************************************
	Copyright SmartAction Tech. 2015.
	All Rights Reserved.

	File: sa_sound_switch.c

	Description:

	TIME LIST:
	CREATE By Ringsd   2015/12/03 14:39:49

*******************************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/irq.h>
#include <linux/notifier.h>
#include <linux/gpio.h>
#include <linux/slab.h>

#include <sound/sa_sound_switch.h>
//#include <plat/comip-pmic.h>
#if 0
#define printk_debug    printk
#else
#define printk_debug(...)
#endif

//extern void earpods_wire_set_switch(int on);

static void sa_switch_schedule(struct sa_sound_switch *psa_sound_switch)
{
    int i;
    struct sa_sound_switch_item* item;
    
    item = psa_sound_switch->item_list;
    for( i=0; i<psa_sound_switch->item_count; i++ )
    {
        if( item->detect )
        {
            item->detect( psa_sound_switch, item );
        }
        item ++;
    }

    item = psa_sound_switch->item_list;
    for( i=0; i<psa_sound_switch->item_count; i++ )
    {
        if( item->report )
        {
            item->report( psa_sound_switch, item );
        }
        item ++;
    }
}

static void sa_switch_work_schedule(struct work_struct *work)
{
	struct sa_sound_switch *psa_sound_switch;
    
	psa_sound_switch = container_of((struct delayed_work *)work, struct sa_sound_switch, switch_work);
    sa_switch_schedule(psa_sound_switch);
    schedule_delayed_work(&psa_sound_switch->switch_work, psa_sound_switch->interval);
    
    return;
}

static ssize_t sa_sound_print_name(struct switch_dev *sdev, char *buf)
{
    return sprintf(buf, "%s\n", sdev->name);
}

int pmic_get_adc_conversion(int gpio_index)
{
    int adc_value = 0;
#ifdef CONFIG_KP_AXP
    axp_get_adc_value(gpio_index, &adc_value);
#endif
    printk_debug("index:%d,adc val:%d\n",gpio_index, adc_value);
    return adc_value;
}

static int sa_switch_init( struct sa_sound_switch* psa_sound_switch, struct sa_sound_switch_item* item )
{
    int ret = 0;
    int id = SA_SOUND_SWITCH_ID(item->switch_id);
    if( item->sdev_name )
    {
        item->sdev.name = item->sdev_name;
        item->sdev.print_name = sa_sound_print_name;
        ret = switch_dev_register(&item->sdev);
        if (ret < 0)
        {
            printk("switch_dev_register %s fail\n", item->sdev_name);
            return -1;
        }

        //if the item->switch_id is gpio0 ,this code will error.
        if( SA_SOUND_SWITCH_IS_GPIO(item->switch_id) )
        {
            gpio_request(id, item->name);
            gpio_direction_input(id);
        }
        else if(SA_SOUND_SWITCH_IS_PMIC(item->switch_id)){
#ifdef CONFIG_KP_AXP
            axp_init_gpio_adc(id);
#endif
        }

        return 0;
    }
    else
    {
        return -1;
    }
}

static void sa_switch_done( struct sa_sound_switch* psa_sound_switch, struct sa_sound_switch_item* item )
{
    switch_dev_unregister(&item->sdev);
    return;
}

static int sa_switch_detect( struct sa_sound_switch* psa_sound_switch, struct sa_sound_switch_item* item )
{
    int id = SA_SOUND_SWITCH_ID(item->switch_id);
    //adc
    if( SA_SOUND_SWITCH_IS_PMIC(item->switch_id) )
    {
#ifdef CONFIG_KP_AXP
        item->result = pmic_get_adc_conversion( id );
        //printk_debug( "sa_switch_detect adc[%d] = %d\n", id, item->result );
#endif
        return 0;
    }
    //gpio
    else if( SA_SOUND_SWITCH_IS_GPIO(item->switch_id) )
    {
        item->result = gpio_get_value( id );
        //printk_debug( "sa_switch_detect gpio[%d] = %d\n", id, item->result );
    }
    return 0;
}

#define MAX_COUNT 3
static int sa_switch_report( struct sa_sound_switch* psa_sound_switch, struct sa_sound_switch_item* item )
{
    if( SA_SOUND_SWITCH_IS_PMIC(item->switch_id) )
    {
        if( item->result >= item->voltage_min && item->result < item->voltage_max )
        {
            item->new_status = item->switch_status;
        }
        else
        {
            item->new_status = !item->switch_status;
        }
    }
    //gpio
    else if( SA_SOUND_SWITCH_IS_GPIO(item->switch_id) )
    {
        if( item->result )
        {
            item->new_status = item->switch_status;
        }
        else
        {
            item->new_status = !item->switch_status;
        }
    }

    if( item->new_status != item->old_status )
    {
        item->detect_count++;

        if(item->detect_count >= MAX_COUNT){
            printk_debug( "%s:%d\n", item->name, item->new_status );
            switch_set_state(&item->sdev, item->new_status);
            item->old_status = item->new_status;
        }
    }else if(item->detect_count)
        item->detect_count = 0;
    return 0;
}

void refine_switch_item(struct sa_sound_switch_item *item)
{
    if(!item->init){
        item->init = sa_switch_init;
    }
    if(!item->detect){
        item->detect = sa_switch_detect;
    }
    if(!item->report){
        item->report = sa_switch_report;
    }
    if(!item->done){
        item->done = sa_switch_done;
    }
}
static int sa_sound_switch_probe(struct platform_device *pdev)
{
    int i = 0;
    struct sa_sound_switch_item *item;
    struct sa_sound_switch *psa_sound_switch = (struct sa_sound_switch *)pdev->dev.platform_data;
    if(psa_sound_switch->item_count == 0){
        printk("switch item not exist!\n");
        return -1;
    }
    
    item = psa_sound_switch->item_list;
    for(i=0; i<psa_sound_switch->item_count; i++){
        refine_switch_item(item);
        if( item->init( psa_sound_switch, item ) < 0 ){
            printk( "sa sound switch[%d][%s] init fail\n", i, item->name );
        }
        item ++;
    }

    psa_sound_switch->interval = msecs_to_jiffies(500);

    INIT_DELAYED_WORK(&psa_sound_switch->switch_work, sa_switch_work_schedule);
    schedule_delayed_work(&psa_sound_switch->switch_work, psa_sound_switch->interval);
    
    printk( "sa_sound_switch_init\n" );
    return 0;
}

static int sa_sound_switch_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver sa_sound_switch_driver = {
	.probe  = sa_sound_switch_probe,
	.remove = sa_sound_switch_remove,
	.driver = {
		.name = "sa_sound_switch",
		.owner = THIS_MODULE,
	},
};

static int __init sa_sound_switch_module_init(void)
{
	return platform_driver_register(&sa_sound_switch_driver);
}

static void __exit sa_sound_switch_module_exit(void)
{
	platform_driver_unregister(&sa_sound_switch_driver);
}

module_init(sa_sound_switch_module_init);
module_exit(sa_sound_switch_module_exit);

/* Module information */
MODULE_AUTHOR("ringsd");
MODULE_DESCRIPTION("SmartAction Sound Switch");
MODULE_LICENSE("GPL");
/*******************************************************************************
	END OF FILE
*******************************************************************************/

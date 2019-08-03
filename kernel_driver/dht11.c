/*

 * Simple kernel driver to get temperature and humidity from hardware
 * Copyright (C) 2019  Kayas Ahmed

 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *   Losely based on leds-bcm6328.c (for understanding this was refered)
******************************************************************
*               Revision Detail
*  | Date           | Comment                     | Author       |
*  | 25.juni.2019:- | First version  		  | Kayas Ahmed  |
*  | 3.Aug.2019:-   | Ioctl Added  		  | Kayas Ahmed  |
*
*
*
*/
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/miscdevice.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/fs.h>
#include <linux/of.h>        

#define REG_CTRL        0x00
#define REG_DEBUG       0x04
#define REG_TEMPERATURE 0x08 
#define REG_HUMIDITY    0x0c
#define REG_CRC         0x10
#define REG_ACK         0x14
        
	
#define ENABLE_DEVICE _IOR('a','a',int32_t*)
#define DISABLE_DEVICE _IOR('a','b',int32_t*)
/*@struct dht11
 dhtstatus(32 bit status)
 *
 *
 */


/*
 * Idea here is to develop a generic driver for FPGA peripherals
 * Here it is for DHT11 which can also to extended for other devices
 * dht11 sturct is very specific to my implementation in hardware, this can be changes according to your setup in hardware
 * Any doubts on this mail @ kayasdev@gmail.com
 **@strucrt dht11
 ***@dht11_status (32 bit register)
 **
 **
 **
 */

struct dht_sensor_data
{
	uint32_t ctrl;
	uint32_t debug;
	uint32_t temp;
	uint32_t humidity;
	uint32_t crc;
};
struct dht11_dev
{
	void  *regs;
        struct miscdevice mdev;
	spinlock_t lock;
	struct platform_device *pdev;
	struct dht_sensor_data dht_data;
	struct tasklet_struct tasklet;
	

};

/*
 * This part of code does probing and gets device register address from device tree \
 * and saves it to our device structure
 * cool ryt :)
 *
 *
 */

void dht11_write(void __iomem *reg, uint32_t off,uint32_t val)
{
	iowrite32(val,  reg+off);

}
uint32_t dht11_read(void __iomem *reg, uint32_t off)
{
	return ioread32(reg+off);

}

static inline struct dht11_dev *to_my_struct(struct file *file)
{
	struct miscdevice *miscdev = file->private_data;

    return container_of(miscdev, struct dht11_dev, mdev);
}

static void dht11_do_tasklet(unsigned long data)
{
	struct dht11_dev *priv= (struct dht11_dev *)data;
	dev_info(&priv->pdev->dev, "Tasklet : \n");
	//spin_lock(&priv->lock);
	//priv->dht_data.=dht11_read(priv->regs,REG_STATUS);
	priv->dht_data.debug=dht11_read(priv->regs,REG_DEBUG);
	priv->dht_data.temp=dht11_read(priv->regs,REG_TEMPERATURE);
	priv->dht_data.humidity=dht11_read(priv->regs,REG_HUMIDITY);
	priv->dht_data.crc=dht11_read(priv->regs,REG_CRC);
	//spin_unlock(&priv->lock);
	
}

static irqreturn_t
dht11_handle_irq(int irq, void *ctx)
{

struct dht11_dev *priv=ctx;


	unsigned long flags;
	dev_info(&priv->pdev->dev, "IRQ : \n");
	spin_lock_irqsave(&priv->lock, flags);
	//dht11_do_tasklet(ctx);
	tasklet_schedule(&priv->tasklet);
	spin_unlock_irqrestore(&priv->lock, flags);
        dht11_write(priv->regs,REG_ACK,1);
	return IRQ_HANDLED;

}
static int dht11_open(struct inode *inode, struct file *file)
{
        return 0;
}

static ssize_t dht_read(struct file *file, char __user *buf, size_t count,loff_t *pos)
{
    struct  dht11_dev  *priv = to_my_struct(file); 
    int ret;
	unsigned long flags;

	spin_lock_irqsave(&priv->lock, flags);
	//spin_lock(&priv->lock);
	ret=copy_to_user(buf,&priv->dht_data,sizeof(priv->dht_data));
	spin_unlock_irqrestore(&priv->lock, flags);
	//spin_unlock(&priv->lock);
    return ret;
}

static long
dht11_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
        struct dht11_dev *priv = to_my_struct(file);

        switch (cmd) {
       case ENABLE_DEVICE:
       		dht11_write(priv->regs,REG_CTRL,1);
		dev_info(&priv->pdev->dev, "Device enabled\n");
		break;
       case DISABLE_DEVICE:
       		dht11_write(priv->regs,REG_CTRL,0);
		dev_info(&priv->pdev->dev, "Device Disabled \n");
		break;
	}
	return 0;
}
static const struct file_operations my_fops = {
    .owner  = THIS_MODULE,
    .read   = dht_read,
    .open   = dht11_open,
    .unlocked_ioctl =   dht11_ioctl,
};

static int dht11_probe(struct platform_device *pdev)
{
	int  ret, irq;
	struct device_node *np = pdev->dev.of_node;
	struct resource *mem_r;
	struct dht11_dev *priv;
	void __iomem *mem;
	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (priv==NULL)
	{
	 return -EINVAL;
	}
	// gets the reg property from device tree
	priv->pdev=pdev;
	mem_r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	dev_info(&pdev->dev, "Probing the dht11 devvice\n");
	if (!mem_r)
		return -EINVAL;
	
	//requests iomap region pointed by mem_r
	mem = devm_ioremap_resource(&pdev->dev, mem_r);
	if (IS_ERR(mem))
		return PTR_ERR(mem);
	priv->regs=mem;
	//mutual exclusive use
	//lock = devm_kzalloc(&pdev->dev, sizeof(*lock), GFP_KERNEL);
	
	//priv->lock=lock;
	tasklet_init(&priv->tasklet, dht11_do_tasklet,(unsigned long)priv);
//	if (!lock)
//		return -ENOMEM;	
        irq = platform_get_irq(pdev, 0);
        if (irq < 0) {
                dev_err(&pdev->dev, "no IRQ defined: %d\n", irq);
                return irq;
        }
	dev_info(&pdev->dev, "IRQ received: %d\n", irq);
	 ret = request_irq(irq,dht11_handle_irq,IRQF_TRIGGER_HIGH, "DH11", priv);
        //ret = devm_request_irq(&pdev->dev, irq, dht11_handle_irq,
         //                      0, "DH11", priv);
        if (ret < 0) {
                dev_err(&pdev->dev, "request_irq failed\n");
                return ret;
        }
 

        platform_set_drvdata(pdev, priv);
	
	priv->mdev.minor  = 200;
    	priv->mdev.name   = "DH11";
    	priv->mdev.fops   = &my_fops;
    	priv->mdev.parent = NULL;

    	ret = misc_register(&priv->mdev);
    	if (ret) {
        	dev_err(&pdev->dev, "Failed to register miscdev\n");
        	return ret;
    	}

    	dev_info(&pdev->dev, "Registered\n");
	return 0;

}
static int dht11_remove(struct platform_device *pdev)
{

struct dht11_dev *priv=platform_get_drvdata(pdev);
	misc_deregister(&priv->mdev);
	


}
static const struct of_device_id dht11_of_match[] = {
	{ .compatible = "kayas,dht11", },
	{ },
};
MODULE_DEVICE_TABLE(of, dht11_of_match);

 static struct platform_driver dht11_driver = {
	.probe = dht11_probe,
	.remove  = dht11_remove,
	.driver = {
		.name = "DHT",
		.of_match_table = of_match_ptr(dht11_of_match),
	},
};
#ifdef LOAD_ENABLED 
int init_module(void)
{
	printk(KERN_ALERT "I am in init_module\n");
	platform_driver_register(&dht11_driver);
	return 0;
}

void cleanup_module(void)
{
	platform_driver_unregister(&dht11_driver);
	printk(KERN_ALERT "LED_driver!!, Returning from cleanup\n");
}
#else
module_platform_driver(dht11_driver);
#endif


MODULE_AUTHOR("kayas Ahmed <kayasdev@gmail.com>");
MODULE_DESCRIPTION("Driver for DHT controllers implemented in FPGA(Zynq7000)");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:DHT");

/***************************************************************************//**
*  \file       i2c-sgp30.c
*
*  \details    sgp30 i2c driver
*
*  \author     g8row
*
* *******************************************************************************/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>

#define I2C_BUS_AVAILABLE   (          1 )              // I2C Bus available in our Raspberry Pi
#define SLAVE_DEVICE_NAME   ( 	 "SGP30" )              // Device and Driver Name
#define SGP30_SLAVE_ADDR    (       0x58 )              // sgp30 Slave Address

static struct i2c_adapter *sgp30_i2c_adapter     = NULL;  // I2C Adapter Structure
static struct i2c_client  *sgp30_i2c_client      = NULL;  // I2C Client Structure

/*
** This function writes the data into the I2C client
**
**  Arguments:
**      buff -> buffer to be sent
**      len  -> Length of the data
**   
*/
static int I2C_Write(unsigned char *buf, unsigned int len)
{
    /*
    ** Sending Start condition, Slave address with R/W bit, 
    ** ACK/NACK and Stop condtions will be handled internally.
    */ 
    int ret = i2c_master_send(sgp30_i2c_client, buf, len);
    
    return ret;
}

/*
** This function reads one byte of the data from the I2C client
**
**  Arguments:
**      out_buff -> buffer wherer the data to be copied
**      len      -> Length of the data to be read
** 
*/
static int I2C_Read(unsigned char *out_buf, unsigned int len)
{
    /*
    ** Sending Start condition, Slave address with R/W bit, 
    ** ACK/NACK and Stop condtions will be handled internally.
    */ 
    int ret = i2c_master_recv(sgp30_i2c_client, out_buf, len);
    
    return ret;
}


static void SGP30_Write(unsigned char command, unsigned char data)
{
    unsigned char buf[2] = {0};
    int ret;
   
    buf[0] = command; 
    buf[1] = data;
    
    ret = I2C_Write(buf, 2);
}

static int SGP30_Init(void)
{
	pr_info("entered init");
	msleep(100);               // delay
    SGP30_Write(0x20,0x03); 
	pr_info("exit init");
    return 0;
}

struct sgp30_measurement {
	int co2;
	int tvoc;
};

static struct sgp30_measurement SGP30_Measure(void)
{
	unsigned char buf[6];
	struct sgp30_measurement s;

	SGP30_Write(0x20,0x08);
	I2C_Read(buf, 6);
	s.co2 = (buf[0]<<8)|buf[1];
       	s.tvoc = (buf[3]<<8)|buf[4];
	return s;
}

/*
** proc_fs
*/

static ssize_t sgp30_read(struct file *File, char *user_buffer, size_t count, loff_t *offs) {
        struct sgp30_measurement s = SGP30_Measure();
	return sprintf(user_buffer, "%d CO2, %d TVOC\n", s.co2, s.tvoc);
}

static struct proc_dir_entry *proc_file;

static struct proc_ops fops = {
	.proc_read = sgp30_read,
};

/*
** This function getting called when the slave has been found
** Note : This will be called only once when we load the driver.
*/
static int sgp30_probe(struct i2c_client *client,
                         const struct i2c_device_id *id)
{

    pr_info("sgp30 init started\n");
    if(client->addr != SGP30_SLAVE_ADDR) {
        printk("dt_i2c - Wrong I2C address!\n");
	return -1;
    }

    sgp30_i2c_client = client;
    proc_file = proc_create("sgp30", 0666, NULL, &fops);
    if(proc_file == NULL) {
	printk("dt_i2c - Error creating /proc/myadc\n");
	return -ENOMEM;
    }
    SGP30_Init();
    pr_info("sgp30 initialized and probed!!!\n");
    return 0;
}

/*
** This function getting called when the slave has been removed
** Note : This will be called only once when we unload the driver.
*/
static void sgp30_remove(struct i2c_client *client)
{    
    proc_remove(proc_file);	
    pr_info("sgp30 removed\n");
}

/*
** Structure that has slave device id
*/
static const struct i2c_device_id sgp30_id[] = {
        { SLAVE_DEVICE_NAME, 0 },
        { }
};
MODULE_DEVICE_TABLE(i2c, sgp30_id);

static struct of_device_id sgp30_driver_ids[] = {
	{
		.compatible = "sgp30",
	}, { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, sgp30_driver_ids);

/*
** I2C driver Structure that has to be added to linux
*/
static struct i2c_driver sgp30_driver = {
        .driver = {
            .name   = SLAVE_DEVICE_NAME,
	    .of_match_table = sgp30_driver_ids,
        },
        .probe          = sgp30_probe,
        .remove         = sgp30_remove,
        .id_table       = sgp30_id,
};

/*
** I2C Board Info strucutre
*/
static struct i2c_board_info sgp30_board_info = {
        I2C_BOARD_INFO(SLAVE_DEVICE_NAME, SGP30_SLAVE_ADDR)
    };

module_i2c_driver(sgp30_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("g8row <alexander.k.gurov@gmail.com>");
MODULE_DESCRIPTION("sgp30 i2c kernel driver");
MODULE_VERSION("0.0.1");

#include "xiaomi_touch.h"

static struct xiaomi_touch_pdata *touch_pdata;

static int xiaomi_touch_dev_open(struct inode *inode, struct file *file)
{
	struct xiaomi_touch *dev = NULL;
	int i = MINOR(inode->i_rdev);
	struct xiaomi_touch_pdata *touch_pdata;

	printk("%s\n", __func__);
	dev = xiaomi_touch_dev_get(i);
	if (!dev) {
		printk("%s cant get dev\n", __func__);
		return -ENOMEM;
	}
	touch_pdata = dev_get_drvdata(dev->dev);

	file->private_data = touch_pdata;
	return 0;
}

static ssize_t xiaomi_touch_dev_read(struct file *file, char __user *buf,
			   size_t count, loff_t *pos)
{
	return 0;
}

static ssize_t xiaomi_touch_dev_write(struct file *file,
		const char __user *buf, size_t count, loff_t *pos)
{
	return 0;
}

static unsigned int xiaomi_touch_dev_poll(struct file *file,
		poll_table *wait)
{
	return 0;
}

static long xiaomi_touch_dev_ioctl(struct file *file, unsigned int cmd,
		unsigned long arg)
{
	int ret = -EINVAL;
	int buf[VALUE_TYPE_SIZE] = {0,};
	struct xiaomi_touch_pdata *pdata = file->private_data;
	void __user *argp = (void __user *) arg;
	struct xiaomi_touch_interface *touch_data = pdata->touch_data;
	struct xiaomi_touch *dev = pdata->device;
	int user_cmd = _IOC_NR(cmd);

	if (!pdata || !touch_data || !dev) {
		printk("%s invalid memory\n", __func__);
		return -ENOMEM;
	}

	mutex_lock(&dev->mutex);
	ret = copy_from_user(&buf, (int __user *)argp, sizeof(buf));

	printk("%s cmd:%d, mode:%d, value:%d\n", __func__, user_cmd, buf[0], buf[1]);

	switch (user_cmd) {
	case SET_CUR_VALUE:
		if (touch_data->setModeValue)
			buf[0] = touch_data->setModeValue(buf[0], buf[1]);
		break;
	case GET_CUR_VALUE:
	    if (touch_data->getModeCurValue)
			buf[0] = touch_data->getModeCurValue(buf[0]);
		break;
	case GET_DEF_VALUE:
		if (touch_data->getModeValue)
			buf[0] = touch_data->getModeValue(buf[0], user_cmd);
		break;
	case GET_MIN_VALUE:
		if (touch_data->getModeValue)
			buf[0] = touch_data->getModeValue(buf[0], user_cmd);
		break;
	case GET_MAX_VALUE:
		if (touch_data->getModeValue)
			buf[0] = touch_data->getModeValue(buf[0], user_cmd);
		break;
	case RESET_MODE:
		if (touch_data->resetMode)
			buf[0] = touch_data->resetMode(buf[0]);
		break;
	case GET_MODE_VALUE:
		if (touch_data->getModeValue)
			ret = touch_data->getModeAll(buf[0], buf);
		break;
	default:
		printk("%s don't support mode\n", __func__);
		ret = -EINVAL;
		break;
	}

	if (ret >= 0)
		ret = copy_to_user((int __user *)argp, &buf, sizeof(buf));
	else
		printk("%s can't get data from touch driver\n", __func__);

	mutex_unlock(&dev->mutex);

	return ret;
}

static int xiaomi_touch_dev_release(struct inode *inode, struct file *file)
{
	return 0;
}

static const struct file_operations xiaomitouch_dev_fops = {
	.owner = THIS_MODULE,
	.open = xiaomi_touch_dev_open,
	.read = xiaomi_touch_dev_read,
	.write = xiaomi_touch_dev_write,
	.poll = xiaomi_touch_dev_poll,
	.unlocked_ioctl = xiaomi_touch_dev_ioctl,
	.compat_ioctl = xiaomi_touch_dev_ioctl,
	.release = xiaomi_touch_dev_release,
	.llseek	= no_llseek,
};

static struct xiaomi_touch xiaomi_touch_dev = {
	.misc_dev = {
		.minor = MISC_DYNAMIC_MINOR,
		.name = "xiaomi-touch",
		.fops = &xiaomitouch_dev_fops,
		.parent = NULL,
	},
	.mutex = __MUTEX_INITIALIZER(xiaomi_touch_dev.mutex),
	.palm_mutex = __MUTEX_INITIALIZER(xiaomi_touch_dev.palm_mutex),
	.psensor_mutex = __MUTEX_INITIALIZER(xiaomi_touch_dev.psensor_mutex),
	.wait_queue = __WAIT_QUEUE_HEAD_INITIALIZER(xiaomi_touch_dev.wait_queue),
};

struct xiaomi_touch *xiaomi_touch_dev_get(int minor)
{
	if (xiaomi_touch_dev.misc_dev.minor == minor)
		return &xiaomi_touch_dev;
	else
		return NULL;
}
EXPORT_SYMBOL_GPL(xiaomi_touch_dev_get);

struct class *get_xiaomi_touch_class()
{
	return xiaomi_touch_dev.class;
}
EXPORT_SYMBOL_GPL(get_xiaomi_touch_class);

struct device *get_xiaomi_touch_dev()
{
	return xiaomi_touch_dev.dev;
}
EXPORT_SYMBOL_GPL(get_xiaomi_touch_dev);

int xiaomitouch_register_modedata(struct xiaomi_touch_interface *data)
{
	int ret = 0;
	struct xiaomi_touch_interface *touch_data = NULL;

	if (!touch_pdata)
		ret = -ENOMEM;

	touch_data = touch_pdata->touch_data;
	printk("%s\n", __func__);

	mutex_lock(&xiaomi_touch_dev.mutex);

	touch_data->setModeValue = data->setModeValue;
	touch_data->getModeValue = data->getModeValue;
	touch_data->resetMode = data->resetMode;
	touch_data->getModeAll = data->getModeAll;
	touch_data->getModeCurValue = data->getModeCurValue;
	touch_data->palm_sensor_read = data->palm_sensor_read;
	touch_data->palm_sensor_write = data->palm_sensor_write;
/* N19A code for HQ-370325 by p-huangyunbiao at 2024/2/2 start */
	touch_data->panel_vendor_read = data->panel_vendor_read;
	touch_data->panel_color_read = data->panel_color_read;
	touch_data->panel_display_read = data->panel_display_read;
	touch_data->touch_vendor_read = data->touch_vendor_read;
/* N19A code for HQ-370325 by p-huangyunbiao at 2024/2/2 end */

	mutex_unlock(&xiaomi_touch_dev.mutex);

	return ret;
}
EXPORT_SYMBOL_GPL(xiaomitouch_register_modedata);

int update_palm_sensor_value(int value)
{
	struct xiaomi_touch *dev = NULL;

	mutex_lock(&xiaomi_touch_dev.palm_mutex);

	if (!touch_pdata) {
		mutex_unlock(&xiaomi_touch_dev.palm_mutex);
		return -ENODEV;
	}

	dev = touch_pdata->device;

	if (value != touch_pdata->palm_value) {
		printk("%s value:%d\n", __func__, value);
		touch_pdata->palm_value = value;
		touch_pdata->palm_changed = true;
		sysfs_notify(&xiaomi_touch_dev.dev->kobj, NULL,
		     "palm_sensor");
	}

	mutex_unlock(&xiaomi_touch_dev.palm_mutex);
	return 0;
}
EXPORT_SYMBOL_GPL(update_palm_sensor_value);

static ssize_t palm_sensor_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
	struct xiaomi_touch_pdata *pdata = dev_get_drvdata(dev);

	pdata->palm_changed = false;

	return snprintf(buf, PAGE_SIZE, "%d\n", pdata->palm_value);
}

static ssize_t palm_sensor_store(struct device *dev,
struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned int input;
	struct xiaomi_touch_pdata *pdata = dev_get_drvdata(dev);

	if (sscanf(buf, "%d", &input) < 0)
			return -EINVAL;

	if (pdata->touch_data->palm_sensor_write)
		pdata->touch_data->palm_sensor_write(!!input);
	else {
		printk("%s has not implement\n", __func__);
	}
	printk("%s value:%d\n", __func__, !!input);

	return count;
}

static DEVICE_ATTR(palm_sensor, (S_IRUGO | S_IWUSR | S_IWGRP),
		   palm_sensor_show, palm_sensor_store);

/* N19A code for HQ-370325 by p-huangyunbiao at 2024/2/2 start */
static ssize_t panel_vendor_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
	struct xiaomi_touch_pdata *pdata = dev_get_drvdata(dev);

	if (pdata && pdata->touch_data && pdata->touch_data->panel_vendor_read)
		return snprintf(buf, PAGE_SIZE, "%c", pdata->touch_data->panel_vendor_read());
	else
		return -EINVAL;
}

static ssize_t panel_color_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
	struct xiaomi_touch_pdata *pdata = dev_get_drvdata(dev);

	if (pdata && pdata->touch_data && pdata->touch_data->panel_color_read)
		return snprintf(buf, PAGE_SIZE, "%c", pdata->touch_data->panel_color_read());
	else
		return -EINVAL;
}

static ssize_t panel_display_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
	struct xiaomi_touch_pdata *pdata = dev_get_drvdata(dev);

	if (pdata && pdata->touch_data && pdata->touch_data->panel_display_read)
		return snprintf(buf, PAGE_SIZE, "%c", pdata->touch_data->panel_display_read());
	else
		return -EINVAL;
}

static ssize_t touch_vendor_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
	struct xiaomi_touch_pdata *pdata = dev_get_drvdata(dev);

	if (pdata && pdata->touch_data && pdata->touch_data->touch_vendor_read)
		return snprintf(buf, PAGE_SIZE, "%c", pdata->touch_data->touch_vendor_read());
	else
		return -EINVAL;
}

static DEVICE_ATTR(panel_vendor, (S_IRUGO), panel_vendor_show, NULL);
static DEVICE_ATTR(panel_color, (S_IRUGO), panel_color_show, NULL);
static DEVICE_ATTR(panel_display, (S_IRUGO), panel_display_show, NULL);
static DEVICE_ATTR(touch_vendor, (S_IRUGO), touch_vendor_show, NULL);

static struct attribute *touch_attr_group[] = {
	&dev_attr_palm_sensor.attr,
	&dev_attr_panel_vendor.attr,
	&dev_attr_panel_color.attr,
	&dev_attr_panel_display.attr,
	&dev_attr_touch_vendor.attr,
	NULL,
};
/* N19A code for HQ-370325 by p-huangyunbiao at 2024/2/2 end */

static const struct of_device_id xiaomi_touch_of_match[] = {
	{ .compatible = "xiaomi-touch", },
	{ },
};

static int xiaomi_touch_parse_dt(struct device *dev, struct xiaomi_touch_pdata *data)
{
	int ret;
	struct device_node *np;

	np = dev->of_node;
	if (!np)
		return -ENODEV;

	ret = of_property_read_string(np, "touch,name", &data->name);
	if (ret)
		return ret;

	printk("%s touch,name:%s\n", __func__, data->name);

	return 0;
}

static int xiaomi_touch_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct device *dev = &pdev->dev;
	struct xiaomi_touch_pdata *pdata;

	pdata = devm_kzalloc(dev, sizeof(struct xiaomi_touch_pdata), GFP_KERNEL);
	if (!pdata)
		return -ENOMEM;

	printk("%s enter\n", __func__);

	ret = xiaomi_touch_parse_dt(dev, pdata);
	if (ret < 0) {
		printk("%s parse dt error:%d\n", __func__, ret);
		goto parse_dt_err;
	}

	ret = misc_register(&xiaomi_touch_dev.misc_dev);
	if (ret) {
		printk("%s create misc device err:%d\n", __func__, ret);
		goto parse_dt_err;
	}

	if (!xiaomi_touch_dev.class)
		xiaomi_touch_dev.class = class_create(THIS_MODULE, "touch");

	if (!xiaomi_touch_dev.class) {
		printk("%s create device class err\n", __func__);
		goto class_create_err;
	}

	xiaomi_touch_dev.dev = device_create(xiaomi_touch_dev.class, NULL, 'T', NULL, "touch_dev");
	if (!xiaomi_touch_dev.dev) {
		printk("%s create device dev err\n", __func__);
		goto device_create_err;
	}

	pdata->touch_data = (struct xiaomi_touch_interface *)kzalloc(sizeof(struct xiaomi_touch_interface), GFP_KERNEL);
	if (pdata->touch_data == NULL) {
		ret = -ENOMEM;
		printk("%s alloc mem for touch_data\n", __func__);
		goto data_mem_err;
	}

	pdata->device = &xiaomi_touch_dev;
	dev_set_drvdata(xiaomi_touch_dev.dev, pdata);

	touch_pdata = pdata;

	xiaomi_touch_dev.attrs.attrs = touch_attr_group;
	ret = sysfs_create_group(&xiaomi_touch_dev.dev->kobj, &xiaomi_touch_dev.attrs);
	if (ret) {
		printk("%s ERROR: Cannot create sysfs structure!:%d\n", __func__, ret);
		ret = -ENODEV;
		goto sys_group_err;
	}

	printk("%s over\n", __func__);

	return ret;

sys_group_err:
	if (pdata->touch_data) {
		kfree(pdata->touch_data);
		pdata->touch_data = NULL;
	}
data_mem_err:
	device_destroy(xiaomi_touch_dev.class, 0);
device_create_err:
	class_destroy(xiaomi_touch_dev.class);
class_create_err:
	misc_deregister(&xiaomi_touch_dev.misc_dev);
parse_dt_err:
	printk("%s fail!\n", __func__);
	return ret;
}

static int xiaomi_touch_remove(struct platform_device *pdev)
{
	device_destroy(xiaomi_touch_dev.class, 0);
	class_destroy(xiaomi_touch_dev.class);
	misc_deregister(&xiaomi_touch_dev.misc_dev);
	if (touch_pdata->touch_data) {
		kfree(touch_pdata->touch_data);
		touch_pdata->touch_data = NULL;
	}

	return 0;
}

MODULE_DEVICE_TABLE(of, xiaomi_touch_of_match);

static struct platform_driver xiaomi_touch_device_driver = {
	.probe		= xiaomi_touch_probe,
	.remove		= xiaomi_touch_remove,
	.driver		= {
		.name	= "xiaomi-touch",
		.of_match_table = of_match_ptr(xiaomi_touch_of_match),
	}
};

static int __init xiaomi_touch_init(void)
{
	return platform_driver_register(&xiaomi_touch_device_driver);

}

static void __exit xiaomi_touch_exit(void)
{
	platform_driver_unregister(&xiaomi_touch_device_driver);
}

subsys_initcall(xiaomi_touch_init);
module_exit(xiaomi_touch_exit);
/* N19A code for HQ-359934 by p-huangyunbiao at 2023/12/19 start */
MODULE_LICENSE("GPL");
/* N19A code for HQ-359934 by p-huangyunbiao at 2023/12/19 end */

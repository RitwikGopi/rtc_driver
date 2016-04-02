#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/sched.h>
#include <linux/rtc.h>

#define MAXSIZE 512

static char *name = "foo";
static int major;
static char msg[MAXSIZE];
static int curr_size = 0;

static int foo_open(struct inode* inode, struct file *filp)
{
    /*printk("Major=%d, Minor=%d\n",MAJOR(inode->i_rdev), 
	    MINOR(inode->i_rdev));
    printk("Offset=%d\n", filp->f_pos);
    printk("filp->f_op->open=%x\n", filp->f_op->open);
    printk("address of foo_open=%x\n", foo_open);*/
    printk("Driver open\n");
    return 0;
}

static int foo_close(struct inode *inode, struct file *filp)
{
    printk("Closing device...\n");
    return 0;
}

static int foo_read(struct file *filp, char *buf,
	size_t count, loff_t *f_pos)
{
    printk("%d,%d\n", count, *f_pos);
    sprintf(msg, "%u", current->pid);
    int data_len = strlen(msg);
    int curr_off = *f_pos, remaining;

    if(curr_off >= data_len) return 0;
    remaining = data_len - curr_off;
    if(count <= remaining){
	if(copy_to_user(buf, msg+curr_off, count))
	    return -EFAULT;
	*f_pos = *f_pos + count;
	return count;
    }else{
	if(copy_to_user(buf, msg+curr_off, remaining))
	    return -EFAULT;
	*f_pos = *f_pos + remaining;
	return remaining;
    }
}

static int foo_write(struct filr *filp, const char *buf,
	size_t count, loff_t *offp)
{
    int curr_off = *offp;
    int remaining = MAXSIZE - curr_off;
    if(curr_off >= MAXSIZE) return -ENOSPC;
    if(count <= remaining){
	if(copy_from_user(msg+curr_off, buf, count))
	    return -EFAULT;
	*offp = *offp + count;
	curr_size = *offp;
	return count;
    }else{
	if(copy_from_user(msg+curr_off, buf, remaining))
	    return -EFAULT;
	*offp = *offp + remaining;
	curr_size = *offp;
	return remaining;
    }
}

static int foo_ioctl(struct file *filp,
	unsigned long cmd, unsigned long arg)
{
    printk("%x:%x\n", cmd, RTC_RD_TIME);
    struct rtc_time time;
    switch(cmd){
	case RTC_RD_TIME:
	    outb(0x00, 0x70);
	    time.tm_sec = inb(0x71);
	    outb(0x02, 0x70);
	    time.tm_min = inb(0x71);
	    outb(0x04, 0x70);
	    time.tm_hour = inb(0x71);
	    outb(0x07, 0x70);
	    time.tm_mday = inb(0x71);
	    outb(0x08, 0x70);
	    time.tm_mon = inb(0x71);
	    outb(0x09, 0x70);
	    time.tm_year = inb(0x71);
	    outb(0x06, 0x70);
	    time.tm_wday = inb(0x71);
	    printk("%x:%x:%x,%d\n", time.tm_hour, time.tm_min, time.tm_sec, 
		    sizeof(time));
	    copy_to_user( arg, &time, sizeof(struct rtc_time));
	    break;
    }
}

static struct file_operations fops = {
    open: foo_open,
    release: foo_close,
    read: foo_read,
    write: foo_write,
    unlocked_ioctl: foo_ioctl,
};

int init_module(void)
{
    major = register_chrdev(0, name, &fops);
    printk("Registered, got major = %d\n", major);
    return 0;
}
void cleanup_module(void)
{
    printk("Cleaning up...\n");
    unregister_chrdev(major, name);
}

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>

#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>

MODULE_LICENSE("GPL");

#define MS_TO_NS(x) (x * 1E6L)

static struct hrtimer hr_timer;

unsigned long timer_interval_ns = 100000; //1e6;
ktime_t currtime;
ktime_t interval;

mm_segment_t fs;
mm_segment_t oldfs;

struct file *fgpio;
char valBuffer[1];

int value = 0;

struct file *file_open(const char *path, int flags, int rights) {
    struct file *filp = NULL;
    mm_segment_t oldfs;
    int err = 0;

    oldfs = get_fs();
    set_fs(get_ds());
    filp = filp_open(path, flags, rights);
    set_fs(oldfs);
    if (IS_ERR(filp)) {
        err = PTR_ERR(filp);
        return NULL;
    }
    return filp;
}

void file_close(struct file *file) {
    filp_close(file, NULL);
}

int file_read(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size) {
    mm_segment_t oldfs;
    int ret;

    oldfs = get_fs();
    set_fs(get_ds());

    ret = vfs_read(file, data, size, &offset);

    set_fs(oldfs);
    return ret;
}

int file_write(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size) {
    mm_segment_t oldfs;
    int ret;

    oldfs = get_fs();
    set_fs(get_ds());
    loff_t pos = 0;
    ret = vfs_write(file, data, size, &pos);
    set_fs(oldfs);
    return ret;
}

enum hrtimer_restart my_hrtimer_callback(struct hrtimer *timer) {
    // printk( "my_hrtimer_callback %d called (%ld).\n", value, jiffies );
    //  return HRTIMER_NORESTART;
    //ktime_t currtime;
    //ktime_t interval;
    currtime = ktime_get();
    interval = ktime_set(0, timer_interval_ns);
    hrtimer_forward(timer, currtime, interval);

    if (value == 0) {
        valBuffer[0] = '1';
        file_write(fgpio, 0, valBuffer, 1);
        value = 1;
    } else {
        valBuffer[0] = '0';
        file_write(fgpio, 0, valBuffer, 1);
        value = 0;
    }

    return HRTIMER_RESTART;
}

int init_module(void) {
    //init gpio  
    //open file
    fgpio = file_open("/sys/class/gpio/gpio393/value", O_RDWR, 0);
    //fgpio = file_open("/home/robot/test", O_RDWR, 0);   
    //read value
    valBuffer[0] = 9;
    file_read(fgpio, 0, valBuffer, 1);
    printk(KERN_INFO "buf:%s\n", valBuffer);

    //write value
    valBuffer[0] = '1';
    printk(KERN_INFO "write buf:%s\n", valBuffer);
    int ret = file_write(fgpio, 0, valBuffer, 1);
    printk(KERN_INFO "write done %d\n", ret);



    ktime_t ktime;
    unsigned long delay_in_ms = 200L;

    printk("HR Timer module installing\n");

    ktime = ktime_set(0, MS_TO_NS(delay_in_ms));

    hrtimer_init(&hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);

    hr_timer.function = &my_hrtimer_callback;

    printk("Starting timer to fire in %ldms (%ld)\n", delay_in_ms, jiffies);

    hrtimer_start(&hr_timer, ktime, HRTIMER_MODE_REL);


    //read_gpio();   
    //struct file *file_open(const char *path, int flags, int rights){
    //file_read(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size){
    //file_write(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size){


    return 0;
}

void cleanup_module(void) {

    int ret;
    ret = hrtimer_cancel(&hr_timer);
    if (ret) printk("The timer was still in use...\n");

    printk("HR Timer module uninstalling\n");

    file_close(fgpio); //close fd
    return;
}


/////

int read_gpio(void) {
    /*
       // Create variables
        struct file *f;
        char buf[128];

        int i;
        // Init the buffer with 0
        for(i=0;i<128;i++){
            buf[i] = 0;
        }
        // To see in /var/log/messages that the module is operating
        printk(KERN_INFO "My module is loaded\n");
        f = filp_open("/sys/class/gpio/gpio456/value", O_RDONLY, 0);
            if(f == NULL)
            printk(KERN_ALERT "filp_open error!!.\n");
        else{
            // Get current segment descriptor
            fs = get_fs();
            // Set segment descriptor associated to kernel space
            set_fs(get_ds());
            // Read the file
            f->f_op->read(f, buf, 1, &f->f_pos);
            // Restore segment descriptor
            set_fs(fs);
            // See what we read from file
            printk(KERN_INFO "buf:%s\n",buf);

        return 0;
        }
    
    
        //write value

    printk(KERN_INFO "Write port value\n");
        oldfs = get_fs();
        set_fs(get_ds());
        char outbuf[1];
        outbuf[0]=1;
        //vfs_write("/sys/class/gpio/gpio456/value", outbuf, 1, 0);
        vfs_write(f, outbuf,1,0);
        set_fs(oldfs);
    
    
    
        filp_close(f,NULL);
     */return 0;


}

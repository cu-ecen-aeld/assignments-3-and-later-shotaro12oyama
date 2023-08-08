/*
 * aesdchar.h
 *
 *  Created on: Oct 23, 2019
 *      Author: Dan Walkes
 */

#include <linux/mutex.h>

#ifndef AESD_CHAR_DRIVER_AESDCHAR_H_
#define AESD_CHAR_DRIVER_AESDCHAR_H_

#include "aesd-circular-buffer.h"

#define AESD_DEBUG 1  
// (above) Remove comment on this line to enable debug

#undef PDEBUG             /* undef it, just in case */
#ifdef AESD_DEBUG
#  ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
#    define PDEBUG(fmt, args...) printk( KERN_DEBUG "aesdchar: " fmt, ## args)
#  else
     /* This one for user space */
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

struct aesd_dev
{
    /**
     * TODO: Add structure(s) and locks needed to complete assignment requirements
     */

	// circular buffer
	//struct aesd_circular_buffer buffer;

	// working buffer
	//struct aesd_buffer_entry working;

	char * buffer[10];
    	char * temp;	
	// lock
	struct mutex lock;

	// character device structure
   	struct cdev cdev;     
    	int w_pos;
    	int r_pos;
};	


#endif /* AESD_CHAR_DRIVER_AESDCHAR_H_ */

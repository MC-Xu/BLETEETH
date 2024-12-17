#ifndef HOG_H
#define HOG_H

#include <stdint.h>


#define CONSUMER_KEY_VAL_POWER                	0x0001  /* Power */
#define CONSUMER_KEY_VAL_AC_HOME		        0x0002  /* AC Home */
#define CONSUMER_KEY_VAL_AC_BACK          		0x0004  /* AC Back */
#define CONSUMER_KEY_VAL_MENU        			0x0008  /* Menu */
#define CONSUMER_KEY_VAL_VOLUME_INCREMENT       0x0010  /* Volume Increment */
#define CONSUMER_KEY_VAL_VOLUME_DECREMENT    	0x0020  /* Volume Decrement */
#define CONSUMER_KEY_VAL_PLAY_PAUSE             0x0040  /* Play/Pause */
#define CONSUMER_KEY_VAL_AC_ZOOM        		0x0080  /* AC Zoom */
#define CONSUMER_KEY_VAL_VCR_PLUS               0x0100  /* VCR Plus */
#define CONSUMER_KEY_VAL_MODE_STEP       		0x0200  /* Mode Step */
#define CONSUMER_KEY_VAL_AC_ZOOM_IN		        0x0400  /* AC Zoom In */
#define CONSUMER_KEY_VAL_AC_ZOOM_OUT		    0x0800  /* AC Zoom Out */
#define CONSUMER_KEY_VAL_FAST_FORWARD           0x1000  /* Fast Forward */
#define CONSUMER_KEY_VAL_REWIND		            0x2000  /* Rewind */
#define CONSUMER_KEY_VAL_SCAN_NEXT_TRACK        0x4000  /* Scan Next Track */
#define CONSUMER_KEY_VAL_SCAN_PREVIOUS_TRACK    0x8000  /* Scan Previous Track */
#define CONSUMER_KEY_VAL_REALEASED              0x0000  /* Realeased */

void hog_init(void);
int hog_send_key(uint16_t key_val);
int hog_send_consumer_key(uint16_t key_val);

void hog_init(void);

#endif

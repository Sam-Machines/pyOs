
#include "uart.h"
#include "delays.h"
#include "lfb.h"
#include "sd.h"
#include "fat.h"
#include "power.h"
#include "mbox.h"
#include "sprintf.h"

int h;
h = 15;

int line;
line = 0;

void kprintf(char *str){
	char *buf;
	sprintf(str, buf,0);
	lfb_proprint(0, line*h, str);
	line += 1;	
}


void main()
{
    unsigned int cluster;
    char c;

    // set up serial console and linear frame buffer
    uart_init();
    lfb_init();

    // display an ASCII string on screen with PSF
	//UsbInitialise();
    lfb_proprint(0, 0, "Loading GT-MOS");
    wait_msec(1000000);
    lfb_proprint(0, 0, "Loading GT-MOS [OK]");
    lfb_proprint(0, 1*h, "Init EMMC");

    // initialize EMMC and detect SD card type
    if(sd_init()==SD_OK) {
        // read the master boot record and find our partition
        if(fat_getpartition()) {
            // find out file in root directory entries
            cluster=fat_getcluster("LICENC~1BRO");
            if(cluster==0)
                cluster=fat_getcluster("KERNEL8 IMG");
            if(cluster) {
                // read into memory
                uart_dump(fat_readfile(cluster));
            }
	    lfb_proprint(0, 1*h, "Init EMMC [OK]");
        } else {
            uart_puts("FAT partition not found???\n");
	    	lfb_proprint(0, 1*h, "Init EMMC [FAT PARTITION: Not Found]");
        }
    } else {
		lfb_proprint(0, 1*h, "Init EMMC [FAIL]");
    }


    // get the board's unique serial number with a mailbox call
    mbox[0] = 8*4;                  // length of the message
    mbox[1] = MBOX_REQUEST;         // this is a request message
    
    mbox[2] = MBOX_TAG_GETSERIAL;   // get serial number command
    mbox[3] = 8;                    // buffer size
    mbox[4] = 8;
    mbox[5] = 0;                    // clear output buffer
    mbox[6] = 0;

    mbox[7] = MBOX_TAG_LAST;

    // send the message to the GPU and receive answer
    if (mbox_call(MBOX_CH_PROP)) {
		lfb_proprint(0, 2*h, "Serial Number:");

		char str[80];
		sprintf(str, "hex>: '%8x', '%8x'\n", mbox[6], mbox[5]);
		lfb_proprint(0, 3*h, str);

        uart_puts("My serial number is: ");
        uart_hex(mbox[6]);
        uart_hex(mbox[5]);
        uart_puts("\n");
    } else {
		uart_puts("Unable to query serial!\n");
		lfb_proprint(0, 2*h, "Unable to find serial number");
    }

	/* Detect the first keyboard on USB bus */
	//uint8_t firstKbd = 0;
	//for (int i = 1; i <= MaximumDevices; i++) {
	//	if (IsKeyboard(i)) {
	//		firstKbd = i;
	//		break;
	//	}
	//}
	//if (firstKbd) lfb_proprint(0, 3*h,"Keyboard detected\r\n");

	char *str = "                                ";
	lfb_print(0, 0*h, str);
	lfb_print(0, 1*h, str);
	lfb_print(0, 2*h, str);
	lfb_print(0, 3*h, str);
	lfb_print(0, 4*h, str);

	line = 0;
	kprintf("Welcome to GT-MOS!");
	kprintf("root@pi / $ ");
    while(1) {
		// echo everything back
        //uart_puts(" 1 - power off\n 2 - reset\nChoose one: ");
        c=uart_getc();
        uart_send(c);

    }
}

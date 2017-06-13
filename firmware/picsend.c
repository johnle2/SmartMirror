/*
	This program will read a bitmap file (256 color), x and y coordinate on screen
	and transmit raw RGB data over bluetooth port.
*/

#include <stdlib.h>
#include <stdio.h>
//#include <windows.h>
#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <unistd.h>

//This function create a pallete for the 256 color BMP from filename 
//and return pointer to 756 bytes of memory

unsigned char* read256BMPpallet(char* filename){
	FILE* f = fopen(filename, "rb");
    unsigned char info[54];
    fread(info, sizeof(unsigned char), 54, f); // read the 54-byte header
	
    // extract image height and width from header
    unsigned char pallet[1024];
	fread(pallet, sizeof(unsigned char), 1024, f); // read the 54-byte header
    fclose(f);
	
	unsigned char *cpallet = malloc(sizeof(unsigned char)*768);
	int i;
	for (i = 0; i < 1024; i+=4){
		cpallet[i*3/4] = pallet[i+2];
		cpallet[i*3/4+1] = pallet[i+1];
		cpallet[i*3/4+2] = pallet[i];
		//4 bytes for pallet entry, ignore 1st byte
	}
	//unsigned char *data = malloc(sizeof(unsigned char)*size); // allocate 3 bytes per pixel
	
	return cpallet;	
	return NULL;
}



//The pixel data in forms of index to the pallete of 256 color BMP
unsigned char* read256BMPdata(char* filename){
	FILE* f = fopen(filename, "rb");
    unsigned char info[1078];
    fread(info, sizeof(unsigned char), 1078, f); // read the 54-byte header

    // extract image height and width from header
    int width = *(int*)&info[18];
    int height = *(int*)&info[22];
	int size = width*height;
	unsigned char *data = malloc(sizeof(unsigned char)*size);
	fread(data, sizeof(unsigned char), size, f); // read the 54-byte header
    fclose(f);
	int row=0, col=0;
	/*
	while (row*width+col < size/2){
		unsigned char temp = data[row*width+col];
		data[row*width+col] = data[(height-1-row)*width+col];
		data[(height-1-row)*width+col] = temp;
		col++;
		if (col == width){
			col = 0;
			row++;
		}
	}
	*/
	return data;
}	


//This read 24bit BMP for max resolution, but slow
unsigned char* readBMP(char* filename)
{
    int i;
    FILE* f = fopen(filename, "rb");
    unsigned char info[54];
    fread(info, sizeof(unsigned char), 54, f); // read the 54-byte header

    // extract image height and width from header
    int width = *(int*)&info[18];
    int height = *(int*)&info[22];

    int size = 3 * width * height;
	//int size = width*height;
    unsigned char* data = malloc(sizeof(unsigned char)*size); // allocate 3 bytes per pixel
    fread(data, sizeof(unsigned char), size, f); // read the rest of the data at once
	//printf("%d %d\n", width, height);
    fclose(f);
	return data;
}
//return width of image
int filewidth(char* filename){
    FILE* f = fopen(filename, "rb");
    unsigned char info[54];
    fread(info, sizeof(unsigned char), 54, f); // read the 54-byte header
    // extract image height and width from header
    int width = *(int*)&info[18];
	fclose(f);
	return width;
}
//return height of image
int fileheight(char*filename){
	FILE* f = fopen(filename, "rb");
    unsigned char info[54];
    fread(info, sizeof(unsigned char), 54, f); // read the 54-byte header
	int height = *(int*)&info[22];
	fclose(f);
	return height;
}

//run length encoding to shorten data sent 
int encode(const unsigned char* data, int len, unsigned char * res) {
	/*unsigned char * */
	int i,j = 0;
	for (i = 0; i < len; i++){
		res[j] = data[i];
		j++;
		res[j]++;
		while(i+1< len && data[i] == data[i+1] && res[j] < 0xCF){
			//Note: 0xCF is set to the max repeat index since only 8 bits + write pixel takes time,
			// so any faster may cause UART reading errors
		  res[j]++;
		  i++;
		}
		j++;
	}
	if (len < j)
		printf("Run length inefficient!\n");
	else
		printf("%d vs %d",j,len);
	
	/*for (i = 0; i < j; i+=2){
		printf("0x%x- %u ", res[i], res[i+1]);
		if (i % 10 == 0)
			printf("\n");
	}*/
	//free(res);
	return j;
}
 

int
set_interface_attribs (int fd, int speed, int parity)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                perror("error from tcgetattr");
                return -1;
        }

        cfsetospeed (&tty, speed);
        cfsetispeed (&tty, speed);

        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
        // disable IGNBRK for mismatched speed tests; otherwise receive break
        // as \000 chars
        tty.c_iflag &= ~IGNBRK;         // disable break processing
        tty.c_lflag = 0;                // no signaling chars, no echo,
                                        // no canonical processing
        tty.c_oflag = 0;                // no remapping, no delays
        tty.c_cc[VMIN]  = 0;            // read doesn't block
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

        tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                        // enable reading
        tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
        tty.c_cflag |= parity;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CRTSCTS;

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
        {
                perror("error from tcsetattr");
                return -1;
        }
        return 0;
}


//Below is USB send code for Windows, not LINUX
int main( int argc, char *argv[] )
{
	//char *portname = "/dev/serial/by-id/usb-FTDI_FT232R_USB_UART_A50285BI-if00-port0";
	if (argc != 4){
		perror("program filename x_coor, y_coor");
		exit(1);
	}
	char *portname = "/dev/rfcomm0";
	int fd = open (portname, O_RDWR | O_NOCTTY | O_SYNC);
	if (fd < 0){
		perror("Error open USB port");
		exit(1);
	}
	set_interface_attribs (fd, B115200, 0);  // set speed to 115,200 bps, 8n1 (no parity)
	//char buffer[50] = "./a_test.bmp";
	//DWORD bytes_to_read = 50;
	//unsigned char *pic_ptr = readBMP(buffer);
	unsigned char *pal_ptr = read256BMPpallet(argv[1]);
	unsigned char *pic_ptr2 = read256BMPdata(argv[1]);
	
	int width = filewidth(argv[1]);	//800 -3 char
	int height = fileheight(argv[1]);	//480 -3 char
	int startx = atoi(argv[2]);
	int starty = atoi(argv[3]);
	char k[8];
	k[0] = startx & 0xFF;
	k[1] = (startx >> 8) & 0xFF;
	k[2] = starty & 0xFF;
	k[3] = (starty >> 8) & 0xFF;
	k[4] = width & 0xFF;
	k[5] = (width >> 8) & 0xFF;
	k[6] = height & 0xFF;
	k[7] = (height >> 8) & 0xFF;
	write(fd, k, 8);
/*
	char start[3];
	sprintf(start,"%03d", width);
	write(fd, start, 3);
	//WriteFile(hSerial, &start, 3, &bytes_written, NULL))
	sprintf(start,"%03d", height);
	write(fd,start,3);
	//WriteFile(hSerial, &start, 3, &bytes_written, NULL))
	
	
	printf("\n%x, %x\n", k[0], k[1]);
	write(fd, k, 2);
	write(fd, k, 2);
*/
	unsigned char *shortpic = malloc(sizeof(char)*(width*height*2));
	memset(shortpic,0,(width*height*2)*sizeof(unsigned char));
	
	int len = encode(pic_ptr2, width*height, shortpic);
	printf("Encode finish, begin send\n");
	//printf("%x-%x-%x-%x", shortpic[0], pal_ptr[shortpic[1]], pal_ptr[shortpic[1]+1], pal_ptr[shortpic[1]+2]);
	//WriteFile(hSerial, pal_ptr, 768, &bytes_written, NULL);
	write(fd, pal_ptr, 768);
	write(fd, shortpic, len);
	printf("Send finished, free memory\n");
	//WriteFile(hSerial, shortpic, len, &bytes_written, NULL);
	//WriteFile(hSerial, pic_ptr2, width*height, &bytes_written, NULL);
	free(shortpic);
	free(pal_ptr);
    free(pic_ptr2);
	return 0;
}

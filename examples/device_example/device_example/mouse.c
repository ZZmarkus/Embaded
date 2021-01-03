#include <linux/input.h>

#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <unistd.h>

#include <sys/types.h>

#include <fcntl.h>                    


//frame buffer
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <sys/mman.h>


#define POS_DEBUG
typedef unsigned short int U32;
unsigned int* pfbdata;
unsigned int makepixel(U32 r, U32 g, U32 b){
	unsigned long int reval;
	reval = b;
	reval |=((unsigned long)g<<8);
	reval |=((unsigned long)r<<16);
	return reval;
}

struct fb_var_screeninfo fvs;
int frameBufferInit(void){
    int frame_fd;
    int check;
    if((frame_fd = open("/dev/fb0",O_RDWR))<0) {
        return -1;       
    }   
    if((check=ioctl(frame_fd,FBIOGET_VSCREENINFO,&fvs))<0) {
        perror("Get Information Error - VSCREENINFO!");
        exit(1);
    }
 
    if(fvs.bits_per_pixel != 32) {
        perror("Unsupport Mode. 32Bpp Only.");
        exit(1);
    }
 
    if(lseek(frame_fd, 0, SEEK_SET) < 0) {  // Set Pixel Map
        perror("LSeek Error.");
        exit(1);
    }
    pfbdata = (unsigned int*)mmap(0,fvs.xres*fvs.yres*32/8,PROT_READ|PROT_WRITE,MAP_SHARED,frame_fd,0);
    if((unsigned int)pfbdata ==(unsigned int)-1){
	perror("Error Mapping!\n");
    }
    return frame_fd; 
}

int makeRect(int frame_fd,U32 R,U32 G,U32 B,int posx1,int posx2,int posy1,int posy2){
	unsigned int pixel;
	int offset;
	int repx, repy;

	pixel=makepixel(R,G,B);
/*	
	for(repy=posy1; repy < posy2; repy++) {
        	offset = repy * fvs.xres * (32/8) + posx1 * (32/8);
       		if(lseek(frame_fd, offset, SEEK_SET) < 0) {
            	perror("LSeek Error!");
            	exit(1);
        }
        	for(repx = posx1; repx <= posx2; repx++)
            		write(frame_fd, &pixel,(32/8));
    	}
*/
	for(repy=posy1;repy<=posy2;repy++){
		offset = repy * fvs.xres;
		for(repx=posx1;repx<=posx2;repx++)
			*(pfbdata+offset+repx) =pixel;//0xFF0000;// pixel;

	}
	return 0;
}
typedef struct input_event	input_event_t;

int main(void)
{        

	int fd,frame_fd;
	int posX=10,posY=10;
	int mouseR=0xFF;
	int mouseG=0xFF;
	int mouseB=0xFF;
	int color=0;
	input_event_t  buf;
	memset(&buf, 0,sizeof(input_event_t));      
                   
	frame_fd = frameBufferInit();
	//open keyboard Driver
	fd = open( "/dev/input/event1", O_RDWR | O_NOCTTY | O_NONBLOCK ); 
	if(0>fd)
	{        
		printf("open errorn");
		return -1;
	} 
   
	while(1)
   	{
		//delete mouse
		read( fd, &buf, sizeof(input_event_t));  
		switch(buf.type){
		case 2: //moving
			//delete mouse
			if(*(pfbdata+posY*fvs.xres+posX)!=makepixel(0,0,0)){
				makeRect(frame_fd,0,0,0,posX,posX+10,posY,posY+10);
			}
			if(buf.code==0){//left&right
				posX+=(int)buf.value/2;
			}else if(buf.code==1){//up&down
				posY+=(int)buf.value/2;
			}
			if(posX<0)
				posX=0;
			if(posY<0)
				posY=0;
			if(posX>fvs.xres-11){
				posX=fvs.xres-11;
			}
			if(posY>fvs.yres-11){
				posY=fvs.yres-11;
			}
			//draw mouse
	   		if(*(pfbdata+posY*fvs.xres+posX)==makepixel(0,0,0)){
				makeRect(frame_fd,mouseR,mouseG,mouseB,posX,posX+10,posY,posY+10);
			}
			printf("mouse pos(%d,%d)\n",posX,posY);
			break;
		case 1:	//mouse click
			if(buf.code==0x110&&buf.value==0x1){//left button,clicked
				switch(color){
				case 0:
					mouseR+=5;
					if(mouseR>0xFF)
						mouseR=0x00;
					break;
				case 1:
					mouseG+=5;
					if(mouseG>0xFF)
						mouseG=0x00;
					break;
				case 2:
					mouseB+=5;
					if(mouseB>0xFF)
						mouseB=0x00;
					break;
				}
			}
			if(buf.code==0x111&&buf.value==0x1){//right button
				color++;
				if(color>2)
					color=0;
			}
			printf("mouse color (R:%d,G:%d,B:%d)\n",mouseR,mouseG,mouseB);		
		}

	//	if(buf.type!=0)
	//	printf("t:%x,c:%x,v:%x\n",buf.type,buf.code,buf.value);
		usleep(10000);
	}
	

	close(fd);
	close(frame_fd);
	munmap(&buf,sizeof(input_event_t));
	munmap(pfbdata,fvs.xres*fvs.yres*32/8);
	return 0;

}

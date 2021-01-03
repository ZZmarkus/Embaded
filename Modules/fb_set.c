#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<linux/fb.h>


typedef struct _user_fb { // 사용자 지정 정보를 담는 구조체 함수 타입 선언
int xres;
int yres;
int bpps;
} user_fb;
int main(int argc, char** argv)
{
int frame_fd;
int check;
struct fb_var_screeninfo st_fvs;
user_fb my_fb = {800,480,32}; // my_fb 생성과 동시에 초기화
frame_fd = open("/dev/fb0",O_RDWR);
if(frame_fd < 0)
{
perror("Frame Buffer Open Error\n");
exit(1);
}
if(check=ioctl(frame_fd, FBIOGET_VSCREENINFO,&st_fvs))
{
perror("GET Information Error - VSCREENINFO!");
exit(1);
}
st_fvs.xres = my_fb.xres; //사용자 지정값을 프레임버퍼 구조체 값으로 변경
st_fvs.yres = my_fb.yres;
st_fvs.bits_per_pixel = my_fb.bpps;
if(check=ioctl(frame_fd, FBIOPUT_VSCREENINFO,&st_fvs))
{
perror("PUT Information Error - VSCREENINFO!");
exit(1);
}
printf("==================\n");
printf("Frame Buffer Info\n");
printf("------------------\n");
printf("X - res : %d\n",st_fvs.xres);
printf("Y - res : %d\n",st_fvs.yres);
printf("X - v_res : %d\n",st_fvs.xres_virtual);
printf("Y - v_res : %d\n",st_fvs.yres_virtual);
printf("Bit/Pixel : %d\n",st_fvs.bits_per_pixel);
printf("==================\n\n");
close(frame_fd);
return 0;
}

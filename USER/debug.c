#include "debug.h"
#include "stdio.h"

extern volatile u8 CmdTp;// flag for Bluetooth Command Type: 0 - Unknown yet,1 - basic Instrution, 2 - defined motion
extern volatile u8 newCmdType2Value; 

void debugForCmd(u8 tempCmd){
			if(tempCmd=='0'){
				CmdTp=2;
				newCmdType2Value = 0xff;		
			}else if(tempCmd=='1'){
				CmdTp=2;
				newCmdType2Value = 0x01;		
			}else if(tempCmd=='2'){
				CmdTp=2;
				newCmdType2Value = 0x02;
			}else if(tempCmd=='3'){
				CmdTp=2;
				newCmdType2Value = 0x03;
			}else if(tempCmd=='4'){
				CmdTp=2;
				newCmdType2Value = 0x04;
			}else if(tempCmd=='5'){
				CmdTp=2;
				newCmdType2Value = 0x05;
			}else if(tempCmd=='6'){
				CmdTp=2;
				newCmdType2Value = 0x06;
			}else if(tempCmd=='7'){
				CmdTp=2;
				newCmdType2Value = 0x07;
			}else if(tempCmd=='8'){
				CmdTp=2;
				newCmdType2Value = 0x08;
			}else if(tempCmd=='9'){
				CmdTp=2;
				newCmdType2Value = 0x09;
			}else if(tempCmd=='a'){
				CmdTp=2;
				newCmdType2Value = 0x0a;
			}else if(tempCmd=='b'){
				CmdTp=2;
				newCmdType2Value = 0x0b;
			}else if(tempCmd=='c'){
				CmdTp=2;
				newCmdType2Value = 0x0c;
			}else if(tempCmd=='d'){
				CmdTp=2;
				newCmdType2Value = 0x0d;
			}else if(tempCmd=='e'){
				CmdTp=2;
				newCmdType2Value = 0x0e;
			}else if(tempCmd=='f'){
				CmdTp=2;
				newCmdType2Value = 0x0f;
			}else if(tempCmd=='g'){
				CmdTp=2;
				newCmdType2Value = 0x10;
			}else if(tempCmd=='h'){
				CmdTp=2;
				newCmdType2Value = 0x11;
			}else if(tempCmd=='i'){
				CmdTp=2;
				newCmdType2Value = 0x12;
			}else if(tempCmd=='j'){
				CmdTp=2;
				newCmdType2Value = 0x13;
			}else if(tempCmd=='k'){
				CmdTp=2;
				newCmdType2Value = 0x14;
			}else if(tempCmd=='l'){
				CmdTp=2;
				newCmdType2Value = 0x15;
			}else if(tempCmd=='m'){
				CmdTp=2;
				newCmdType2Value = 0x16;
			}else if(tempCmd=='n'){
				CmdTp=2;
				newCmdType2Value = 0x17;
			}else if(tempCmd=='o'){
				CmdTp=2;
				newCmdType2Value = 0x13;
			}else if(tempCmd=='p'){
				CmdTp=2;
				newCmdType2Value = 0x18;
			}else if(tempCmd=='q'){
				CmdTp=2;
				newCmdType2Value = 0x19;
			}
			printf("tempCmd: %c\n",tempCmd);
			printf("newCmdType2Value: %c\n", (u8)newCmdType2Value);	
}

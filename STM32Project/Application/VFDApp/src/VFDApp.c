/**
  ******************************************************************************
  * @file    VFDApp.c
  * @author  �Ŷ�
  * @version V1.0.0
  * @date    2020-9-30
  * @brief   �Թ���Ӧ�ó���
  ******************************************************************************
  */
#include "stm32f1xx_hal.h"
#include "VFDApp.h"
#include "tim.h"
//hard ware
#include "oled.h"
#include "DS3231.h"
//#include "VFDDriver.h"
#include "nixie.h"
#include "keyBoard.h"
#include "bmp.h"

/*Global data space -----------------------------------------------*/

DisModTypedef VFDDisMod = NTIME;
CFGTypedef  SysState = NORMAL;

/*VFD display group -----------------------------------------------*/
void VFDNTIMEDisplay(void)//����ʱ����ʾģʽ
{
	uint8_t str[7] = {0,0,0,0,0,0};
	
	str[6] = TimeValue.week;
	
	str[0] = TimeValue.hour / 10;
	str[1] = TimeValue.hour % 10;
	str[2] = TimeValue.minute / 10;
	str[3] = TimeValue.minute % 10;
	str[4] = TimeValue.second / 10;
	str[5] = TimeValue.second % 10;
	
	NixieTubeDisplay(str);
}

void VFDNDATEDisplay(void)//����������ʾģʽ
{
	uint8_t str[7] = {0,0,0,0,0,0};
	
	str[0] = 23;
	
	str[4] = TimeValue.year / 10;
	str[5] = TimeValue.year % 10;
	str[0] = TimeValue.month / 10;
	str[1] = TimeValue.month % 10;
	str[2] = TimeValue.date / 10;
	str[3] = TimeValue.date % 10;
	
	NixieTubeDisplay(str);
}
void VFDTEMPDisplay(void)//�¶���ʾģʽ
{
	uint8_t str[7] = {0,0,0,0,0,0};
	
	str[6] = 23;
	
	str[0] = TimeValue.str[0] - 0x30;
	str[1] = TimeValue.str[1] - 0x30;
	str[2] = TimeValue.str[3] - 0x30;
	str[3] = TimeValue.str[4] - 0x30;
	str[5] = 23;
	str[6] = 0x0C;
	
	NixieTubeDisplay(str);
	
}



//���ʱ����Ϣ
void GetTimeValue(void)
{
	static uint32_t lasttick = 0;
	if((HAL_GetTick() - lasttick) > 100)
	{
		lasttick = HAL_GetTick();
		Time_Handle(); //ÿ����DS3231 ��ȡ10��ʱ����Ϣ
	}
}


//�ָ���ÿ����˸һ�� �ߵ͸�����500ms
void SymbolFlash(void)
{
	static uint32_t histTick = 0;
	if((HAL_GetTick() - histTick) <= 500)
	{
			//״̬1
	}
	else if((HAL_GetTick() - histTick) < 1000)
	{
			//״̬2
	}
	else if((HAL_GetTick() - histTick) > 1000)
	{
		histTick = HAL_GetTick();
	}
}


//LED��˸
void LEDFlash(void)
{
	static uint32_t lasttick = 0;
	
	if((HAL_GetTick() - lasttick) > 5000 && (HAL_GetTick() - lasttick) < 5010)
	{
		GPIOC->BRR = GPIO_PIN_13;
	}
	else if((HAL_GetTick() - lasttick) > 5010)
	{
		lasttick = HAL_GetTick();
	}
	else
	{
		GPIOC->BSRR = GPIO_PIN_13;
	}
}




/*OLED display group------------------------------------------------------*/
void InfoDisplay(void)
{
	static uint32_t lasttick = 0;
	if((HAL_GetTick() - lasttick) > 300)
	{
		lasttick = HAL_GetTick();
		uint8_t str[30];
		uint8_t temp[30];
		//��ʾʱ��
		sprintf((char *)str,"Time>:%02d:%02d:%02d",TimeValue.hour,TimeValue.minute,TimeValue.second);
		OLED_ShowString(0,1,(uint8_t *)str,13);
		//��ʾ����
		sprintf((char *)str,"Date>:20%02d/%02d/%02d",TimeValue.year,TimeValue.month,TimeValue.date);
		OLED_ShowString(0,3,(uint8_t *)str,13);
		//��ʾ�¶�
		sprintf((char *)str,"Temp>:%c%c.%c%cC",TimeValue.str[0],TimeValue.str[1],TimeValue.str[3],TimeValue.str[4]);
		OLED_ShowString(0,5,(uint8_t *)str,13);
		//��ʾѡ��͹��
		sprintf((char *)str,"->Menu");
		OLED_ShowString(80,7,(uint8_t *)str,13);
	}
}



/*Menu and state node Function -------------------------------------------*/
void NormalDisModeChange(void)
{
	static uint8_t lastkey = 0;
	if((lastkey == 0)&&(getKey_Ctrl() != 0))
	{
		switch(getKey_Ctrl())
		{
			case 3: //��ʾģʽ�����л�
			{
				VFDDisMod = (VFDDisMod == CLOSE) ? NTIME : VFDDisMod + 1;
				break;
			}
			case 4://��ʾģʽ�����л�
			{
				VFDDisMod = (VFDDisMod == NTIME) ? CLOSE : VFDDisMod - 1;
				break;
			}
			case 5://������ʾ�˵�
			{
				SysState = MAIN;
				while(getKey_Ctrl() == 5);
				OLED_Clear();
				break;
			}
			default:break;
		}
	}
	lastkey = getKey_Ctrl();
}

void MainFunctionChose(void)
{
	static uint8_t cursor = 0;//������ setdate��0 settime:1 back:2
		
	//��ʾ����ѡ��͹��
	if(cursor == 0)OLED_ShowString(0,3,(uint8_t *)"->TIME&DATE",13);
	else OLED_ShowString(0,3,(uint8_t *)"  TIME&DATE",13);
	if(cursor == 1)OLED_ShowString(0,5,(uint8_t *)"->WLAN",13);
	else OLED_ShowString(0,5,(uint8_t *)"  WLAN",13);
	//��Ӧ�����л� ��+ ��- �󷵻� ��ȷ��
	switch(getKey_Ctrl())
	{
		case 1://�� �ƶ����
		{
			cursor = (cursor == 0)?1:0;
			while(getKey_Ctrl()==1);
			break;
		}
		case 3://�� ����
		{
			SysState = NORMAL;
			while(getKey_Ctrl()==3);
			OLED_Clear();
			cursor = 0;
		}
		case 2://�� �ƶ����
		{
			cursor = (cursor == 0)?1:0;
			while(getKey_Ctrl()==2);
			break;
		}
		case 4://��
		case 5://ȷ��
		{
			switch(cursor)
			{
				case 0:SysState = TIMEDATE;break;
				case 1:SysState = WLAN;break;
			}
			while(getKey_Ctrl()==5 ||getKey_Ctrl()== 4);
			OLED_Clear();
			cursor = 0;
			break;
		}
	}
}
void TimeDateConfig(void)
{
	static uint8_t cursor = 0;//������ setdate��0 settime:1 back:2
		
	//��ʾ����ѡ��͹��
	if(cursor == 0)OLED_ShowString(0,3,(uint8_t *)"->Set Date",13);
	else OLED_ShowString(0,3,(uint8_t *)"  Set Date",13);
	if(cursor == 1)OLED_ShowString(0,5,(uint8_t *)"->Set Time",13);
	else OLED_ShowString(0,5,(uint8_t *)"  Set Time",13);
	if(cursor == 2)OLED_ShowString(0,7,(uint8_t *)"->Set Day",13);
	else OLED_ShowString(0,7,(uint8_t *)"  Set Day",13);
	//��Ӧ�����л� ��+ ��- �󷵻� ��ȷ��
	switch(getKey_Ctrl())
	{
		case 1://�� �ƶ����
		{
			cursor = (cursor > 0)?cursor-1:2;
			while(getKey_Ctrl()==1);
			break;
		}
		case 3://�� ����
		{
			SysState = MAIN;
			while(getKey_Ctrl()==3);
			OLED_Clear();
			cursor = 0;
		}
		case 2://�� �ƶ����
		{
			cursor = (cursor < 2)?cursor+1:0;
			while(getKey_Ctrl()==2);
			break;
		}
		case 4://��
		case 5://ȷ��
		{
			switch(cursor)
			{
				case 0:SysState = DATE;break;
				case 1:SysState = TIME;break;
				case 2:SysState = DAY;break;
			}
			while(getKey_Ctrl()==5 ||getKey_Ctrl()== 4);
			OLED_Clear();
			cursor = 0;
			break;
		}
	}
}
//�趨ʱ��ģʽ
void TimeStateFunc(void)
{
	static uint8_t cursor = 0;
	static uint8_t flag_now = 0;
	static Time_Typedef timeVal;
	uint8_t str[30];
	//��ǰδ��������
	if(flag_now == 0)
	{
		flag_now = 1;//ʹ�����������ݱ�־λ
		memcpy(&timeVal,&TimeValue,sizeof(Time_Typedef));
	}
	//��ʾ����
	sprintf((char *)str,"    %02d:%02d:%02d",timeVal.hour,timeVal.minute,timeVal.second);
	OLED_ShowString(0,3,(uint8_t *)str,12);
	//��ʾ���
	switch(cursor)
	{
		case 0:OLED_ShowString(0,4,(uint8_t *)"    ^        ",12);break;
		case 1:OLED_ShowString(0,4,(uint8_t *)"     ^       ",12);break;
		case 2:OLED_ShowString(0,4,(uint8_t *)"       ^     ",12);break;
		case 3:OLED_ShowString(0,4,(uint8_t *)"        ^    ",12);break;
		case 4:OLED_ShowString(0,4,(uint8_t *)"          ^  ",12);break;
		case 5:OLED_ShowString(0,4,(uint8_t *)"           ^ ",12);break;
		default:OLED_ShowString(0,4,(uint8_t *)"             ",12);break;
	}
	if(cursor>5)
	{
		if(cursor == 6)OLED_ShowString(0,7,(uint8_t *)"->Save     Back",12);
		else OLED_ShowString(0,7,(uint8_t *)"  Save   ->Back",12);
	}
	else
	{
		OLED_ShowString(0,7,(uint8_t *)"  Save     Back",12);
	}
	//��Ӧ�����л� ����+ ����- ȷ����ת
	switch(getKey_Ctrl())
	{ 
		case 1://��
		{
			switch(cursor)
			{
				case 0:timeVal.hour = ((timeVal.hour/10) == 2)?timeVal.hour%10:timeVal.hour+10;break;
				case 1:timeVal.hour = ((timeVal.hour%10) == 9)?(timeVal.hour/10)*10:timeVal.hour+1;break;
				case 2:timeVal.minute = ((timeVal.minute/10) == 5)?timeVal.minute%10:timeVal.minute+10;break;
				case 3:timeVal.minute = ((timeVal.minute%10) == 9)?(timeVal.minute/10)*10:timeVal.minute+1;break;break;
				case 4:timeVal.second = ((timeVal.second/10) == 5)?timeVal.second%10:timeVal.second+10;break;break;
				case 5:timeVal.second = ((timeVal.second%10) == 9)?(timeVal.second/10)*10:timeVal.second+1;break;
			}
			HAL_Delay(200);
			break;
		}
		case 2://��
		{
			switch(cursor)
			{
				case 0:timeVal.hour = ((timeVal.hour/10) == 0)?timeVal.hour + 20:timeVal.hour-10;break;
				case 1:timeVal.hour = ((timeVal.hour%10) == 0)?(timeVal.hour/10)*10+9:timeVal.hour-1;break;
				case 2:timeVal.minute = ((timeVal.minute/10) == 0)?timeVal.minute + 50:timeVal.minute-10;break;
				case 3:timeVal.minute = ((timeVal.minute%10) == 0)?(timeVal.minute/10)*10+9:timeVal.minute-1;break;
				case 4:timeVal.second = ((timeVal.second/10) == 0)?timeVal.second + 50:timeVal.second-10;break;
				case 5:timeVal.second = ((timeVal.second%10) == 0)?(timeVal.second/10)*10+9:timeVal.second-1;break;
			}
			HAL_Delay(200);
			break;
		}
		case 3://��
		{
			cursor = (cursor > 0)?cursor-1:7;
			while(getKey_Ctrl()==1 || getKey_Ctrl()== 3);
			break;
		}
		case 4://��
		{
			cursor = (cursor < 7)?cursor+1:0;
			while(getKey_Ctrl()==2 || getKey_Ctrl()== 4);
			break;
		}
		case 5://ȷ��
		{
			switch(cursor)
			{
				case 6://save
				{
					Time_Handle();
					TimeValue.hour = timeVal.hour;
					TimeValue.minute = timeVal.minute;
					TimeValue.second = timeVal.second;
					DS3231_ReadWrite_Time(0);
					SysState = NORMAL;
					flag_now = 0;
					break;
				}
				case 7:SysState = TIMEDATE;flag_now = 0;break;//back
			}
			while(getKey_Ctrl()==5);
			OLED_Clear();
			cursor = 0;
			break;
		}
	}
	
	//ʱ����
	if(timeVal.hour > 23)
	{
		timeVal.hour = 23;
	}
	if(timeVal.minute > 59)
	{
		timeVal.minute = 59;
	}
	if(timeVal.second > 59)
	{
		timeVal.second  = 59;
	}
}
//�趨����ģʽ
void DateStateFunc(void)
{
	static uint8_t cursor = 0;
	static uint8_t flag_now = 0;
	static uint8_t lastkey = 0;
	static Time_Typedef timeVal;
	uint8_t str[30];
	//��ǰδ��������
	if(flag_now == 0)
	{
		flag_now = 1;//ʹ�����������ݱ�־λ
		memcpy(&timeVal,&TimeValue,sizeof(Time_Typedef));
	}
	//��ʾ����
	sprintf((char *)str,"  20%02d/%02d/%02d",timeVal.year,timeVal.month,timeVal.date);
	OLED_ShowString(0,3,(uint8_t *)str,12);
	//��ʾ���
	switch(cursor)
	{
		case 0:OLED_ShowString(0,4,(uint8_t *)"    ^        ",12);break;
		case 1:OLED_ShowString(0,4,(uint8_t *)"     ^       ",12);break;
		case 2:OLED_ShowString(0,4,(uint8_t *)"        ^    ",12);break;
		case 3:OLED_ShowString(0,4,(uint8_t *)"          ^  ",12);break;
		case 4:OLED_ShowString(0,4,(uint8_t *)"           ^ ",12);break;
		default:OLED_ShowString(0,4,(uint8_t *)"             ",12);break;
	}
	if(cursor>4)
	{
		if(cursor == 5)OLED_ShowString(0,7,(uint8_t *)"->Save     Back",12);
		else OLED_ShowString(0,7,(uint8_t *)"  Save   ->Back",12);
	}
	else
	{
		OLED_ShowString(0,7,(uint8_t *)"  Save     Back",12);
	}
	if((lastkey != 0) && (getKey_Ctrl() == 0))
	{
	//��Ӧ�����л� ����+ ����- ȷ����ת
		switch(lastkey)
		{ 
			case 1://��
			{
				switch(cursor)
				{
					case 0:timeVal.year = ((timeVal.year/10) == 9)?timeVal.year%10:timeVal.year+10;break;
					case 1:timeVal.year = ((timeVal.year%10) == 9)?(timeVal.year/10)*10:timeVal.year+1;break;
					case 2:timeVal.month = (timeVal.month < 12)? timeVal.month + 1:1;break;
					case 3:timeVal.date = ((timeVal.date/10) == 3)?timeVal.date%10:timeVal.date+10;break;
					case 4:timeVal.date = ((timeVal.date%10) == 9)?(timeVal.date/10)*10:timeVal.date+1;break;
				}
				break;
			}
			case 2://��
			{
				switch(cursor)
				{
					case 0:timeVal.year = ((timeVal.year/10) == 0)?timeVal.year + 90:timeVal.year-10;break;
					case 1:timeVal.year = ((timeVal.year%10) == 0)?(timeVal.year/10)*10+9:timeVal.year-1;break;
					case 2:timeVal.month = (timeVal.month > 1)? timeVal.month - 1:12;break;
					case 3:timeVal.date = ((timeVal.date/10) == 0)?timeVal.date + 30:timeVal.date-10;break;
					case 4:timeVal.date = ((timeVal.date%10) == 0)?(timeVal.date/10)*10+9:timeVal.date-1;break;
				}
				break;
			}
			case 3://��
			{
				cursor = (cursor > 0)?cursor-1:6;
				while(getKey_Ctrl()==1 || getKey_Ctrl()== 3);
				break;
			}
			case 4://��
			{
				cursor = (cursor < 6)?cursor+1:0;
				while(getKey_Ctrl()==2 || getKey_Ctrl()== 4);
				break;
			}
			case 5://ȷ��
			{
				switch(cursor)
				{
					case 5://save
					{
						Time_Handle();
						TimeValue.year = timeVal.year;
						TimeValue.month = timeVal.month;
						TimeValue.date = timeVal.date;
						DS3231_ReadWrite_Time(0);
						SysState = NORMAL;
						flag_now = 0;
						break;
					}
					case 6:SysState = TIMEDATE;flag_now = 0;break;//back
				}
				while(getKey_Ctrl()==5);
				OLED_Clear();
				cursor = 0;
				break;
			}
		}
	}
	lastkey = getKey_Ctrl();
	
	
	//���ڼ��
	if(timeVal.date > 31)
	{
		timeVal.date = 31;
	}
	if(timeVal.month == 2 && timeVal.date > 29)
	{
		timeVal.date = 29;
	}
	if(timeVal.month == 4||timeVal.month == 6||timeVal.month == 9||timeVal.month == 11)
	{
		if(timeVal.date > 30)
		{
			timeVal.date = 30;
		}
	}
}

void DayStateFunc(void)
{
	static uint8_t flag_now = 0;
	static uint8_t cursor = 0;
	static Time_Typedef timeVal;
	uint8_t str[30];
	//��ǰδ��������
	if(flag_now == 0)
	{
		flag_now = 1;//ʹ�����������ݱ�־λ
		memcpy(&timeVal,&TimeValue,sizeof(Time_Typedef));
	}
	//��ʾ����
	sprintf(str,"    %02d:%02d:%02d",timeVal.hour,timeVal.minute,timeVal.second);
	sprintf(str,"    Week:%d",timeVal.week);
	OLED_ShowString(0,3,(uint8_t *)str,12);
	//��ʾ���
	
	if(cursor == 0)
	{
		OLED_ShowString(0,4,(uint8_t *)"         ^   ",12);
	}
	else
	{
		OLED_ShowString(0,4,(uint8_t *)"             ",12);
	}
	if(cursor>0)
	{
		if(cursor == 1)OLED_ShowString(0,7,(uint8_t *)"->Save     Back",12);
		else OLED_ShowString(0,7,(uint8_t *)"  Save   ->Back",12);
	}
	else
	{
		OLED_ShowString(0,7,(uint8_t *)"  Save     Back",12);
	}
	//��Ӧ�����л� ����+ ����- ȷ����ת
	switch(getKey_Ctrl())
	{ 
		case 1://��
		{
			if(cursor == 0)
			{
				timeVal.week = (timeVal.week == 7) ? 1 : timeVal.week + 1;
			}
			HAL_Delay(200);
			break;
		}
		case 2://��
		{
			if(cursor == 0)
			{
				timeVal.week = (timeVal.week == 1) ? 7 : timeVal.week - 1;
			}
			HAL_Delay(200);
			break;
		}
		case 3://��
		{
			cursor = (cursor > 0) ? cursor - 1 :2;
			while(getKey_Ctrl()==1)
			break;
		}
		case 4://��
		{
			cursor = (cursor < 2) ? cursor + 1:0;
			while(getKey_Ctrl()== 4)
			break;
		}
		case 5://ȷ��
		{
			switch(cursor)
			{
				case 1://save
				{
					Time_Handle();
					TimeValue.week = timeVal.week;
					DS3231_ReadWrite_Time(0);
					SysState = NORMAL;
					flag_now = 0;
					break;
				}
				case 3:SysState = TIMEDATE;flag_now = 0;break;//back
			}
			while(getKey_Ctrl()==5)
			OLED_Clear();
			cursor = 0;
			break;
		}
	}
}


/*Setup Loop and Interrupt Function --------------------------------------*/
void VFDApp_Setup(void)
{
	//��ʼVFDˢ��
	HAL_TIM_Base_Start_IT(&htim1);
	//�򿪻Թ��ӿ���
	NixieTubeControl(ENABLE);
	
	//������һ�׶�
	OLED_Clear();
	OLED_DrawBMP(0,0,128,8,(uint8_t *)BMP_Start);
	
	HAL_Delay(1000);
	
	//��������
	OLED_Clear();

	//�Թ�ܲ���
	NixieTubeTest(50);
//	uint8_t num[8] = {0,0,0,0,0,0,0,0};
//	for(int i = 0;i < 8;i++)
//	{
//		num[i] = 0;
//	}
//	NixieTubeDisplay(num);

	
}

void VFDApp_Loop(void) 
{
	//��ʾģʽ״̬��---------------------------------------------------------/
	switch(VFDDisMod)
	{
		case NTIME:
		{
			VFDNTIMEDisplay();
			SymbolFlash();
			break;
		}
		case NDATE:
		{
			VFDNDATEDisplay();
			break;
		}
		case TEMP:
		{
			VFDTEMPDisplay();
			break;
		}
		case CLOSE:
		{
			
			break;
		}
		default:break;
	}
	
	//ϵͳ����״̬��--------------------------------------------------------/
	switch(SysState)
	{
		case NORMAL:
		{
			InfoDisplay();//OLED��ʾ
			NormalDisModeChange();		//��ʾģʽ�л������˵��Ľ���
			break;
		}
		case MAIN:
		{
			MainFunctionChose();	//���˵�ѡ������ѡ��ѡ��
			break;
		}
		case TIMEDATE:
		{
			TimeDateConfig();		//ѡ����Ҫ�޸ĵ�value
			break;
		}
					case TIME:
					{
						TimeStateFunc();//ʱ���޸�
						break;
					}
					case DATE:
					{
						DateStateFunc();//�����޸�
						break;
					}
					case DAY:
					{
						DayStateFunc();//�����޸�
						break;
					}
		case WLAN:
		{
			SysState = NORMAL;
			break;
		}
					case SSID:
					{
						break;
					}
					case PSWD:
					{
						break;
					}
		default:break;
	}
	
	/*�������� -------------------------------------------------------------*/
	LEDFlash();	//led����ָʾ��
	GetTimeValue();//���RTC����ʱ��
}

void VFDApp_Interrupt(void)
{

}
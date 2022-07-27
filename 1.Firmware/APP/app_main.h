#ifndef __APP_MAIN_H
#define __APP_MAIN_H
#include "SystemConfig.h"

typedef struct ap_info
{
	char ssid[32] ;
	char bssid[32] ;
	char channel[10] ;
	char rssi[10] ;
}ap_info;

typedef struct GCodeCmdHandler_t
{
	uint8_t Type;
	char *GcodeCmd;
}GCodeCmdHandler_t;

//��Ļ��ʾ��ȡ��߶ȶ���
#define SCREEN_WIDTH    240
#define SCREEN_HEIGHT   240

//������ƽ̨��Ԫ��
#define PRODUCT_ID              ""
#define DEVICE_NAME             ""
#define DEVICE_KEY              ""

/*��ͷ�¶��ϱ�*/
#define REPORT_NOZZLE_TEMP_DATA_TEMPLATE  \
"{\\\"method\\\":\\\"report\\\"\\,\\\"clientToken\\\":\\\"00000001\\\"\\,\\\"params\\\":{\\\"nozzle_temp\\\":\\\"%s\\\"}}"

/*��ͷ�¶ȸ澯�ϱ�*/
#define REPORT_NOZZLE_TEMP_ALARM_DATA_TEMPLATE  \
"{\\\"method\\\":\\\"report\\\"\\,\\\"clientToken\\\":\\\"00000001\\\"\\,\\\"params\\\":{\\\"nozzle_temp_alarm\\\":\\\"%s\\\"}}"

/*�ȴ��¶��ϱ�*/
#define REPORT_HOTBED_TEMP_DATA_TEMPLATE  \
"{\\\"method\\\":\\\"report\\\"\\,\\\"clientToken\\\":\\\"00000001\\\"\\,\\\"params\\\":{\\\"hotbed_temp\\\":\\\"%s\\\"}}"

/*��ͷλ���ϱ�*/
#define REPORT_POS_DATA_TEMPLATE  \
"{\\\"method\\\":\\\"report\\\"\\,\\\"clientToken\\\":\\\"00000001\\\"\\,\\\"params\\\":{\\\"axis_text\\\":\\\"%s\\\"}}"

/*�ظ���Ϣ�ϱ�*/
#define CONTROL_REPLY_DATA_TEMPLATE       \
"{\\\"method\\\":\\\"control_reply\\\"\\,\\\"clientToken\\\":\\\"%s\\\"\\,\\\"code\\\":0\\,\\\"status\\\":\\\"ok\\\"}"

/*��ƽ�����ϱ�*/
#define REPORT_LEVELING_DATA1_TEMPLATE  \
"{\\\"method\\\":\\\"report\\\"\\,\\\"clientToken\\\":\\\"00000001\\\"\\,\\\"params\\\":{\\\"level1_data\\\":\\\"%s\\\"}}"

#define REPORT_LEVELING_DATA2_TEMPLATE  \
"{\\\"method\\\":\\\"report\\\"\\,\\\"clientToken\\\":\\\"00000001\\\"\\,\\\"params\\\":{\\\"level2_data\\\":\\\"%s\\\"}}"

#define REPORT_LEVELING_DATA3_TEMPLATE  \
"{\\\"method\\\":\\\"report\\\"\\,\\\"clientToken\\\":\\\"00000001\\\"\\,\\\"params\\\":{\\\"level3_data\\\":\\\"%s\\\"}}"

#define REPORT_LEVELING_DATA4_TEMPLATE  \
"{\\\"method\\\":\\\"report\\\"\\,\\\"clientToken\\\":\\\"00000001\\\"\\,\\\"params\\\":{\\\"level4_data\\\":\\\"%s\\\"}}"

#define REPORT_LEVELING_DATA_STATUS_TEMPLATE  \
"{\\\"method\\\":\\\"report\\\"\\,\\\"clientToken\\\":\\\"00000001\\\"\\,\\\"params\\\":{\\\"levelDataStatus\\\":\\\"%s\\\"}}"

enum MAPPING_CONTROL
{
	PRINTER_RESPONSE = 0, //��Ӧ
	PRINTER_PLA_PRE,   //PLAԤ��
	PRINTER_ABS_PRE,    //ABSԤ��
	PRINTER_TEMP_DROP,  //����
	PRINTER_X_ADD_10,   //X���ƶ�+10mm
	PRINTER_X_SUB_10,   //X���ƶ�-10mm
	PRINTER_Y_ADD_10,   //Y���ƶ�+10mm
	PRINTER_Y_SUB_10,   //Y���ƶ�-10mm
	PRINTER_Z_ADD_10,   //Z���ƶ�+10mm
	PRINTER_Z_SUB_10,   //Z���ƶ�-10mm
	PRINTER_X_ZERO,     //X�����
	PRINTER_Y_ZERO,     //Y�����
	PRINTER_Z_ZERO,     //Z�����
	PRINTER_ALL_ZERO,   //ȫ������
	PRINTER_LEVEING_GET,//��ƽ���ݻ�ȡ
	PRINTER_PRINTING    //��ʼ��ӡ
};

#define DEBUG_GET_TEMP_LED(STATUS)	\
		do{	\
			GPIO_WriteBit(GPIOE, GPIO_Pin_2, !STATUS); \
		}while(0);

#define DEBUG_LED(STATUS) \
		do{ \
			{GPIO_WriteBit(GPIOE, GPIO_Pin_3, !STATUS); \
			GPIO_WriteBit(GPIOE, GPIO_Pin_4, !STATUS); \
			GPIO_WriteBit(GPIOE, GPIO_Pin_5, !STATUS);}\
		}while(0);

void MqttTask(void);

#endif //__APP_MAIN_H


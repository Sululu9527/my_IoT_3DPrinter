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

typedef struct mqtt_report_info
{
	double hotend_cur;
	double hotend_target;
	double hotbed_cur;
	double hotbed_target;
	float X;
	float Y;
	float Z;
}report_info;

typedef struct mqtt_leveling_data
{
	char leveling_data1[30];
	char leveling_data2[30];
	char leveling_data3[30];
	char leveling_data4[30];
}report_leveling_data;

typedef struct GCodeCmdHandler_t
{
	uint8_t Index;
	char *GcodeCmd;
}GCodeCmdHandler_t;

//��Ļ��ʾ��ȡ��߶ȶ���
#define SCREEN_WIDTH    240
#define SCREEN_HEIGHT   240

//������ƽ̨��Ԫ��
#define PRODUCT_ID              "99LPJ2KWLM"
#define DEVICE_NAME             "vyper"
#define DEVICE_KEY              "WehUw9Da/ML2ngraKNEEEg=="

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

#define GCODE_PLA_PRE 	  	  "M140 S60\nM104 S190\n"
#define GCODE_ABS_PRE 	  	  "M140 S100\nM104 S240\n"
#define GCODE_TEMP_DROP 	  "M140 S0\nM104 S0\n"
#define GCODE_X_MOVE_ADD 	  "G91\nG1 F1500 X+10\nG90\nM114\n"
#define GCODE_X_MOVE_SUB      "G91\nG1 F1500 X-10\nG90\nM114\n"
#define GCODE_Y_MOVE_ADD 	  "G91\nG1 F1500 Y+10\nG90\nM114\n"
#define GCODE_Y_MOVE_SUB      "G91\nG1 F1500 Y-10\nG90\nM114\n"
#define GCODE_Z_MOVE_ADD 	  "G91\nG1 F1500 Z+10\nG90\nM114\nM114\n"
#define GCODE_Z_MOVE_SUB      "G91\nG1 F1500 Z-10\nG90\nM114\nM114\n"
#define GCODE_ZERO_OF_X	      "G28 X\n"
#define GCODE_ZERO_OF_Y	      "G28 Y\n"
#define GCODE_ZERO_OF_Z	      "G28 Z\n"
#define GCODE_ZERO_OF_ALL	  "G28\n"
#define GCODE_LEVELING_DATA   "M420V\n"
#define GCODE_FAN_SETTING 	  "M106 S%d\n"
#define GCODE_START_PRINT     "M23 TENCEN~1.GCO\nM24\n"

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


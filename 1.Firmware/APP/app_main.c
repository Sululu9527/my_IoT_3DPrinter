#include "app_main.h"

ap_info esp8266_info;

char payload[256] =
{ 0 };
char token_cache[128] =
{ 0 };
static char report_topic_name[TOPIC_NAME_MAX_SIZE] =
{ 0 };
static char report_reply_topic_name[TOPIC_NAME_MAX_SIZE] =
{ 0 };

static void WifiSmartConfig(void);
static void ReportClientToken(void);
static void ReportDeviceTemp(marlin_temp *temp);
static void ReportDeviceAxis(marlin_coordinate *axis);
static void ReportDeviceLevelData(uint8_t Type, char *data);

void MessageParamsHandler(mqtt_message_t* msg)
{
	struct Msg_t Msg;
	cJSON *root = NULL;
	cJSON *token = NULL;
	cJSON *params = NULL;
	cJSON *method = NULL;
	cJSON *led_control = NULL;
	cJSON *printer_control = NULL;
	cJSON *printer_fan_speed = NULL;
	char GCodeBuf[15] =
	{ 0 };
	double result_fan_speed;
#if 0
	printf("mqtt callback:\r\n");
	printf("---------------------------------------------------------\r\n");
	printf("\ttopic:%s\r\n", msg->topic);
	printf("\tpayload:%s\r\n", msg->payload);
	printf("---------------------------------------------------------\r\n");
#endif
	/*1. �������ƶ��յ��Ŀ�����Ϣ*/
	root = cJSON_Parse(msg->payload + 1);
	if (!root)
	{
		printf("Invalid json root\r\n");
		return;
	}
	/*2. ����method*/
	method = cJSON_GetObjectItem(root, "method");
	if (!method)
	{
		printf("Invalid json method\r\n");
		cJSON_Delete(root);
		return;
	}
	/*3. �������ƶ��·��� control ���ݣ�report_reply�ݲ�����*/
	if (0 != strncmp(method->valuestring, "control", strlen("control")))
	{
		cJSON_Delete(root);
		return;
	}
	/*4. ������params*/
	params = cJSON_GetObjectItem(root, "params");
	if (!params)
	{
		printf("Invalid json params\r\n");
		cJSON_Delete(root);
		return;
	}
	/*5. ����params������"params":{"power_switch":0}*/
	led_control = cJSON_GetObjectItem(params, "power_switch");
	if (led_control)
	{
		if (led_control->valueint)
		{
			DEBUG_LED(1)
		}
		else
		{
			DEBUG_LED(0)
		}
	}
	/*6. ����params������"params":{"printing_control":3}*/
	printer_control = cJSON_GetObjectItem(params, "printing_control");
	if (printer_control)
	{
		Msg.Type = printer_control->valueint;
		tos_msg_q_post(&GCodeMsg, (void *) &Msg);
	}

	printer_fan_speed = cJSON_GetObjectItem(params, "fan_speed");
	if (printer_fan_speed)
	{
		memset(GCodeBuf, 0, 15);
		result_fan_speed = (double) printer_fan_speed->valueint / 255 * 2.55
				* 100 * 2.55;
		snprintf(GCodeBuf, sizeof(GCodeBuf), GCODE_FAN_SETTING,
				(int) result_fan_speed);
		Msg.Type = MSG_2_GCODE_CMD_FAN_SETTING;
		memcpy(Msg.Data, GCodeBuf, sizeof(GCodeBuf));
		tos_msg_q_post(&GCodeMsg, (void *) &Msg);
	}

	/*7. ����clientToken�ظ�*/
	token = cJSON_GetObjectItem(root, "clientToken");
	if (token)
	{
		Msg.Type = MSG_CMD_UPDATE_TOKEN;
		tos_msg_q_post(&DataMsg, (void *) &Msg);
	}
	cJSON_Delete(root);
	root = NULL;
}

void MqttTask(void)
{
	int ret = 0;
	int size = 0;
	k_err_t err;
	void *MsgRecv;
	mqtt_state_t state;
	char *key = DEVICE_KEY;
	device_info_t dev_info;
	char *product_id = PRODUCT_ID;
	char *device_name = DEVICE_NAME;
	memset(&dev_info, 0, sizeof(device_info_t));
	LCD_ShowString(10, 10, "Wait Connecting...", WHITE, BLACK, 16, 0);
	ret = esp8266_tencent_firmware_sal_init(HAL_UART_PORT_2);
	if (ret < 0)
	{
		printf("esp8266 tencent firmware sal init fail, ret is %d\r\n", ret);
		NVIC_SystemReset();
	}
	tos_task_delay(6000);
	/*ִ��WIFI�����߼�*/
	WifiSmartConfig();
	strncpy(dev_info.product_id, product_id, PRODUCT_ID_MAX_SIZE);
	strncpy(dev_info.device_name, device_name, DEVICE_NAME_MAX_SIZE);
	strncpy(dev_info.device_serc, key, DEVICE_SERC_MAX_SIZE);
	tos_tf_module_info_set(&dev_info, TLS_MODE_PSK);

	mqtt_param_t init_params = DEFAULT_MQTT_PARAMS;
	if (tos_tf_module_mqtt_conn(init_params) != 0)
	{
		printf("module mqtt conn fail\n");
		NVIC_SystemReset();
	}

	if (tos_tf_module_mqtt_state_get(&state) != -1)
	{
		printf("MQTT: %s\n",
				state == MQTT_STATE_CONNECTED ? "CONNECTED" : "DISCONNECTED");
	}

	/* ��ʼ����topic */
	size = snprintf(report_reply_topic_name, TOPIC_NAME_MAX_SIZE,
			"$thing/down/property/%s/%s", product_id, device_name);
	if (size < 0 || size > sizeof(report_reply_topic_name) - 1)
	{
		printf(
				"sub topic content length not enough! content size:%d  buf size:%d",
				size, (int) sizeof(report_reply_topic_name));
		return;
	}
	if (tos_tf_module_mqtt_sub(report_reply_topic_name, QOS0,
			MessageParamsHandler) != 0)
	{
		printf("module mqtt sub fail\n");
		NVIC_SystemReset();
	}

	memset(report_topic_name, 0, sizeof(report_topic_name));
	size = snprintf(report_topic_name, TOPIC_NAME_MAX_SIZE,
			"$thing/up/property/%s/%s", product_id, device_name);
	if (size < 0 || size > sizeof(report_topic_name) - 1)
	{
		printf(
				"pub topic content length not enough! content size:%d  buf size:%d",
				size, (int) sizeof(report_topic_name));
	}

	LCD_ShowString(10, 10, "MQTT Connect OK", WHITE, BLACK, 16, 0);
	MqttInitFlag = 1;
	struct Msg_t Msg;
	while (1)
	{
		err = tos_msg_q_pend(&DataMsg, &MsgRecv, TOS_TIME_FOREVER);
		if(K_ERR_NONE == err)
		{
			memcpy(&Msg,MsgRecv,sizeof(struct Msg_t));
			switch(Msg.Type)
			{
				case MSG_CMD_UPDATE_TEMP:
					ReportDeviceTemp((marlin_temp *)&Msg.Data);
				break;

				case MSG_CMD_UPDATE_AXIS:
					ReportDeviceAxis((marlin_coordinate *)&Msg.Data);
				break;

				case MSG_CMD_UPDATE_TOKEN:
					ReportClientToken();
				break;

				case MSG_CMD_LEVELING_1:case MSG_CMD_LEVELING_2:
				case MSG_CMD_LEVELING_3:case MSG_CMD_LEVELING_4:
					ReportDeviceLevelData(Msg.Type,Msg.Data);
				break;

				default:
				break;
			}
		}
		osDelay(5);
	}
}

//�ϱ��豸�¶�
static void ReportDeviceTemp(marlin_temp *temp)
{
	char buf[50] =
	{ 0 };
	int nozzle_temp = 0;
	static uint8_t report_alarm = 0;
	/*�ȴ��¶��ϱ�*/
	memset(buf, 0, sizeof(buf));
	memset(payload, 0, sizeof(payload));
	snprintf(buf, sizeof(buf), "%.2lf/%.2lf", temp->hotbed_cur_temp,
			temp->hotbed_target_temp);
	snprintf(payload, sizeof(payload), REPORT_HOTBED_TEMP_DATA_TEMPLATE, buf);
	if (tos_tf_module_mqtt_pub(report_topic_name, QOS0, payload) != 0)
		NVIC_SystemReset();
	/*��ͷ�¶��ϱ�*/
	memset(buf, 0, sizeof(buf));
	memset(payload, 0, sizeof(payload));
	snprintf(buf, sizeof(buf), "%.2lf/%.2lf", temp->nozzle_cur_temp,
			temp->nozzle_target_temp);
	snprintf(payload, sizeof(payload), REPORT_NOZZLE_TEMP_DATA_TEMPLATE, buf);
	if (tos_tf_module_mqtt_pub(report_topic_name, QOS0, payload) != 0)
		NVIC_SystemReset();
	//�¶ȸ澯�ϱ�
	memset(payload, 0, sizeof(payload));
	nozzle_temp = (int) temp->nozzle_cur_temp;
	if (nozzle_temp > 120)
	{
		if (0 == report_alarm)
		{
			report_alarm = 1;
			memset(buf, 0, sizeof(buf));
			snprintf(buf, sizeof(buf), "%d", nozzle_temp);
			snprintf(payload, sizeof(payload),
			REPORT_NOZZLE_TEMP_ALARM_DATA_TEMPLATE, buf);
			if (tos_tf_module_mqtt_pub(report_topic_name, QOS0, payload) != 0)
				NVIC_SystemReset();
		}
	}
	if (nozzle_temp < 120)
		nozzle_temp = 0;

}

//�ϱ��豸����
static void ReportDeviceAxis(marlin_coordinate *axis)
{
	char buf[50] =
	{ 0 };
	memset(buf, 0, sizeof(buf));
	memset(payload, 0, sizeof(payload));
	snprintf(buf, sizeof(buf), "X:%.1f Y:%.1f Z:%.1f", axis->X, axis->Y,
			axis->Z);
	snprintf(payload, sizeof(payload), REPORT_POS_DATA_TEMPLATE, buf);
	if (tos_tf_module_mqtt_pub(report_topic_name, QOS0, payload) != 0)
		NVIC_SystemReset();
}

//�ϱ�ClientToken
static void ReportClientToken(void)
{
	char buf[50] =
	{ 0 };
	static uint32_t counter = 0;
	memset(buf, 0, sizeof(buf));
	memset(payload, 0, sizeof(payload));
	memset(token_cache, 0, sizeof(token_cache));
	snprintf(token_cache, sizeof(token_cache), "%d", counter++);
	snprintf(payload, sizeof(payload), CONTROL_REPLY_DATA_TEMPLATE,
			token_cache);
	if (tos_tf_module_mqtt_pub(report_topic_name, QOS0, payload) != 0)
		NVIC_SystemReset();
}

//�ϱ���ƽ����
static void ReportDeviceLevelData(uint8_t Type, char *data)
{
	char buf[50] =
	{ 0 };
	static uint8_t flag = 0;
	if (0 == flag)
	{
		memset(buf, 0, sizeof(buf));
		memset(payload, 0, sizeof(payload));
		snprintf(buf, sizeof(buf), "%s", "Get Leveling data...");
		snprintf(payload, sizeof(payload), REPORT_LEVELING_DATA_STATUS_TEMPLATE,
				buf);
		if (tos_tf_module_mqtt_pub(report_topic_name, QOS0, payload) != 0)
			NVIC_SystemReset();
		memset(buf, 0, sizeof(buf));
		memset(payload, 0, sizeof(payload));
		snprintf(buf, sizeof(buf), "%s", "N/A");
		snprintf(payload, sizeof(payload), REPORT_LEVELING_DATA1_TEMPLATE, buf);
		if (tos_tf_module_mqtt_pub(report_topic_name, QOS0, payload) != 0)
			NVIC_SystemReset();
		memset(buf, 0, sizeof(buf));
		memset(payload, 0, sizeof(payload));
		snprintf(buf, sizeof(buf), "%s", "N/A");
		snprintf(payload, sizeof(payload), REPORT_LEVELING_DATA2_TEMPLATE, buf);
		if (tos_tf_module_mqtt_pub(report_topic_name, QOS0, payload) != 0)
			NVIC_SystemReset();
		memset(buf, 0, sizeof(buf));
		memset(payload, 0, sizeof(payload));
		snprintf(buf, sizeof(buf), "%s", "N/A");
		snprintf(payload, sizeof(payload), REPORT_LEVELING_DATA3_TEMPLATE, buf);
		if (tos_tf_module_mqtt_pub(report_topic_name, QOS0, payload) != 0)
			NVIC_SystemReset();
		memset(buf, 0, sizeof(buf));
		memset(payload, 0, sizeof(payload));
		snprintf(buf, sizeof(buf), "%s", "N/A");
		snprintf(payload, sizeof(payload), REPORT_LEVELING_DATA4_TEMPLATE, buf);
		if (tos_tf_module_mqtt_pub(report_topic_name, QOS0, payload) != 0)
			NVIC_SystemReset();
		flag = 1;
	}
	memset(buf, 0, sizeof(buf));
	memset(payload, 0, sizeof(payload));
	switch (Type)
	{
	case MSG_CMD_LEVELING_1:
		snprintf(buf, sizeof(buf), "%s", data);
		snprintf(payload, sizeof(payload), REPORT_LEVELING_DATA1_TEMPLATE, buf);
		break;
	case MSG_CMD_LEVELING_2:
		snprintf(buf, sizeof(buf), "%s", data);
		snprintf(payload, sizeof(payload), REPORT_LEVELING_DATA2_TEMPLATE, buf);
		break;
	case MSG_CMD_LEVELING_3:
		snprintf(buf, sizeof(buf), "%s", data);
		snprintf(payload, sizeof(payload), REPORT_LEVELING_DATA3_TEMPLATE, buf);
		break;
	case MSG_CMD_LEVELING_4:
		snprintf(buf, sizeof(buf), "%s", data);
		snprintf(payload, sizeof(payload), REPORT_LEVELING_DATA4_TEMPLATE, buf);
		flag = 0;
		break;
	default:
		break;
	}

	if (tos_tf_module_mqtt_pub(report_topic_name, QOS0, payload) != 0)
		NVIC_SystemReset();

	if (MSG_CMD_LEVELING_4 == Type)
	{
		memset(buf, 0, sizeof(buf));
		memset(payload, 0, sizeof(payload));
		snprintf(buf, sizeof(buf), "%s", "Get Data Success!");
		snprintf(payload, sizeof(payload), REPORT_LEVELING_DATA_STATUS_TEMPLATE,
				buf);
		if (tos_tf_module_mqtt_pub(report_topic_name, QOS0, payload) != 0)
			NVIC_SystemReset();
	}
}

static void WifiSmartConfig(void)
{
	int rssi;
	int channel = -1;
	static uint8_t ConfigWifi = 0;
	char ssid[50] =
	{ 0 };
	char bssid[50] =
	{ 0 };
	/*��ȡWIFI AP��Ϣ���������-1����˵����ȡ���ɹ�*/
	/*������ǰ��������3�������WIFI����ģʽ*/
	if (-1 == tos_tf_module_get_info(ssid, bssid, &channel, &rssi)
			|| 3 == Key_Scan())
	{
		/*��������ģʽ & ��ʾWIFI������ά��*/
		LCD_Fill(0, 0, 240, 240, WHITE);
		GPIO_WriteBit(GPIOE, GPIO_Pin_5, 0); //����
		LCD_ShowString(30, 10, "3D Printer Add", BLACK, WHITE, 24, 0);
		LCD_ShowPicture((240 - 150) / 2, (240 - 150) / 2, 150, 150,
				gImage_wifi_config);
		if (0 == tos_tf_module_smartconfig_start())
		{
			ConfigWifi = 1;
			LCD_Fill(0, 0, 240, 240, BLACK);
			tos_tf_module_smartconfig_stop();
		}
		else
		{
			LCD_ShowString(10, 10, "WifiConfig Error!", WHITE, BLACK, 32, 0);
			NVIC_SystemReset();
		}
	}
	if (0 == ConfigWifi)
		LCD_Fill(0, 0, 240, 120, BLACK);
	GPIO_WriteBit(GPIOE, GPIO_Pin_5, 1); //���
	snprintf(esp8266_info.ssid, sizeof(esp8266_info.ssid), "ssid:%s", ssid);
	snprintf(esp8266_info.bssid, sizeof(esp8266_info.bssid), "bssid:%s", bssid);
	snprintf(esp8266_info.channel, sizeof(esp8266_info.channel), "channel:%d",
			channel);
	snprintf(esp8266_info.rssi, sizeof(esp8266_info.rssi), "rssi:%d", rssi);
	LCD_ShowString(10, 10, "Wifi Connect OK", WHITE, BLACK, 16, 0);
	LCD_ShowString(10, 26, esp8266_info.ssid, WHITE, BLACK, 16, 0);
	LCD_ShowString(10, 26 + 16, esp8266_info.bssid, WHITE, BLACK, 16, 0);
	LCD_ShowString(10, 26 + 16 + 16, esp8266_info.channel, WHITE, BLACK, 16, 0);
	LCD_ShowString(10, 26 + 16 + 16 + 16, esp8266_info.rssi, WHITE, BLACK, 16,
			0);
	LCD_ShowPicture(0, 190, 240, 50, gImage_icon_for_tencentos_tiny);
}

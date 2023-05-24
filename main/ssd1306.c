/* i2c - Example

   For other examples please check:
   https://github.com/espressif/esp-idf/tree/master/examples

   See README.md file to get detailed usage of this example.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "sdkconfig.h"
#include <string.h>
#include "ssd1306.h"
#include "font8x8_basic.h"
//#include "logouit.h"

static const char *TAG = "i2c-example";

#define _I2C_NUMBER(num) I2C_NUM_##num
#define I2C_NUMBER(num) _I2C_NUMBER(num)

#define I2C_MASTER_SCL_IO 22               /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO 21              /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUMBER(0) /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ 10000        /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0                           /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0                           /*!< I2C master doesn't need buffer */

#define WRITE_BIT I2C_MASTER_WRITE              /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ                /*!< I2C master read */
#define ACK_CHECK_EN 0x1                        /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0                       /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                             /*!< I2C ack value */
#define NACK_VAL 0x1                            /*!< I2C nack value */

SemaphoreHandle_t print_mux = NULL;

uint8_t image1[128][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 
	{0x00, 0x1f, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00}, 
	{0x00, 0x7f, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00}, 
	{0x00, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00}, 
	{0x01, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00}, 
	{0x01, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00}, 
	{0x03, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00}, 
	{0x03, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00}, 
	{0x07, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00}, 
	{0x07, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00}, 
	{0x0f, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00}, 
	{0x0f, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00}, 
	{0x0f, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00}, 
	{0x0f, 0xff, 0x8f, 0xff, 0x00, 0x00, 0x00, 0x00}, 
	{0x1f, 0xfc, 0x01, 0xff, 0x80, 0x00, 0x00, 0x00}, 
	{0x1f, 0xf8, 0x00, 0x7f, 0xc2, 0x00, 0x00, 0x00}, 
	{0x1f, 0xf0, 0x00, 0x3f, 0xc1, 0x20, 0x00, 0x00}, 
	{0x1f, 0xe0, 0x00, 0x1f, 0xe1, 0xa0, 0x00, 0x00}, 
	{0x1f, 0xe0, 0x00, 0x0f, 0xf0, 0x96, 0x00, 0x00}, 
	{0x1f, 0xc0, 0x00, 0x07, 0xf8, 0x53, 0x00, 0x00}, 
	{0x3f, 0xc0, 0x00, 0x03, 0xf8, 0x4b, 0x80, 0x00}, 
	{0x3f, 0x80, 0x00, 0x01, 0xfc, 0x2d, 0xc0, 0x00}, 
	{0x3f, 0x80, 0x00, 0x00, 0xfc, 0x24, 0xe0, 0x00}, 
	{0x3f, 0x80, 0x08, 0x00, 0x7e, 0x16, 0xf0, 0x00}, 
	{0x3f, 0x00, 0x18, 0x00, 0x7f, 0x1a, 0x78, 0x00}, 
	{0x3f, 0x00, 0x38, 0x00, 0x3f, 0x0b, 0x7c, 0x00}, 
	{0x3f, 0x00, 0x38, 0x00, 0x1f, 0x8d, 0x7c, 0x00}, 
	{0x3f, 0x00, 0x78, 0x00, 0x0f, 0x85, 0xbe, 0x00}, 
	{0x3e, 0x00, 0x78, 0x0c, 0x0f, 0xc6, 0xbf, 0x00}, 
	{0x3e, 0x00, 0xf8, 0x3f, 0x07, 0xc2, 0x9f, 0x00}, 
	{0x3e, 0x00, 0xf8, 0x3f, 0x03, 0xe2, 0x5f, 0x80}, 
	{0x3e, 0x01, 0xf8, 0x7f, 0x03, 0xe1, 0x4f, 0x80}, 
	{0x3e, 0x01, 0xf8, 0x7f, 0x01, 0xf1, 0x2f, 0xc0}, 
	{0x3e, 0x03, 0xf0, 0x7f, 0x01, 0xf0, 0xa7, 0xc0}, 
	{0x3e, 0x03, 0xf0, 0x7f, 0x00, 0xf8, 0x97, 0xc0}, 
	{0x3e, 0x07, 0xe1, 0x7f, 0x00, 0x78, 0x57, 0xe0}, 
	{0x1e, 0x07, 0xe3, 0x3f, 0x00, 0x7c, 0x5b, 0xe0}, 
	{0x1e, 0x07, 0xc3, 0x00, 0x00, 0x3c, 0x2b, 0xe0}, 
	{0x1e, 0x0f, 0xc3, 0x00, 0x00, 0x3c, 0x29, 0xf0}, 
	{0x1e, 0x0f, 0x87, 0x0e, 0x00, 0x1e, 0x25, 0xf0}, 
	{0x1e, 0x0f, 0x87, 0x06, 0x60, 0x1e, 0x15, 0xf0}, 
	{0x1e, 0x1f, 0x8f, 0x62, 0x70, 0x0f, 0x14, 0xf8}, 
	{0x1e, 0x1f, 0x0f, 0x70, 0x7c, 0x0f, 0x0a, 0xf8}, 
	{0x1e, 0x1f, 0x1f, 0x78, 0x7e, 0x07, 0x8a, 0xf8}, 
	{0x1e, 0x1f, 0x1f, 0x7e, 0x3f, 0x07, 0x89, 0x78}, 
	{0x0e, 0x1f, 0x3f, 0x7e, 0x3f, 0x07, 0x85, 0x7c}, 
	{0x0e, 0x3e, 0x3f, 0x7e, 0x1f, 0x83, 0xc5, 0x3c}, 
	{0x0e, 0x3e, 0x3e, 0x3e, 0x8f, 0xc3, 0xc4, 0xbc}, 
	{0x0e, 0x3e, 0x7c, 0x3c, 0xc7, 0xe1, 0xc2, 0xbc}, 
	{0x0e, 0x3e, 0x78, 0x3c, 0xe7, 0xe1, 0xe2, 0x9c}, 
	{0x0e, 0x3e, 0xf8, 0x1c, 0xf3, 0xe0, 0xe2, 0x5c}, 
	{0x06, 0x3e, 0xf0, 0x1c, 0xf3, 0xe0, 0xe1, 0x5c}, 
	{0x06, 0x3f, 0xe0, 0x0d, 0xf9, 0xc0, 0xf1, 0x5c}, 
	{0x06, 0x3f, 0xc0, 0x09, 0xf8, 0xc0, 0x71, 0x2e}, 
	{0x07, 0x3f, 0x80, 0x09, 0xfc, 0xc0, 0x70, 0xae}, 
	{0x07, 0x7f, 0x80, 0x01, 0xfe, 0x00, 0x78, 0xae}, 
	{0x03, 0x7f, 0x00, 0x03, 0xfe, 0x00, 0x38, 0xa6}, 
	{0x03, 0x7e, 0x00, 0x03, 0xfe, 0x00, 0x38, 0x56}, 
	{0x03, 0x7c, 0x00, 0x03, 0xff, 0x00, 0x3c, 0x56}, 
	{0x03, 0x7e, 0x00, 0x03, 0xff, 0x00, 0x1c, 0x56}, 
	{0x01, 0x7e, 0x00, 0x03, 0xfe, 0x00, 0x1c, 0x4a}, 
	{0x01, 0x7f, 0x00, 0x03, 0xfe, 0x00, 0x1e, 0x2a}, 
	{0x01, 0xff, 0x80, 0x01, 0xfc, 0x80, 0x0e, 0x28}, 
	{0x01, 0xff, 0xc0, 0x09, 0xfc, 0xc0, 0x0e, 0x28}, 
	{0x00, 0xff, 0xe0, 0x0d, 0xf9, 0xc0, 0x0e, 0x24}, 
	{0x00, 0xfe, 0xe0, 0x1d, 0xf9, 0xc0, 0x0f, 0x14}, 
	{0x00, 0xfe, 0xf0, 0x1c, 0xf3, 0xe0, 0x07, 0x14}, 
	{0x00, 0x7e, 0xf8, 0x1c, 0xe3, 0xe0, 0x07, 0x14}, 
	{0x00, 0x7e, 0x7c, 0x3c, 0xe7, 0xe0, 0x07, 0x10}, 
	{0x00, 0x7e, 0x7e, 0x3e, 0xcf, 0xc0, 0x07, 0x88}, 
	{0x00, 0x7e, 0x3e, 0x7e, 0x0f, 0xc0, 0x03, 0x88}, 
	{0x00, 0x3f, 0x3f, 0x7e, 0x1f, 0x80, 0x03, 0x88}, 
	{0x00, 0x3f, 0x1f, 0x7e, 0x3f, 0x00, 0x03, 0x88}, 
	{0x00, 0x3f, 0x1f, 0x7c, 0x7e, 0x00, 0x03, 0xc0}, 
	{0x00, 0x1f, 0x0f, 0x78, 0x7c, 0x00, 0x01, 0xc0}, 
	{0x00, 0x1f, 0x8f, 0x60, 0x78, 0x00, 0x01, 0xc0}, 
	{0x00, 0x1f, 0x8f, 0x02, 0x60, 0x00, 0x01, 0xc0}, 
	{0x00, 0x0f, 0x87, 0x06, 0x00, 0x00, 0x01, 0xc0}, 
	{0x00, 0x0f, 0xc7, 0x00, 0x00, 0x00, 0x01, 0xc0}, 
	{0x00, 0x07, 0xc3, 0x00, 0x00, 0x00, 0x00, 0xe0}, 
	{0x00, 0x07, 0xe3, 0x00, 0x00, 0x00, 0x00, 0xe0}, 
	{0x00, 0x07, 0xe1, 0x7f, 0x00, 0x00, 0x00, 0xe0}, 
	{0x00, 0x03, 0xf1, 0x7f, 0x00, 0x00, 0x00, 0xe0}, 
	{0x00, 0x03, 0xf0, 0x7f, 0x00, 0x00, 0x00, 0xe0}, 
	{0x00, 0x03, 0xf8, 0x7f, 0x00, 0x00, 0x00, 0x60}, 
	{0x00, 0x01, 0xf8, 0x7f, 0x00, 0x00, 0x00, 0xf0}, 
	{0x00, 0x01, 0xf8, 0x7f, 0x00, 0x00, 0x00, 0xf0}, 
	{0x00, 0x00, 0xf8, 0x3f, 0x00, 0x00, 0x01, 0x70}, 
	{0x00, 0x00, 0xf8, 0x1e, 0x00, 0x00, 0x01, 0x70}, 
	{0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x02, 0x70}, 
	{0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x06, 0x70}, 
	{0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x04, 0x30}, 
	{0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x08, 0x30}, 
	{0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x18, 0x30}, 
	{0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x30, 0x30}, 
	{0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x60, 0x30}, 
	{0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0xc0, 0x30}, 
	{0x00, 0x00, 0x01, 0x80, 0x00, 0x01, 0x80, 0x30}, 
	{0x00, 0x00, 0x00, 0xe0, 0x00, 0x03, 0x00, 0x38}, 
	{0x00, 0x00, 0x00, 0x30, 0x00, 0x0c, 0x00, 0x38}, 
	{0x00, 0x00, 0x00, 0x1e, 0x00, 0x78, 0x00, 0x78}, 
	{0x00, 0x00, 0x00, 0x07, 0xff, 0xe0, 0x00, 0xfc}, 
	{0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xfe}, 
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xfe}, 
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xfe}, 
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff}, 
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff}, 
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff}, 
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff}, 
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xfe}, 
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xfe}, 
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xfe}, 
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc}, 
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc}, 
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70}, 
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0}, 
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0}, 
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xc0}, 
	{0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x01, 0xc0}, 
	{0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x03, 0x80}, 
	{0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x03, 0x80}, 
	{0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x07, 0x00}, 
	{0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x0f, 0x00}, 
	{0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x3e, 0x00}, 
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xfc, 0x00}, 
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xf0, 0x00}, 
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x00}, 
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
};

/**
 * @brief i2c master initialization
 */
static esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    i2c_param_config(i2c_master_port, &conf);
    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

void ssd1306_init() {
	ESP_ERROR_CHECK(i2c_master_init());
	esp_err_t espRc;

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();

	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);

	i2c_master_write_byte(cmd, OLED_CMD_SET_CHARGE_PUMP, true); // setting charge pump
	i2c_master_write_byte(cmd, 0x14, true);// enable charge pump

	i2c_master_write_byte(cmd, OLED_CMD_SET_SEGMENT_REMAP, true); // reverse left-right mapping
	i2c_master_write_byte(cmd, OLED_CMD_SET_COM_SCAN_MODE, true); // reverse up-bottom mapping

	i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_NORMAL, true);
    i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_OFF, true);
	i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_ON, true);
	i2c_master_stop(cmd);

	espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
	if (espRc == ESP_OK) {
		ESP_LOGI(TAG, "OLED configured successfully");
	} else {
		ESP_LOGE(TAG, "OLED configuration failed. code: 0x%.2X", espRc);
	}
	i2c_cmd_link_delete(cmd);
}



void task_ssd1306_display_text(const void *arg_text) {
	char *text = (char*)arg_text;
	uint8_t text_len = strlen(text);

	i2c_cmd_handle_t cmd;

	uint8_t cur_page = 0;

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
	i2c_master_write_byte(cmd, 0x00, true); // reset column - choose column --> 0
	i2c_master_write_byte(cmd, 0x10, true); // reset line - choose line --> 0
	i2c_master_write_byte(cmd, 0xB0 | cur_page, true); // reset page

	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

	for (uint8_t i = 0; i < text_len; i++) {
		if (text[i] == '\n') {
			cmd = i2c_cmd_link_create();
			i2c_master_start(cmd);
			i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

			i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
			i2c_master_write_byte(cmd, 0x00, true); // reset column
			i2c_master_write_byte(cmd, 0x10, true);
			i2c_master_write_byte(cmd, 0xB0 | ++cur_page, true); // increment page

			i2c_master_stop(cmd);
			i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
			i2c_cmd_link_delete(cmd);
		} else {
			cmd = i2c_cmd_link_create();
			i2c_master_start(cmd);
			i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

			i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
			i2c_master_write(cmd, font8x8_basic_tr[(uint8_t)text[i]], 8, true);

			i2c_master_stop(cmd);
			i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
			i2c_cmd_link_delete(cmd);
		}
	}

	//vTaskDelete(NULL);
}

void task_ssd1306_display_text_in_line(const void *arg_text, uint8_t curpage) {
	char *text = (char*)arg_text;
	uint8_t text_len = strlen(text);

	i2c_cmd_handle_t cmd;

	//uint8_t cur_page = 0;

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
	i2c_master_write_byte(cmd, 0x00, true); // reset column - choose column --> 0
	i2c_master_write_byte(cmd, 0x10, true); // reset line - choose line --> 0
	i2c_master_write_byte(cmd, 0xB0 | curpage, true); // reset page

	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

	for (uint8_t i = 0; i < text_len; i++) {
		
			cmd = i2c_cmd_link_create();
			i2c_master_start(cmd);
			i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

			i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
			i2c_master_write(cmd, font8x8_basic_tr[(uint8_t)text[i]], 8, true);

			i2c_master_stop(cmd);
			i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
			i2c_cmd_link_delete(cmd);
		}
	

	//vTaskDelete(NULL);
}

void task_ssd1306_display_image()
{
	i2c_cmd_handle_t cmd;

	uint8_t cur_page = 7;
	uint8_t segs[128];

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
	i2c_master_write_byte(cmd, 0x00, true); // reset column - choose column --> 0
	i2c_master_write_byte(cmd, 0x10, true); // reset line - choose line --> 0
	i2c_master_write_byte(cmd, 0xB0 | cur_page, true); // reset page

	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

	for (uint8_t j = 0; j < 8; j++) {
        for (uint8_t i = 0; i < 128; i++){
			segs[i] = image1[i][j];	
		}
		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);		
		i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
	    i2c_master_write(cmd, segs, 128, true);

		i2c_master_stop(cmd);
		i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);
        
        // Move next pag
		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
		i2c_master_write_byte(cmd, 0x00, true); // reset column
		i2c_master_write_byte(cmd, 0x10, true);
		i2c_master_write_byte(cmd, 0xB0 | --cur_page, true); // increment page

		i2c_master_stop(cmd);
		i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);
	}

	//vTaskDelete(NULL);
}

void task_ssd1306_display_clear() {
	i2c_cmd_handle_t cmd;

	uint8_t clear[128];
	for (uint8_t i = 0; i < 128; i++) {
		clear[i] = 0;
	}
	for (uint8_t i = 0; i < 8; i++) {
		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true);
		i2c_master_write_byte(cmd, 0xB0 | i, true);

		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
		i2c_master_write(cmd, clear, 128, true);
		i2c_master_stop(cmd);
		i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);
	}

	//vTaskDelete(NULL);
}

void OLED_TASK(void* param)
{
	//task_ssd1306_display_clear();
    ssd1306_init();
    task_ssd1306_display_text("Group 2:\n20521365\n20521406\n20521647\n20521664");
	//vTaskDelay(pdMS_TO_TICKS(2000));
	//task_ssd1306_display_clear();
	//task_ssd1306_display_pattern();
}

/**
 * @brief test function to show buffer
 *
static void disp_buf(uint8_t *buf, int len)
{
    int i;
    for (i = 0; i < len; i++) {
        printf("%02x ", buf[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}
*/

/*void app_main(void)
{
    ESP_LOGI(TAG, "I2C initialized successfully");
    xTaskCreate(&OLED_TASK, "i2c_test_task_0", 1024 * 2, (void *)0, 10, NULL);

}*/

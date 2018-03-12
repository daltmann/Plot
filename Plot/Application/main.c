/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/** @file
 *
 * @defgroup ble_sdk_app_template_main main.c
 * @{
 * @ingroup ble_sdk_app_template
 * @brief Template project main file.
 *
 * This file contains a template for creating a new application. It has the code necessary to wakeup
 * from button, advertise, get a connection restart advertising on disconnect and if no new
 * connection created go back to system-off mode.
 * It can easily be used as a starting point for creating a new application, the comments identified
 * with 'YOUR_JOB' indicates where and how you can customize.
 */

#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"

#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"

#include "app_timer.h"
#include "app_error.h"
//#include "device_manager.h"
//#include "pstorage.h"
//#include "app_trace.h"
#include "bsp.h"
#include "bsp_btn_ble.h"

#include "peer_manager.h"
#include "fds.h"
#include "ble_conn_state.h"
#include "nrf_ble_gatt.h"

#include "nrf_delay.h"

#include "Scenario.h"

#include "nrf_pwr_mgmt.h"

// Version PLOT
#ifdef PLOT_BOARD
#define CUSTOM_SERVICES
#define POWER_SERVICES
#define PWM_DRIVER
#define UART_BOARD
#define UART_BOARD_SEND
#define MAIN_TIMER
#define ADC
#define BOARD_PLOT

#define SCENARIO_C5

#define RESEAUX

#define BLE_GENERIC

#define RTC_PLOT

//#define LOG_BOARD
#endif

// Version SPLIT
#ifdef SPLIT_BOARD
#define SPLIT_VERSION
#define BOARD_PLOT
#define MAIN_TIMER
#define UART_BOARD

#define PIR_SENSOR
//#define LOG_BOARD
#endif

// Activation du mode veille, pour descendre la consommation
#define PWR_MGMT

// Activation du mode DFU
//#define DFU_PLOT

// Cas de la démo du PLOT
//#define PLOT_DEMO



#ifdef LOG_BOARD
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
//#include "nrf_log_internal.h"

//#include "nrf_log_backend_uart.h"
//#include "nrf_log_backend_serial.h"
#endif

#ifdef CUSTOM_SERVICES
#include "ble_cus.h"
#endif

#ifdef POWER_SERVICES
#include "ble_power.h"
#endif

#ifdef SCENARIO_C5
#include "ble_Scenario_C5.h"
#endif

#ifdef RESEAUX
#include "ble_Reseaux.h"
#include "Reseaux.h"
#include "Sequence.h"
#endif

#ifdef BLE_GENERIC
#include "ble_Generic.h"
#endif

//#include "nrf_ble_gatt.h"
//#include "ble_conn_state.h"
#ifdef PWM_DRIVER
#include "nrf_drv_pwm.h"
#endif

#ifdef MAIN_TIMER
#include "nrf_drv_clock.h"
#endif

#ifdef UART_BOARD
#include "app_uart.h"
#include <stdio.h>

#include "nrf_drv_uart.h"
//#include "app_util_platform.h"
#endif

#ifdef ADC
#include "nrf_saadc.h"
#include "nrf_drv_saadc.h"
#include "nrf_drv_ppi.h"
#include "nrf_drv_timer.h"
#endif

#ifdef DFU_PLOT
#include "ble_dfu.h"
#endif

#ifdef SPLIT_VERSION
#include "Epever_Serie.h"
#include "ble_readEpever.h"
#include "ble_Scenario.h"
#include "ble_compute.h"
#include "Compute.h"
#endif

#ifdef PLOT_BOARD
#include "Compute.h"
#endif

#ifdef RTC_PLOT
#include "nrf_drv_rtc.h"
#endif

#define APP_FEATURE_NOT_SUPPORTED        BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2       /**< Reply when unsupported features are requested. */

#define IS_SRVC_CHANGED_CHARACT_PRESENT  1                                          /**< Include or not the service_changed characteristic. if not enabled, the server's database cannot be changed for the lifetime of the device*/

#define CENTRAL_LINK_COUNT               0                                          /**<number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT            1                                          /**<number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/

#ifdef SPLIT_VERSION
#define DEVICE_NAME                      "Split"                                     /**< Name of device. Will be included in the advertising data. */
#else
#define DEVICE_NAME                      "Plot"                                     /**< Name of device. Will be included in the advertising data. */
#endif
#define MANUFACTURER_NAME                "Nowatt-Lighting"                          /**< Manufacturer. Will be passed to Device Information Service. */
//#define APP_ADV_INTERVAL                 3000                                        /**< The advertising interval (in units of 0.625 ms. This value corresponds to 25 ms). */
#define APP_ADV_INTERVAL                 300                                        /**< The advertising interval (in units of 0.625 ms. This value corresponds to 25 ms). */
#define APP_ADV_TIMEOUT_IN_SECONDS       0                                          /**< The advertising timeout in units of seconds. */

#ifdef SPLIT_VERSION
#define MODEL_NUM              			"Model01"
//#define SERIAL_NUM           			"10000"
#define HW_REV_NUM       				"1.0"
#define FW_REV_NUM       				"1.1"
#endif

#ifdef PLOT_BOARD
#define MODEL_NUM              			"Model01"
//#define SERIAL_NUM           			"10000"
#define HW_REV_NUM       				"1.0"
#define FW_REV_NUM       				"1.2"
#define FIRMWARE_VERSION				0x12
#endif

#define APP_BLE_OBSERVER_PRIO            1                                          /**< Application's BLE observer priority. You shouldn't need to modify this value. */
#define APP_BLE_CONN_CFG_TAG             1                                          /**< A tag identifying the SoftDevice BLE configuration. */

#define APP_TIMER_PRESCALER              0                                          /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE          4                                          /**< Size of timer operation queues. */

#define MIN_CONN_INTERVAL                MSEC_TO_UNITS(100, UNIT_1_25_MS)           /**< Minimum acceptable connection interval (0.1 seconds). */
//#define MIN_CONN_INTERVAL                MSEC_TO_UNITS(1000, UNIT_1_25_MS)           /**< Minimum acceptable connection interval (0.1 seconds). */
#define MAX_CONN_INTERVAL                MSEC_TO_UNITS(200, UNIT_1_25_MS)           /**< Maximum acceptable connection interval (0.2 second). */
//#define MAX_CONN_INTERVAL                MSEC_TO_UNITS(2000, UNIT_1_25_MS)           /**< Maximum acceptable connection interval (0.2 second). */
#define SLAVE_LATENCY                    0                                          /**< Slave latency. */
#define CONN_SUP_TIMEOUT                 MSEC_TO_UNITS(4000, UNIT_10_MS)            /**< Connection supervisory timeout (4 seconds). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(5000) 						/**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY    APP_TIMER_TICKS(30000)						/**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT     3                                          /**< Number of attempts before giving up the connection parameter negotiation. */

#define SEC_PARAM_BOND                   1                                          /**< Perform bonding. */
#define SEC_PARAM_MITM                   0                                          /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC                   0                                          /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS               0                                          /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES        BLE_GAP_IO_CAPS_NONE                       /**< No I/O capabilities. */
#define SEC_PARAM_OOB                    0                                          /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE           7                                          /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE           16                                         /**< Maximum encryption key size. */

#define DEAD_BEEF                        0xDEADBEEF                                 /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

//static dm_application_instance_t         m_app_handle;                              /**< Application identifier allocated by device manager */

static uint16_t                          m_conn_handle = BLE_CONN_HANDLE_INVALID;   /**< Handle of the current connection. */

NRF_BLE_GATT_DEF(m_gatt);                                                       /**< GATT module instance. */
BLE_ADVERTISING_DEF(m_advertising);                                             /**< Advertising module instance. */

/* YOUR_JOB: Declare all services structure your application is using
static ble_xx_service_t                     m_xxs;
static ble_yy_service_t                     m_yys;
*/

#ifdef CUSTOM_SERVICES
static ble_cus_t                     m_cus;
BLE_CUS_DEF(m_cus);
#endif

#ifdef POWER_SERVICES
static ble_power_t                   m_power;
BLE_POWER_DEF(m_power);
#endif

#ifdef SPLIT_VERSION
static ble_readEpever_t              m_readEpever;
BLE_READEPEVER_DEF(m_readEpever);

static ble_scenario_t              m_scenario;
BLE_SCENARIO_DEF(m_scenario);

static ble_compute_t              m_compute;
BLE_COMPUTE_DEF(m_compute);
#endif

#ifdef SCENARIO_C5
static ble_scenario_C5_t              m_scenario_C5;
BLE_SCENARIO_C5_DEF(m_scenario_C5);
#endif

#ifdef RESEAUX
static ble_reseaux_t              m_reseaux;
BLE_RESEAUX_DEF(m_reseaux);
#endif

#ifdef BLE_GENERIC
static ble_generic_t              m_generic;
BLE_GENERIC_DEF(m_generic);
#endif

#ifdef MAIN_TIMER
APP_TIMER_DEF(m_notif_timer);

APP_TIMER_DEF(m_analog_timer);
#endif

#ifdef PWM_DRIVER
// Définition de l'utilisation des PWM
static nrf_drv_pwm_t m_pwm0 = NRF_DRV_PWM_INSTANCE(0);

static uint16_t const m_demo1_top  = 0xFF;

#define USED_PWM(idx) (1UL << idx)

static nrf_pwm_values_individual_t m_demo1_seq_values;

unsigned char colorChannel_0 = 0xFF;
unsigned char colorChannel_1 = 0xFF;
unsigned char colorChannel_2 = 0xFF;
unsigned char colorChannel_3 = 0xFF;
/*
static nrf_pwm_sequence_t const m_demo1_seq =
{
    .values.p_individual = &m_demo1_seq_values,
    .length              = NRF_PWM_VALUES_LENGTH(m_demo1_seq_values),
    .repeats             = 0,
    .end_delay           = 0
};
*/

bool fForcePWMColor = false;
#endif

#ifdef UART_BOARD
#define UART_TX_BUF_SIZE 256 /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE 1   /**< UART RX buffer size. */

nrf_drv_uart_t m_uart_main = NRF_DRV_UART_INSTANCE(0);
#endif

#ifdef ADC
#define SAMPLES_IN_BUFFER   5
#define SAADC_SAMPLE_RATE   1000     /**< SAADC sample rate in ms. */

static nrf_saadc_value_t       bufferADC[2][SAMPLES_IN_BUFFER];

#define CHANNEL_I_PV    0
#define CHANNEL_U_PV    1
#define CHANNEL_U_BATT  2
#define CHANNEL_I_BATT  3
#define CHANNEL_V_NTC   4

static nrf_ppi_channel_t       m_ppi_channel;
static const nrf_drv_timer_t   m_timer = NRF_DRV_TIMER_INSTANCE(3);

unsigned int u16PV_Voltage = 0;
#define VALUE_PV_VOLTAGE_JOUR	360		// 10V environ
#define VALUE_PV_VOLTAGE_NUIT	170		// 5V environ

#define COUNTER_JOUR_DEBOUNCE	2
#define COUNTER_NUIT_DEBOUNCE	2

#define VALUE_SHOW_COLOR	10		// 1à secondes pour montrer la nouvelle couleur sélectionnée
#endif

#ifdef BOARD_PLOT
// Définition des sorties PWM
#define PWM_RED     13
#define PWM_GREEN   15
#define PWM_BLUE    17
#define PWM_WHITE   19

int32_t s32CurrentBattery = 0;
uint32_t u32CurrentPV = 0;

uint64_t u64MaximumCapacity = 0;
uint64_t u64CoulombCounter = 0;
unsigned char u08PourcentageCapacityBattery = 0;

bool bFlagUpdateColorIntoLed = false;
#else
// Définition des sorties PWM
#define PWM_RED     17
#define PWM_GREEN   18
#define PWM_BLUE    19
#define PWM_WHITE   20
#endif

#ifdef SPLIT_VERSION
#define PV_VOLTAGE_JOUR_VALUE	1200	// Represente la tension minimum du PV pour la détection du jour
#define PV_VOLTAGE_NUIT_VALUE	1000		// Represente la tension maximale du PV pour la détection de la nuit

// ATTENTION, diviser le temps par le temps du timer, ici, 3 secondes
#define MAXIMUM_COUNTER_TO_START_SCENARIO	10	// Temps pour démarrer un nouveau scénario après la nuit : 30 secondes


#define COUNTER_NUIT_TO_JOUR	200				// Compteur anti rebond pour le passage de nuit à jour : 10 minutes

#define COUNTER_JOUR_TO_NUIT	200				// Compteur anti rebond pour le passage de jour à nuit : 10 minutes

bool bFlagManageEpeverRS485 = false;
bool bUpdateValueEpever = false;

bool bWriteHourEpever = false;
#endif

#ifdef ADC
bool bFlagManagePlot = false;

bool bFlagShowColor = false;
unsigned char u08CounterShowColor = 0;
#endif

#ifdef PLOT_DEMO
bool bFlagManagePLOT_DEMO = false;
#endif

#ifdef RTC_PLOT
const nrf_drv_rtc_t rtc = NRF_DRV_RTC_INSTANCE(2); /**< Declaring an instance of nrf_drv_rtc for RTC2. */

//static nrf_ppi_channel_t       m_ppi_channel_rtc;

bool bFlagIncrementHour = false;
#endif

bool bClientBLEConnected = false;

bool bNoSleep = false;


// YOUR_JOB: Use UUIDs for service(s) used in your application.
static ble_uuid_t m_adv_uuids[] = {{BLE_UUID_DEVICE_INFORMATION_SERVICE, BLE_UUID_TYPE_BLE}}; /**< Universally unique service identifiers. */

static void advertising_start(bool erase_bonds);

static uint32_t findMacAdress();
                                   
/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num   Line number of the failing ASSERT call.
 * @param[in] file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**@brief Function for handling Peer Manager events.
 *
 * @param[in] p_evt  Peer Manager event.
 */
static void pm_evt_handler(pm_evt_t const * p_evt)
{
    ret_code_t err_code;

    switch (p_evt->evt_id)
    {
        case PM_EVT_BONDED_PEER_CONNECTED:
        {
//            NRF_LOG_INFO("Connected to a previously bonded device.");
        } break;

        case PM_EVT_CONN_SEC_SUCCEEDED:
        {
//            NRF_LOG_INFO("Connection secured: role: %d, conn_handle: 0x%x, procedure: %d.",
//                         ble_conn_state_role(p_evt->conn_handle),
//                         p_evt->conn_handle,
//                         p_evt->params.conn_sec_succeeded.procedure);
        } break;

        case PM_EVT_CONN_SEC_FAILED:
        {
            /* Often, when securing fails, it shouldn't be restarted, for security reasons.
             * Other times, it can be restarted directly.
             * Sometimes it can be restarted, but only after changing some Security Parameters.
             * Sometimes, it cannot be restarted until the link is disconnected and reconnected.
             * Sometimes it is impossible, to secure the link, or the peer device does not support it.
             * How to handle this error is highly application dependent. */
        } break;

        case PM_EVT_CONN_SEC_CONFIG_REQ:
        {
            // Reject pairing request from an already bonded peer.
            pm_conn_sec_config_t conn_sec_config = {.allow_repairing = false};
            pm_conn_sec_config_reply(p_evt->conn_handle, &conn_sec_config);
        } break;

        case PM_EVT_STORAGE_FULL:
        {
            // Run garbage collection on the flash.
            err_code = fds_gc();
            if (err_code == FDS_ERR_BUSY || err_code == FDS_ERR_NO_SPACE_IN_QUEUES)
            {
                // Retry.
            }
            else
            {
                APP_ERROR_CHECK(err_code);
            }
        } break;

        case PM_EVT_PEERS_DELETE_SUCCEEDED:
        {
            advertising_start(false);
        } break;

        case PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED:
        {
            // The local database has likely changed, send service changed indications.
            pm_local_database_has_changed();
        } break;

        case PM_EVT_PEER_DATA_UPDATE_FAILED:
        {
            // Assert.
            APP_ERROR_CHECK(p_evt->params.peer_data_update_failed.error);
        } break;

        case PM_EVT_PEER_DELETE_FAILED:
        {
            // Assert.
            APP_ERROR_CHECK(p_evt->params.peer_delete_failed.error);
        } break;

        case PM_EVT_PEERS_DELETE_FAILED:
        {
            // Assert.
            APP_ERROR_CHECK(p_evt->params.peers_delete_failed_evt.error);
        } break;

        case PM_EVT_ERROR_UNEXPECTED:
        {
            // Assert.
            APP_ERROR_CHECK(p_evt->params.error_unexpected.error);
        } break;

        case PM_EVT_CONN_SEC_START:
        case PM_EVT_PEER_DATA_UPDATE_SUCCEEDED:
        case PM_EVT_PEER_DELETE_SUCCEEDED:
        case PM_EVT_LOCAL_DB_CACHE_APPLIED:
        case PM_EVT_SERVICE_CHANGED_IND_SENT:
        case PM_EVT_SERVICE_CHANGED_IND_CONFIRMED:
        default:
            break;
    }
}

#ifdef MAIN_TIMER
static void timer_timeout_handler(void * p_context)
{
    //nrf_gpio_pin_toggle(LED_4);
#ifdef ADC
	bFlagManagePlot = true;
#endif
#ifdef SPLIT_VERSION
	bFlagManageEpeverRS485 = true;
#endif
#ifdef PLOT_DEMO
	bFlagManagePLOT_DEMO = true;
#endif
}

#ifdef BOARD_PLOT
static void timer_analog_timeout_handler(void * p_context)
{
	// On remet les mesures analogiques
	nrf_gpio_pin_set(11);
}
#endif


/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
static void timers_init(void)
{
    uint32_t err_code = 0;
    
    //err_code = nrf_drv_clock_init();
    //APP_ERROR_CHECK(err_code);

    // Initialize timer module.
    //APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, false);
    
    // Initialize timer module.
    err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    // Create timers.

    /* YOUR_JOB: Create any timers to be used by the application.
                 Below is an example of how to create a timer.
                 For every new timer needed, increase the value of the macro APP_TIMER_MAX_TIMERS by
                 one.
    */
    err_code = app_timer_create(&m_notif_timer, APP_TIMER_MODE_REPEATED, timer_timeout_handler);
    APP_ERROR_CHECK(err_code);

#ifdef BOARD_PLOT
    // Timer pour le controle de l'alimentatino de la partie analogique
	err_code = app_timer_create(&m_analog_timer, APP_TIMER_MODE_SINGLE_SHOT, timer_analog_timeout_handler);
	APP_ERROR_CHECK(err_code);
#endif

#ifdef PLOT_BOARD
    err_code = app_timer_start(m_notif_timer, APP_TIMER_TICKS(1000), NULL);
	APP_ERROR_CHECK(err_code);
#endif
#ifdef SPLIT_BOARD
    err_code = app_timer_start(m_notif_timer, APP_TIMER_TICKS(3000), NULL);
    APP_ERROR_CHECK(err_code);
#endif
}
#endif


/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;
    //uint8_t serial_char[4];
    uint32_t serial_number = 0;
    uint8_t tab_name[20];
    uint8_t u08NumberOfChar = 0;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    // Cette partie est supprimée pour aller lire l'adresse MAC du chip
    //memcpy(serial_char, (uint32_t *)0x10001080, 4);
    //serial_number = (((uint16_t)serial_char[1]) << 8) + ((uint16_t)serial_char[0]);
    // Ecriture du numéro de série : 0x1234
	// nrfjprog.exe -f nrf52 --memwr 0x10001080 --val 0xFFFF1234

    serial_number = findMacAdress();
    u08NumberOfChar = sprintf((char *)tab_name, "%s-%X%X%X", DEVICE_NAME, (unsigned char)(serial_number >> 16), (unsigned char)(serial_number >> 8), (unsigned char)(serial_number & 0xFF));


    /*err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));*/
    err_code = sd_ble_gap_device_name_set(&sec_mode,
										  (const uint8_t *)tab_name,
										  u08NumberOfChar);
    APP_ERROR_CHECK(err_code);

    /* YOUR_JOB: Use an appearance value matching the application's use case.
    err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_);
    APP_ERROR_CHECK(err_code); */

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the GATT module.
 */
static void gatt_init(void)
{
    ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, NULL);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the DIS (Device Information Characteristic) module.
 */
/*
static void dis_init(void)
{
	ret_code_t err_code;
	uint8_t serial_char[4];
	uint16_t serial_number = 0;
	uint8_t tab_name[20];
	uint8_t u08NumberOfChar = 0;

	memset(&dis_init, 0, sizeof(dis_init));

	ble_srv_ascii_to_utf8(&dis_init.manufact_name_str, MANUFACTURER_NAME);
	ble_srv_ascii_to_utf8(&dis_init.model_num_str,     MODEL_NUM);

	memcpy(serial_char, (uint32_t *)0x10001080, 4);
	serial_number = (((uint16_t)serial_char[1]) << 8) + ((uint16_t)serial_char[0]);
	u08NumberOfChar = sprintf((char *)tab_name, "%d", serial_number);

	ble_srv_ascii_to_utf8(&dis_init.serial_num_str,     tab_name);
	ble_srv_ascii_to_utf8(&dis_init.hw_rev_str,     HW_REV_NUM);
	ble_srv_ascii_to_utf8(&dis_init.fw_rev_str,         FW_REV_NUM);

	sys_id.manufacturer_id            = MANUFACTURER_ID;
	sys_id.organizationally_unique_id = ORG_UNIQUE_ID;

	dis_init.p_sys_id                 = &sys_id;

	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&dis_init.dis_attr_md.read_perm);
	BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&dis_init.dis_attr_md.write_perm);

	err_code = ble_dis_init(&dis_init);
	APP_ERROR_CHECK(err_code);
}
*/

/**@brief Function for handling the YYY Service events. 
 * YOUR_JOB implement a service handler function depending on the event the service you are using can generate
 *
 * @details This function will be called for all YY Service events which are passed to
 *          the application.
 *
 * @param[in]   p_yy_service   YY Service structure.
 * @param[in]   p_evt          Event received from the YY Service.
 *
 *
static void on_yys_evt(ble_yy_service_t     * p_yy_service, 
                       ble_yy_service_evt_t * p_evt)
{
    switch (p_evt->evt_type)
    {
        case BLE_YY_NAME_EVT_WRITE:
            APPL_LOG("[APPL]: charact written with value %s. \r\n", p_evt->params.char_xx.value.p_str);
            break;
        
        default:
            // No implementation needed.
            break;
    }
}*/

/**@brief Function for starting timers.
 */
static void application_timers_start(void)
{
#ifdef CUSTOM_SERVICES
//       ret_code_t err_code;
//       err_code = app_timer_start(m_notif_timer, APP_TIMER_TICKS(1000, APP_TIMER_PRESCALER), NULL);
//       APP_ERROR_CHECK(err_code);
//       NRF_LOG_INFO("Notification Timer Starting ... (Err_code: %d).\r\n",err_code); 
#endif
}

/**@brief Function for handling the Custom Service Service events.
 *
 * @details This function will be called for all Custom Service events which are passed to
 *          the application.
 *
 * @param[in]   p_cus_service  Custom Service structure.
 * @param[in]   p_evt          Event received from the Custom Service.
 *
 */
#ifdef CUSTOM_SERVICES
static void on_cus_evt(ble_cus_t     * p_cus_service,
                       ble_cus_evt_t * p_evt)
{
//	uint32_t u32ValuePWM = 0;
//    NRF_LOG_INFO("CUS event received. Event type = %d\r\n", p_evt->evt_type); 
    //ret_code_t err_code;
    switch(p_evt->evt_type)
    {
        case BLE_CUS_EVT_NOTIFICATION_ENABLED:
              //nrf_gpio_pin_clear(20);
              application_timers_start();
        case BLE_CUS_EVT_DISCONNECTED:
              //err_code = app_timer_stop(m_notif_timer);
              //NRF_LOG_INFO("Notification Timer Stopped.\r\n"); 
              break;

        case BLE_CUS_EVT_CONNECTED:
            //application_timers_start();
            break;

        case BLE_CUS_WRITE:
#ifdef PWM_DRIVER
        	//nrf_gpio_pin_toggle(LED_3);
            bFlagUpdateColorIntoLed = true;

            /*u32ValuePWM = ble_cus_readPWMValue();

			m_demo1_seq_values.channel_0 = 0xFF - ((u32ValuePWM >> 24) & 0xFF);
			m_demo1_seq_values.channel_1 = 0xFF - ((u32ValuePWM >> 16) & 0xFF);
			m_demo1_seq_values.channel_2 = 0xFF - ((u32ValuePWM >> 8) & 0xFF);
			m_demo1_seq_values.channel_3 = 0xFF - (u32ValuePWM & 0xFF);*/
#endif
            break;

        case BLE_CUS_WRITE_FONCTIONNEMENT_MODE:
        	break;

        case BLE_CUS_WRITE_SEQUENCE_NUMBER:
        	break;

        default:
              // No implementation needed.
              break;
    }
}
#endif

/**@brief Function for handling the Custom Power Service Service events.
 *
 * @details This function will be called for all Custom Power Service events which are passed to
 *          the application.
 *
 * @param[in]   p_power_service  Custom Power Service structure.
 * @param[in]   p_evt            Event received from the Custom Service.
 *
 */
#ifdef POWER_SERVICES
static void on_power_evt(ble_power_t     * p_power_service,
                         ble_power_evt_t * p_evt)
{
//    NRF_LOG_INFO("CUS event received. Event type = %d\r\n", p_evt->evt_type); 
    //ret_code_t err_code;
//    uint32_t u32Value = 0;

    switch(p_evt->evt_type)
    {
        case BLE_POWER_EVT_NOTIFICATION_ENABLED:
              //nrf_gpio_pin_clear(20);
              application_timers_start();
        case BLE_POWER_EVT_DISCONNECTED:
              //err_code = app_timer_stop(m_notif_timer);
              //NRF_LOG_INFO("Notification Timer Stopped.\r\n"); 
              break;

        case BLE_POWER_EVT_CONNECTED:
            //application_timers_start();
            break;
				
        default:
              // No implementation needed.
              break;
    }
}
#endif

#ifdef DFU_PLOT
/**@brief Function for handling dfu events from the Buttonless Secure DFU service
 *
 * @param[in]   event   Event from the Buttonless Secure DFU service.
 */
static void ble_dfu_evt_handler(ble_dfu_buttonless_evt_type_t event)
{
    switch (event)
    {
        case BLE_DFU_EVT_BOOTLOADER_ENTER_PREPARE:
//            NRF_LOG_INFO("Device is preparing to enter bootloader mode.");
#ifdef UART_BOARD_SEND
        	printf("Device is preparing to enter bootloader mode.");
#endif
            // YOUR_JOB: Disconnect all bonded devices that currently are connected.
            //           This is required to receive a service changed indication
            //           on bootup after a successful (or aborted) Device Firmware Update.
            break;

        case BLE_DFU_EVT_BOOTLOADER_ENTER:
            // YOUR_JOB: Write app-specific unwritten data to FLASH, control finalization of this
            //           by delaying reset by reporting false in app_shutdown_handler
//            NRF_LOG_INFO("Device will enter bootloader mode.");
#ifdef UART_BOARD_SEND
        	printf("Device will enter bootloader mode.");
#endif
            break;

        case BLE_DFU_EVT_BOOTLOADER_ENTER_FAILED:
//            NRF_LOG_ERROR("Request to enter bootloader mode failed asynchroneously.");
            // YOUR_JOB: Take corrective measures to resolve the issue
            //           like calling APP_ERROR_CHECK to reset the device.
#ifdef UART_BOARD_SEND
        	printf("Request to enter bootloader mode failed asynchroneously.");
#endif
            break;

        case BLE_DFU_EVT_RESPONSE_SEND_ERROR:
//            NRF_LOG_ERROR("Request to send a response to client failed.");
            // YOUR_JOB: Take corrective measures to resolve the issue
            //           like calling APP_ERROR_CHECK to reset the device.
#ifdef UART_BOARD_SEND
        	printf("Request to send a response to client failed.");
#endif
            APP_ERROR_CHECK(false);
            break;

        default:
//            NRF_LOG_ERROR("Unknown event from ble_dfu_buttonless.");
#ifdef UART_BOARD_SEND
        	printf("Unknown event from ble_dfu_buttonless.");
#endif
            break;
    }
}
#endif

/**@brief Function for handling the Read Epever Service Service events.
 *
 * @details This function will be called for all Custom Read Epever Service events which are passed to
 *          the application.
 *
 * @param[in]   p_readEpever_service  Custom Power Service structure.
 * @param[in]   p_evt            Event received from the Custom Service.
 *
 */
#if (defined SPLIT_VERSION) || (defined PLOT_DEMO)
static void on_readEpever_evt(ble_readEpever_t     * p_readEpever_service,
                         	 ble_readEpever_evt_t * p_evt)
{
//    NRF_LOG_INFO("CUS event received. Event type = %d\r\n", p_evt->evt_type);
    //ret_code_t err_code;
//    uint32_t u32Value = 0;

    switch(p_evt->evt_type)
    {
        case BLE_READEPEVER_EVT_NOTIFICATION_ENABLED:
              //nrf_gpio_pin_clear(20);
              application_timers_start();
        case BLE_READEPEVER_EVT_DISCONNECTED:
              //err_code = app_timer_stop(m_notif_timer);
              //NRF_LOG_INFO("Notification Timer Stopped.\r\n");
              break;
        case BLE_READEPEVER_EVT_CONNECTED:
            //application_timers_start();
            break;
        case BLE_READEPEVER_WRITE:
        	break;
        case BLE_READEPEVER_WRITE_CLOCK:
        	bWriteHourEpever = true;
        	break;
        default:
              // No implementation needed.
              break;
    }
}
#endif
#ifdef SPLIT_VERSION
static void on_scenario_evt(ble_scenario_t     * p_scenario_service,
                         	 ble_scenario_evt_t * p_evt)
{
//    NRF_LOG_INFO("CUS event received. Event type = %d\r\n", p_evt->evt_type);
    //ret_code_t err_code;
//    uint32_t u32Value = 0;
	uint8_t u08Value = 0;

    switch(p_evt->evt_type)
    {
        case BLE_SCENARIO_EVT_NOTIFICATION_ENABLED:
              //nrf_gpio_pin_clear(20);
              application_timers_start();
        case BLE_SCENARIO_EVT_DISCONNECTED:
              //err_code = app_timer_stop(m_notif_timer);
              //NRF_LOG_INFO("Notification Timer Stopped.\r\n");
              break;
        case BLE_SCENARIO_EVT_CONNECTED:
            //application_timers_start();
            break;
        case BLE_SCENARIO_WRITE:
        	u08Value = ble_scenario_readScenarioNumber();
			setScenarioNumberEpever(u08Value);
			setScenarioNumber(u08Value);
			// TODO : pour ne pas envoyer tous de suite un ordre vers l'EPEVER : bSendNewScenario = true;
			break;
        default:
              // No implementation needed.
              break;
    }
}

static void on_compute_evt(ble_compute_t     * p_compute_service,
                         	 ble_compute_evt_t * p_evt)
{
//    NRF_LOG_INFO("CUS event received. Event type = %d\r\n", p_evt->evt_type);
    //ret_code_t err_code;
//    uint32_t u32Value = 0;

    switch(p_evt->evt_type)
    {
        case BLE_COMPUTE_EVT_NOTIFICATION_ENABLED:
              //nrf_gpio_pin_clear(20);
              application_timers_start();
        case BLE_COMPUTE_EVT_DISCONNECTED:
              //err_code = app_timer_stop(m_notif_timer);
              //NRF_LOG_INFO("Notification Timer Stopped.\r\n");
              break;
        case BLE_COMPUTE_EVT_CONNECTED:
            //application_timers_start();
            break;
        case BLE_COMPUTE_WRITE:
			//bSendNewScenario = true;
			break;
        default:
              // No implementation needed.
              break;
    }
}
#endif

#ifdef SCENARIO_C5
static void on_scenario_C5_evt(ble_scenario_C5_t     * p_scenario_service,
                         	   ble_scenario_C5_evt_t * p_evt)
{
//    NRF_LOG_INFO("CUS event received. Event type = %d\r\n", p_evt->evt_type);
    //ret_code_t err_code;
//    uint32_t u32Value = 0;
//	uint8_t u08Value = 0;

    switch(p_evt->evt_type)
    {
        case BLE_SCENARIO_C5_EVT_NOTIFICATION_ENABLED:
              //nrf_gpio_pin_clear(20);
              application_timers_start();
        case BLE_SCENARIO_C5_EVT_DISCONNECTED:
              //err_code = app_timer_stop(m_notif_timer);
              //NRF_LOG_INFO("Notification Timer Stopped.\r\n");
              break;
        case BLE_SCENARIO_C5_EVT_CONNECTED:
            //application_timers_start();
            break;
        case BLE_SCENARIO_C5_WRITE_UNIVERS_NUMBER:
        	//   u08Value = ble_scenario_readScenarioNumber();
        	//   setScenarioNumberEpever(u08Value);
        	//   setScenarioNumber(u08Value);
			break;

        case BLE_SCENARIO_C5_WRITE_WRITE_SEQUENCE:
        	break;

        case BLE_SCENARIO_C5_WRITE_DEMO_MODE:
        	break;
        default:
              // No implementation needed.
              break;
    }
}
#endif

#ifdef RESEAUX
static void on_reseaux_evt(ble_reseaux_t     * p_reseaux_service,
                         	   ble_reseaux_evt_t * p_evt)
{
//    NRF_LOG_INFO("CUS event received. Event type = %d\r\n", p_evt->evt_type);
    //ret_code_t err_code;
//    uint32_t u32Value = 0;
//	uint8_t u08Value = 0;

    switch(p_evt->evt_type)
    {
        case BLE_RESEAUX_EVT_NOTIFICATION_ENABLED:
              //nrf_gpio_pin_clear(20);
              application_timers_start();
        case BLE_RESEAUX_EVT_DISCONNECTED:
              //err_code = app_timer_stop(m_notif_timer);
              //NRF_LOG_INFO("Notification Timer Stopped.\r\n");
              break;
        case BLE_RESEAUX_EVT_CONNECTED:
            //application_timers_start();
            break;

        case BLE_RESEAUX_WRITE_NAME:
        	break;

        case BLE_RESEAUX_WRITE_SERIAL_UNIVERS_NUMBER:
        	break;

        case BLE_RESEAUX_WRITE_PRODUCT_NUMBER:
			break;

        case BLE_RESEAUX_WRITE_LINE_NUMBER:
			break;

        case BLE_RESEAUX_WRITE_CONFIG_TABLE:
			break;

        default:
              // No implementation needed.
              break;
    }
}
#endif

#ifdef BLE_GENERIC
static void on_generic_evt(ble_generic_t     * p_generic_service,
                         	   ble_generic_evt_t * p_evt)
{
//    NRF_LOG_INFO("CUS event received. Event type = %d\r\n", p_evt->evt_type);
    //ret_code_t err_code;
//    uint32_t u32Value = 0;
//	uint8_t u08Value = 0;

    switch(p_evt->evt_type)
    {
        case BLE_GENERIC_EVT_NOTIFICATION_ENABLED:
              //nrf_gpio_pin_clear(20);
              application_timers_start();
        case BLE_GENERIC_EVT_DISCONNECTED:
              //err_code = app_timer_stop(m_notif_timer);
              //NRF_LOG_INFO("Notification Timer Stopped.\r\n");
              break;
        case BLE_GENERIC_EVT_CONNECTED:
            //application_timers_start();
            break;

        case BLE_GENERIC_WRITE_REQUEST:
        	break;

        case BLE_GENERIC_WRITE_DATE:
        	break;

        case BLE_GENERIC_WRITE_HOUR:
        	break;

        //case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
        //	nrf_gpio_pin_set(13);
        //	break;

        default:
              // No implementation needed.
              break;
    }
}
#endif

/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
#if (defined CUSTOM_SERVICES) || (defined POWER_SERVICES) || (defined DFU_PLOT) || (defined SPLIT_VERSION)
    ret_code_t                         err_code;
#endif

#if (defined SPLIT_VERSION) || (defined PLOT_DEMO)
    // Service de lecture des paramètres SPLIT
    ble_readEpever_init_t                   readEpever_init;

	// Initialize CUS POWER Service.
	memset(&readEpever_init, 0, sizeof(readEpever_init));

	// Set the cus event handler
	readEpever_init.evt_handler                = on_readEpever_evt;

	// Here the Sec level for the Custom Service can be changed/increased.
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&readEpever_init.custom_value_char_attr_md.cccd_write_perm);
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&readEpever_init.custom_value_char_attr_md.read_perm);
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&readEpever_init.custom_value_char_attr_md.write_perm);	// TODO : à supprimer

	readEpever_init.initial_custom_value = 0x0;	// Valeur initiale

	err_code = ble_readEpever_init(&m_readEpever, &readEpever_init);
	APP_ERROR_CHECK(err_code);
#endif
#ifdef SPLIT_VERSION
	// Service des scénarios SPLIT
	ble_scenario_init_t                   scenario_init;

	// Initialize Scenario Service.
	memset(&scenario_init, 0, sizeof(scenario_init));

	// Set the cus event handler
	scenario_init.evt_handler                = on_scenario_evt;

	// Here the Sec level for the Custom Service can be changed/increased.
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&scenario_init.custom_value_char_attr_md.cccd_write_perm);
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&scenario_init.custom_value_char_attr_md.read_perm);
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&scenario_init.custom_value_char_attr_md.write_perm);

	scenario_init.initial_custom_value = 0x0;	// Valeur initiale

	err_code = ble_scenario_init(&m_scenario, &scenario_init);
	APP_ERROR_CHECK(err_code);

	// Service des compute du SPLIT : PDU, temps d'allmage/Extinction
	ble_compute_init_t                   compute_init;

	// Initialize compute Service.
	memset(&compute_init, 0, sizeof(compute_init));

	// Set the cus event handler
	compute_init.evt_handler                = on_compute_evt;

	// Here the Sec level for the Custom Service can be changed/increased.
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&compute_init.custom_value_char_attr_md.cccd_write_perm);
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&compute_init.custom_value_char_attr_md.read_perm);
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&compute_init.custom_value_char_attr_md.write_perm);

	compute_init.initial_custom_value = 0x0;	// Valeur initiale

	err_code = ble_compute_init(&m_compute, &compute_init);
	APP_ERROR_CHECK(err_code);
#endif
#ifdef CUSTOM_SERVICES
    ble_cus_init_t                     cus_init;

     // Initialize CUS Service.
    memset(&cus_init, 0, sizeof(cus_init));

    // Set the cus event handler
    cus_init.evt_handler                = on_cus_evt;

    // Here the Sec level for the Custom Service can be changed/increased.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cus_init.custom_value_char_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cus_init.custom_value_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cus_init.custom_value_char_attr_md.write_perm);
		
    cus_init.initial_custom_value = 0x0;	// Valeur initiale

    err_code = ble_cus_init(&m_cus, &cus_init);
    APP_ERROR_CHECK(err_code);
#endif

#ifdef POWER_SERVICES
    ble_power_init_t                   power_init;

     // Initialize CUS POWER Service.
    memset(&power_init, 0, sizeof(power_init));

    // Set the cus event handler
    power_init.evt_handler                = on_power_evt;

    // Here the Sec level for the Custom Service can be changed/increased.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&power_init.custom_value_char_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&power_init.custom_value_char_attr_md.read_perm);
    //BLE_GAP_CONN_SEC_MODE_SET_OPEN(&power_init.custom_value_char_attr_md.write_perm);
		
    power_init.initial_custom_value = 0x0;	// Valeur initiale

    err_code = ble_power_init(&m_power, &power_init);
    APP_ERROR_CHECK(err_code);
#endif

#ifdef SCENARIO_C5
    // Service des scénarios SPLIT
	ble_scenario_C5_init_t                   scenario_C5_init;

	// Initialize Scenario Service.
	memset(&scenario_C5_init, 0, sizeof(scenario_C5_init));

	// Set the cus event handler
	scenario_C5_init.evt_handler                = on_scenario_C5_evt;

	// Here the Sec level for the Custom Service can be changed/increased.
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&scenario_C5_init.custom_value_char_attr_md.cccd_write_perm);
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&scenario_C5_init.custom_value_char_attr_md.read_perm);
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&scenario_C5_init.custom_value_char_attr_md.write_perm);

	scenario_C5_init.initial_custom_value = 0x0;	// Valeur initiale

	err_code = ble_scenario_C5_init(&m_scenario_C5, &scenario_C5_init);
	APP_ERROR_CHECK(err_code);
#endif

#ifdef RESEAUX
	// Service des scénarios SPLIT
	ble_reseaux_init_t                   reseaux_init;

	// Initialize Scenario Service.
	memset(&reseaux_init, 0, sizeof(reseaux_init));

	// Set the cus event handler
	reseaux_init.evt_handler                = on_reseaux_evt;

	// Here the Sec level for the Custom Service can be changed/increased.
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&reseaux_init.custom_value_char_attr_md.cccd_write_perm);
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&reseaux_init.custom_value_char_attr_md.read_perm);
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&reseaux_init.custom_value_char_attr_md.write_perm);

	reseaux_init.initial_custom_value = 0x0;	// Valeur initiale

	err_code = ble_reseaux_init(&m_reseaux, &reseaux_init);
	APP_ERROR_CHECK(err_code);
#endif

#ifdef BLE_GENERIC
	// Service des scénarios SPLIT
	ble_generic_init_t                   generic_init;

	// Initialize Scenario Service.
	memset(&generic_init, 0, sizeof(generic_init));

	// Set the cus event handler
	generic_init.evt_handler                = on_generic_evt;

	// Here the Sec level for the Custom Service can be changed/increased.
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&generic_init.custom_value_char_attr_md.cccd_write_perm);
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&generic_init.custom_value_char_attr_md.read_perm);
	BLE_GAP_CONN_SEC_MODE_SET_OPEN(&generic_init.custom_value_char_attr_md.write_perm);

	generic_init.initial_custom_value = 0x0;	// Valeur initiale

	err_code = ble_generic_init(&m_generic, &generic_init);
	APP_ERROR_CHECK(err_code);
#endif

#ifdef DFU_PLOT
    ble_dfu_buttonless_init_t dfus_init =
	{
		.evt_handler = ble_dfu_evt_handler
	};

	// Initialize the async SVCI interface to bootloader.
	err_code = ble_dfu_buttonless_async_svci_init();
	APP_ERROR_CHECK(err_code);

	err_code = ble_dfu_buttonless_init(&dfus_init);
	APP_ERROR_CHECK(err_code);
#endif
}


/**@brief Function for handling the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module which
 *          are passed to the application.
 *          @note All this function does is to disconnect. This could have been done by simply
 *                setting the disconnect_on_fail config parameter, but instead we use the event
 *                handler mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
static void sleep_mode_enter(void)
{
    uint32_t err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);

/*    // Prepare wakeup buttons.
    err_code = bsp_btn_ble_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
*/
}


/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    uint32_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            APP_ERROR_CHECK(err_code);
            break;
        case BLE_ADV_EVT_IDLE:
            sleep_mode_enter();
            break;
        default:
            break;
    }
}


/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
/* AVANT
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t err_code;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            break;

        default:
            // No implementation needed.
            break;
    }
}
*/
/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    ret_code_t err_code = NRF_SUCCESS;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_DISCONNECTED:
//            NRF_LOG_INFO("Disconnected.");
            err_code = bsp_indication_set(BSP_INDICATE_IDLE);
            APP_ERROR_CHECK(err_code);
            bClientBLEConnected = false;
            break;

        case BLE_GAP_EVT_CONNECTED:
//            NRF_LOG_INFO("Connected.");
            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            bClientBLEConnected = true;
            break;

#if defined(S132)
        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
//            NRF_LOG_DEBUG("PHY update request.");
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;
#endif

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
//            NRF_LOG_DEBUG("GATT Client Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
//            NRF_LOG_DEBUG("GATT Server Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_EVT_USER_MEM_REQUEST:
            err_code = sd_ble_user_mem_reply(p_ble_evt->evt.gattc_evt.conn_handle, NULL);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
        {
            ble_gatts_evt_rw_authorize_request_t  req;
            ble_gatts_rw_authorize_reply_params_t auth_reply;

            req = p_ble_evt->evt.gatts_evt.params.authorize_request;

            if (req.type != BLE_GATTS_AUTHORIZE_TYPE_INVALID)
            {
                if ((req.request.write.op == BLE_GATTS_OP_PREP_WRITE_REQ)     ||
                    (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW) ||
                    (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_CANCEL))
                {
                    if (req.type == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
                    {
                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
                    }
                    else
                    {
                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
                    }
                    auth_reply.params.write.gatt_status = APP_FEATURE_NOT_SUPPORTED;
                    err_code = sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gatts_evt.conn_handle,
                                                               &auth_reply);
                    APP_ERROR_CHECK(err_code);
                }
            }
        } break; // BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST

        default:
            // No implementation needed.
            break;
    }
}



/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    uint32_t err_code;

    // Initialize the SoftDevice handler module.
    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}

/**@brief Function for the Peer Manager initialization.
 */
static void peer_manager_init(void)
{
    ble_gap_sec_params_t sec_param;
    ret_code_t           err_code;

    err_code = pm_init();
    APP_ERROR_CHECK(err_code);

    memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

    // Security parameters to be used for all security procedures.
    sec_param.bond           = SEC_PARAM_BOND;
    sec_param.mitm           = SEC_PARAM_MITM;
    sec_param.lesc           = SEC_PARAM_LESC;
    sec_param.keypress       = SEC_PARAM_KEYPRESS;
    sec_param.io_caps        = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob            = SEC_PARAM_OOB;
    sec_param.min_key_size   = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size   = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc  = 1;
    sec_param.kdist_own.id   = 1;
    sec_param.kdist_peer.enc = 1;
    sec_param.kdist_peer.id  = 1;

    err_code = pm_sec_params_set(&sec_param);
    APP_ERROR_CHECK(err_code);

    err_code = pm_register(pm_evt_handler);
    APP_ERROR_CHECK(err_code);
}

/**@brief Clear bond information from persistent storage.
 */
static void delete_bonds(void)
{
    ret_code_t err_code;

    //NRF_LOG_INFO("Erase bonds!");

    err_code = pm_peers_delete();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling events from the BSP module.
 *
 * @param[in]   event   Event generated by button press.
 */
void bsp_event_handler(bsp_event_t event)
{
    uint32_t err_code;
    switch (event)
    {
        case BSP_EVENT_SLEEP:
            sleep_mode_enter();
            break;

        case BSP_EVENT_DISCONNECT:
            err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;

        case BSP_EVENT_WHITELIST_OFF:
            if (m_conn_handle == BLE_CONN_HANDLE_INVALID)
            {
                err_code = ble_advertising_restart_without_whitelist(&m_advertising);
                if (err_code != NRF_ERROR_INVALID_STATE)
                {
                    APP_ERROR_CHECK(err_code);
                }
            }
            break;

        default:
            break;
    }
}

/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void)
{
    ret_code_t             err_code;
    ble_advertising_init_t init;

//    ble_advdata_t advdata;
//	ble_advdata_t srdata;


    memset(&init, 0, sizeof(init));

    init.advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    init.advdata.include_appearance      = true;
    init.advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    init.advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.advdata.uuids_complete.p_uuids  = m_adv_uuids;

    init.config.ble_adv_fast_enabled  = true;
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout  = APP_ADV_TIMEOUT_IN_SECONDS;

    init.evt_handler = on_adv_evt;

    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);

// TODO : DAMIEN : essai de mettre un service dans le BONd durant le scan de la tablette
/*	ble_uuid_t adv_uuids[] = {{GENERIC_SERVICE_UUID_BASE, m_generic.uuid_type}};

	// Build and set advertising data
	memset(&advdata, 0, sizeof(advdata));

	advdata.name_type          = BLE_ADVDATA_FULL_NAME;
	advdata.include_appearance = true;
	advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;


	memset(&srdata, 0, sizeof(srdata));
	srdata.uuids_complete.uuid_cnt = sizeof(adv_uuids) / sizeof(adv_uuids[0]);
	srdata.uuids_complete.p_uuids  = adv_uuids;

	err_code = ble_advdata_set(&advdata, &srdata);
	APP_ERROR_CHECK(err_code);
*/
}


/**@brief Function for the Device Manager initialization.
 *
 * @param[in] erase_bonds  Indicates whether bonding information should be cleared from
 *                         persistent storage during initialization of the Device Manager.
 */
/*static void device_manager_init(bool erase_bonds)
{
*/
/*    uint32_t               err_code;
    dm_init_param_t        init_param = {.clear_persistent_data = erase_bonds};
    dm_application_param_t register_param;

    // Initialize persistent storage module.
    err_code = pstorage_init();
    APP_ERROR_CHECK(err_code);

    err_code = dm_init(&init_param);
    APP_ERROR_CHECK(err_code);

    memset(&register_param.sec_param, 0, sizeof(ble_gap_sec_params_t));

    register_param.sec_param.bond         = SEC_PARAM_BOND;
    register_param.sec_param.mitm         = SEC_PARAM_MITM;
    register_param.sec_param.io_caps      = SEC_PARAM_IO_CAPABILITIES;
    register_param.sec_param.oob          = SEC_PARAM_OOB;
    register_param.sec_param.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    register_param.sec_param.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
    register_param.evt_handler            = device_manager_evt_handler;
    register_param.service_type           = DM_PROTOCOL_CNTXT_GATT_SRVR_ID;

    err_code = dm_register(&m_app_handle, &register_param);
    APP_ERROR_CHECK(err_code);
*/
/*    ble_gap_sec_params_t sec_param;
    ret_code_t           err_code;

    err_code = pm_init();
    APP_ERROR_CHECK(err_code);

    memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

    // Security parameters to be used for all security procedures.
    sec_param.bond           = SEC_PARAM_BOND;
    sec_param.mitm           = SEC_PARAM_MITM;
    sec_param.lesc           = SEC_PARAM_LESC;
    sec_param.keypress       = SEC_PARAM_KEYPRESS;
    sec_param.io_caps        = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob            = SEC_PARAM_OOB;
    sec_param.min_key_size   = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size   = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc  = 1;
    sec_param.kdist_own.id   = 1;
    sec_param.kdist_peer.enc = 1;
    sec_param.kdist_peer.id  = 1;

    err_code = pm_sec_params_set(&sec_param);
    APP_ERROR_CHECK(err_code);

    err_code = pm_register(pm_evt_handler);
    APP_ERROR_CHECK(err_code);
}
*/


/**@brief Function for initializing buttons and leds.
 *
 * @param[out] p_erase_bonds  Will be true if the clear bonding button was pressed to wake the application up.
 */
static void buttons_leds_init(bool * p_erase_bonds)
{
    bsp_event_t startup_event;

    uint32_t err_code = bsp_init(BSP_INIT_LED | BSP_INIT_BUTTONS, bsp_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ble_init(NULL, &startup_event);
    APP_ERROR_CHECK(err_code);

    *p_erase_bonds = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);
}

#ifdef PWM_DRIVER
/**@brief Function for initializing PWM output
 *
 */
static void pwm_init(void)
{
    uint32_t err_code;
    nrf_drv_pwm_config_t const config0 =
    {
        .output_pins =
        {
            PWM_RED,    // channel 0
            PWM_GREEN,  // channel 1
            PWM_BLUE,   // channel 2
            PWM_WHITE   // channel 3
        },
        .base_clock = NRF_PWM_CLK_250kHz,
        .count_mode = NRF_PWM_MODE_UP,
        .top_value  = m_demo1_top,
        .load_mode  = NRF_PWM_LOAD_INDIVIDUAL,
        .step_mode  = NRF_PWM_STEP_AUTO
    };
    err_code = nrf_drv_pwm_init(&m_pwm0, &config0, NULL);
    APP_ERROR_CHECK(err_code);
    
    m_demo1_seq_values.channel_0 = 0xFF;
    m_demo1_seq_values.channel_1 = 0xFF;
    m_demo1_seq_values.channel_2 = 0xFF;
    m_demo1_seq_values.channel_3 = 0xFF;
    
    static nrf_pwm_sequence_t const m_demo1_seq =
    {
        .values.p_individual = &m_demo1_seq_values,
        .length              = NRF_PWM_VALUES_LENGTH(m_demo1_seq_values),
        .repeats             = 0,
        .end_delay           = 0
    };
    
    err_code = nrf_drv_pwm_simple_playback(&m_pwm0, &m_demo1_seq, 1, NRF_DRV_PWM_FLAG_LOOP);
    APP_ERROR_CHECK(err_code);
}
#endif

#ifdef UART_BOARD
static void uart_event_handler(app_uart_evt_t *p_app_uart_event)
{
#ifdef SPLIT_VERSION
    uint8_t u08CharReceive = 0;

    //app_uart_get(&u08CharReceive);
    //app_uart_put(u08CharReceive);
    if (p_app_uart_event->evt_type== APP_UART_TX_EMPTY)
    {	// La transmission est terminée
    	activateReceive_rs485();
    }
	if ((p_app_uart_event->evt_type == APP_UART_DATA_READY) || (p_app_uart_event->evt_type == APP_UART_DATA))
	{
		app_uart_get(&u08CharReceive);
		receiveData(u08CharReceive);
	}
#endif
}

/**@brief Function for initializing the UART output
 *
 */
static void init_uart(void)
{
    uint32_t err_code;

    const app_uart_comm_params_t comm_params =
    {
        RX_PIN_NUMBER,
        TX_PIN_NUMBER,
        RTS_PIN_NUMBER,
        CTS_PIN_NUMBER,
        APP_UART_FLOW_CONTROL_DISABLED,
        false,
        UART_BAUDRATE_BAUDRATE_Baud115200
    };

    APP_UART_FIFO_INIT(&comm_params,
                       UART_RX_BUF_SIZE,
                       UART_TX_BUF_SIZE,
                       uart_event_handler,
                       APP_IRQ_PRIORITY_LOW,
                       err_code);
    APP_ERROR_CHECK(err_code);

    // Si on veux utiliser le cable RS485 MAIS cela consomme beaucoup !!!
    // On le désactive
    // Signal DE à 1
//	nrf_gpio_cfg_output(24);
//	nrf_gpio_pin_clear(24);

	// Signal /RE à 1
//	nrf_gpio_cfg_output(23);
//	nrf_gpio_pin_set(23);
}
#endif


#ifdef ADC
void timer_handler(nrf_timer_event_t event_type, void* p_context)
{
	//printf("ADC ---- \r\n");
	bNoSleep = true;
	nrf_gpio_pin_set(30);
}

void saadc_callback(nrf_drv_saadc_evt_t const * p_event)
{
    if (p_event->type == NRF_DRV_SAADC_EVT_DONE)
    {
        ret_code_t err_code;
        uint16_t u16TempValue = 0;
        uint32_t u32TempValue = 0;
     
        err_code = nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, SAMPLES_IN_BUFFER);
        APP_ERROR_CHECK(err_code);

#ifdef UART_BOARD_SEND
//       	printf("%d : %d, %d, %d, %d, %d\r\n", p_event->data.done.size, p_event->data.done.p_buffer[0], p_event->data.done.p_buffer[1],
//                                              p_event->data.done.p_buffer[2], p_event->data.done.p_buffer[3], p_event->data.done.p_buffer[4]);
#endif

#ifdef POWER_SERVICES
        if (bClientBLEConnected == true)
        {	// Seulement si 1 périphérique est connecté
			// Send the new value of ADC to the BLE services (to update the value)
			// Calcul du courant PV en mA : formule Y = 0.3913 * X - 8.8261
        	// On multiplie tous par 1000, ce qui donnera un courant en µA
			if (p_event->data.done.p_buffer[0] >= 23)
			{
				u32CurrentPV =  ((uint32_t)p_event->data.done.p_buffer[0] * 391);
				u32CurrentPV += (((uint32_t)p_event->data.done.p_buffer[0] / 10) * 3);
				u32CurrentPV -= 8826;
			}
			else
			{
				u32CurrentPV = 0;
			}
			ble_power_pv_current_value_update(&m_power, u32CurrentPV);
#ifdef UART_BOARD_SEND
//			printf("PV current = %ld\r\n", u32CurrentPV);
#endif
        }

        // Mémorisation de la variable, permettant de connaitre l'état du JOUR/NUIT
        u16PV_Voltage = p_event->data.done.p_buffer[1];

        if (bClientBLEConnected == true)
        {	// Seulement si 1 périphérique est connecté
			// Calcul de la tension PV
			// Formule : tensionADC = Vpv * 6.81 / 53.81 = Vpv * 0.126
			// ATTENTION : cette formule ne fonctionen pas !!!
			// Il faut appliquer :
			// Vpv = (ValeurADC * 0.0281) - 0.1397  (en Volt)
			// Vpv = (ValeurADC * 28) - 140  (en mV)
			// On ajoute (ValeurADC / 10) en plus pour être plsu précis !!!
			if (p_event->data.done.p_buffer[1] >= 5)
			{
				u16TempValue = ((p_event->data.done.p_buffer[1] * 28)) - 140 + (p_event->data.done.p_buffer[1] / 10);
			}
			else
			{
				u16TempValue = 0;
			}

        	ble_power_pv_voltage_value_update(&m_power, u16TempValue);	//p_event->data.done.p_buffer[1]);
#ifdef UART_BOARD_SEND
//       	printf("Vpv = %d\r\n", u16TempValue);
#endif
        }
        else
        {
#ifdef UART_BOARD_SEND
//        	printf("Vpv J/N = %d\r\n", u16PV_Voltage);
#endif
        }


		// Calcul de la tension batterie en mV
		// Formule : tensionADC = Vbatt * 24 / 30.81 = Vbatt * 0.78
		// tensionADC = ValeurADC * 3.3 / 1024
		// Vbatt = (ValeurADC * 3.3 / 1024) / 0.78  (en Volt)
		// Vbatt = (ValeurADC * 33 / 1024) * 128    (en mV)
		// (le "/ 0.78" se transforme en "* 1.28"  et on multiplie par 1000 le tout, en faisant 3.3->33 et 1.28->128
		//      u16TempValue = ((p_event->data.done.p_buffer[2] * 33) / 1024) * 128;
		// Après modif de la carte, on trouve l'équation : Vbatt = Vadc * 0.0045 - 0.0173
		u16TempValue = (((uint32_t)p_event->data.done.p_buffer[2]) * 4);
		u16TempValue += ((uint32_t)p_event->data.done.p_buffer[2] / 10) * 5;
		u16TempValue -= 17;

#ifdef UART_BOARD_SEND
//        printf("Vbatt = %d\r\n", u16TempValue);
#endif

		if (bClientBLEConnected == true)
		{	// Seulement si 1 périphérique est connecté
			ble_power_battery_voltage_value_update(&m_power, u16TempValue);
		}

        // Calcul du courant batterie en mA : formule Y = 0.336 * X - 153.01
        // On multiplie tous par 1000, poru avoir un courant en µA
        //if (bClientBLEConnected == true)
		{	// Seulement si 1 périphérique est connecté

        	// On multiplie par le coefficient dû à la tension batterie
        	// Car la formule a été faite pour une tension de 3.3V
        	u32TempValue = ((uint32_t)p_event->data.done.p_buffer[3] * 3300) / u16TempValue;

        	s32CurrentBattery =  ((int32_t)u32TempValue * 336);
        	//s32CurrentBattery += (((int32_t)u32TempValue / 10) * 6);
        	s32CurrentBattery -= 153010;

        	ble_power_battery_current_value_update(&m_power, s32CurrentBattery);

        	// Calcul de la nouvelle capacité
        	if (s32CurrentBattery > 0)
        	{
        		u64CoulombCounter = u64CoulombCounter + s32CurrentBattery;
        	}
        	else
        	{
        		if (u64CoulombCounter > -s32CurrentBattery)
        		{
        			u64CoulombCounter = u64CoulombCounter + s32CurrentBattery;
        		}
        		else
        		{
        			u64CoulombCounter = 0;
        		}
        	}

        	if (u64CoulombCounter > u64MaximumCapacity)
			{
				u64CoulombCounter = u64MaximumCapacity;
			}

        	u08PourcentageCapacityBattery = (unsigned char)((u64CoulombCounter * 100) / u64MaximumCapacity);
        	ble_generic_pourcentage_battery_update(&m_generic, u08PourcentageCapacityBattery);
        	ble_battery_pourcentage_value_update(&m_power, u08PourcentageCapacityBattery);
#ifdef UART_BOARD_SEND
//        	printf("Coulomb = %lld, Batt = %li, pourc = %d\r\n", u64CoulombCounter, s32CurrentBattery, u08PourcentageCapacityBattery);
#endif
		}
/* Essai mais ne fait pas consommer moins !!
        else
        {
        	static unsigned char u08CounterToComputeBatteryCurrent = 0;
        	static unsigned int u16AdditionAdcValueBatteryCurrent = 0;

        	if (u08CounterToComputeBatteryCurrent >= 4)
        	{	// Faire le calcul du nouveau courant
        		// On addition les 4 dernières valeurs de l'ADC et on enlève 4 fois le (-157.49)=-629.96
//        		fCurrentBattery =  ((float)u16AdditionAdcValueBatteryCurrent * 0.3496) - 629.96;


        		// Calcul de la nouvelle capacité
        		u64CoulombCounter = u64CoulombCounter + s32CurrentBattery;

				if (u64CoulombCounter < 0)
				{
					u64CoulombCounter = 0;
				}
				if (u64CoulombCounter > u64MaximumCapacity)
				{
					u64CoulombCounter = u64MaximumCapacity;
				}

        		u08CounterToComputeBatteryCurrent = 0;
        		u16AdditionAdcValueBatteryCurrent = 0;

        		// Désactivation de la FPU pour moins consommer
        		//SCB->CPACR = 0;
        		//__DSB();
        		//__ISB();
#ifdef UART_BOARD_SEND
        		printf("Coulomb = %lld, Batt = %li\r\n", u64CoulombCounter, s32CurrentBattery);
#endif
        	}
        	else
        	{
        		u16AdditionAdcValueBatteryCurrent += p_event->data.done.p_buffer[3];
        		u08CounterToComputeBatteryCurrent++;
        	}
        }
*/

        if (bClientBLEConnected == true)
		{	// Seulement si 1 périphérique est connecté
			// Calcul de la température provenant de la NTC
			// Formule de la NTC très approximatif
			// ValeurTemp = -(ValeurADC * 0.1259) + 90.991
			// On va rajouter 40°C en plus, pour ne pas avoir de valeur négative
			// La formule devient : ValeurTemp = -(ValeurADC * 0.1259) + 130.991
        	u32TempValue = 130991 - ((uint32_t)p_event->data.done.p_buffer[4] * 126);
        	u32TempValue = u32TempValue / 1000;
			u16TempValue = (unsigned int)u32TempValue;
			ble_power_ntc_voltage_value_update(&m_power, u16TempValue);	//p_event->data.done.p_buffer[4]);
#ifdef UART_BOARD_SEND
//			printf("Temp = %d\r\n", u16TempValue);
#endif
		}
#endif
#ifdef MAIN_TIMER
#ifdef BOARD_PLOT
        // On coupe les mesures analogiques
    	nrf_gpio_pin_clear(11);

    	err_code = app_timer_start(m_analog_timer, APP_TIMER_TICKS(800), NULL);
		APP_ERROR_CHECK(err_code);
#endif
#endif

		bNoSleep = false;
		nrf_gpio_pin_clear(30);
    }
}

// Test des fonctions de Nordic
void saadc_init(void)
{
	//nrf_drv_saadc_config_t analogConfiguration = NRF_DRV_SAADC_DEFAULT_CONFIG;
    ret_code_t err_code;

    nrf_saadc_channel_config_t channel_config =
        NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN0);
    
    nrf_saadc_channel_config_t channel_config_CH2 =
        NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN1);
    
    nrf_saadc_channel_config_t channel_config_CH3 =
        NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN2);
    
    nrf_saadc_channel_config_t channel_config_CH4 =
        NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN3);

    nrf_saadc_channel_config_t channel_config_CH5 =
    	NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN4);

    err_code = nrf_drv_saadc_init(NULL /*&analogConfiguration*/, saadc_callback);
    APP_ERROR_CHECK(err_code);

    //channel_config.gain = NRF_SAADC_GAIN1;
    channel_config.reference = NRF_SAADC_REFERENCE_VDD4;	//NRF_SAADC_REFERENCE_INTERNAL;
    channel_config.gain = NRF_SAADC_GAIN1_4;
    err_code = nrf_drv_saadc_channel_init(0, &channel_config);
    APP_ERROR_CHECK(err_code);
    
    //channel_config_CH2.gain = NRF_SAADC_GAIN1;
    channel_config_CH2.reference = NRF_SAADC_REFERENCE_VDD4;	//NRF_SAADC_REFERENCE_INTERNAL;
    channel_config_CH2.gain = NRF_SAADC_GAIN1_4;
    err_code = nrf_drv_saadc_channel_init(1, &channel_config_CH2);
    APP_ERROR_CHECK(err_code);
    
    //channel_config_CH3.gain = NRF_SAADC_GAIN1;
    channel_config_CH3.reference = NRF_SAADC_REFERENCE_INTERNAL;
    //channel_config_CH3.gain = NRF_SAADC_GAIN1_4;
    err_code = nrf_drv_saadc_channel_init(2, &channel_config_CH3);
    APP_ERROR_CHECK(err_code);
    
    //channel_config_CH4.gain = NRF_SAADC_GAIN1;
    channel_config_CH4.reference = NRF_SAADC_REFERENCE_INTERNAL;
    //channel_config_CH4.gain = NRF_SAADC_GAIN1_4;
    err_code = nrf_drv_saadc_channel_init(3, &channel_config_CH4);
    APP_ERROR_CHECK(err_code);

    //channel_config_CH5.gain = NRF_SAADC_GAIN1;
    channel_config_CH5.reference = NRF_SAADC_REFERENCE_VDD4;	//NRF_SAADC_REFERENCE_INTERNAL;
    channel_config_CH5.gain = NRF_SAADC_GAIN1_4;
    err_code = nrf_drv_saadc_channel_init(4, &channel_config_CH5);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(bufferADC[0], SAMPLES_IN_BUFFER);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(bufferADC[1], SAMPLES_IN_BUFFER);
    APP_ERROR_CHECK(err_code);

}
void saadc_sampling_event_init(void)
{
    ret_code_t err_code;

    err_code = nrf_drv_ppi_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
    timer_cfg.bit_width = NRF_TIMER_BIT_WIDTH_32;
    err_code = nrf_drv_timer_init(&m_timer, &timer_cfg, timer_handler);
    APP_ERROR_CHECK(err_code);

    /* setup m_timer for compare event every 1000ms */
    uint32_t ticks = nrf_drv_timer_ms_to_ticks(&m_timer, 1000);
    nrf_drv_timer_extended_compare(&m_timer,
                                   NRF_TIMER_CC_CHANNEL0,
                                   ticks,
                                   NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK,
                                   true);	//false);
    nrf_drv_timer_enable(&m_timer);

    uint32_t timer_compare_event_addr = nrf_drv_timer_compare_event_address_get(&m_timer,
                                                                                NRF_TIMER_CC_CHANNEL0);
    uint32_t saadc_sample_task_addr   = nrf_drv_saadc_sample_task_get();    //nrf_drv_saadc_task_address_get(NRF_SAADC_TASK_SAMPLE);   //nrf_drv_saadc_sample_task_get();

    /* setup ppi channel so that timer compare event is triggering sample task in SAADC */
    err_code = nrf_drv_ppi_channel_alloc(&m_ppi_channel);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_ppi_channel_assign(m_ppi_channel,
                                          timer_compare_event_addr,
                                          saadc_sample_task_addr);
    APP_ERROR_CHECK(err_code);
}


void saadc_sampling_event_enable(void)
{
    ret_code_t err_code = nrf_drv_ppi_channel_enable(m_ppi_channel);

    APP_ERROR_CHECK(err_code);
}
#endif


/**@brief Function for starting advertising.
 */
static void advertising_start(bool erase_bonds)
{
    if (erase_bonds == true)
    {
        delete_bonds();
        // Advertising is started by PM_EVT_PEERS_DELETED_SUCEEDED evetnt
    }
    else
    {
        ret_code_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);

        APP_ERROR_CHECK(err_code);
    }
}

#ifdef BOARD_PLOT
/**@brief Function for the Power manager.
 */
static void init_plot_board(void)
{
    // Activer la sortie pour lire les valeurs analogiques(signal ON_MES_UI)
	nrf_gpio_cfg_output(11);
	nrf_gpio_pin_set(11);

	// Configurer la pin nFault en entrée
	nrf_gpio_cfg_input(12, NRF_GPIO_PIN_NOPULL);

	// Configurer la pin CHRG en entrée
	nrf_gpio_cfg_input(14, NRF_GPIO_PIN_NOPULL);
}
#endif

#ifdef PIR_SENSOR
void init_pir_sensor(void)
{
	// Activer l'entrée du capteur de présence PIR
	nrf_gpio_cfg_input(27, NRF_GPIO_PIN_NOPULL);
}
#endif

#ifdef SPLIT_VERSION
/**@brief Function for initialize the RS485 communication.
 */
static void init_rs485()
{
	// Signal DE à 0
	nrf_gpio_cfg_output(24);
	nrf_gpio_pin_clear(24);

	// Signal /RE à 0
	nrf_gpio_cfg_output(23);
	nrf_gpio_pin_clear(23);
}
#endif

#ifdef LOG_BOARD
/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}
#endif

#ifdef UART_BOARD_SEND
static uint32_t findMacAdress()
{
	return NRF_FICR->DEVICEADDR[0];
}
#endif

#ifdef RTC_PLOT
/** @brief Function starting the internal LFCLK XTAL oscillator.
 */
static void lfclk_config(void)
{
    ret_code_t err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_clock_lfclk_request(NULL);
}

/** @brief: Function for handling the RTC0 interrupts.
 * Triggered on TICK and COMPARE0 match.
 */
static void rtc_handler(nrf_drv_rtc_int_type_t int_type)
{
    if (int_type == NRF_DRV_RTC_INT_COMPARE0)
    {
        //err_code = nrf_drv_rtc_cc_set(&rtc,0,nrf_drv_rtc_counter_get(&rtc) + 1,true);	// Compare 3 secondes
		//APP_ERROR_CHECK(err_code);

        /* Ne fonctionen pas :
        nrf_drv_rtc_disable(&rtc);
        nrf_drv_rtc_counter_clear(&rtc);
        nrf_drv_rtc_enable(&rtc);*/
    }
    else if (int_type == NRF_DRV_RTC_INT_TICK)
    {
    	bFlagIncrementHour = true;
    }
}

/** @brief Function initialization and configuration of RTC driver instance.
 */
static void rtc_config(void)
{
    uint32_t err_code;

    //Initialize RTC instance
    nrf_drv_rtc_config_t config = NRF_DRV_RTC_DEFAULT_CONFIG;
    config.prescaler = 4095;
    err_code = nrf_drv_rtc_init(&rtc, &config, rtc_handler);
    APP_ERROR_CHECK(err_code);

    //Enable tick event & interrupt
    nrf_drv_rtc_tick_enable(&rtc,true);

    // Désactiver la comparaison
    nrf_drv_rtc_cc_disable(&rtc, 0);

    /*
    //Set compare channel to trigger interrupt after COMPARE_COUNTERTIME seconds
    err_code = nrf_drv_rtc_cc_set(&rtc, 0, 1, true);
    APP_ERROR_CHECK(err_code);
	*/

    // Test d'utiliser le PPI pour remettre à 0 le counter de la RTC
    /*uint32_t rtc_compare_event_addr = nrf_drv_rtc_event_address_get(&rtc, NRF_DRV_RTC_INT_COMPARE0);
	uint32_t rtc_compare_task_addr  = nrf_drv_rtc_task_address_get(&rtc, NRF_RTC_TASK_CLEAR);

	// setup ppi channel so that timer compare event is triggering sample task in SAADC
	err_code = nrf_drv_ppi_channel_alloc(&m_ppi_channel_rtc);
	APP_ERROR_CHECK(err_code);

	err_code = nrf_drv_ppi_channel_assign(	m_ppi_channel_rtc,
											rtc_compare_event_addr,
											rtc_compare_task_addr);
	*/

    //Power on RTC instance
    nrf_drv_rtc_enable(&rtc);
}

void updateBLEHour(void)
{
	structHour stHour;
	unsigned char u08TableHour[5];

	stHour = readHour();
	//printf("%d:%d:%d:%d,%d\r\n", stHour.u08Hour, stHour.u08Minute, stHour.u08Second, (stHour.u16MilliSecond >> 8), (stHour.u16MilliSecond & 0xFF));
	u08TableHour[0] = stHour.u08Hour;
	u08TableHour[1] = stHour.u08Minute;
	u08TableHour[2] = stHour.u08Second;
	u08TableHour[3] = (stHour.u16MilliSecond >> 8);
	u08TableHour[4] = (stHour.u16MilliSecond & 0xFF);

	ble_generic_Write_Hour_update(&m_generic, u08TableHour);
}
#endif

#ifdef PWM_DRIVER
void forcePWMColor(unsigned char u08Red, unsigned char u08Green, unsigned char u08Blue, unsigned char u08White)
{
	//printf("Force color : %d, %d, %d, %d\r\n", u08Red, u08Green, u08Blue, u08White);
	m_demo1_seq_values.channel_0 = 0xFF - u08Red;
	m_demo1_seq_values.channel_1 = 0xFF - u08Green;
	m_demo1_seq_values.channel_2 = 0xFF - u08Blue;
	m_demo1_seq_values.channel_3 = 0xFF - u08White;

	fForcePWMColor = true;
}

void ActuelPWMColor(unsigned char *u08Red, unsigned char *u08Green, unsigned char *u08Blue, unsigned char *u08White)
{
	*u08Red = (unsigned char)(0xFF - m_demo1_seq_values.channel_0);
	*u08Green = (unsigned char)(0xFF - m_demo1_seq_values.channel_1);
	*u08Blue = (unsigned char)(0xFF - m_demo1_seq_values.channel_2);
	*u08White = (unsigned char)(0xFF - m_demo1_seq_values.channel_3);

	//printf("Actual color : %d, %d, %d, %d\r\n", *u08Red, *u08Green, *u08Blue, *u08White);
}
#endif

/**@brief Function for application main entry.
 */
int main(void)
{
#ifdef UART_BOARD
    //uint32_t err_code;
	//ret_code_t err_code;
#endif
    bool erase_bonds;
    //char u08NumberOfCharacter = 0;
    //char tabCharacter[50];

#ifdef PLOT_DEMO
	unsigned char u08CounterChangeColor = 0;
	unsigned char u08ColorChoice = 0;
#endif

#ifdef SPLIT_VERSION
    bool bDetecteJour = true;
    unsigned int u16CounterToStartNewScenario = 0;
    //bool bStartNewScenario = false;

    unsigned char u08StateOfScenario = 0;
    bool bSendNewScenario = false;
    bool bSwitchOffLed = false;

    bool bStartNewComputeValue = false;

    unsigned char u16CounterToNuitToJour = 0;
    unsigned char u08CounterToJourToNuit = 0;
#endif

#ifdef PWM_DRIVER
    uint32_t u32ValuePWM = 0;
    uint32_t u32ValeurPWM_TEMP = 0;
#endif
    // Initialize.
#ifdef MAIN_TIMER
    timers_init();
#endif

#ifdef PWM_DRIVER
#ifdef ADC
    unsigned int u16CounterJour = 0;
    bool bFlagJour = false;

    unsigned int u16CounterNuit = 0;
    bool bFlagNuit = false;
#endif
#endif

	nrf_gpio_cfg_output(30);
	nrf_gpio_pin_clear(30);

	// Reset the radio module
	nrf_gpio_cfg_output(06);
	nrf_gpio_pin_set(06);

    buttons_leds_init(&erase_bonds);

#ifdef SPLIT_VERSION
    // TODO : test
/*	nrf_gpio_cfg_output(13);
	nrf_gpio_pin_clear(13);
	nrf_gpio_cfg_output(15);
	nrf_gpio_pin_clear(15);
	nrf_gpio_cfg_output(17);
	nrf_gpio_pin_clear(17);
*/
#endif

#ifdef UART_BOARD
    // Initialisation de la sortie UART
    init_uart();
#else
    nrf_gpio_cfg_output(31);
	nrf_gpio_pin_clear(31);
#endif

//	nrf_gpio_cfg_output(13);
//	nrf_gpio_cfg_output(15);
//	nrf_gpio_pin_set(13);
    ble_stack_init();

//    nrf_gpio_pin_clear(13);
//    nrf_gpio_pin_set(15);

    //device_manager_init(erase_bonds);
    gap_params_init();
    gatt_init();

    // TODO : rajouter le DIS avec : dis_init();
	
#ifdef PWM_DRIVER
    // Initialisation des sorties PWM
    pwm_init();
#else
    nrf_gpio_cfg_output(13);
	nrf_gpio_pin_clear(13);
    nrf_gpio_cfg_output(15);
	nrf_gpio_pin_clear(15);
    nrf_gpio_cfg_output(17);
	nrf_gpio_pin_clear(17);
    nrf_gpio_cfg_output(19);
	nrf_gpio_pin_clear(19);
#endif

#ifdef LOG_BOARD
    log_init();
#endif

#ifdef SPLIT_VERSION
    init_rs485();
#endif

#ifdef ADC
    // Mes fonctions
    //init_adc();
    //saadc_sampling_event_init();
    //saadc_sampling_event_enable();    
    
    saadc_init();
    saadc_sampling_event_init();
    saadc_sampling_event_enable();
    
    //nrf_drv_saadc_calibrate_offset();
#endif

#ifdef BOARD_PLOT
    init_plot_board();
#endif
    
#ifdef PIR_SENSOR
    init_pir_sensor();
#endif

    services_init();
    advertising_init();
    conn_params_init();

    peer_manager_init();

//    nrf_gpio_pin_set(15);

#ifdef PWR_MGMT
        // TODO : à tester car mode low power
        //err_code = nrf_drv_power_init(NULL);
        //APP_ERROR_CHECK(err_code);
#ifdef UART_BOARD_SEND
//    printf("Avant PWR MGMT\r\n");
#endif
    ret_code_t ret_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(ret_code);
#ifdef UART_BOARD_SEND
//    printf("Apres PWR MGMT\r\n");
#endif

    nrf_pwr_mgmt_run();
#ifdef UART_BOARD_SEND
//    printf("Apres PWR RUN\r\n");
#endif
#endif


    // Start execution.
    application_timers_start();
    
//    erase_bonds = true;
    advertising_start(erase_bonds);
//    printf("Start with Bond = %d\r\n", erase_bonds);
//    nrf_delay_ms(500);
    
    //err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
    //APP_ERROR_CHECK(err_code);

#ifdef UART_BOARD_SEND
    printf("%s software V%s\r\n", DEVICE_NAME, FW_REV_NUM);
    nrf_delay_ms(500);

    // Update the firmware version
    ble_generic_firmware_version_update(&m_generic, FIRMWARE_VERSION);
#endif

#ifdef SPLIT_VERSION
    printf("%s software V%s\r\n", DEVICE_NAME, FW_REV_NUM);
	nrf_delay_ms(500);
#endif

#ifdef SPLIT_VERSION
    // Initialisation du scénario
    Init_Scenario();
    ble_scenario_value_update(&m_scenario, getScenarioNumber());

    ble_scenario_saveScenarioNumber(getScenarioNumber());

    setScenarioNumberEpever(getScenarioNumber());

    // Initialisation de la valeur de maximumLoadVoltage
    initMaximumVoltageValue();
    nrf_delay_ms(500);
#endif

#ifdef PLOT_BOARD
    u64MaximumCapacity = (((uint64_t)initCapacityBatteryPlot() * 3600) * 1000);
    u64CoulombCounter = (uint64_t)u64MaximumCapacity / 2;
    u08PourcentageCapacityBattery = 50;
#endif

#ifdef UART_BOARD_SEND
    if (nrf_sdh_is_enabled() == true)
	{
    	printf("SoftDevice Enable !!!!!\r\n");
	}
    else
    {
    	printf("SoftDevice Disable !!!!!\r\n");
    }
#endif

#ifdef SPLIT_VERSION
    // Initialisation de la valeur de capacité par défaut et envoi vers le BLE de cette valeur
    ble_readEpever_capacity_battey_value_update(&m_readEpever, initCapacityBattery());
    nrf_delay_ms(500);
#endif

#ifdef RESEAUX
    Init_Reseaux();
    Init_Sequence();
    Init_Timer_Sequence();
    Init_DemoMode();
    Init_Hour();
#endif

#ifdef RTC_PLOT
    //lfclk_config();
    rtc_config();
#endif

    // Enter main loop.
    for (;;)
    {
#ifdef PWM_DRIVER
    	// Mise à jour de la couleur des leds
    	if (bFlagUpdateColorIntoLed == true)
    	{
    		u32ValuePWM = ble_cus_readPWMValue();

    		colorChannel_0 = 0xFF - ((u32ValuePWM >> 24) & 0xFF);
			colorChannel_1 = 0xFF - ((u32ValuePWM >> 16) & 0xFF);
			colorChannel_2 = 0xFF - ((u32ValuePWM >> 8) & 0xFF);
			colorChannel_3 = 0xFF - (u32ValuePWM & 0xFF);

#ifdef ADC
			if (bFlagNuit == true)
#endif
			{
				m_demo1_seq_values.channel_0 = colorChannel_0;
				m_demo1_seq_values.channel_1 = colorChannel_1;
				m_demo1_seq_values.channel_2 = colorChannel_2;
				m_demo1_seq_values.channel_3 = colorChannel_3;

				// Inverser les octets MSB <-> LSB
				u32ValeurPWM_TEMP = ((u32ValuePWM & 0xFF000000) >> 24) + ((u32ValuePWM & 0x00FF0000) >> 8) + ((u32ValuePWM & 0x0000FF00) << 8) + ((u32ValuePWM & 0x000000FF) << 24);
				ble_cus_custom_value_update(&m_cus, u32ValeurPWM_TEMP);
			}
#ifdef ADC
			else
			{	// Montrer pendant 10 secondes la couleur choisie et s'éteindre en cas de Jour après
				m_demo1_seq_values.channel_0 = colorChannel_0;
				m_demo1_seq_values.channel_1 = colorChannel_1;
				m_demo1_seq_values.channel_2 = colorChannel_2;
				m_demo1_seq_values.channel_3 = colorChannel_3;

				// Inverser les octets MSB <-> LSB
				u32ValeurPWM_TEMP = ((u32ValuePWM & 0xFF000000) >> 24) + ((u32ValuePWM & 0x00FF0000) >> 8) + ((u32ValuePWM & 0x0000FF00) << 8) + ((u32ValuePWM & 0x000000FF) << 24);
				ble_cus_custom_value_update(&m_cus, u32ValeurPWM_TEMP);

				bFlagShowColor = true;
				u08CounterShowColor = 0;
			}
#endif

			bFlagUpdateColorIntoLed = false;
    	}

#ifdef ADC
    	if (bFlagManagePlot == true)
    	{
    		if (bFlagShowColor == true)
    		{
    			u08CounterShowColor++;
    			if (u08CounterShowColor >= VALUE_SHOW_COLOR)
    			{
    				// On regarde si il fait toujours jour ET que les couleurs n'ont pas été forcées
    				if ((bFlagJour == true) && (fForcePWMColor == false))
    				{	// Dans ce cas, on éteint les leds car nosu avons montrer la couleur
    					m_demo1_seq_values.channel_0 = 0xFF;
						m_demo1_seq_values.channel_1 = 0xFF;
						m_demo1_seq_values.channel_2 = 0xFF;
						m_demo1_seq_values.channel_3 = 0xFF;

						// On remet à jour la valeur des PWM, soit tout à 0
						ble_cus_custom_value_update(&m_cus, 0);
    				}

    				u08CounterShowColor = 0;
    				bFlagShowColor = false;
    			}
    		}

			if (u16PV_Voltage > VALUE_PV_VOLTAGE_JOUR)
			{
				if (bFlagJour == false)
				{
					u16CounterJour++;
					if (u16CounterJour >= COUNTER_JOUR_DEBOUNCE)
					{	// Détection du jour
						bFlagJour = true;
						bFlagNuit = false;

						u16CounterJour = 0;

#ifdef UART_BOARD_SEND
						printf("DETECT JOUR\r\n");
#endif

						// TODO : peut être mettre le test de : fForcePWMColor
						// Extinction des leds
						m_demo1_seq_values.channel_0 = 0xFF;
						m_demo1_seq_values.channel_1 = 0xFF;
						m_demo1_seq_values.channel_2 = 0xFF;
						m_demo1_seq_values.channel_3 = 0xFF;

						// On remet à jour la valeur des PWM, soit tout à 0
						ble_cus_custom_value_update(&m_cus, 0);
					}
				}
				else
				{
					u16CounterJour = 0;
				}
			}

			if (u16PV_Voltage < VALUE_PV_VOLTAGE_NUIT)
			{
				if (bFlagNuit == false)
				{
					u16CounterNuit++;
					if (u16CounterNuit >= COUNTER_NUIT_DEBOUNCE)
					{	// Détection du jour
						bFlagNuit = true;
						bFlagJour = false;

						u16CounterNuit = 0;

#ifdef UART_BOARD_SEND
					printf("DETECT NUIT\r\n");
#endif

					// TODO : surement mettre un test si : fForcePWMColor = false !!
						// Mise à jour des couleurs sur les leds
						m_demo1_seq_values.channel_0 = colorChannel_0;
						m_demo1_seq_values.channel_1 = colorChannel_1;
						m_demo1_seq_values.channel_2 = colorChannel_2;
						m_demo1_seq_values.channel_3 = colorChannel_3;

						u32ValuePWM = ble_cus_readPWMValue();

						// Inverser les octets MSB <-> LSB
						u32ValeurPWM_TEMP = ((u32ValuePWM & 0xFF000000) >> 24) + ((u32ValuePWM & 0x00FF0000) >> 8) + ((u32ValuePWM & 0x0000FF00) << 8) + ((u32ValuePWM & 0x000000FF) << 24);
						ble_cus_custom_value_update(&m_cus, u32ValeurPWM_TEMP);
					}
				}
				else
				{
					u16CounterNuit = 0;
				}
			}

#ifdef UART_BOARD_SEND
//        	printf("nFAULT = %ld, CHRG = %ld\r\n", nrf_gpio_pin_read(12), nrf_gpio_pin_read(14));
#endif
        	bFlagManagePlot = false;
        }
#endif
#endif

#ifdef RESEAUX
    	if (checkIfNewConfiguration() == true)
    	{	// STILED : nouvelle configuration arrivée : entout cas, au moins 1 nouvelle !!!!
    		// Si plusieurs config en même temps, le flag sera à True !!! et les valeurs des ID sera le dernier plot configuré
    		bool result;
			uint8_t table[4];

			if (returnLastPlotNumberConfiguration() != 0xFF)
			{
				result = readTableConfigurationReseaux(returnLastPlotNumberConfiguration(), table);
			}
			else
			{	// Il y a eut une erreur, on indique un numéro de série à 0
				result = false;

			}

			if (result == true)
			{
				// TODO : STILED : mettre le code pour l'envoie des trames radio ICI
				// TODO : il faudra peut être mettre une machine d'état pour traiter la LECTURE, ENVOI, REPONSE !!
			}


    		// reset du flag de nouvelle configuration réseaux
    		resetFlagNewConfiguration();

    		// On fait une notificatino sur le numéro de série du Plot configuré
    		if (result == true)
    		{
    			ble_reseaux_read_serial_number_update(&m_reseaux, returnLastSerialNumberConfiguration());
    		}
    		else
    		{
    			ble_reseaux_read_serial_number_update(&m_reseaux, 0);
    		}

#ifdef UART_BOARD_SEND
    		printf("New Config Reseaux\r\n");
#endif
    	}

    	if (checkIfNewUniversNumberConfiguration() == true)
		{	// STILED : nouvelle configuration des univers/ID Group est arrivée : entout cas, au moins 1 nouvelle !!!!
			// Si plusieurs config en même temps, le flag sera à True !!! et les valeurs des ID sera les dernières valeurs envoyées par l'appli
			bool result;
			uint8_t u08IdGroup;
			uint8_t u08SequenceNumber;
			uint8_t u08RepartitionNuit;

			if (returnLastIDGroup() != 0)
			{
				result = readUniversNumber(&u08IdGroup, &u08SequenceNumber, &u08RepartitionNuit);
			}
			else
			{	// Il y a eut une erreur, on indique un numéro de série à 0
				result = false;
			}

			if (result == true)
			{
				// TODO : STILED : mettre le code pour l'envoie des trames radio ICI
				// TODO : il faudra peut être mettre une machine d'état pour traiter la LECTURE, ENVOI, REPONSE !!
			}


			// reset du flag de nouvelle configuration réseaux
			resetFlagNewUniversNumberConfiguration();

			// On fait une notificatino sur le numéro de série du Plot configuré
			if (result == true)
			{
				ble_scenario_C5_sequence_number_update(&m_scenario_C5, returnLastSequenceNumberGroup());
			}
			else
			{
				ble_scenario_C5_sequence_number_update(&m_scenario_C5, 0);
			}

#ifdef UART_BOARD_SEND
			printf("New Config Univers Number Sequence\r\n");
#endif
		}

    	if (checkIfNewSequence() == true)
    	{
    		ble_scenario_C5_read_sequence_update(&m_scenario_C5, returnLastSequenceNumber());

    		resetFlagNewSequence();
    	}

    	if (checkIfNewSequenceToSendRadio() == true)
    	{
    		// TODO : il faudra peut être mettre une machine d'état pour traiter la LECTURE, ENVOI, REPONSE !!

    		// STILED : lecture de chaque séquence : Num Seq, Caractéristique, Led rouge, led verte, led bleu, led blanche
    		//readTableSequence(uint8_t u08SequenceNumber, uint8_t *table)

    		// A la fin de l'envoi complet, indiqué que les séquences ont été envoyées
    		ble_scenario_C5_read_sequence_update(&m_scenario_C5, 0);

    		// On reiniaitlise le flag de gestion de la nouvelle séquence
    		resetFlagNewSequenceToSendRadio();

    		// On remet le tableau à 0
    		Init_Sequence();
    	}

    	if (checkIfEndSendModeScenario() == true)
    	{	// Envoi de l'indication de fin de scénario



    		resetFlagEndSendModeScenario();
    	}

    	if (checkIfNewDemoMode() == true)
    	{	// Une nouvelle valeur du DemoMode a été envoyée

    		// STILED : envoi de cette nouvelle valeur sur le réseau

    		// Gestion de la valeur de DemoMode
    		initNewSequenceC5();

    		resetFlagNewDemoMode();
    	}

    	// -------------- MODE C4 --------------------
    	// Gestion de la partie écriture de la séquence en mode C4
    	if (checkIfNewSequenceC4() == true)
    	{	// Si une nouvelle séquence est arrivée, on renvoie la valeur du numéro de la séquence
    		read_sequence_value_update(&m_cus, returnLastSequenceNumberC4());

    		// On reset le flag de la réception
    		resetFlagNewSequenceC4();
    	}
#endif

#ifdef RTC_PLOT
    if (bFlagIncrementHour == true)
    {	// On doit incrémenter l'heure de fonctionnement de 125 ms
    	incrementHour();

    	bFlagIncrementHour = false;
    }
#endif

#ifdef SPLIT_VERSION
#ifdef PIR_SENSOR
        // Gestion du capteur de présence
      	Manage_PresenceSensor();
#endif
        if (bFlagManageEpeverRS485 == true)
        {
        	// Se mettre en réception poru ne rien envoyer vers l'EPEVER
        	activateReceive_rs485();

        	// On execute le traitement toutes les secondes
        	if ((getPvVoltageValue() > PV_VOLTAGE_JOUR_VALUE) && (bDetecteJour == false))
			{	// Ici, nous avons détecté une journée

        		// Reset du compteur de passage Jour vers Nuit
        		u08CounterToJourToNuit = 0;

        		u16CounterToNuitToJour++;
        		if (u16CounterToNuitToJour >= COUNTER_NUIT_TO_JOUR)
        		{
					//NRF_LOG_INFO("Détection JOUR\r\n");
					activateReceive_rs485();
					printf("Détection JOUR\r\n");
					nrf_delay_ms(10);

					// Mémoriser l'heure du lever
					setHourLever();

					// Indication de la détection du jour
					bDetecteJour = true;

					u16CounterToNuitToJour = 0;
        		}

				// Reset du compteur pour ne pas démarrer un nouveau scénario
				u16CounterToStartNewScenario = 0;
			}

			if ((getPvVoltageValue() < PV_VOLTAGE_NUIT_VALUE) && (bDetecteJour == true))
			{	// Ici, on détecte une nuit et on a déjà détecté une journée avant
				// On lance le timer interne de 5 minutes (300 secondes) avant d'exécuter les calculs du nouveau scénario

				// Reset the counter of the detection Nuit to Jour
				// Reset du compteur de passage Nuit vers Jour
				u16CounterToNuitToJour = 0;

				u08CounterToJourToNuit++;
				if (u08CounterToJourToNuit >= COUNTER_JOUR_TO_NUIT)
				{
					u08CounterToJourToNuit = COUNTER_JOUR_TO_NUIT;

					if (u16CounterToStartNewScenario == 0)
					{
						//NRF_LOG_INFO("Détection NUIT\r\n");
						activateReceive_rs485();
						printf("Détection NUIT\r\n");
						nrf_delay_ms(10);

						// Mémoriser l'heure du coucher
						setHourCoucher();
					}

					u16CounterToStartNewScenario++;
					if (u16CounterToStartNewScenario >= MAXIMUM_COUNTER_TO_START_SCENARIO)
					{
						//NRF_LOG_INFO("Demarrer new Scenario\r\n");
						activateReceive_rs485();
						printf("Demarrer new Scenario\r\n");
						nrf_delay_ms(10);

						//bStartNewScenario = true;
						u16CounterToStartNewScenario = 0;
						bDetecteJour = false;

						bStartNewComputeValue = true;

						u08CounterToJourToNuit = 0;
					}
				}
			}

			if ((ble_compute_readSynchroValue() >= 1) || (bStartNewComputeValue == true))
			{
				computeScenarioValue();

				ble_compute_pdu_value_update(&m_compute, getPduValue());
				ble_compute_timeled_value_update(&m_compute, getTimeLedValue());

				// Reset la synchronisation des données
				ble_compute_saveSynchroValue(0);

				bStartNewComputeValue = false;
			}

			// Gestion des scénarios
			Manage_Scenario(bDetecteJour, &bSendNewScenario, &bSwitchOffLed, &u08StateOfScenario);

			// Envoi de l'information vers l'EPEVER
			Manage_Epever_RS485(true, bSendNewScenario, bSwitchOffLed, u08StateOfScenario, bWriteHourEpever);

#ifdef MAIN_TIMER
			// On va arrêter le timer et le relancer pour ne pas interrompre la séquence d'écriture dans l'EPEVER
			if (bSendNewScenario == true)
			{
				app_timer_stop(m_notif_timer);
				app_timer_start(m_notif_timer, APP_TIMER_TICKS(3000), NULL);
			}
#endif

			bFlagManageEpeverRS485 = false;
			bSendNewScenario = false;
			bSwitchOffLed = false;
			bWriteHourEpever = false;

			//nrf_gpio_pin_clear(13);	// Juste pour enlever la led rouge
        }
        else
        {
        	Manage_Epever_RS485(false, bSendNewScenario, bSwitchOffLed, u08StateOfScenario, bWriteHourEpever);
        }

        if (getUpdateBLEValue() == true)
        {
        	//nrf_gpio_pin_toggle(15);
        	//nrf_gpio_pin_clear(13);	// Juste pour enlever la led rouge

        	ble_readEpever_battery_current_value_update(&m_readEpever, getBatteryCurrentValue());
        	ble_readEpever_battery_voltage_value_update(&m_readEpever, getBatteryVoltageValue());
        	ble_readEpever_pv_current_value_update(&m_readEpever, getPvCurrentValue());
        	ble_readEpever_pv_voltage_value_update(&m_readEpever, getPvVoltageValue());
        	ble_readEpever_generated_energy_value_update(&m_readEpever, getGeneratedEnergyValue());
        	ble_readEpever_consumed_energy_value_update(&m_readEpever, getConsumedEnergyValue());
        	ble_readEpever_battery_soc_value_update(&m_readEpever, getBatterySOCValue());
        	ble_readEpever_battery_status_value_update(&m_readEpever, getBatteryStatusValue());
        	ble_readEpever_load_current_value_update(&m_readEpever, getLoadCurrentValue());
        	ble_readEpever_load_voltage_value_update(&m_readEpever, getLoadVoltageValue());
        	ble_readEpever_real_time_clock_value_update(&m_readEpever, getRealTimeHourValue());
       }
#endif

#ifdef PLOT_DEMO
        if (bFlagManagePLOT_DEMO == true)
        {
        	u08CounterChangeColor++;
        	if (u08CounterChangeColor >= 5)
        	{
        		switch(u08ColorChoice)
        		{
        			case 0:
        				// On était en rien, on allume la rouge
        				m_demo1_seq_values.channel_0 = 0;
						m_demo1_seq_values.channel_1 = 0xFF;
						m_demo1_seq_values.channel_2 = 0xFF;
						m_demo1_seq_values.channel_3 = 0xFF;
						u08ColorChoice = 1;
        				break;

        			case 1:
        				// On était en rouge, on allume la verte
        				m_demo1_seq_values.channel_0 = 0xFF;
						m_demo1_seq_values.channel_1 = 0;
						m_demo1_seq_values.channel_2 = 0xFF;
						m_demo1_seq_values.channel_3 = 0xFF;
						u08ColorChoice = 2;
        				break;

        			case 2:
        				// On était en verte, on allume la bleue
        				m_demo1_seq_values.channel_0 = 0xFF;
						m_demo1_seq_values.channel_1 = 0xFF;
						m_demo1_seq_values.channel_2 = 0;
						m_demo1_seq_values.channel_3 = 0xFF;
						u08ColorChoice = 3;
        				break;

        			case 3:
        				// On était en bleue, on allume la blanche
        				m_demo1_seq_values.channel_0 = 0xFF;
						m_demo1_seq_values.channel_1 = 0xFF;
						m_demo1_seq_values.channel_2 = 0xFF;
						m_demo1_seq_values.channel_3 = 0;
						u08ColorChoice = 4;
        				break;

        			case 4:
        				// On était en blanche, on allume la rouge
        				m_demo1_seq_values.channel_0 = 0;
						m_demo1_seq_values.channel_1 = 0xFF;
						m_demo1_seq_values.channel_2 = 0xFF;
						m_demo1_seq_values.channel_3 = 0xFF;
						u08ColorChoice = 1;
        				break;

        			default:
        				u08ColorChoice = 0;
        				break;
        		}
        		u08CounterChangeColor = 0;
        	}

        	bFlagManagePLOT_DEMO = false;
		}
#endif
#ifdef PLOT_BOARD
#ifdef PWR_MGMT
        //if (bNoSleep == false)
        {
        // Ici, on arrive à 3 mA
		sd_power_mode_set(NRF_POWER_MODE_LOWPWR);

		nrf_pwr_mgmt_run();
        }
#endif
#endif
    }
}

/**
 * @}
 */

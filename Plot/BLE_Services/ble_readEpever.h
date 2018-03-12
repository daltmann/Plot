#ifndef BLE_READEPEVER_H__
#define BLE_READEPEVER_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"

/**@brief   Macro for defining a ble_hrs instance. 
 * 
 * @param   _name   Name of the instance. 
 * @hideinitializer 
 */ 
#define BLE_READEPEVER_DEF(_name)                                                                          \
static ble_readEpever_t _name;                                                                             \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                                                 \
                     BLE_HRS_BLE_OBSERVER_PRIO,                                                     \
                     ble_readEpever_on_ble_evt, &_name)
                     
// CUSTOM_SERVICE_UUID_BASE f364adc9-b000-4042-ba50-05ca45bf8abc
							
#define READEPEVER_SERVICE_UUID_BASE     {0xBC, 0x8A, 0xBF, 0x45, 0xCA, 0x05, 0x50, 0xBA, \
                                          0x40, 0x42, 0xB0, 0x00, 0x12, 0x34, 0x78, 0xF7}

#define READEPEVER_SERVICE_UUID                          0x1400
#define READEPEVER_BATTERY_VOLTAGE_VALUE_CHAR_UUID       0x1401
#define READEPEVER_BATTERY_CURRENT_VALUE_CHAR_UUID       0x1402
#define READEPEVER_PV_VOLTAGE_VALUE_CHAR_UUID            0x1403
#define READEPEVER_GENERATED_ENERGY_VALUE_CHAR_UUID      0x1404
#define READEPEVER_BATTERY_SOC_VALUE_CHAR_UUID           0x1405
#define READEPEVER_BATTERY_STATUS_VALUE_CHAR_UUID        0x1406
#define READEPEVER_PV_CURRENT_VALUE_CHAR_UUID            0x1407
#define READEPEVER_LOAD_CURRENT_VALUE_CHAR_UUID          0x1408
#define READEPEVER_LOAD_VOLTAGE_VALUE_CHAR_UUID          0x1409
#define READEPEVER_CONSUMED_ENERGY_TODAY_VALUE_CHAR_UUID 0x1410
#define READEPEVER_CAPACITY_BATTERY_VALUE_CHAR_UUID 	 0x1411
#define READEPEVER_REAL_TIME_CLOCK_VALUE_CHAR_UUID 	 	0x1412

															
/**@brief ReadEpever Service event type. */
typedef enum
{
    BLE_READEPEVER_EVT_NOTIFICATION_ENABLED,                             /**< Custom value notification enabled event. */
    BLE_READEPEVER_EVT_NOTIFICATION_DISABLED,                             /**< Custom value notification disabled event. */
    BLE_READEPEVER_EVT_DISCONNECTED,
    BLE_READEPEVER_EVT_CONNECTED,
	BLE_READEPEVER_WRITE,
	BLE_READEPEVER_WRITE_CLOCK,
} ble_readEpever_evt_type_t;

/**@brief Custom Service event. */
typedef struct
{
    ble_readEpever_evt_type_t evt_type;                                  /**< Type of event. */
} ble_readEpever_evt_t;

// Forward declaration of the ble_cus_t type.
typedef struct ble_readEpever_s ble_readEpever_t;

/**@brief Custom Service event handler type. */
typedef void (*ble_readEpever_evt_handler_t) (ble_readEpever_t * p_bas, ble_readEpever_evt_t * p_evt);

/**@brief Battery Service init structure. This contains all options and data needed for
 *        initialization of the service.*/
typedef struct
{
    ble_readEpever_evt_handler_t       evt_handler;                    /**< Event handler to be called for handling events in the Custom Service. */
    uint32_t                      initial_custom_value;           /**< Initial custom value */
    ble_srv_cccd_security_mode_t  custom_value_char_attr_md;     /**< Initial security level for Custom characteristics attribute */
} ble_readEpever_init_t;

/**@brief Custom Service structure. This contains various status information for the service. */
struct ble_readEpever_s
{
    ble_readEpever_evt_handler_t       evt_handler;                      /**< Event handler to be called for handling events in the readEpever Service. */
    uint16_t                      service_handle;                   	/**< Handle of Custom Service (as provided by the BLE stack). */
    ble_gatts_char_handles_t      battery_current_value_handles;    	/**< Handles related to the Battery Current Value characteristic. */
    ble_gatts_char_handles_t      battery_voltage_value_handles;    	/**< Handles related to the Battery Voltage Value characteristic. */
    ble_gatts_char_handles_t      pv_current_value_handles;         	/**< Handles related to the PV Current Value characteristic. */
    ble_gatts_char_handles_t      pv_voltage_value_handles;         	/**< Handles related to the PV Voltage Value characteristic. */
    ble_gatts_char_handles_t      generated_energy_value_handles;       /**< Handles related to the generated energy value Value characteristic. */
    ble_gatts_char_handles_t      consumed_energy_value_handles;       	/**< Handles related to the consumed energy value Value characteristic. */
    ble_gatts_char_handles_t      battery_soc_value_handles;       		/**< Handles related to the battery SOC value Value characteristic. */
    ble_gatts_char_handles_t      battery_status_value_handles;    		/**< Handles related to the battery status value Value characteristic. */
    ble_gatts_char_handles_t      load_current_value_handles;    		/**< Handles related to the Load Current Value characteristic. */
    ble_gatts_char_handles_t      load_voltage_value_handles;    		/**< Handles related to the Load Voltage Value characteristic. */
    ble_gatts_char_handles_t      capacity_battery_value_handles;  		/**< Handles related to the Capacity battery Value characteristic. */
    ble_gatts_char_handles_t      real_time_clock_value_handles;  		/**< Handles related to the Real Time Clock Value characteristic. */
    uint16_t                      conn_handle;                      	/**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection). */
    uint8_t                       uuid_type; 
};

/**@brief Function for initializing the Custom Service.
 *
 * @param[out]  p_readEpever     readEpever Service structure. This structure will have to be supplied by
 *                          	the application. It will be initialized by this function, and will later
 *                          	be used to identify this particular service instance.
 * @param[in]   p_readEpever_init  Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on successful initialization of service, otherwise an error code.
 */
uint32_t ble_readEpever_init(ble_readEpever_t * p_readEpever, const ble_readEpever_init_t * p_readEpever_init);

/**@brief Function for handling the Application's BLE Stack events.
 *
 * @details Handles all events from the BLE stack of interest to the readEpever Service.
 *
 * @note 
 *
 * @param[in]   p_ble_evt  Event received from the BLE stack.
 * @param[in]   p_context  Custom Service structure.
 */
void ble_readEpever_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);

/**@brief Function for updating the battery current value.
 *
 * @details The application calls this function when the battery current value should be updated. If
 *          notification has been enabled, the battery current value characteristic is sent to the client.
 *
 * @note 
 *       
 * @param[in]   p_readEpever          Custom Service structure.
 * @param[in]   custom_value
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t ble_readEpever_battery_current_value_update(ble_readEpever_t * p_readEpever, int32_t custom_value);

/**@brief Function for updating the battery voltage value.
 *
 * @details The application calls this function when the battery voltage value should be updated. If
 *          notification has been enabled, the battery voltage value characteristic is sent to the client.
 *
 * @note 
 *       
 * @param[in]   p_readEpever          Custom Service structure.
 * @param[in]   Custom value 
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t ble_readEpever_battery_voltage_value_update(ble_readEpever_t * p_readEpever, uint16_t custom_value);

/**@brief Function for updating the PV current value.
 *
 * @details The application calls this function when the PV current value should be updated. If
 *          notification has been enabled, the PV current value characteristic is sent to the client.
 *
 * @note 
 *       
 * @param[in]   p_readEpever          Custom Service structure.
 * @param[in]   custom_value
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t ble_readEpever_pv_current_value_update(ble_readEpever_t * p_readEpever, uint16_t custom_value);

/**@brief Function for updating the PV voltage value.
 *
 * @details The application calls this function when the PV voltage value should be updated. If
 *          notification has been enabled, the PV voltage value characteristic is sent to the client.
 *
 * @note 
 *       
 * @param[in]   p_readEpever          Custom Service structure.
 * @param[in]   Custom value 
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t ble_readEpever_pv_voltage_value_update(ble_readEpever_t * p_readEpever, uint16_t custom_value);

/**@brief Function for updating the Generated Energy value.
 *
 * @details The application calls this function when the Generated Energy value should be updated. If
 *          notification has been enabled, the PV voltage value characteristic is sent to the client.
 *
 * @note
 *
 * @param[in]   p_readEpever          Custom Service structure.
 * @param[in]   Custom value
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t ble_readEpever_generated_energy_value_update(ble_readEpever_t * p_readEpever, uint32_t custom_value);

/**@brief Function for updating the Consumed Energy value.
 *
 * @details The application calls this function when the Consumed Energy value should be updated. If
 *          notification has been enabled, the PV voltage value characteristic is sent to the client.
 *
 * @note
 *
 * @param[in]   p_readEpever          Custom Service structure.
 * @param[in]   Custom value
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t ble_readEpever_consumed_energy_value_update(ble_readEpever_t * p_readEpever, uint32_t custom_value);

/**@brief Function for updating the Battery SOC value.
 *
 * @details The application calls this function when the Battery SOC value should be updated. If
 *          notification has been enabled, the PV voltage value characteristic is sent to the client.
 *
 * @note
 *
 * @param[in]   p_readEpever          Custom Service structure.
 * @param[in]   Custom value
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t ble_readEpever_battery_soc_value_update(ble_readEpever_t * p_readEpever, uint16_t custom_value);

/**@brief Function for updating the Battery Status value.
 *
 * @details The application calls this function when the Battery Status value should be updated. If
 *          notification has been enabled, the PV voltage value characteristic is sent to the client.
 *
 * @note
 *
 * @param[in]   p_readEpever          Custom Service structure.
 * @param[in]   Custom value
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t ble_readEpever_battery_status_value_update(ble_readEpever_t * p_readEpever, uint16_t custom_value);

/**@brief Function for updating the Load current value.
 *
 * @details The application calls this function when the Load current value should be updated. If
 *          notification has been enabled, the PV voltage value characteristic is sent to the client.
 *
 * @note 
 *
 * @param[in]   p_readEpever          Custom Service structure.
 * @param[in]   Custom value
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t ble_readEpever_load_current_value_update(ble_readEpever_t * p_readEpever, uint16_t custom_value);

/**@brief Function for updating the Load voltage value.
 *
 * @details The application calls this function when the Load voltage value should be updated. If
 *          notification has been enabled, the PV voltage value characteristic is sent to the client.
 *
 * @note 
 *
 * @param[in]   p_readEpever          Custom Service structure.
 * @param[in]   Custom value
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t ble_readEpever_load_voltage_value_update(ble_readEpever_t * p_readEpever, uint16_t custom_value);

/**@brief Function for updating the Capacity battery value.
 *
 * @details The application calls this function when the Capacity battery value should be updated. If
 *          notification has been enabled, the Capacity battery value characteristic is sent to the client.
 *
 * @note
 *
 * @param[in]   p_readEpever          Custom Service structure.
 * @param[in]   Custom value
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t ble_readEpever_capacity_battey_value_update(ble_readEpever_t * p_readEpever, uint16_t custom_value);

/**@brief Function for updating the Real Time Clock value.
 *
 * @details The application calls this function when the Real Time Clock value should be updated. If
 *          notification has been enabled, the Real Time Clock value characteristic is sent to the client.
 *
 * @note
 *
 * @param[in]   p_readEpever          Custom Service structure.
 * @param[in]   Custom value
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t ble_readEpever_real_time_clock_value_update(ble_readEpever_t * p_readEpever, unsigned long long custom_value);

#endif // BLE_READEPEVER_H__

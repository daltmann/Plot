#ifndef BLE_POWER_H__
#define BLE_POWER_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"

/**@brief   Macro for defining a ble_hrs instance. 
 * 
 * @param   _name   Name of the instance. 
 * @hideinitializer 
 */ 
#define BLE_POWER_DEF(_name)                                                                          \
static ble_power_t _name;                                                                             \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                                                 \
                     BLE_HRS_BLE_OBSERVER_PRIO,                                                     \
                     ble_power_on_ble_evt, &_name)
                     
// CUSTOM_SERVICE_UUID_BASE f364adc9-b000-4042-ba50-05ca45bf8abc
							
#define POWER_SERVICE_UUID_BASE         {0xBC, 0x8A, 0xBF, 0x45, 0xCA, 0x05, 0x50, 0xBA, \
                                          0x40, 0x42, 0xB0, 0x00, 0x12, 0x34, 0x78, 0xF5}

#define POWER_SERVICE_UUID                          0x1400
#define POWER_PV_CURRENT_VALUE_CHAR_UUID            0x1401
#define POWER_PV_VOLTAGE_VALUE_CHAR_UUID            0x1402
#define POWER_BATTERY_VOLTAGE_VALUE_CHAR_UUID       0x1403
#define POWER_BATTERY_CURRENT_VALUE_CHAR_UUID       0x1404
#define POWER_NTC_VOLTAGE_VALUE_CHAR_UUID       	0x1405
#define POWER_BATTERY_POURCENTAGE_VALUE_CHAR_UUID  	0x1406

															
/**@brief Custom Service event type. */
typedef enum
{
    BLE_POWER_EVT_NOTIFICATION_ENABLED,                             /**< Custom value notification enabled event. */
    BLE_POWER_EVT_NOTIFICATION_DISABLED,                             /**< Custom value notification disabled event. */
    BLE_POWER_EVT_DISCONNECTED,
    BLE_POWER_EVT_CONNECTED,
} ble_power_evt_type_t;

/**@brief Custom Service event. */
typedef struct
{
    ble_power_evt_type_t evt_type;                                  /**< Type of event. */
} ble_power_evt_t;

// Forward declaration of the ble_cus_t type.
typedef struct ble_power_s ble_power_t;

/**@brief Custom Service event handler type. */
typedef void (*ble_power_evt_handler_t) (ble_power_t * p_bas, ble_power_evt_t * p_evt);

/**@brief Battery Service init structure. This contains all options and data needed for
 *        initialization of the service.*/
typedef struct
{
    ble_power_evt_handler_t       evt_handler;                    /**< Event handler to be called for handling events in the Custom Service. */
    uint16_t                      initial_custom_value;           /**< Initial custom value */
    ble_srv_cccd_security_mode_t  custom_value_char_attr_md;     /**< Initial security level for Custom characteristics attribute */
} ble_power_init_t;

/**@brief Custom Service structure. This contains various status information for the service. */
struct ble_power_s
{
    ble_power_evt_handler_t       evt_handler;                      /**< Event handler to be called for handling events in the Custom Service. */
    uint16_t                      service_handle;                   /**< Handle of Custom Service (as provided by the BLE stack). */
    ble_gatts_char_handles_t      battery_current_value_handles;    /**< Handles related to the Battery Current Value characteristic. */
    ble_gatts_char_handles_t      battery_voltage_value_handles;    /**< Handles related to the Battery Voltage Value characteristic. */
    ble_gatts_char_handles_t      pv_current_value_handles;         /**< Handles related to the PV Current Value characteristic. */
    ble_gatts_char_handles_t      pv_voltage_value_handles;         /**< Handles related to the PV Voltage Value characteristic. */
    ble_gatts_char_handles_t      ntc_voltage_value_handles;        /**< Handles related to the NTC Voltage Value characteristic. */
    ble_gatts_char_handles_t      battery_pourcentage_value_handles;/**< Handles related to the Battery pourcentage Value characteristic. */
    uint16_t                      conn_handle;                      /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection). */
    uint8_t                       uuid_type; 
};

/**@brief Function for initializing the Custom Service.
 *
 * @param[out]  p_power     Custom Service structure. This structure will have to be supplied by
 *                          the application. It will be initialized by this function, and will later
 *                          be used to identify this particular service instance.
 * @param[in]   p_power_init  Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on successful initialization of service, otherwise an error code.
 */
uint32_t ble_power_init(ble_power_t * p_power, const ble_power_init_t * p_power_init);

/**@brief Function for handling the Application's BLE Stack events.
 *
 * @details Handles all events from the BLE stack of interest to the Battery Service.
 *
 * @note 
 *
 * @param[in]   p_ble_evt  Event received from the BLE stack.
 * @param[in]   p_context  Custom Service structure.
 */
void ble_power_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);

/**@brief Function for updating the battery current value.
 *
 * @details The application calls this function when the battery current value should be updated. If
 *          notification has been enabled, the battery current value characteristic is sent to the client.
 *
 * @note 
 *       
 * @param[in]   p_power          Custom Service structure.
 * @param[in]   fValueCurrent
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t ble_power_battery_current_value_update(ble_power_t * p_power, int32_t u32ValueCurrent);

/**@brief Function for updating the battery voltage value.
 *
 * @details The application calls this function when the battery voltage value should be updated. If
 *          notification has been enabled, the battery voltage value characteristic is sent to the client.
 *
 * @note 
 *       
 * @param[in]   p_power          Custom Service structure.
 * @param[in]   Custom value 
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t ble_power_battery_voltage_value_update(ble_power_t * p_power, uint16_t custom_value);

/**@brief Function for updating the PV current value.
 *
 * @details The application calls this function when the PV current value should be updated. If
 *          notification has been enabled, the PV current value characteristic is sent to the client.
 *
 * @note 
 *       
 * @param[in]   p_power          Custom Service structure.
 * @param[in]   fValueCurrent
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t ble_power_pv_current_value_update(ble_power_t * p_power, uint32_t u32ValueCurrent);

/**@brief Function for updating the PV voltage value.
 *
 * @details The application calls this function when the PV voltage value should be updated. If
 *          notification has been enabled, the PV voltage value characteristic is sent to the client.
 *
 * @note 
 *       
 * @param[in]   p_power          Custom Service structure.
 * @param[in]   Custom value 
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t ble_power_pv_voltage_value_update(ble_power_t * p_power, uint16_t custom_value);

/**@brief Function for updating the NTC voltage value.
 *
 * @details The application calls this function when the NTC voltage value should be updated. If
 *          notification has been enabled, the PV voltage value characteristic is sent to the client.
 *
 * @note
 *
 * @param[in]   p_power          Custom Service structure.
 * @param[in]   Custom value
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t ble_power_ntc_voltage_value_update(ble_power_t * p_power, uint16_t custom_value);

/**@brief Function for updating the Battery Pourcentage value.
 *
 * @details The application calls this function when the Battery pourcentage value should be updated. If
 *          notification has been enabled, the PV voltage value characteristic is sent to the client.
 *
 * @note
 *
 * @param[in]   p_power          Custom Service structure.
 * @param[in]   Custom value
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t ble_battery_pourcentage_value_update(ble_power_t * p_power, uint8_t custom_value);

#endif // BLE_POWER_H__

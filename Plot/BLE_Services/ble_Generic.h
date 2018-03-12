#ifndef BLE_GENERIC_H__
#define BLE_GENERIC_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"

/**@brief   Macro for defining a ble_hrs instance. 
 * 
 * @param   _name   Name of the instance. 
 * @hideinitializer 
 */ 
#define BLE_GENERIC_DEF(_name)                                                                          \
static ble_generic_t _name;                                                                             \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                                                 \
                     BLE_HRS_BLE_OBSERVER_PRIO,                                                     \
                     ble_generic_on_ble_evt, &_name)

// GENERIC_SERVICE_UUID_BASE fd641400-b000-4042-ba50-05ca45bf8abc
							
#define GENERIC_SERVICE_UUID_BASE     {0xBC, 0x8A, 0xBF, 0x45, 0xCA, 0x05, 0x50, 0xBA, \
                                          0x40, 0x42, 0xB0, 0x00, 0xC9, 0xAD, 0x64, 0xFD}

#define GENERIC_SERVICE_UUID              		0x1400
#define GENERIC_WRITE_REQUEST_UUID           	0x1401
#define GENERIC_POUIRCENTAGE_BATTERY_UUID       0x1402
#define GENERIC_FIRMWARE_VERSION_UUID      		0x1403
#define GENERIC_WRITE_DATE_UUID		       		0x1404
#define GENERIC_WRITE_HOUR_UUID       			0x1405
															
/**@brief Custom Service event type. */
typedef enum
{
    BLE_GENERIC_EVT_NOTIFICATION_ENABLED,                   /**< Custom value notification enabled event. */
    BLE_GENERIC_EVT_NOTIFICATION_DISABLED,                 	/**< Custom value notification disabled event. */
    BLE_GENERIC_EVT_DISCONNECTED,
    BLE_GENERIC_EVT_CONNECTED,
	BLE_GENERIC_WRITE_REQUEST,					/**< Write a new value of write request */
	BLE_GENERIC_WRITE_DATE,						/**< Write a new value of write date */
	BLE_GENERIC_WRITE_HOUR,						/**< Write a new value of write hour */
} ble_generic_evt_type_t;

/**@brief Custom Service event. */
typedef struct
{
    ble_generic_evt_type_t evt_type;                                  /**< Type of event. */
} ble_generic_evt_t;

// Forward declaration of the ble_scenario_t type.
typedef struct ble_generic_s ble_generic_t;


/**@brief Custom Service event handler type. */
typedef void (*ble_generic_evt_handler_t) (ble_generic_t * p_scenario, ble_generic_evt_t * p_evt);

/**@brief Battery Service init structure. This contains all options and data needed for
 *        initialization of the service.*/
typedef struct
{
    ble_generic_evt_handler_t         evt_handler;                    /**< Event handler to be called for handling events in the Custom Service. */
    uint32_t                      initial_custom_value;           /**< Initial custom value */
    ble_srv_cccd_security_mode_t  custom_value_char_attr_md;     /**< Initial security level for Custom characteristics attribute */
} ble_generic_init_t;

/**@brief Custom Service structure. This contains various status information for the service. */
struct ble_generic_s
{
    ble_generic_evt_handler_t         evt_handler;                    /**< Event handler to be called for handling events in the Custom Service. */
    uint16_t                      service_handle;                 /**< Handle of Custom Service (as provided by the BLE stack). */
    ble_gatts_char_handles_t      write_request_handles;
    ble_gatts_char_handles_t      pourcentage_battery_handles;
    ble_gatts_char_handles_t      firmware_version_handles;
    ble_gatts_char_handles_t      date_write_handles;
    ble_gatts_char_handles_t      hour_write_handles;
    uint16_t                      conn_handle;                    /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection). */
    uint8_t                       uuid_type; 
};

/**@brief Function for initializing the Custom Service.
 *
 * @param[out]  p_scenario       Custom Service structure. This structure will have to be supplied by
 *                          the application. It will be initialized by this function, and will later
 *                          be used to identify this particular service instance.
 * @param[in]   p_scenario_init  Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on successful initialization of service, otherwise an error code.
 */
uint32_t ble_generic_init(ble_generic_t * p_scenario, const ble_generic_init_t * p_scenario_init);

/**@brief Function for handling the Application's BLE Stack events.
 *
 * @details Handles all events from the BLE stack of interest to the Battery Service.
 *
 * @note 
 *
 * @param[in]   p_ble_evt  Event received from the BLE stack.
 * @param[in]   p_context  Custom Service structure.
 */
void ble_generic_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);

/**@brief Function for updating the custom value.
 *
 * @details The application calls this function when the cutom value should be updated. If
 *          notification has been enabled, the custom value characteristic is sent to the client.
 *
 * @note 
 *       
 * @param[in]   p_scenario          Custom Service structure.
 * @param[in]   Custom value 
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t ble_generic_pourcentage_battery_update(ble_generic_t * p_scenario, uint8_t custom_value);

uint32_t ble_generic_firmware_version_update(ble_generic_t * p_scenario, uint8_t custom_value);

uint32_t ble_generic_Write_Hour_update(ble_generic_t * p_scenario, uint8_t *custom_value);


#endif // BLE_GENERIC_H__

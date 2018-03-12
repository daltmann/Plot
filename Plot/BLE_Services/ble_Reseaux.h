#ifndef BLE_RESEAUX_H__
#define BLE_RESEAUX_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"

/**@brief   Macro for defining a ble_hrs instance. 
 * 
 * @param   _name   Name of the instance. 
 * @hideinitializer 
 */ 
#define BLE_RESEAUX_DEF(_name)                                                                          \
static ble_reseaux_t _name;                                                                             \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                                                 \
                     BLE_HRS_BLE_OBSERVER_PRIO,                                                     \
                     ble_reseaux_on_ble_evt, &_name)

// RESEAUX_SERVICE_UUID_BASE fb641400-b000-4042-ba50-05ca45bf8abc
							
#define RESEAUX_SERVICE_UUID_BASE     {0xBC, 0x8A, 0xBF, 0x45, 0xCA, 0x05, 0x50, 0xBA, \
                                          0x40, 0x42, 0xB0, 0x00, 0xC9, 0xAD, 0x64, 0xFB}

#define RESEAUX_SERVICE_UUID              		0x1400
#define RESEAUX_NAME_UUID           			0x1401
#define RESEAUX_SERIAL_UNIVERS_NUMBER_UUID      0x1402
#define RESEAUX_READ_SERIAL_NUMBER_UUID    		0x1403
#define RESEAUX_PRODUCT_NUMBER_UUID       		0x1404
#define RESEAUX_LINE_NUMBER_UUID       			0x1405
#define RESEAUX_CONFIG_TABLE_UUID      			0x1406


#define NUMBER_BYTE_NETWORK_NAME	14

#define NUMBER_WRITE_SERIAL_UNIVERS_NUMBER	5
															
/**@brief Custom Service event type. */
typedef enum
{
    BLE_RESEAUX_EVT_NOTIFICATION_ENABLED,                   /**< Custom value notification enabled event. */
    BLE_RESEAUX_EVT_NOTIFICATION_DISABLED,                 	/**< Custom value notification disabled event. */
    BLE_RESEAUX_EVT_DISCONNECTED,
    BLE_RESEAUX_EVT_CONNECTED,
	BLE_RESEAUX_WRITE_NAME,									/**< Write a new value of network name */
	BLE_RESEAUX_WRITE_SERIAL_UNIVERS_NUMBER,				/**< Write a new value of serial and univers number */
	BLE_RESEAUX_WRITE_PRODUCT_NUMBER,						/**< Write a new value of product number */
	BLE_RESEAUX_WRITE_LINE_NUMBER,							/**< Write a new value of line number */
	BLE_RESEAUX_WRITE_CONFIG_TABLE,							/**< Write a new value of configuration table */
} ble_reseaux_evt_type_t;

/**@brief Custom Service event. */
typedef struct
{
    ble_reseaux_evt_type_t evt_type;                                  /**< Type of event. */
} ble_reseaux_evt_t;

// Forward declaration of the ble_scenario_t type.
typedef struct ble_reseaux_s ble_reseaux_t;


/**@brief Custom Service event handler type. */
typedef void (*ble_reseaux_evt_handler_t) (ble_reseaux_t * p_scenario, ble_reseaux_evt_t * p_evt);

/**@brief Battery Service init structure. This contains all options and data needed for
 *        initialization of the service.*/
typedef struct
{
    ble_reseaux_evt_handler_t         evt_handler;                    /**< Event handler to be called for handling events in the Custom Service. */
    uint32_t                      initial_custom_value;           /**< Initial custom value */
    ble_srv_cccd_security_mode_t  custom_value_char_attr_md;     /**< Initial security level for Custom characteristics attribute */
} ble_reseaux_init_t;

/**@brief Custom Service structure. This contains various status information for the service. */
struct ble_reseaux_s
{
    ble_reseaux_evt_handler_t     evt_handler;                  /**< Event handler to be called for handling events in the Custom Service. */
    uint16_t                      service_handle;               /**< Handle of Custom Service (as provided by the BLE stack). */
    ble_gatts_char_handles_t      network_name_handles;         /**< Handles related to the network name Value characteristic. */
    ble_gatts_char_handles_t      serial_univers_handles;       /**< Handles related to the serial and univers Value characteristic. */
    ble_gatts_char_handles_t      read_serial_handles;          /**< Handles related to the serial reading Value characteristic. */
    ble_gatts_char_handles_t      product_number_handles;		/**< Handles related to the product number Value characteristic. */
    ble_gatts_char_handles_t      line_number_handles;          /**< Handles related to the line number Value characteristic. */
    ble_gatts_char_handles_t      config_table_handles;         /**< Handles related to the configuration table Value characteristic. */
    uint16_t                      conn_handle;                  /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection). */
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
uint32_t ble_reseaux_init(ble_reseaux_t * p_scenario, const ble_reseaux_init_t * p_scenario_init);

/**@brief Function for handling the Application's BLE Stack events.
 *
 * @details Handles all events from the BLE stack of interest to the Battery Service.
 *
 * @note 
 *
 * @param[in]   p_ble_evt  Event received from the BLE stack.
 * @param[in]   p_context  Custom Service structure.
 */
void ble_reseaux_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);

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
uint32_t ble_reseaux_read_serial_number_update(ble_reseaux_t * p_scenario, uint32_t custom_value);


void saveNetworkName(const uint8_t *u08ValueName);
void readNetworkName(uint8_t *tableDataNameValue);

void saveConfigBLEReseaux(const uint8_t *u08ValueName);
void readConfigBLEReseaux(uint8_t *tableDataConfigValue);


#endif // BLE_RESEAUX_H__

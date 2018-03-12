#ifndef BLE_SCENARIO_H__
#define BLE_SCENARIO_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"

/**@brief   Macro for defining a ble_hrs instance. 
 * 
 * @param   _name   Name of the instance. 
 * @hideinitializer 
 */ 
#define BLE_SCENARIO_DEF(_name)                                                                          \
static ble_scenario_t _name;                                                                             \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                                                 \
                     BLE_HRS_BLE_OBSERVER_PRIO,                                                     \
                     ble_scenario_on_ble_evt, &_name)

// SCENARIOTOM_SERVICE_UUID_BASE f364adc9-b000-4042-ba50-05ca45bf8abc
							
#define SCENARIO_SERVICE_UUID_BASE         {0xBC, 0x8A, 0xBF, 0x45, 0xCA, 0x05, 0x50, 0xBA, \
                                          0x40, 0x42, 0xB0, 0x00, 0xC9, 0xAD, 0x64, 0xF9}

#define SCENARIO_SERVICE_UUID               0x1400
#define SCENARIO_VALUE_CHAR_UUID            0x1401
#define SCENARIO_DATA_CHAR_UUID            	0x1402
															
/**@brief Custom Service event type. */
typedef enum
{
    BLE_SCENARIO_EVT_NOTIFICATION_ENABLED,                             /**< Custom value notification enabled event. */
    BLE_SCENARIO_EVT_NOTIFICATION_DISABLED,                             /**< Custom value notification disabled event. */
    BLE_SCENARIO_EVT_DISCONNECTED,
    BLE_SCENARIO_EVT_CONNECTED,
	BLE_SCENARIO_WRITE													/**< Write a new value of PWM */
} ble_scenario_evt_type_t;

/**@brief Custom Service event. */
typedef struct
{
    ble_scenario_evt_type_t evt_type;                                  /**< Type of event. */
} ble_scenario_evt_t;

// Forward declaration of the ble_scenario_t type.
typedef struct ble_scenario_s ble_scenario_t;


/**@brief Custom Service event handler type. */
typedef void (*ble_scenario_evt_handler_t) (ble_scenario_t * p_scenario, ble_scenario_evt_t * p_evt);

/**@brief Battery Service init structure. This contains all options and data needed for
 *        initialization of the service.*/
typedef struct
{
    ble_scenario_evt_handler_t         evt_handler;                    /**< Event handler to be called for handling events in the Custom Service. */
    uint32_t                      initial_custom_value;           /**< Initial custom value */
    ble_srv_cccd_security_mode_t  custom_value_char_attr_md;     /**< Initial security level for Custom characteristics attribute */
} ble_scenario_init_t;

/**@brief Custom Service structure. This contains various status information for the service. */
struct ble_scenario_s
{
    ble_scenario_evt_handler_t         evt_handler;                    /**< Event handler to be called for handling events in the Custom Service. */
    uint16_t                      service_handle;                 /**< Handle of Custom Service (as provided by the BLE stack). */
    ble_gatts_char_handles_t      scenario_value_handles;           /**< Handles related to the Scenario number Value characteristic. */
    ble_gatts_char_handles_t      scenario_data_value_handles;           /**< Handles related to the Scenario Data Value characteristic. */
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
uint32_t ble_scenario_init(ble_scenario_t * p_scenario, const ble_scenario_init_t * p_scenario_init);

/**@brief Function for handling the Application's BLE Stack events.
 *
 * @details Handles all events from the BLE stack of interest to the Battery Service.
 *
 * @note 
 *
 * @param[in]   p_ble_evt  Event received from the BLE stack.
 * @param[in]   p_context  Custom Service structure.
 */
void ble_scenario_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);

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
uint32_t ble_scenario_value_update(ble_scenario_t * p_scenario, uint8_t custom_value);

/**@brief Function to save the value of the Scenario Number
 *
 * @details Save the new value of the Scenario Number
 *
 * @note 
 *
 * @param[in]   u08Value      New value of the Scenario Number
 */
void ble_scenario_saveScenarioNumber(uint8_t u08Value);

/**@brief Function to return the valeu of the Scenario Number
 *
 * @details Return the Scenario Number value
 *
 * @note 
 *
 * @return      u08ScenarioNumber Value of the Scenario Number
 */
uint8_t ble_scenario_readScenarioNumber(void);

/**@brief Function to save the value of the Scenario Number
 *
 * @details Save the new value of the Scenario Number
 *
 * @note
 *
 * @param[in]   u08Value      Data from the scenario of the Scenario Number
 * @param[in]   u08LenghtData Lenght data from the scenario of the Scenario Number
 */
void ble_scenario_saveScenarioData(uint8_t *u08Value, uint8_t u08LenghtData);

/**@brief Function to save the value of the Scenario Number
 *
 * @details Save the new value of the Scenario Number
 *
 * @note
 *
 * @param[in]   tableDataScenarioValue Table of the data for the scenario data
 *
 * @return      u08NumberOfDataScenario Value of the number of data from Data scenario
 */
uint8_t ble_scenario_readScenarioData(uint8_t *tableDataScenarioValue);

#endif // BLE_SCENARIO_H__

#ifndef BLE_CUS_H__
#define BLE_CUS_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"

/**@brief   Macro for defining a ble_hrs instance. 
 * 
 * @param   _name   Name of the instance. 
 * @hideinitializer 
 */ 
#define BLE_CUS_DEF(_name)                                                                          \
static ble_cus_t _name;                                                                             \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                                                 \
                     BLE_HRS_BLE_OBSERVER_PRIO,                                                     \
                     ble_cus_on_ble_evt, &_name)

// CUSTOM_SERVICE_UUID_BASE f364adc9-b000-4042-ba50-05ca45bf8abc
							
#define CUSTOM_SERVICE_UUID_BASE         {0xBC, 0x8A, 0xBF, 0x45, 0xCA, 0x05, 0x50, 0xBA, \
                                          0x40, 0x42, 0xB0, 0x00, 0xC9, 0xAD, 0x64, 0xF3}

#define CUSTOM_SERVICE_UUID               		0x1400
#define CUSTOM_VALUE_CHAR_UUID            		0x1401
#define RED_CURRENT_VALUE_CHAR_UUID 		    0x1402
#define GREEN_CURRENT_VALUE_CHAR_UUID     		0x1403
#define BLUE_CURRENT_VALUE_CHAR_UUID      		0x1404
#define WHITE_CURRENT_VALUE_CHAR_UUID     		0x1405
#define FONCTIONNEMENT_MODE_VALUE_CHAR_UUID     0x1406
#define TIME_REPARTITION_VALUE_CHAR_UUID  		0x1407
#define SEQUENCE_NUMBER_VALUE_CHAR_UUID   		0x1408
#define SEQUENCE_WRITE_VALUE_CHAR_UUID    		0x1409
#define SEQUENCE_READ_VALUE_CHAR_UUID     		0x140A

#define NB_TABLE_PWM	10
															
/**@brief Custom Service event type. */
typedef enum
{
    BLE_CUS_EVT_NOTIFICATION_ENABLED,           /**< Custom value notification enabled event. */
    BLE_CUS_EVT_NOTIFICATION_DISABLED,          /**< Custom value notification disabled event. */
    BLE_CUS_EVT_DISCONNECTED,
    BLE_CUS_EVT_CONNECTED,
	BLE_CUS_WRITE,								/**< Write a new value of PWM */
	BLE_CUS_WRITE_FONCTIONNEMENT_MODE,			/**< Write a new value of Fonctionnement mode */
	BLE_CUS_WRITE_TIME_REPARTITION,				/**< Write a new value of time repartition */
	BLE_CUS_WRITE_SEQUENCE_NUMBER,				/**< Write a new value of sequence number */
	BLE_CUS_WRITE_WRITE_SEQUENCE,				/**< Write a new value of write sequence */
} ble_cus_evt_type_t;

/**@brief Custom Service event. */
typedef struct
{
    ble_cus_evt_type_t evt_type;                                  /**< Type of event. */
} ble_cus_evt_t;

// Forward declaration of the ble_cus_t type.
typedef struct ble_cus_s ble_cus_t;


/**@brief Custom Service event handler type. */
typedef void (*ble_cus_evt_handler_t) (ble_cus_t * p_cus, ble_cus_evt_t * p_evt);

/**@brief Battery Service init structure. This contains all options and data needed for
 *        initialization of the service.*/
typedef struct
{
    ble_cus_evt_handler_t         evt_handler;                    /**< Event handler to be called for handling events in the Custom Service. */
    uint32_t                      initial_custom_value;           /**< Initial custom value */
    ble_srv_cccd_security_mode_t  custom_value_char_attr_md;     /**< Initial security level for Custom characteristics attribute */
} ble_cus_init_t;

/**@brief Custom Service structure. This contains various status information for the service. */
struct ble_cus_s
{
    ble_cus_evt_handler_t         evt_handler;                    		/**< Event handler to be called for handling events in the Custom Service. */
    uint16_t                      service_handle;                 		/**< Handle of Custom Service (as provided by the BLE stack). */
	ble_gatts_char_handles_t      custom_value_handles;           		/**< Handles related to the Custom Value characteristic. */
    ble_gatts_char_handles_t      red_current_value_handles;      		/**< Handles related to the Red current Value characteristic. */
    ble_gatts_char_handles_t      green_current_value_handles;    		/**< Handles related to the Red current Value characteristic. */
    ble_gatts_char_handles_t      blue_current_value_handles;     		/**< Handles related to the Red current Value characteristic. */
    ble_gatts_char_handles_t      white_current_value_handles;    		/**< Handles related to the Red current Value characteristic. */
    ble_gatts_char_handles_t      fonctionnement_mode_value_handles;	/**< Handles related to the fonctionnement mode Value characteristic. */
    ble_gatts_char_handles_t      time_repartition_value_handles;    	/**< Handles related to the time repartition Value characteristic. */
    ble_gatts_char_handles_t      sequence_number_value_handles;    	/**< Handles related to the sequence number Value characteristic. */
	ble_gatts_char_handles_t      write_sequence_value_handles;    		/**< Handles related to the write sequence Value characteristic. */
	ble_gatts_char_handles_t      read_sequence_value_handles;    		/**< Handles related to the read sequence Value characteristic. */
    uint16_t                      conn_handle;                    		/**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection). */
    uint8_t                       uuid_type; 
};

typedef enum
{
	RED 	= 0,
	GREEN 	= 1,
	BLUE 	= 2,
	WHITE 	= 3,
} nameOfLedColor;

/**@brief Function for initializing the Custom Service.
 *
 * @param[out]  p_cus       Custom Service structure. This structure will have to be supplied by
 *                          the application. It will be initialized by this function, and will later
 *                          be used to identify this particular service instance.
 * @param[in]   p_cus_init  Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on successful initialization of service, otherwise an error code.
 */
uint32_t ble_cus_init(ble_cus_t * p_cus, const ble_cus_init_t * p_cus_init);

/**@brief Function for handling the Application's BLE Stack events.
 *
 * @details Handles all events from the BLE stack of interest to the Battery Service.
 *
 * @note 
 *
 * @param[in]   p_ble_evt  Event received from the BLE stack.
 * @param[in]   p_context  Custom Service structure.
 */
void ble_cus_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);

/**@brief Function for updating the custom value.
 *
 * @details The application calls this function when the cutom value should be updated. If
 *          notification has been enabled, the custom value characteristic is sent to the client.
 *
 * @note 
 *       
 * @param[in]   p_cus          Custom Service structure.
 * @param[in]   Custom value 
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t ble_cus_custom_value_update(ble_cus_t * p_cus, uint32_t custom_value);

/**@brief Function for updating the read sequece value.
 *
 * @details The application calls this function when the cutom value should be updated. If
 *          notification has been enabled, the custom value characteristic is sent to the client.
 *
 * @note
 *
 * @param[in]   p_cus          Custom Service structure.
 * @param[in]   Custom value
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t read_sequence_value_update(ble_cus_t * p_cus, uint8_t custom_value);


/**@brief Function to save the value of the PWM
 *
 * @details Save the new value of the PWM
 *
 * @note 
 *
 * @param[in]   u32ValueOfPWM      New value of the PWM
 */
void ble_cus_savePWMValue(uint32_t u32Value);

/**@brief Function to return the valeu of the PWM
 *
 * @details Return the PWM value
 *
 * @note 
 *
 * @return      u32PWMValue Value of the PWM, in 4 bytes
 */
uint32_t ble_cus_readPWMValue(void);

/**@brief Function to return the value of the new PWM, after a new command with current value
 *
 * @details Return the PWM value
 *
 * @note
 *
 * @return      u32PWMValue Value of the PWM, in 4 bytes
 */
uint32_t manageCurrentTable(nameOfLedColor color, unsigned int u16Value);

/**@brief Function to return the value of the PWM, after a new command with current value
 *
 * @details Return the PWM value
 *
 * @note
 *
 * @return      u08PWMValue Value of the PWM, in 1 bytes
 */
unsigned char findPWMWithCurrent(unsigned char color, unsigned int u16Value);

#endif // BLE_CUS_H__

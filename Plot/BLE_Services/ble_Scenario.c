#include "sdk_common.h"
#include "ble_scenario.h"
#include <string.h>
#include "ble_srv_common.h"
#include "nrf_gpio.h"
//#include "boards.h"
//#include "nrf_log.h"

#include "nrf_drv_uart.h"

#include "nrf_log.h"

#define NUMBER_OF_DATA_SCENARIO		20		// Nombre d'octet contenu dans le service pour les donn�es du sc�nario
uint8_t tabDataScenario[NUMBER_OF_DATA_SCENARIO];
uint8_t u08NumberOfDataScenario = 0;

static uint8_t u08ScenarioNumber = 0;

nrf_drv_uart_t m_uart_EpeverSerieAA = NRF_DRV_UART_INSTANCE(0);
unsigned char tabCharacter[100];

/**@brief Function for handling the Connect event.
 *
 * @param[in]   p_scenario       Custom Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_connect(ble_scenario_t * p_scenario, ble_evt_t const * p_ble_evt)
{
    p_scenario->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;

    ble_scenario_evt_t evt;

    evt.evt_type = BLE_SCENARIO_EVT_CONNECTED;

    p_scenario->evt_handler(p_scenario, &evt);
}

/**@brief Function for handling the Disconnect event.
 *
 * @param[in]   p_scenario       Custom Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_disconnect(ble_scenario_t * p_scenario, ble_evt_t const * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_scenario->conn_handle = BLE_CONN_HANDLE_INVALID;
    
    ble_scenario_evt_t evt;

    evt.evt_type = BLE_SCENARIO_EVT_DISCONNECTED;

    p_scenario->evt_handler(p_scenario, &evt);
}

/**@brief Function for handling the Write event.
 *
 * @param[in]   p_scenario       Custom Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_write(ble_scenario_t * p_scenario, ble_evt_t const * p_ble_evt)
{
    ble_gatts_evt_write_t const * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    // Scenario Value Characteristic Written to.
    if (p_evt_write->handle == p_scenario->scenario_value_handles.value_handle)
    {
        if (p_evt_write->len == 1)
        {
            // Gestion du num�ro de Sc�nario
            ble_scenario_evt_t evt;
            
            ble_scenario_saveScenarioNumber(p_evt_write->data[0]);
            
            // Indication de l'�criture provenant de la caract�ristique
            evt.evt_type = BLE_SCENARIO_WRITE;
            
            // Appel de la fonction de handle pour signal� l'�criture d'un nouveau sc�nario
            p_scenario->evt_handler(p_scenario, &evt);

            NRF_LOG_INFO("Write Scenario : %d\r\n", p_evt_write->data[0]);
        }
    }

    if (p_evt_write->handle == p_scenario->scenario_data_value_handles.value_handle)
	{
		//if (p_evt_write->len == 1)
		{
			// Gestion des donn�es de Sc�nario
			//ble_scenario_evt_t evt;

			ble_scenario_saveScenarioData(p_evt_write->data, p_evt_write->len);

			// Indication de l'�criture provenant de la caract�ristique
			//evt.evt_type = BLE_SCENARIO_WRITE;

			// Appel de la fonction de handle pour signal� l'�criture d'un nouveau sc�nario
			//p_scenario->evt_handler(p_scenario, &evt);
		}
	}

    // Check if the Custom value CCCD is written to and that the value is the appropriate length, i.e 2 bytes.
    /*
    if ((p_evt_write->handle == p_scenario->scenario_value_handles.cccd_handle)
        && (p_evt_write->len == 2))
    {
        // CCCD written, call application event handler
        if (p_scenario->evt_handler != NULL)
        {
            ble_scenario_evt_t evt;

            if (ble_srv_is_notification_enabled(p_evt_write->data))
            {
                evt.evt_type = BLE_SCENARIO_EVT_NOTIFICATION_ENABLED;
            }
            else
            {
                evt.evt_type = BLE_SCENARIO_EVT_NOTIFICATION_DISABLED;
            }
            // Call the application event handler.
            p_scenario->evt_handler(p_scenario, &evt);
        }
    }
	*/
}

/**@brief Function for handling the Application's BLE Stack events.
 *
 * @details Handles all events from the BLE stack of interest to the Battery Service.
 *
 * @note 
 *
 * @param[in]   p_ble_evt  Event received from the BLE stack.
 * @param[in]   p_context  Custom Service structure.
 */
void ble_scenario_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
    ble_scenario_t * p_scenario = (ble_scenario_t *) p_context;
    
    if (p_scenario == NULL || p_ble_evt == NULL)
    {
        return;
    }
    
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_scenario, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_scenario, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_scenario, p_ble_evt);
            break;
/* Handling this event is not necessary
        case BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST:
            NRF_LOG_INFO("EXCHANGE_MTU_REQUEST event received.\r\n");
            break;
*/
        default:
            // No implementation needed.
            break;
    }
}

/**@brief Function for adding the Scenario Value characteristic.
 *
 * @param[in]   p_scenario        Scenario Service structure.
 * @param[in]   p_scenario_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t scenario_value_char_add(ble_scenario_t * p_scenario, const ble_scenario_init_t * p_scenario_init)
{
    static char user_desc[] = "Numero Scenario";
	
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    // Add Custom Value characteristic
    memset(&cccd_md, 0, sizeof(cccd_md));

    //  Read  operation on cccd should be possible without authentication.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    
    cccd_md.write_perm = p_scenario_init->custom_value_char_attr_md.cccd_write_perm;
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.write  = 1;
    char_md.char_props.notify = 0; 
    char_md.p_char_user_desc  = (uint8_t *) user_desc;	// Ici, c'est la valeur de la description
    char_md.char_user_desc_size = strlen(user_desc);
    char_md.char_user_desc_max_size = strlen(user_desc);
		
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md; 
    char_md.p_sccd_md         = NULL;
		
    ble_uuid.type = p_scenario->uuid_type;
    ble_uuid.uuid = SCENARIO_VALUE_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_scenario_init->custom_value_char_attr_md.read_perm;
    attr_md.write_perm = p_scenario_init->custom_value_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = sizeof(uint8_t);
	attr_char_value.p_value = (uint8_t *)&p_scenario_init->initial_custom_value;

    err_code = sd_ble_gatts_characteristic_add(p_scenario->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_scenario->scenario_value_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

/**@brief Function for adding the Scenario Data Value characteristic.
 *
 * @param[in]   p_scenario        Scenario Service structure.
 * @param[in]   p_scenario_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t scenario_data_value_char_add(ble_scenario_t * p_scenario, const ble_scenario_init_t * p_scenario_init)
{
    static char user_desc[] = "Donnees Scenario";

    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    // Add Custom Value characteristic
    memset(&cccd_md, 0, sizeof(cccd_md));

    //  Read  operation on cccd should be possible without authentication.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.write_perm = p_scenario_init->custom_value_char_attr_md.cccd_write_perm;
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.write  = 1;
    char_md.char_props.notify = 0;
    char_md.p_char_user_desc  = (uint8_t *) user_desc;	// Ici, c'est la valeur de la description
    char_md.char_user_desc_size = strlen(user_desc);
    char_md.char_user_desc_max_size = strlen(user_desc);

    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_scenario->uuid_type;
    ble_uuid.uuid = SCENARIO_DATA_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_scenario_init->custom_value_char_attr_md.read_perm;
    attr_md.write_perm = p_scenario_init->custom_value_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint8_t) * NUMBER_OF_DATA_SCENARIO;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = sizeof(uint8_t) * NUMBER_OF_DATA_SCENARIO;
    memset(tabDataScenario, 0, sizeof(tabDataScenario));
	attr_char_value.p_value = (uint8_t *)&tabDataScenario;	//p_scenario_init->initial_custom_value;

    err_code = sd_ble_gatts_characteristic_add(p_scenario->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_scenario->scenario_data_value_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

uint32_t ble_scenario_init(ble_scenario_t * p_scenario, const ble_scenario_init_t * p_scenario_init)
{
    if (p_scenario == NULL || p_scenario_init == NULL)
    {
        return NRF_ERROR_NULL;
    }

    uint32_t   err_code;
    ble_uuid_t ble_uuid;

    // Initialize service structure
    p_scenario->evt_handler               = p_scenario_init->evt_handler;
    p_scenario->conn_handle               = BLE_CONN_HANDLE_INVALID;

    // Add Custom Service UUID
    ble_uuid128_t base_uuid = {SCENARIO_SERVICE_UUID_BASE};
    err_code =  sd_ble_uuid_vs_add(&base_uuid, &p_scenario->uuid_type);
    VERIFY_SUCCESS(err_code);
    
    ble_uuid.type = p_scenario->uuid_type;
    ble_uuid.uuid = SCENARIO_SERVICE_UUID;

    // Add the Custom Service
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_scenario->service_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Custom Value characteristic
    err_code = scenario_value_char_add(p_scenario, p_scenario_init);
	VERIFY_SUCCESS(err_code);

	err_code = scenario_data_value_char_add(p_scenario, p_scenario_init);
	VERIFY_SUCCESS(err_code);

	return NRF_SUCCESS;
}

uint32_t ble_scenario_value_update(ble_scenario_t * p_scenario, uint8_t custom_value)
{
    if (p_scenario == NULL)
    {
        return NRF_ERROR_NULL;
    }

    uint32_t err_code = NRF_SUCCESS;
    ble_gatts_value_t gatts_value;

    // Initialize value struct.
    memset(&gatts_value, 0, sizeof(gatts_value));

    gatts_value.len     = sizeof(uint8_t);
    gatts_value.offset  = 0;
    gatts_value.p_value = (uint8_t *)&custom_value;

    // Update database.
    err_code = sd_ble_gatts_value_set(p_scenario->conn_handle,
                                      p_scenario->scenario_value_handles.value_handle,
                                      &gatts_value);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Send value if connected and notifying.
    if ((p_scenario->conn_handle != BLE_CONN_HANDLE_INVALID))
    {
        ble_gatts_hvx_params_t hvx_params;

        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_scenario->scenario_value_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = gatts_value.offset;
        hvx_params.p_len  = &gatts_value.len;
        hvx_params.p_data = gatts_value.p_value;

        err_code = sd_ble_gatts_hvx(p_scenario->conn_handle, &hvx_params);
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}

void ble_scenario_saveScenarioNumber(uint8_t u08Value)
{
	u08ScenarioNumber = u08Value;
}

uint8_t ble_scenario_readScenarioNumber(void)
{
    return u08ScenarioNumber;
}

void ble_scenario_saveScenarioData(uint8_t *u08Value, uint8_t u08LenghtData)
{
	//int s16NumberOfCharacter;

	memcpy(&tabDataScenario, u08Value, u08LenghtData);
	u08NumberOfDataScenario = u08LenghtData;

	//s16NumberOfCharacter = sprintf(tabCharacter, "\r\nSave : %d %d %d %d %d = %d %d %d %d\r\n", u08LenghtData, u08Value[0], u08Value[1], u08Value[2], u08Value[3],
	//		tabDataScenario[0], tabDataScenario[1], tabDataScenario[2], tabDataScenario[3]);
	//nrf_drv_uart_tx(&m_uart_EpeverSerieAA, (uint8_t *)&tabCharacter[0], (uint8_t)s16NumberOfCharacter);
}

uint8_t ble_scenario_readScenarioData(uint8_t *tableDataScenarioValue)
{
	//int s16NumberOfCharacter;

	memcpy(tableDataScenarioValue, tabDataScenario, u08NumberOfDataScenario);

	//s16NumberOfCharacter = sprintf(tabCharacter, "\r\nRead : %d %d %d %d %d = %d %d %d %d\r\n", u08NumberOfDataScenario, tabDataScenario[0], tabDataScenario[1], tabDataScenario[2], tabDataScenario[3],
	//		tableDataScenarioValue[0], tableDataScenarioValue[1], tableDataScenarioValue[2], tableDataScenarioValue[3]);
	//nrf_drv_uart_tx(&m_uart_EpeverSerieAA, (uint8_t *)&tabCharacter[0], (uint8_t)s16NumberOfCharacter);

    return u08NumberOfDataScenario;
}

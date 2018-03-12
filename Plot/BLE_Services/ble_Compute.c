#include "sdk_common.h"
#include "ble_compute.h"
#include <string.h>
#include "ble_srv_common.h"
#include "nrf_gpio.h"
//#include "boards.h"
//#include "nrf_log.h"

#include "nrf_drv_uart.h"


unsigned char u08SynchroValue = 0;


/**@brief Function for handling the Connect event.
 *
 * @param[in]   p_compute       Custom Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_connect(ble_compute_t * p_compute, ble_evt_t const * p_ble_evt)
{
    p_compute->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;

    ble_compute_evt_t evt;

    evt.evt_type = BLE_COMPUTE_EVT_CONNECTED;

    p_compute->evt_handler(p_compute, &evt);
}

/**@brief Function for handling the Disconnect event.
 *
 * @param[in]   p_compute       Custom Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_disconnect(ble_compute_t * p_compute, ble_evt_t const * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_compute->conn_handle = BLE_CONN_HANDLE_INVALID;
    
    ble_compute_evt_t evt;

    evt.evt_type = BLE_COMPUTE_EVT_DISCONNECTED;

    p_compute->evt_handler(p_compute, &evt);
}

/**@brief Function for handling the Write event.
 *
 * @param[in]   p_compute       Custom Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_write(ble_compute_t * p_compute, ble_evt_t const * p_ble_evt)
{
    ble_gatts_evt_write_t const * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    // Scenario Value Characteristic Written to.
    if (p_evt_write->handle == p_compute->compute_synchro_value_handles.value_handle)
    {
        if (p_evt_write->len == 1)
        {
        	ble_compute_evt_t evt;

        	ble_compute_saveSynchroValue(p_evt_write->data[0]);

        	// Indication de l'écriture provenant de la caractéristique
			evt.evt_type = BLE_COMPUTE_WRITE;

			// Appel de la fonction de handle pour signalé l'écriture d'un nouveau scénario
			p_compute->evt_handler(p_compute, &evt);
        }
    }
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
void ble_compute_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
    ble_compute_t * p_compute = (ble_compute_t *) p_context;
    
    if (p_compute == NULL || p_ble_evt == NULL)
    {
        return;
    }
    
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_compute, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_compute, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_compute, p_ble_evt);
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
 * @param[in]   p_compute        Scenario Service structure.
 * @param[in]   p_compute_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t compute_synchro_value_char_add(ble_compute_t * p_compute, const ble_compute_init_t * p_compute_init)
{
    static char user_desc[] = "Compute synchro";
	
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
    
    cccd_md.write_perm = p_compute_init->custom_value_char_attr_md.cccd_write_perm;
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;	// BLE_GATTS_VLOC_USER

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
		
    ble_uuid.type = p_compute->uuid_type;
    ble_uuid.uuid = COMPUTE_SYNCHRO_VALUE_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_compute_init->custom_value_char_attr_md.read_perm;
    attr_md.write_perm = p_compute_init->custom_value_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;	// BLE_GATTS_VLOC_USER
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = sizeof(uint8_t);
	attr_char_value.p_value = (uint8_t *)&p_compute_init->initial_custom_value;

    err_code = sd_ble_gatts_characteristic_add(p_compute->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_compute->compute_synchro_value_handles);

    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

/**@brief Function for adding the PDU Value characteristic.
 *
 * @param[in]   p_compute      Compute Service structure.
 * @param[in]   p_compute_init Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t pdu_value_char_add(ble_compute_t * p_compute, const ble_compute_init_t * p_compute_init)
{
    static char user_desc_readEpever[] = "PDU Value";

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
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);    // NO_ACCESS

    cccd_md.write_perm = p_compute_init->custom_value_char_attr_md.cccd_write_perm;
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.write  = 0;
    char_md.char_props.notify = 1;
    char_md.p_char_user_desc  = (uint8_t *) user_desc_readEpever;	// Ici, c'est la valeur de la description
    char_md.char_user_desc_size = strlen(user_desc_readEpever);
    char_md.char_user_desc_max_size = strlen(user_desc_readEpever);

    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_compute->uuid_type;
    ble_uuid.uuid = COMPUTE_PDU_VALUE_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.read_perm  = p_compute_init->custom_value_char_attr_md.read_perm;
    //BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
    attr_md.write_perm = p_compute_init->custom_value_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(int16_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = sizeof(int16_t);
	attr_char_value.p_value = (uint8_t *)&p_compute_init->initial_custom_value;

    err_code = sd_ble_gatts_characteristic_add(p_compute->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_compute->compute_pdu_value_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

/**@brief Function for adding the Led Time Value characteristic.
 *
 * @param[in]   p_compute      Compute Service structure.
 * @param[in]   p_compute_init Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t timeLed_value_char_add(ble_compute_t * p_compute, const ble_compute_init_t * p_compute_init)
{
    static char user_desc_readEpever[] = "Led time Value";

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
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);    // NO_ACCESS

    cccd_md.write_perm = p_compute_init->custom_value_char_attr_md.cccd_write_perm;
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.write  = 0;
    char_md.char_props.notify = 1;
    char_md.p_char_user_desc  = (uint8_t *) user_desc_readEpever;	// Ici, c'est la valeur de la description
    char_md.char_user_desc_size = strlen(user_desc_readEpever);
    char_md.char_user_desc_max_size = strlen(user_desc_readEpever);

    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_compute->uuid_type;
    ble_uuid.uuid = COMPUTE_TIME_LED_VALUE_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.read_perm  = p_compute_init->custom_value_char_attr_md.read_perm;
    //BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
    attr_md.write_perm = p_compute_init->custom_value_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(int16_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = sizeof(int16_t);
	attr_char_value.p_value = (uint8_t *)&p_compute_init->initial_custom_value;

    err_code = sd_ble_gatts_characteristic_add(p_compute->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_compute->compute_timeled_value_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

uint32_t ble_compute_init(ble_compute_t * p_compute, const ble_compute_init_t * p_compute_init)
{
    if (p_compute == NULL || p_compute_init == NULL)
    {
        return NRF_ERROR_NULL;
    }

    uint32_t   err_code;
    ble_uuid_t ble_uuid;

    // Initialize service structure
    p_compute->evt_handler               = p_compute_init->evt_handler;
    p_compute->conn_handle               = BLE_CONN_HANDLE_INVALID;

    // Add Custom Service UUID
    ble_uuid128_t base_uuid = {COMPUTE_SERVICE_UUID_BASE};
    err_code =  sd_ble_uuid_vs_add(&base_uuid, &p_compute->uuid_type);
    VERIFY_SUCCESS(err_code);
    
    ble_uuid.type = p_compute->uuid_type;
    ble_uuid.uuid = COMPUTE_SERVICE_UUID;

    // Add the Custom Service
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_compute->service_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Custom Value characteristic
    err_code = compute_synchro_value_char_add(p_compute, p_compute_init);
	VERIFY_SUCCESS(err_code);

	err_code = pdu_value_char_add(p_compute, p_compute_init);
	VERIFY_SUCCESS(err_code);

	err_code = timeLed_value_char_add(p_compute, p_compute_init);
	VERIFY_SUCCESS(err_code);

	return NRF_SUCCESS;
}

uint32_t ble_compute_synchro_value_update(ble_compute_t * p_compute, uint8_t custom_value)
{
    if (p_compute == NULL)
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
    err_code = sd_ble_gatts_value_set(p_compute->conn_handle,
                                      p_compute->compute_synchro_value_handles.value_handle,
                                      &gatts_value);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Send value if connected and notifying.
    if ((p_compute->conn_handle != BLE_CONN_HANDLE_INVALID))
    {
        ble_gatts_hvx_params_t hvx_params;

        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_compute->compute_synchro_value_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = gatts_value.offset;
        hvx_params.p_len  = &gatts_value.len;
        hvx_params.p_data = gatts_value.p_value;

        err_code = sd_ble_gatts_hvx(p_compute->conn_handle, &hvx_params);
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}

void ble_compute_saveSynchroValue(uint8_t u08Value)
{
	u08SynchroValue = u08Value;
}

uint8_t ble_compute_readSynchroValue(void)
{
    return u08SynchroValue;
}

uint32_t ble_compute_pdu_value_update(ble_compute_t * p_compute, uint16_t custom_value)
{
    if (p_compute == NULL)
    {
        return NRF_ERROR_NULL;
    }

    uint32_t err_code = NRF_SUCCESS;
    ble_gatts_value_t gatts_value;

    // Initialize value struct.
    memset(&gatts_value, 0, sizeof(gatts_value));

    gatts_value.len     = sizeof(uint16_t);
    gatts_value.offset  = 0;
    gatts_value.p_value = (uint8_t *)&custom_value;

    // Update database.
    err_code = sd_ble_gatts_value_set(p_compute->conn_handle,
    								  p_compute->compute_pdu_value_handles.value_handle,
                                      &gatts_value);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Send value if connected and notifying.
    if ((p_compute->conn_handle != BLE_CONN_HANDLE_INVALID))
    {
        ble_gatts_hvx_params_t hvx_params;

        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_compute->compute_pdu_value_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = gatts_value.offset;
        hvx_params.p_len  = &gatts_value.len;
        hvx_params.p_data = gatts_value.p_value;

        err_code = sd_ble_gatts_hvx(p_compute->conn_handle, &hvx_params);
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}

uint32_t ble_compute_timeled_value_update(ble_compute_t * p_compute, uint16_t custom_value)
{
    if (p_compute == NULL)
    {
        return NRF_ERROR_NULL;
    }

    uint32_t err_code = NRF_SUCCESS;
    ble_gatts_value_t gatts_value;

    // Initialize value struct.
    memset(&gatts_value, 0, sizeof(gatts_value));

    gatts_value.len     = sizeof(uint16_t);
    gatts_value.offset  = 0;
    gatts_value.p_value = (uint8_t *)&custom_value;

    // Update database.
    err_code = sd_ble_gatts_value_set(p_compute->conn_handle,
    								  p_compute->compute_timeled_value_handles.value_handle,
                                      &gatts_value);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Send value if connected and notifying.
    if ((p_compute->conn_handle != BLE_CONN_HANDLE_INVALID))
    {
        ble_gatts_hvx_params_t hvx_params;

        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_compute->compute_timeled_value_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = gatts_value.offset;
        hvx_params.p_len  = &gatts_value.len;
        hvx_params.p_data = gatts_value.p_value;

        err_code = sd_ble_gatts_hvx(p_compute->conn_handle, &hvx_params);
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}

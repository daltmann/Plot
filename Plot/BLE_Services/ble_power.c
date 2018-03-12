#include "sdk_common.h"
#include "ble_power.h"
#include <string.h>
#include "ble_srv_common.h"
#include "nrf_gpio.h"
//#include "boards.h"
//#include "nrf_log.h"
#include "nrf_drv_uart.h"


static uint32_t u32PWMValue = 0;

/**@brief Function for handling the Connect event.
 *
 * @param[in]   p_power     Custom Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_connect(ble_power_t * p_power, ble_evt_t const * p_ble_evt)
{
    p_power->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;

    ble_power_evt_t evt;

    evt.evt_type = BLE_POWER_EVT_CONNECTED;

    p_power->evt_handler(p_power, &evt);
}

/**@brief Function for handling the Disconnect event.
 *
 * @param[in]   p_power     Custom Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_disconnect(ble_power_t * p_power, ble_evt_t const * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_power->conn_handle = BLE_CONN_HANDLE_INVALID;
    
    ble_power_evt_t evt;

    evt.evt_type = BLE_POWER_EVT_DISCONNECTED;

    p_power->evt_handler(p_power, &evt);
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
void ble_power_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
//    NRF_LOG_INFO("BLE event received. Event type = %d\r\n", p_ble_evt->header.evt_id); 
    ble_power_t * p_power = (ble_power_t *) p_context;
    
    if (p_power == NULL || p_ble_evt == NULL)
    {
        return;
    }
    
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_power, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_power, p_ble_evt);
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

/**@brief Function for adding the Battery Current Value characteristic.
 *
 * @param[in]   p_power      Battery Service structure.
 * @param[in]   p_power_init Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t battery_current_value_char_add(ble_power_t * p_power, const ble_power_init_t * p_power_init)
{
    static char user_desc_power[] = "Battery Current";
	
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
    
    cccd_md.write_perm = p_power_init->custom_value_char_attr_md.cccd_write_perm;
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.write  = 0;
    char_md.char_props.notify = 1; 
    char_md.p_char_user_desc  = (uint8_t *) user_desc_power;	// Ici, c'est la valeur de la description
    char_md.char_user_desc_size = strlen(user_desc_power);
    char_md.char_user_desc_max_size = strlen(user_desc_power);
		
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md; 
    char_md.p_sccd_md         = NULL;
		
    ble_uuid.type = p_power->uuid_type;
    ble_uuid.uuid = POWER_BATTERY_CURRENT_VALUE_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.read_perm  = p_power_init->custom_value_char_attr_md.read_perm;
    //BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
    attr_md.write_perm = p_power_init->custom_value_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(int32_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = sizeof(int32_t);
	attr_char_value.p_value = (uint8_t *)&p_power_init->initial_custom_value;
    
    err_code = sd_ble_gatts_characteristic_add(p_power->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_power->battery_current_value_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    return NRF_SUCCESS;
}

/**@brief Function for adding the Battery Voltage Value characteristic.
 *
 * @param[in]   p_power      Battery Service structure.
 * @param[in]   p_power_init Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t battery_voltage_value_char_add(ble_power_t * p_power, const ble_power_init_t * p_power_init)
{
    static char user_desc_power[] = "Battery Voltage";
	
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
    
    cccd_md.write_perm = p_power_init->custom_value_char_attr_md.cccd_write_perm;
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.write  = 0;
    char_md.char_props.notify = 1; 
    char_md.p_char_user_desc  = (uint8_t *) user_desc_power;	// Ici, c'est la valeur de la description
    char_md.char_user_desc_size = strlen(user_desc_power);
    char_md.char_user_desc_max_size = strlen(user_desc_power);
		
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md; 
    char_md.p_sccd_md         = NULL;
		
    ble_uuid.type = p_power->uuid_type;
    ble_uuid.uuid = POWER_BATTERY_VOLTAGE_VALUE_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.read_perm  = p_power_init->custom_value_char_attr_md.read_perm;
    //BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
    attr_md.write_perm = p_power_init->custom_value_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint16_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = sizeof(uint16_t);
	attr_char_value.p_value = (uint8_t *)&p_power_init->initial_custom_value;
    
    err_code = sd_ble_gatts_characteristic_add(p_power->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_power->battery_voltage_value_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    return NRF_SUCCESS;
}

/**@brief Function for adding the PV Current Value characteristic.
 *
 * @param[in]   p_power      Battery Service structure.
 * @param[in]   p_power_init Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t pv_current_value_char_add(ble_power_t * p_power, const ble_power_init_t * p_power_init)
{
    static char user_desc_power[] = "PV Current";
	
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
    
    cccd_md.write_perm = p_power_init->custom_value_char_attr_md.cccd_write_perm;
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.write  = 0;
    char_md.char_props.notify = 1; 
    char_md.p_char_user_desc  = (uint8_t *) user_desc_power;	// Ici, c'est la valeur de la description
    char_md.char_user_desc_size = strlen(user_desc_power);
    char_md.char_user_desc_max_size = strlen(user_desc_power);
		
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md; 
    char_md.p_sccd_md         = NULL;
		
    ble_uuid.type = p_power->uuid_type;
    ble_uuid.uuid = POWER_PV_CURRENT_VALUE_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.read_perm  = p_power_init->custom_value_char_attr_md.read_perm;
    //BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
    attr_md.write_perm = p_power_init->custom_value_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint32_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = sizeof(uint32_t);
	attr_char_value.p_value = (uint8_t *)&p_power_init->initial_custom_value;
    
    err_code = sd_ble_gatts_characteristic_add(p_power->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_power->pv_current_value_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    return NRF_SUCCESS;
}

/**@brief Function for adding the PV Voltage Value characteristic.
 *
 * @param[in]   p_power      Battery Service structure.
 * @param[in]   p_power_init Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t pv_voltage_value_char_add(ble_power_t * p_power, const ble_power_init_t * p_power_init)
{
    static char user_desc_power[] = "PV Voltage";
	
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
    
    cccd_md.write_perm = p_power_init->custom_value_char_attr_md.cccd_write_perm;
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.write  = 0;
    char_md.char_props.notify = 1; 
    char_md.p_char_user_desc  = (uint8_t *) user_desc_power;	// Ici, c'est la valeur de la description
    char_md.char_user_desc_size = strlen(user_desc_power);
    char_md.char_user_desc_max_size = strlen(user_desc_power);
		
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md; 
    char_md.p_sccd_md         = NULL;
		
    ble_uuid.type = p_power->uuid_type;
    ble_uuid.uuid = POWER_PV_VOLTAGE_VALUE_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.read_perm  = p_power_init->custom_value_char_attr_md.read_perm;
    //BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
    attr_md.write_perm = p_power_init->custom_value_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint16_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = sizeof(uint16_t);
	attr_char_value.p_value = (uint8_t *)&p_power_init->initial_custom_value;
    
    err_code = sd_ble_gatts_characteristic_add(p_power->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_power->pv_voltage_value_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    return NRF_SUCCESS;
}

/**@brief Function for adding the NTC Voltage Value characteristic.
 *
 * @param[in]   p_power      Battery Service structure.
 * @param[in]   p_power_init Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t ntc_voltage_value_char_add(ble_power_t * p_power, const ble_power_init_t * p_power_init)
{
    static char user_desc_power[] = "NTC Temperature";

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

    cccd_md.write_perm = p_power_init->custom_value_char_attr_md.cccd_write_perm;
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.write  = 0;
    char_md.char_props.notify = 1;
    char_md.p_char_user_desc  = (uint8_t *) user_desc_power;	// Ici, c'est la valeur de la description
    char_md.char_user_desc_size = strlen(user_desc_power);
    char_md.char_user_desc_max_size = strlen(user_desc_power);

    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_power->uuid_type;
    ble_uuid.uuid = POWER_NTC_VOLTAGE_VALUE_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.read_perm  = p_power_init->custom_value_char_attr_md.read_perm;
    //BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
    attr_md.write_perm = p_power_init->custom_value_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint16_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = sizeof(uint16_t);
	attr_char_value.p_value = (uint8_t *)&p_power_init->initial_custom_value;

    err_code = sd_ble_gatts_characteristic_add(p_power->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_power->ntc_voltage_value_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

/**@brief Function for adding the Battery capacity pourcentage Value characteristic.
 *
 * @param[in]   p_power      Battery Service structure.
 * @param[in]   p_power_init Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t battery_pourcentage_value_char_add(ble_power_t * p_power, const ble_power_init_t * p_power_init)
{
    static char user_desc_power[] = "Capacity Pourcentage";

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

    cccd_md.write_perm = p_power_init->custom_value_char_attr_md.cccd_write_perm;
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.write  = 0;
    char_md.char_props.notify = 1;
    char_md.p_char_user_desc  = (uint8_t *) user_desc_power;	// Ici, c'est la valeur de la description
    char_md.char_user_desc_size = strlen(user_desc_power);
    char_md.char_user_desc_max_size = strlen(user_desc_power);

    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_power->uuid_type;
    ble_uuid.uuid = POWER_BATTERY_POURCENTAGE_VALUE_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.read_perm  = p_power_init->custom_value_char_attr_md.read_perm;
    //BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
    attr_md.write_perm = p_power_init->custom_value_char_attr_md.write_perm;
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
	attr_char_value.p_value = (uint8_t *)&p_power_init->initial_custom_value;

    err_code = sd_ble_gatts_characteristic_add(p_power->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_power->battery_pourcentage_value_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

uint32_t ble_power_init(ble_power_t * p_power, const ble_power_init_t * p_power_init)
{
    if (p_power == NULL || p_power_init == NULL)
    {
        return NRF_ERROR_NULL;
    }

    uint32_t   err_code;
    ble_uuid_t ble_uuid;
    
    // Initialize service structure
    p_power->evt_handler               = p_power_init->evt_handler;
    p_power->conn_handle               = BLE_CONN_HANDLE_INVALID;

    // Add Custom Service UUID
    ble_uuid128_t base_uuid = {POWER_SERVICE_UUID_BASE};
    err_code =  sd_ble_uuid_vs_add(&base_uuid, &p_power->uuid_type);
    VERIFY_SUCCESS(err_code);
    
    ble_uuid.type = p_power->uuid_type;
    ble_uuid.uuid = POWER_SERVICE_UUID;

    // Add the Custom Service
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_power->service_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    // Add Custom Value characteristic
    err_code = pv_current_value_char_add(p_power, p_power_init);
    VERIFY_SUCCESS(err_code);
    
    err_code = pv_voltage_value_char_add(p_power, p_power_init);
    VERIFY_SUCCESS(err_code);

    err_code = battery_voltage_value_char_add(p_power, p_power_init);
    VERIFY_SUCCESS(err_code);
    
    err_code = battery_current_value_char_add(p_power, p_power_init);
    VERIFY_SUCCESS(err_code);
    
	err_code = ntc_voltage_value_char_add(p_power, p_power_init);
	VERIFY_SUCCESS(err_code);

	err_code = battery_pourcentage_value_char_add(p_power, p_power_init);
	VERIFY_SUCCESS(err_code);
    
    return NRF_SUCCESS;
}

uint32_t ble_power_battery_current_value_update(ble_power_t * p_power, int32_t u32ValueCurrent)
{
//    NRF_LOG_INFO("In ble_cus_custom_value_update. \r\n"); 
    if (p_power == NULL)
    {
        return NRF_ERROR_NULL;
    }

    uint32_t err_code = NRF_SUCCESS;
    ble_gatts_value_t gatts_value;

    // Initialize value struct.
    memset(&gatts_value, 0, sizeof(gatts_value));

    gatts_value.len     = sizeof(int32_t);
	gatts_value.offset  = 0;
	gatts_value.p_value = (uint8_t *)&u32ValueCurrent;

    // Update database.
    err_code = sd_ble_gatts_value_set(p_power->conn_handle,
                                      p_power->battery_current_value_handles.value_handle,
                                      &gatts_value);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Send value if connected and notifying.
    if ((p_power->conn_handle != BLE_CONN_HANDLE_INVALID)) 
    {
        ble_gatts_hvx_params_t hvx_params;

        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_power->battery_current_value_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = gatts_value.offset;
        hvx_params.p_len  = &gatts_value.len;
        hvx_params.p_data = gatts_value.p_value;

        err_code = sd_ble_gatts_hvx(p_power->conn_handle, &hvx_params);
//        NRF_LOG_INFO("sd_ble_gatts_hvx result: %x. \r\n", err_code); 
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
//        NRF_LOG_INFO("sd_ble_gatts_hvx result: NRF_ERROR_INVALID_STATE. \r\n"); 
    }


    return err_code;
}

uint32_t ble_power_battery_voltage_value_update(ble_power_t * p_power, uint16_t custom_value)
{
//    NRF_LOG_INFO("In ble_cus_custom_value_update. \r\n"); 
    if (p_power == NULL)
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
    err_code = sd_ble_gatts_value_set(p_power->conn_handle,
                                      p_power->battery_voltage_value_handles.value_handle,
                                      &gatts_value);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Send value if connected and notifying.
    if ((p_power->conn_handle != BLE_CONN_HANDLE_INVALID)) 
    {
        ble_gatts_hvx_params_t hvx_params;

        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_power->battery_voltage_value_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = gatts_value.offset;
        hvx_params.p_len  = &gatts_value.len;
        hvx_params.p_data = gatts_value.p_value;

        err_code = sd_ble_gatts_hvx(p_power->conn_handle, &hvx_params);
//        NRF_LOG_INFO("sd_ble_gatts_hvx result: %x. \r\n", err_code); 
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
//        NRF_LOG_INFO("sd_ble_gatts_hvx result: NRF_ERROR_INVALID_STATE. \r\n"); 
    }


    return err_code;
}

uint32_t ble_power_pv_current_value_update(ble_power_t * p_power, uint32_t u32ValueCurrent)
{
//	char tabCharacter[50];
//	char u08NumberOfCharacter = 0;
//	nrf_drv_uart_t m_uart = NRF_DRV_UART_INSTANCE(0);

//    NRF_LOG_INFO("In ble_cus_custom_value_update. \r\n");
//	nrf_drv_uart_tx(&m_uart, (uint8_t *)"PO_PV_CU\r\n", 10);
    if (p_power == NULL)
    {
//    	nrf_drv_uart_tx(&m_uart, (uint8_t *)"NOT OK PO_PV_CU\r\n", 17);
        return NRF_ERROR_NULL;
    }

    uint32_t err_code = NRF_SUCCESS;
    ble_gatts_value_t gatts_value;

    // Initialize value struct.
    memset(&gatts_value, 0, sizeof(gatts_value));

    gatts_value.len     = sizeof(uint32_t);	//sizeof(uint16_t);
    gatts_value.offset  = 0;
    gatts_value.p_value = (uint8_t *)&u32ValueCurrent;	//custom_value;

    // Update database.
    err_code = sd_ble_gatts_value_set(p_power->conn_handle,
                                      p_power->pv_current_value_handles.value_handle,
                                      &gatts_value);
    if (err_code != NRF_SUCCESS)
    {
//    	nrf_drv_uart_tx(&m_uart, (uint8_t *)"NOT OK 2 PO_PV_CU\r\n", 19);
        return err_code;
    }

    // Send value if connected and notifying.
    if ((p_power->conn_handle != BLE_CONN_HANDLE_INVALID)) 
    {
        ble_gatts_hvx_params_t hvx_params;

        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_power->pv_current_value_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = gatts_value.offset;
        hvx_params.p_len  = &gatts_value.len;
        hvx_params.p_data = gatts_value.p_value;

        err_code = sd_ble_gatts_hvx(p_power->conn_handle, &hvx_params);
//        NRF_LOG_INFO("sd_ble_gatts_hvx result: %x. \r\n", err_code); 

//        u08NumberOfCharacter = sprintf(tabCharacter, "PV_CU %d : %d\r\n", p_power->conn_handle, custom_value);
//        err_code = nrf_drv_uart_tx(&m_uart, (uint8_t *)&tabCharacter[0], u08NumberOfCharacter);
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
//        NRF_LOG_INFO("sd_ble_gatts_hvx result: NRF_ERROR_INVALID_STATE. \r\n"); 
    }


    return err_code;
}

uint32_t ble_power_pv_voltage_value_update(ble_power_t * p_power, uint16_t custom_value)
{
//    NRF_LOG_INFO("In ble_cus_custom_value_update. \r\n"); 
    if (p_power == NULL)
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
    err_code = sd_ble_gatts_value_set(p_power->conn_handle,
                                      p_power->pv_voltage_value_handles.value_handle,
                                      &gatts_value);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Send value if connected and notifying.
    if ((p_power->conn_handle != BLE_CONN_HANDLE_INVALID)) 
    {
        ble_gatts_hvx_params_t hvx_params;

        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_power->pv_voltage_value_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = gatts_value.offset;
        hvx_params.p_len  = &gatts_value.len;
        hvx_params.p_data = gatts_value.p_value;

        err_code = sd_ble_gatts_hvx(p_power->conn_handle, &hvx_params);
//        NRF_LOG_INFO("sd_ble_gatts_hvx result: %x. \r\n", err_code); 
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
//        NRF_LOG_INFO("sd_ble_gatts_hvx result: NRF_ERROR_INVALID_STATE. \r\n"); 
    }


    return err_code;
}

uint32_t ble_power_ntc_voltage_value_update(ble_power_t * p_power, uint16_t custom_value)
{
//    NRF_LOG_INFO("In ble_cus_custom_value_update. \r\n");
    if (p_power == NULL)
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
    err_code = sd_ble_gatts_value_set(p_power->conn_handle,
                                      p_power->ntc_voltage_value_handles.value_handle,
                                      &gatts_value);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Send value if connected and notifying.
    if ((p_power->conn_handle != BLE_CONN_HANDLE_INVALID))
    {
        ble_gatts_hvx_params_t hvx_params;

        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_power->ntc_voltage_value_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = gatts_value.offset;
        hvx_params.p_len  = &gatts_value.len;
        hvx_params.p_data = gatts_value.p_value;

        err_code = sd_ble_gatts_hvx(p_power->conn_handle, &hvx_params);
//        NRF_LOG_INFO("sd_ble_gatts_hvx result: %x. \r\n", err_code);
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
//        NRF_LOG_INFO("sd_ble_gatts_hvx result: NRF_ERROR_INVALID_STATE. \r\n");
    }


    return err_code;
}

uint32_t ble_battery_pourcentage_value_update(ble_power_t * p_power, uint8_t custom_value)
{
//    NRF_LOG_INFO("In ble_cus_custom_value_update. \r\n");
    if (p_power == NULL)
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
    err_code = sd_ble_gatts_value_set(p_power->conn_handle,
                                      p_power->battery_pourcentage_value_handles.value_handle,
                                      &gatts_value);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Send value if connected and notifying.
    if ((p_power->conn_handle != BLE_CONN_HANDLE_INVALID))
    {
        ble_gatts_hvx_params_t hvx_params;

        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_power->battery_pourcentage_value_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = gatts_value.offset;
        hvx_params.p_len  = &gatts_value.len;
        hvx_params.p_data = gatts_value.p_value;

        err_code = sd_ble_gatts_hvx(p_power->conn_handle, &hvx_params);
//        NRF_LOG_INFO("sd_ble_gatts_hvx result: %x. \r\n", err_code);
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
//        NRF_LOG_INFO("sd_ble_gatts_hvx result: NRF_ERROR_INVALID_STATE. \r\n");
    }


    return err_code;
}

void ble_power_savePWMValue(uint32_t u32Value)
{
    u32PWMValue = u32Value;
}

uint32_t ble_power_readPWMValue(void)
{
    return u32PWMValue;
}

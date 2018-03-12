#include "sdk_common.h"
#include "ble_readEpever.h"
#include <string.h>
#include "ble_srv_common.h"
#include "nrf_gpio.h"
//#include "boards.h"
//#include "nrf_log.h"
#include "nrf_drv_uart.h"

#include "Epever_Serie.h"

#include "nrf_delay.h"

//#include "nrf_log.h"
#include <stdio.h>


/**@brief Function for handling the Connect event.
 *
 * @param[in]   p_readEpever     Custom Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_connect(ble_readEpever_t * p_readEpever, ble_evt_t const * p_ble_evt)
{
    p_readEpever->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;

    ble_readEpever_evt_t evt;

    evt.evt_type = BLE_READEPEVER_EVT_CONNECTED;

    p_readEpever->evt_handler(p_readEpever, &evt);
}

/**@brief Function for handling the Disconnect event.
 *
 * @param[in]   p_readEpever     Custom Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_disconnect(ble_readEpever_t * p_readEpever, ble_evt_t const * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_readEpever->conn_handle = BLE_CONN_HANDLE_INVALID;
    
    ble_readEpever_evt_t evt;

    evt.evt_type = BLE_READEPEVER_EVT_DISCONNECTED;

    p_readEpever->evt_handler(p_readEpever, &evt);
}

/**@brief Function for handling the Write event.
 *
 * @param[in]   p_readEpever       Custom Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_write(ble_readEpever_t * p_readEpever, ble_evt_t const * p_ble_evt)
{
    ble_gatts_evt_write_t const * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    // Scenario Value Characteristic Written to.
    if (p_evt_write->handle == p_readEpever->generated_energy_value_handles.value_handle)
    {
        if (p_evt_write->len == 4)
        {
            // Gestion du numéro de Scénario
            ble_readEpever_evt_t evt;
            unsigned long u32Value = 0;

            u32Value = (((unsigned long)p_evt_write->data[0]) << 24) + (((unsigned long)p_evt_write->data[1]) << 16) + (((unsigned long)p_evt_write->data[2]) << 8) + ((unsigned long)p_evt_write->data[3]);
            setGeneratedEnergyValue(u32Value);

            printf("Write Gen Energy : %ld\r\n", u32Value);

            // Indication de l'écriture provenant de la caractéristique
            evt.evt_type = BLE_READEPEVER_WRITE;

            // Appel de la fonction de handle pour signalé l'écriture d'un nouveau scénario
            p_readEpever->evt_handler(p_readEpever, &evt);
        }
    }

    if (p_evt_write->handle == p_readEpever->battery_soc_value_handles.value_handle)
	{
		if (p_evt_write->len == 2)
		{
			// Gestion du numéro de Scénario
			ble_readEpever_evt_t evt;
			unsigned int u16Value = 0;

			u16Value = (((unsigned long)p_evt_write->data[0]) << 8) + ((unsigned long)p_evt_write->data[1]);
			setSOCValue(u16Value);

			printf("Write SOC : %d\r\n", u16Value);

			// Indication de l'écriture provenant de la caractéristique
			evt.evt_type = BLE_READEPEVER_WRITE;

			// Appel de la fonction de handle pour signalé l'écriture d'un nouveau scénario
			p_readEpever->evt_handler(p_readEpever, &evt);
		}
	}

    if (p_evt_write->handle == p_readEpever->pv_voltage_value_handles.value_handle)
	{
		if (p_evt_write->len == 2)
		{
			// Gestion du numéro de Scénario
			ble_readEpever_evt_t evt;
			unsigned int u16Value = 0;

			u16Value = (((unsigned long)p_evt_write->data[0]) << 8) + ((unsigned long)p_evt_write->data[1]);
			setPvVoltageMaxValue(u16Value);

			printf("Write PV Voltage Max : %d\r\n", u16Value);

			// Indication de l'écriture provenant de la caractéristique
			evt.evt_type = BLE_READEPEVER_WRITE;

			// Appel de la fonction de handle pour signalé l'écriture d'un nouveau scénario
			p_readEpever->evt_handler(p_readEpever, &evt);
		}
	}

    if (p_evt_write->handle == p_readEpever->load_voltage_value_handles.value_handle)
	{
		if (p_evt_write->len == 2)
		{
			// Gestion de la tension Load
			ble_readEpever_evt_t evt;
			unsigned int u16Value = 0;

			u16Value = (((unsigned int)p_evt_write->data[0]) << 8) + ((unsigned int)p_evt_write->data[1]);
			setLoadMaximumVoltageValue(u16Value);

			printf("Write Load Voltage Max : %d\r\n", u16Value);

			// Indication de l'écriture provenant de la caractéristique
			evt.evt_type = BLE_READEPEVER_WRITE;

			// Appel de la fonction de handle pour signalé l'écriture d'un nouveau scénario
			p_readEpever->evt_handler(p_readEpever, &evt);
		}
	}

    if (p_evt_write->handle == p_readEpever->capacity_battery_value_handles.value_handle)
	{
		if (p_evt_write->len == 2)
		{
			// Gestion de la capacité batterie
			ble_readEpever_evt_t evt;
			unsigned int u16Value = 0;

			u16Value = (((unsigned long)p_evt_write->data[0]) << 8) + ((unsigned long)p_evt_write->data[1]);
			setCapacityBatteryValue(u16Value);

			printf("Write Capacity battery Max : %d\r\n", u16Value);

			// Indication de l'écriture provenant de la caractéristique
			evt.evt_type = BLE_READEPEVER_WRITE;

			// Appel de la fonction de handle pour signalé l'écriture d'un nouveau scénario
			p_readEpever->evt_handler(p_readEpever, &evt);
		}
	}

    if (p_evt_write->handle == p_readEpever->real_time_clock_value_handles.value_handle)
	{
		if (p_evt_write->len == 8)
		{
			// Gestion de l'horloge de l'EPEVER
			ble_readEpever_evt_t evt;
			unsigned long long u64Value = 0;

			u64Value = (((unsigned long long)p_evt_write->data[0]) << 56) + (((unsigned long long)p_evt_write->data[1]) << 48)
					+  (((unsigned long long)p_evt_write->data[2]) << 40) + (((unsigned long long)p_evt_write->data[3]) << 32)
					+  (((unsigned long long)p_evt_write->data[4]) << 24) + (((unsigned long long)p_evt_write->data[5]) << 16)
					+  (((unsigned long long)p_evt_write->data[6]) << 8)  +  ((unsigned long long)p_evt_write->data[7]);
			setRealTimeClock(u64Value);

			activateReceive_rs485();
			printf("Write Real Time Clock : 0x%04x %04x %04x %04x\r\n", (unsigned int)((u64Value & 0xFFFF000000000000) >> 48), (unsigned int)((u64Value & 0x0000FFFF00000000) >> 32), (unsigned int)((u64Value & 0x00000000FFFF0000) >> 16), (unsigned int)(u64Value & 0x000000000000FFFF));
			nrf_delay_ms(50);

			// Indication de l'écriture provenant de la caractéristique
			evt.evt_type = BLE_READEPEVER_WRITE_CLOCK;

			// Appel de la fonction de handle pour signalé l'écriture d'un nouveau scénario
			p_readEpever->evt_handler(p_readEpever, &evt);
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
void ble_readEpever_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
//    NRF_LOG_INFO("BLE event received. Event type = %d\r\n", p_ble_evt->header.evt_id); 
    ble_readEpever_t * p_readEpever = (ble_readEpever_t *) p_context;
    
    if (p_readEpever == NULL || p_ble_evt == NULL)
    {
        return;
    }
    
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_readEpever, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_readEpever, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
			on_write(p_readEpever, p_ble_evt);
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
 * @param[in]   p_readEpever      Battery Service structure.
 * @param[in]   p_readEpever_init Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t battery_current_value_char_add(ble_readEpever_t * p_readEpever, const ble_readEpever_init_t * p_readEpever_init)
{
    static char user_desc_readEpever[] = "Battery Current";
	
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
    
    cccd_md.write_perm = p_readEpever_init->custom_value_char_attr_md.cccd_write_perm;
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
		
    ble_uuid.type = p_readEpever->uuid_type;
    ble_uuid.uuid = READEPEVER_BATTERY_CURRENT_VALUE_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.read_perm  = p_readEpever_init->custom_value_char_attr_md.read_perm;
    //BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
    attr_md.write_perm = p_readEpever_init->custom_value_char_attr_md.write_perm;
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
	attr_char_value.p_value = (uint8_t *)&p_readEpever_init->initial_custom_value;
    
    err_code = sd_ble_gatts_characteristic_add(p_readEpever->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_readEpever->battery_current_value_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    return NRF_SUCCESS;
}

/**@brief Function for adding the Battery Voltage Value characteristic.
 *
 * @param[in]   p_readEpever      Battery Service structure.
 * @param[in]   p_readEpever_init Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t battery_voltage_value_char_add(ble_readEpever_t * p_readEpever, const ble_readEpever_init_t * p_readEpever_init)
{
    static char user_desc_readEpever[] = "Battery Voltage";
	
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
    
    cccd_md.write_perm = p_readEpever_init->custom_value_char_attr_md.cccd_write_perm;
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
		
    ble_uuid.type = p_readEpever->uuid_type;
    ble_uuid.uuid = READEPEVER_BATTERY_VOLTAGE_VALUE_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.read_perm  = p_readEpever_init->custom_value_char_attr_md.read_perm;
    //BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
    attr_md.write_perm = p_readEpever_init->custom_value_char_attr_md.write_perm;
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
	attr_char_value.p_value = (uint8_t *)&p_readEpever_init->initial_custom_value;
    
    err_code = sd_ble_gatts_characteristic_add(p_readEpever->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_readEpever->battery_voltage_value_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    return NRF_SUCCESS;
}

/**@brief Function for adding the PV Current Value characteristic.
 *
 * @param[in]   p_readEpever      Battery Service structure.
 * @param[in]   p_readEpever_init Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t pv_current_value_char_add(ble_readEpever_t * p_readEpever, const ble_readEpever_init_t * p_readEpever_init)
{
    static char user_desc_readEpever[] = "PV Current";
	
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
    
    cccd_md.write_perm = p_readEpever_init->custom_value_char_attr_md.cccd_write_perm;
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
		
    ble_uuid.type = p_readEpever->uuid_type;
    ble_uuid.uuid = READEPEVER_PV_CURRENT_VALUE_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.read_perm  = p_readEpever_init->custom_value_char_attr_md.read_perm;
    //BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
    attr_md.write_perm = p_readEpever_init->custom_value_char_attr_md.write_perm;
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
	attr_char_value.p_value = (uint8_t *)&p_readEpever_init->initial_custom_value;
    
    err_code = sd_ble_gatts_characteristic_add(p_readEpever->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_readEpever->pv_current_value_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    return NRF_SUCCESS;
}

/**@brief Function for adding the PV Voltage Value characteristic.
 *
 * @param[in]   p_readEpever      Battery Service structure.
 * @param[in]   p_readEpever_init Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t pv_voltage_value_char_add(ble_readEpever_t * p_readEpever, const ble_readEpever_init_t * p_readEpever_init)
{
    static char user_desc_readEpever[] = "PV Voltage";
	
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
    
    cccd_md.write_perm = p_readEpever_init->custom_value_char_attr_md.cccd_write_perm;
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.write  = 1;	// TODO : remettre : 0;
    char_md.char_props.notify = 1; 
    char_md.p_char_user_desc  = (uint8_t *) user_desc_readEpever;	// Ici, c'est la valeur de la description
    char_md.char_user_desc_size = strlen(user_desc_readEpever);
    char_md.char_user_desc_max_size = strlen(user_desc_readEpever);
		
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md; 
    char_md.p_sccd_md         = NULL;
		
    ble_uuid.type = p_readEpever->uuid_type;
    ble_uuid.uuid = READEPEVER_PV_VOLTAGE_VALUE_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.read_perm  = p_readEpever_init->custom_value_char_attr_md.read_perm;
    //BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
    attr_md.write_perm = p_readEpever_init->custom_value_char_attr_md.write_perm;
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
	attr_char_value.p_value = (uint8_t *)&p_readEpever_init->initial_custom_value;

    err_code = sd_ble_gatts_characteristic_add(p_readEpever->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_readEpever->pv_voltage_value_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

/**@brief Function for adding the Generated Energy Value characteristic.
 *
 * @param[in]   p_readEpever      Read Epever Service structure.
 * @param[in]   p_readEpever_init Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t generated_energy_value_char_add(ble_readEpever_t * p_readEpever, const ble_readEpever_init_t * p_readEpever_init)
{
    static char user_desc_readEpever[] = "Generated Energy";

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

    cccd_md.write_perm = p_readEpever_init->custom_value_char_attr_md.cccd_write_perm;
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.write  = 1;	// TODO : repasser à 0
    char_md.char_props.notify = 1;
    char_md.p_char_user_desc  = (uint8_t *) user_desc_readEpever;	// Ici, c'est la valeur de la description
    char_md.char_user_desc_size = strlen(user_desc_readEpever);
    char_md.char_user_desc_max_size = strlen(user_desc_readEpever);

    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_readEpever->uuid_type;
    ble_uuid.uuid = READEPEVER_GENERATED_ENERGY_VALUE_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.read_perm  = p_readEpever_init->custom_value_char_attr_md.read_perm;
    //BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
    attr_md.write_perm = p_readEpever_init->custom_value_char_attr_md.write_perm;
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
	attr_char_value.p_value = (uint8_t *)&p_readEpever_init->initial_custom_value;

    err_code = sd_ble_gatts_characteristic_add(p_readEpever->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_readEpever->generated_energy_value_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

/**@brief Function for adding the Battery SOC Value characteristic.
 *
 * @param[in]   p_readEpever      Read Epever Service structure.
 * @param[in]   p_readEpever_init Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t battery_soc_value_char_add(ble_readEpever_t * p_readEpever, const ble_readEpever_init_t * p_readEpever_init)
{
    static char user_desc_readEpever[] = "Battery SOC";

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

    cccd_md.write_perm = p_readEpever_init->custom_value_char_attr_md.cccd_write_perm;
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.write  = 1;	// TODO remettre : 0;
    char_md.char_props.notify = 1;
    char_md.p_char_user_desc  = (uint8_t *) user_desc_readEpever;	// Ici, c'est la valeur de la description
    char_md.char_user_desc_size = strlen(user_desc_readEpever);
    char_md.char_user_desc_max_size = strlen(user_desc_readEpever);

    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_readEpever->uuid_type;
    ble_uuid.uuid = READEPEVER_BATTERY_SOC_VALUE_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.read_perm  = p_readEpever_init->custom_value_char_attr_md.read_perm;
    //BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
    attr_md.write_perm = p_readEpever_init->custom_value_char_attr_md.write_perm;
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
	attr_char_value.p_value = (uint8_t *)&p_readEpever_init->initial_custom_value;

    err_code = sd_ble_gatts_characteristic_add(p_readEpever->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_readEpever->battery_soc_value_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

/**@brief Function for adding the Battery Status Value characteristic.
 *
 * @param[in]   p_readEpever      Read Epever Service structure.
 * @param[in]   p_readEpever_init Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t battery_status_value_char_add(ble_readEpever_t * p_readEpever, const ble_readEpever_init_t * p_readEpever_init)
{
    static char user_desc_readEpever[] = "Battery Status";

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

    cccd_md.write_perm = p_readEpever_init->custom_value_char_attr_md.cccd_write_perm;
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

    ble_uuid.type = p_readEpever->uuid_type;
    ble_uuid.uuid = READEPEVER_BATTERY_STATUS_VALUE_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.read_perm  = p_readEpever_init->custom_value_char_attr_md.read_perm;
    //BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
    attr_md.write_perm = p_readEpever_init->custom_value_char_attr_md.write_perm;
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
	attr_char_value.p_value = (uint8_t *)&p_readEpever_init->initial_custom_value;

    err_code = sd_ble_gatts_characteristic_add(p_readEpever->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_readEpever->battery_status_value_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

/**@brief Function for adding the Load Current Value characteristic.
 *
 * @param[in]   p_readEpever      Read Epever Service structure.
 * @param[in]   p_readEpever_init Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t load_current_value_char_add(ble_readEpever_t * p_readEpever, const ble_readEpever_init_t * p_readEpever_init)
{
    static char user_desc_readEpever[] = "Load Current";

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

    cccd_md.write_perm = p_readEpever_init->custom_value_char_attr_md.cccd_write_perm;
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

    ble_uuid.type = p_readEpever->uuid_type;
    ble_uuid.uuid = READEPEVER_LOAD_CURRENT_VALUE_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.read_perm  = p_readEpever_init->custom_value_char_attr_md.read_perm;
    //BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
    attr_md.write_perm = p_readEpever_init->custom_value_char_attr_md.write_perm;
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
	attr_char_value.p_value = (uint8_t *)&p_readEpever_init->initial_custom_value;

    err_code = sd_ble_gatts_characteristic_add(p_readEpever->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_readEpever->load_current_value_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

/**@brief Function for adding the Load Voltage Value characteristic.
 *
 * @param[in]   p_readEpever      Read Epever Service structure.
 * @param[in]   p_readEpever_init Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t load_voltage_value_char_add(ble_readEpever_t * p_readEpever, const ble_readEpever_init_t * p_readEpever_init)
{
    static char user_desc_readEpever[] = "Load Voltage";

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

    cccd_md.write_perm = p_readEpever_init->custom_value_char_attr_md.cccd_write_perm;
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.write  = 1;
    char_md.char_props.notify = 1;
    char_md.p_char_user_desc  = (uint8_t *) user_desc_readEpever;	// Ici, c'est la valeur de la description
    char_md.char_user_desc_size = strlen(user_desc_readEpever);
    char_md.char_user_desc_max_size = strlen(user_desc_readEpever);

    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_readEpever->uuid_type;
    ble_uuid.uuid = READEPEVER_LOAD_VOLTAGE_VALUE_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.read_perm  = p_readEpever_init->custom_value_char_attr_md.read_perm;
    //BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
    attr_md.write_perm = p_readEpever_init->custom_value_char_attr_md.write_perm;
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
	attr_char_value.p_value = (uint8_t *)&p_readEpever_init->initial_custom_value;
    
    err_code = sd_ble_gatts_characteristic_add(p_readEpever->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_readEpever->load_voltage_value_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    return NRF_SUCCESS;
}

/**@brief Function for adding the Consumed Energy Value characteristic.
 *
 * @param[in]   p_readEpever      Read Epever Service structure.
 * @param[in]   p_readEpever_init Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t consumed_energy_value_char_add(ble_readEpever_t * p_readEpever, const ble_readEpever_init_t * p_readEpever_init)
{
    static char user_desc_readEpever[] = "Consumed Energy";

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

    cccd_md.write_perm = p_readEpever_init->custom_value_char_attr_md.cccd_write_perm;
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

    ble_uuid.type = p_readEpever->uuid_type;
    ble_uuid.uuid = READEPEVER_CONSUMED_ENERGY_TODAY_VALUE_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.read_perm  = p_readEpever_init->custom_value_char_attr_md.read_perm;
    //BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
    attr_md.write_perm = p_readEpever_init->custom_value_char_attr_md.write_perm;
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
	attr_char_value.p_value = (uint8_t *)&p_readEpever_init->initial_custom_value;

    err_code = sd_ble_gatts_characteristic_add(p_readEpever->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_readEpever->consumed_energy_value_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

/**@brief Function for adding the Battery capacity Value characteristic.
 *
 * @param[in]   p_readEpever      Read Epever Service structure.
 * @param[in]   p_readEpever_init Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t battery_capacity_value_char_add(ble_readEpever_t * p_readEpever, const ble_readEpever_init_t * p_readEpever_init)
{
    static char user_desc_readEpever[] = "Battery capacity";

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

    cccd_md.write_perm = p_readEpever_init->custom_value_char_attr_md.cccd_write_perm;
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.write  = 1;
    char_md.char_props.notify = 1;
    char_md.p_char_user_desc  = (uint8_t *) user_desc_readEpever;	// Ici, c'est la valeur de la description
    char_md.char_user_desc_size = strlen(user_desc_readEpever);
    char_md.char_user_desc_max_size = strlen(user_desc_readEpever);

    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_readEpever->uuid_type;
    ble_uuid.uuid = READEPEVER_CAPACITY_BATTERY_VALUE_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.read_perm  = p_readEpever_init->custom_value_char_attr_md.read_perm;
    //BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
    attr_md.write_perm = p_readEpever_init->custom_value_char_attr_md.write_perm;
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
	attr_char_value.p_value = (uint8_t *)&p_readEpever_init->initial_custom_value;

    err_code = sd_ble_gatts_characteristic_add(p_readEpever->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_readEpever->capacity_battery_value_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

 /**@brief Function for adding the Real Time Clock Value characteristic.
  *
  * @param[in]   p_readEpever      Read Epever Service structure.
  * @param[in]   p_readEpever_init Information needed to initialize the service.
  *
  * @return      NRF_SUCCESS on success, otherwise an error code.
  */
 static uint32_t real_time_clock_value_char_add(ble_readEpever_t * p_readEpever, const ble_readEpever_init_t * p_readEpever_init)
 {
     static char user_desc_readEpever[] = "Real Time Clock ";

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

     cccd_md.write_perm = p_readEpever_init->custom_value_char_attr_md.cccd_write_perm;
     cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

     memset(&char_md, 0, sizeof(char_md));

     char_md.char_props.read   = 1;
     char_md.char_props.write  = 1;
     char_md.char_props.notify = 1;
     char_md.p_char_user_desc  = (uint8_t *) user_desc_readEpever;	// Ici, c'est la valeur de la description
     char_md.char_user_desc_size = strlen(user_desc_readEpever);
     char_md.char_user_desc_max_size = strlen(user_desc_readEpever);

     char_md.p_char_pf         = NULL;
     char_md.p_user_desc_md    = NULL;
     char_md.p_cccd_md         = &cccd_md;
     char_md.p_sccd_md         = NULL;

     ble_uuid.type = p_readEpever->uuid_type;
     ble_uuid.uuid = READEPEVER_REAL_TIME_CLOCK_VALUE_CHAR_UUID;

     memset(&attr_md, 0, sizeof(attr_md));
     attr_md.read_perm  = p_readEpever_init->custom_value_char_attr_md.read_perm;
     //BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
     attr_md.write_perm = p_readEpever_init->custom_value_char_attr_md.write_perm;
     attr_md.vloc       = BLE_GATTS_VLOC_STACK;
     attr_md.rd_auth    = 0;
     attr_md.wr_auth    = 0;
     attr_md.vlen       = 0;

     memset(&attr_char_value, 0, sizeof(attr_char_value));

     attr_char_value.p_uuid    = &ble_uuid;
     attr_char_value.p_attr_md = &attr_md;
     attr_char_value.init_len  = sizeof(unsigned long long);
     attr_char_value.init_offs = 0;
     attr_char_value.max_len   = sizeof(unsigned long long);
 	attr_char_value.p_value = (uint8_t *)&p_readEpever_init->initial_custom_value;

     err_code = sd_ble_gatts_characteristic_add(p_readEpever->service_handle, &char_md,
                                                &attr_char_value,
                                                &p_readEpever->real_time_clock_value_handles);
     if (err_code != NRF_SUCCESS)
     {
         return err_code;
     }

     return NRF_SUCCESS;
}

uint32_t ble_readEpever_init(ble_readEpever_t * p_readEpever, const ble_readEpever_init_t * p_readEpever_init)
{
    if (p_readEpever == NULL || p_readEpever_init == NULL)
    {
        return NRF_ERROR_NULL;
    }

    uint32_t   err_code;
    ble_uuid_t ble_uuid;
    
    // Initialize service structure
    p_readEpever->evt_handler               = p_readEpever_init->evt_handler;
    p_readEpever->conn_handle               = BLE_CONN_HANDLE_INVALID;

    // Add Custom Service UUID
    ble_uuid128_t base_uuid = {READEPEVER_SERVICE_UUID_BASE};
    err_code =  sd_ble_uuid_vs_add(&base_uuid, &p_readEpever->uuid_type);
    VERIFY_SUCCESS(err_code);
    
    ble_uuid.type = p_readEpever->uuid_type;
    ble_uuid.uuid = READEPEVER_SERVICE_UUID;

    // Add the Custom Service
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_readEpever->service_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    // Add Custom Value characteristic
    err_code = pv_current_value_char_add(p_readEpever, p_readEpever_init);
    VERIFY_SUCCESS(err_code);
    
    err_code = pv_voltage_value_char_add(p_readEpever, p_readEpever_init);
    VERIFY_SUCCESS(err_code);

    err_code = battery_voltage_value_char_add(p_readEpever, p_readEpever_init);
    VERIFY_SUCCESS(err_code);
    
    err_code = battery_current_value_char_add(p_readEpever, p_readEpever_init);
    VERIFY_SUCCESS(err_code);
    
    err_code = generated_energy_value_char_add(p_readEpever, p_readEpever_init);
    VERIFY_SUCCESS(err_code);

    err_code = battery_soc_value_char_add(p_readEpever, p_readEpever_init);
    VERIFY_SUCCESS(err_code);

    err_code = battery_status_value_char_add(p_readEpever, p_readEpever_init);
	VERIFY_SUCCESS(err_code);

	err_code = load_current_value_char_add(p_readEpever, p_readEpever_init);
	VERIFY_SUCCESS(err_code);

	err_code = load_voltage_value_char_add(p_readEpever, p_readEpever_init);
	VERIFY_SUCCESS(err_code);

	err_code = consumed_energy_value_char_add(p_readEpever, p_readEpever_init);
	VERIFY_SUCCESS(err_code);

	err_code = battery_capacity_value_char_add(p_readEpever, p_readEpever_init);
	VERIFY_SUCCESS(err_code);

	err_code = real_time_clock_value_char_add(p_readEpever, p_readEpever_init);
	VERIFY_SUCCESS(err_code);

    return NRF_SUCCESS;
}

uint32_t ble_readEpever_battery_current_value_update(ble_readEpever_t * p_readEpever, int32_t custom_value)
{
    if (p_readEpever == NULL)
    {
        return NRF_ERROR_NULL;
    }

    uint32_t err_code = NRF_SUCCESS;
    ble_gatts_value_t gatts_value;

    // Initialize value struct.
    memset(&gatts_value, 0, sizeof(gatts_value));

    gatts_value.len     = sizeof(int32_t);
	gatts_value.offset  = 0;
	gatts_value.p_value = (uint8_t *)&custom_value;

    // Update database.
    err_code = sd_ble_gatts_value_set(p_readEpever->conn_handle,
                                      p_readEpever->battery_current_value_handles.value_handle,
                                      &gatts_value);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Send value if connected and notifying.
    if ((p_readEpever->conn_handle != BLE_CONN_HANDLE_INVALID))
    {
        ble_gatts_hvx_params_t hvx_params;

        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_readEpever->battery_current_value_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = gatts_value.offset;
        hvx_params.p_len  = &gatts_value.len;
        hvx_params.p_data = gatts_value.p_value;

        err_code = sd_ble_gatts_hvx(p_readEpever->conn_handle, &hvx_params);
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}

uint32_t ble_readEpever_battery_voltage_value_update(ble_readEpever_t * p_readEpever, uint16_t custom_value)
{
    if (p_readEpever == NULL)
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
    err_code = sd_ble_gatts_value_set(p_readEpever->conn_handle,
                                      p_readEpever->battery_voltage_value_handles.value_handle,
                                      &gatts_value);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Send value if connected and notifying.
    if ((p_readEpever->conn_handle != BLE_CONN_HANDLE_INVALID))
    {
        ble_gatts_hvx_params_t hvx_params;

        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_readEpever->battery_voltage_value_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = gatts_value.offset;
        hvx_params.p_len  = &gatts_value.len;
        hvx_params.p_data = gatts_value.p_value;

        err_code = sd_ble_gatts_hvx(p_readEpever->conn_handle, &hvx_params);
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}

uint32_t ble_readEpever_pv_current_value_update(ble_readEpever_t * p_readEpever, uint16_t custom_value)
{
    if (p_readEpever == NULL)
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
    err_code = sd_ble_gatts_value_set(p_readEpever->conn_handle,
                                      p_readEpever->pv_current_value_handles.value_handle,
                                      &gatts_value);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Send value if connected and notifying.
    if ((p_readEpever->conn_handle != BLE_CONN_HANDLE_INVALID))
    {
        ble_gatts_hvx_params_t hvx_params;

        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_readEpever->pv_current_value_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = gatts_value.offset;
        hvx_params.p_len  = &gatts_value.len;
        hvx_params.p_data = gatts_value.p_value;

        err_code = sd_ble_gatts_hvx(p_readEpever->conn_handle, &hvx_params);
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}

uint32_t ble_readEpever_pv_voltage_value_update(ble_readEpever_t * p_readEpever, uint16_t custom_value)
{
    if (p_readEpever == NULL)
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
    err_code = sd_ble_gatts_value_set(p_readEpever->conn_handle,
                                      p_readEpever->pv_voltage_value_handles.value_handle,
                                      &gatts_value);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Send value if connected and notifying.
    if ((p_readEpever->conn_handle != BLE_CONN_HANDLE_INVALID))
    {
        ble_gatts_hvx_params_t hvx_params;

        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_readEpever->pv_voltage_value_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = gatts_value.offset;
        hvx_params.p_len  = &gatts_value.len;
        hvx_params.p_data = gatts_value.p_value;

        err_code = sd_ble_gatts_hvx(p_readEpever->conn_handle, &hvx_params);
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}

uint32_t ble_readEpever_generated_energy_value_update(ble_readEpever_t * p_readEpever, uint32_t custom_value)
{
	if (p_readEpever == NULL)
	{
		return NRF_ERROR_NULL;
	}

	uint32_t err_code = NRF_SUCCESS;
	ble_gatts_value_t gatts_value;

	// Initialize value struct.
	memset(&gatts_value, 0, sizeof(gatts_value));

	gatts_value.len     = sizeof(uint32_t);
	gatts_value.offset  = 0;
	gatts_value.p_value = (uint8_t *)&custom_value;

	// Update database.
	err_code = sd_ble_gatts_value_set(p_readEpever->conn_handle,
									  p_readEpever->generated_energy_value_handles.value_handle,
									  &gatts_value);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}

	// Send value if connected and notifying.
	if ((p_readEpever->conn_handle != BLE_CONN_HANDLE_INVALID))
	{
		ble_gatts_hvx_params_t hvx_params;

		memset(&hvx_params, 0, sizeof(hvx_params));

		hvx_params.handle = p_readEpever->generated_energy_value_handles.value_handle;
		hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
		hvx_params.offset = gatts_value.offset;
		hvx_params.p_len  = &gatts_value.len;
		hvx_params.p_data = gatts_value.p_value;

		err_code = sd_ble_gatts_hvx(p_readEpever->conn_handle, &hvx_params);
	}
	else
	{
		err_code = NRF_ERROR_INVALID_STATE;
	}

	return err_code;
}

uint32_t ble_readEpever_consumed_energy_value_update(ble_readEpever_t * p_readEpever, uint32_t custom_value)
{
	if (p_readEpever == NULL)
	{
		return NRF_ERROR_NULL;
	}

	uint32_t err_code = NRF_SUCCESS;
	ble_gatts_value_t gatts_value;

	// Initialize value struct.
	memset(&gatts_value, 0, sizeof(gatts_value));

	gatts_value.len     = sizeof(uint32_t);
	gatts_value.offset  = 0;
	gatts_value.p_value = (uint8_t *)&custom_value;

	// Update database.
	err_code = sd_ble_gatts_value_set(p_readEpever->conn_handle,
									  p_readEpever->consumed_energy_value_handles.value_handle,
									  &gatts_value);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}

	// Send value if connected and notifying.
	if ((p_readEpever->conn_handle != BLE_CONN_HANDLE_INVALID))
	{
		ble_gatts_hvx_params_t hvx_params;

		memset(&hvx_params, 0, sizeof(hvx_params));

		hvx_params.handle = p_readEpever->consumed_energy_value_handles.value_handle;
		hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
		hvx_params.offset = gatts_value.offset;
		hvx_params.p_len  = &gatts_value.len;
		hvx_params.p_data = gatts_value.p_value;

		err_code = sd_ble_gatts_hvx(p_readEpever->conn_handle, &hvx_params);
	}
	else
	{
		err_code = NRF_ERROR_INVALID_STATE;
	}

	return err_code;
}

uint32_t ble_readEpever_battery_soc_value_update(ble_readEpever_t * p_readEpever, uint16_t custom_value)
{
	if (p_readEpever == NULL)
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
	err_code = sd_ble_gatts_value_set(p_readEpever->conn_handle,
									  p_readEpever->battery_soc_value_handles.value_handle,
									  &gatts_value);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}

	// Send value if connected and notifying.
	if ((p_readEpever->conn_handle != BLE_CONN_HANDLE_INVALID))
	{
		ble_gatts_hvx_params_t hvx_params;

		memset(&hvx_params, 0, sizeof(hvx_params));

		hvx_params.handle = p_readEpever->battery_soc_value_handles.value_handle;
		hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
		hvx_params.offset = gatts_value.offset;
		hvx_params.p_len  = &gatts_value.len;
		hvx_params.p_data = gatts_value.p_value;

		err_code = sd_ble_gatts_hvx(p_readEpever->conn_handle, &hvx_params);
	}
	else
	{
		err_code = NRF_ERROR_INVALID_STATE;
	}

	return err_code;
}

uint32_t ble_readEpever_battery_status_value_update(ble_readEpever_t * p_readEpever, uint16_t custom_value)
{
	if (p_readEpever == NULL)
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
	err_code = sd_ble_gatts_value_set(p_readEpever->conn_handle,
									  p_readEpever->battery_status_value_handles.value_handle,
									  &gatts_value);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}

	// Send value if connected and notifying.
	if ((p_readEpever->conn_handle != BLE_CONN_HANDLE_INVALID))
	{
		ble_gatts_hvx_params_t hvx_params;

		memset(&hvx_params, 0, sizeof(hvx_params));

		hvx_params.handle = p_readEpever->battery_status_value_handles.value_handle;
		hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
		hvx_params.offset = gatts_value.offset;
		hvx_params.p_len  = &gatts_value.len;
		hvx_params.p_data = gatts_value.p_value;

		err_code = sd_ble_gatts_hvx(p_readEpever->conn_handle, &hvx_params);
	}
	else
	{
		err_code = NRF_ERROR_INVALID_STATE;
	}

	return err_code;
}

uint32_t ble_readEpever_load_current_value_update(ble_readEpever_t * p_readEpever, uint16_t custom_value)
{
	if (p_readEpever == NULL)
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
	err_code = sd_ble_gatts_value_set(p_readEpever->conn_handle,
									  p_readEpever->load_current_value_handles.value_handle,
									  &gatts_value);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}

	// Send value if connected and notifying.
	if ((p_readEpever->conn_handle != BLE_CONN_HANDLE_INVALID))
	{
		ble_gatts_hvx_params_t hvx_params;

		memset(&hvx_params, 0, sizeof(hvx_params));

		hvx_params.handle = p_readEpever->load_current_value_handles.value_handle;
		hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
		hvx_params.offset = gatts_value.offset;
		hvx_params.p_len  = &gatts_value.len;
		hvx_params.p_data = gatts_value.p_value;

		err_code = sd_ble_gatts_hvx(p_readEpever->conn_handle, &hvx_params);
	}
	else
	{
		err_code = NRF_ERROR_INVALID_STATE;
	}

	return err_code;
}

uint32_t ble_readEpever_load_voltage_value_update(ble_readEpever_t * p_readEpever, uint16_t custom_value)
{
	if (p_readEpever == NULL)
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
	err_code = sd_ble_gatts_value_set(p_readEpever->conn_handle,
									  p_readEpever->load_voltage_value_handles.value_handle,
									  &gatts_value);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}

	// Send value if connected and notifying.
	if ((p_readEpever->conn_handle != BLE_CONN_HANDLE_INVALID))
	{
		ble_gatts_hvx_params_t hvx_params;

		memset(&hvx_params, 0, sizeof(hvx_params));

		hvx_params.handle = p_readEpever->load_voltage_value_handles.value_handle;
		hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
		hvx_params.offset = gatts_value.offset;
		hvx_params.p_len  = &gatts_value.len;
		hvx_params.p_data = gatts_value.p_value;

		err_code = sd_ble_gatts_hvx(p_readEpever->conn_handle, &hvx_params);
	}
	else
	{
		err_code = NRF_ERROR_INVALID_STATE;
	}

	return err_code;
}

uint32_t ble_readEpever_capacity_battey_value_update(ble_readEpever_t * p_readEpever, uint16_t custom_value)
{
	if (p_readEpever == NULL)
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
	err_code = sd_ble_gatts_value_set(p_readEpever->conn_handle,
									  p_readEpever->capacity_battery_value_handles.value_handle,
									  &gatts_value);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}

	// Send value if connected and notifying.
	if ((p_readEpever->conn_handle != BLE_CONN_HANDLE_INVALID))
	{
		ble_gatts_hvx_params_t hvx_params;

		memset(&hvx_params, 0, sizeof(hvx_params));

		hvx_params.handle = p_readEpever->capacity_battery_value_handles.value_handle;
		hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
		hvx_params.offset = gatts_value.offset;
		hvx_params.p_len  = &gatts_value.len;
		hvx_params.p_data = gatts_value.p_value;

		err_code = sd_ble_gatts_hvx(p_readEpever->conn_handle, &hvx_params);
	}
	else
	{
		err_code = NRF_ERROR_INVALID_STATE;
	}

	return err_code;
}


uint32_t ble_readEpever_real_time_clock_value_update(ble_readEpever_t * p_readEpever, unsigned long long custom_value)
{
	if (p_readEpever == NULL)
	{
		return NRF_ERROR_NULL;
	}

	uint32_t err_code = NRF_SUCCESS;
	ble_gatts_value_t gatts_value;

	// Initialize value struct.
	memset(&gatts_value, 0, sizeof(gatts_value));

	gatts_value.len     = sizeof(unsigned long long);
	gatts_value.offset  = 0;
	gatts_value.p_value = (uint8_t *)&custom_value;

	// Update database.
	err_code = sd_ble_gatts_value_set(p_readEpever->conn_handle,
									  p_readEpever->real_time_clock_value_handles.value_handle,
									  &gatts_value);
	if (err_code != NRF_SUCCESS)
	{
		return err_code;
	}

	// Send value if connected and notifying.
	if ((p_readEpever->conn_handle != BLE_CONN_HANDLE_INVALID))
	{
		ble_gatts_hvx_params_t hvx_params;

		memset(&hvx_params, 0, sizeof(hvx_params));

		hvx_params.handle = p_readEpever->real_time_clock_value_handles.value_handle;
		hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
		hvx_params.offset = gatts_value.offset;
		hvx_params.p_len  = &gatts_value.len;
		hvx_params.p_data = gatts_value.p_value;

		err_code = sd_ble_gatts_hvx(p_readEpever->conn_handle, &hvx_params);
	}
	else
	{
		err_code = NRF_ERROR_INVALID_STATE;
	}

	return err_code;
}

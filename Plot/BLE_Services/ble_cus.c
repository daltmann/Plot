#include "sdk_common.h"
#include "ble_cus.h"
#include <string.h>
#include "ble_srv_common.h"
#include "nrf_gpio.h"
//#include "boards.h"
//#include "nrf_log.h"

#include "Sequence.h"

#include <stdio.h>


static uint32_t u32PWMValue = 0;

static uint8_t u16TableOfPWM[4][NB_TABLE_PWM] = {	8, 17, 25, 33, 42, 50, 58, 67, 76, 85,			// Rouge
													10, 20, 30, 40, 50, 60, 70, 80, 90, 100,		// Vert
													14, 28, 42, 56, 70, 84, 98, 112, 126, 140,		// Bleue
													5, 11, 16, 22, 28, 33, 39, 45, 51, 56};			// Blanc

/**@brief Function for handling the Connect event.
 *
 * @param[in]   p_cus       Custom Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_connect(ble_cus_t * p_cus, ble_evt_t const * p_ble_evt)
{
    p_cus->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;

    ble_cus_evt_t evt;

    evt.evt_type = BLE_CUS_EVT_CONNECTED;

    p_cus->evt_handler(p_cus, &evt);
}

/**@brief Function for handling the Disconnect event.
 *
 * @param[in]   p_cus       Custom Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_disconnect(ble_cus_t * p_cus, ble_evt_t const * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_cus->conn_handle = BLE_CONN_HANDLE_INVALID;
    
    ble_cus_evt_t evt;

    evt.evt_type = BLE_CUS_EVT_DISCONNECTED;

    p_cus->evt_handler(p_cus, &evt);
}

/**@brief Function for handling the Write event.
 *
 * @param[in]   p_cus       Custom Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_write(ble_cus_t * p_cus, ble_evt_t const * p_ble_evt)
{
    ble_gatts_evt_write_t const * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    
    // Custom Value Characteristic Written to.
    if (p_evt_write->handle == p_cus->custom_value_handles.value_handle)
    {
        if (p_evt_write->len == 4)
        {
            // Gestion des PWM
            ble_cus_evt_t evt;
            uint32_t u32ValueOfPWM = 0;
            
            // Sauvegarde du valeur de la PWM
            u32ValueOfPWM = (uint32_t)((p_evt_write->data[0] & 0xFF) << 24);
            u32ValueOfPWM += (uint32_t)((p_evt_write->data[1] & 0xFF) << 16);
            u32ValueOfPWM += (uint32_t)((p_evt_write->data[2] & 0xFF) << 8);
            u32ValueOfPWM += (uint32_t)(p_evt_write->data[3] & 0xFF);
            
            ble_cus_savePWMValue(u32ValueOfPWM);
            
            // Indication de l'écriture provenant de la caractéristique
            evt.evt_type = BLE_CUS_WRITE;
            
            // Appel de la fonction de handle pour signalé l'écriture d'un nouveau PWM
            p_cus->evt_handler(p_cus, &evt);
        }
    }

    // Custom Value Characteristic Written to.
	if (p_evt_write->handle == p_cus->red_current_value_handles.value_handle)
	{
		if (p_evt_write->len == 2)
		{
			// Gestion des PWM
			ble_cus_evt_t evt;
			uint16_t u16ReadValue = 0;
			uint32_t u32ValuePWM = 0;

			// Sauvegarde du valeur de la PWM
			u16ReadValue = (uint16_t)((p_evt_write->data[0] & 0xFF) << 8);
			u16ReadValue += (uint16_t)(p_evt_write->data[1] & 0xFF);

			// Recherche de la valeur du PWM pour la led rouge
			u32ValuePWM = manageCurrentTable(RED, u16ReadValue);

			// On écrit la nouvelle valeur du PWM de la led rouge
			ble_cus_savePWMValue(u32ValuePWM);

			printf("Red : current = %d, PWM = %lx\r\n", u16ReadValue, u32ValuePWM);

			// Indication de l'écriture provenant de la caractéristique
			evt.evt_type = BLE_CUS_WRITE;

			// Appel de la fonction de handle pour signalé l'écriture d'un nouveau PWM
			p_cus->evt_handler(p_cus, &evt);
		}
	}

	// Custom Value Characteristic Written to.
	if (p_evt_write->handle == p_cus->green_current_value_handles.value_handle)
	{
		if (p_evt_write->len == 2)
		{
			// Gestion des PWM
			ble_cus_evt_t evt;
			uint16_t u16ReadValue = 0;
			uint32_t u32ValuePWM = 0;

			// Sauvegarde du valeur de la PWM
			u16ReadValue = (uint16_t)((p_evt_write->data[0] & 0xFF) << 8);
			u16ReadValue += (uint16_t)(p_evt_write->data[1] & 0xFF);

			// Recherche de la valeur du PWM pour la led rouge
			u32ValuePWM = manageCurrentTable(GREEN, u16ReadValue);

			// On écrit la nouvelle valeur du PWM de la led rouge
			ble_cus_savePWMValue(u32ValuePWM);

			printf("Green : current = %d, PWM = %lx\r\n", u16ReadValue, u32ValuePWM);

			// Indication de l'écriture provenant de la caractéristique
			evt.evt_type = BLE_CUS_WRITE;

			// Appel de la fonction de handle pour signalé l'écriture d'un nouveau PWM
			p_cus->evt_handler(p_cus, &evt);
		}
	}

	// Custom Value Characteristic Written to.
	if (p_evt_write->handle == p_cus->blue_current_value_handles.value_handle)
	{
		if (p_evt_write->len == 2)
		{
			// Gestion des PWM
			ble_cus_evt_t evt;
			uint16_t u16ReadValue = 0;
			uint32_t u32ValuePWM = 0;

			// Sauvegarde du valeur de la PWM
			u16ReadValue = (uint16_t)((p_evt_write->data[0] & 0xFF) << 8);
			u16ReadValue += (uint16_t)(p_evt_write->data[1] & 0xFF);

			// Recherche de la valeur du PWM pour la led rouge
			u32ValuePWM = manageCurrentTable(BLUE, u16ReadValue);

			// On écrit la nouvelle valeur du PWM de la led rouge
			ble_cus_savePWMValue(u32ValuePWM);

			printf("Blue : current = %d, PWM = %lx\r\n", u16ReadValue, u32ValuePWM);

			// Indication de l'écriture provenant de la caractéristique
			evt.evt_type = BLE_CUS_WRITE;

			// Appel de la fonction de handle pour signalé l'écriture d'un nouveau PWM
			p_cus->evt_handler(p_cus, &evt);
		}
	}

	// Custom Value Characteristic Written to.
	if (p_evt_write->handle == p_cus->white_current_value_handles.value_handle)
	{
		if (p_evt_write->len == 2)
		{
			// Gestion des PWM
			ble_cus_evt_t evt;
			uint16_t u16ReadValue = 0;
			uint32_t u32ValuePWM = 0;

			// Sauvegarde du valeur de la PWM
			u16ReadValue = (uint16_t)((p_evt_write->data[0] & 0xFF) << 8);
			u16ReadValue += (uint16_t)(p_evt_write->data[1] & 0xFF);

			// Recherche de la valeur du PWM pour la led rouge
			u32ValuePWM = manageCurrentTable(WHITE, u16ReadValue);

			// On écrit la nouvelle valeur du PWM de la led rouge
			ble_cus_savePWMValue(u32ValuePWM);

			printf("White : current = %d, PWM = %lx\r\n", u16ReadValue, u32ValuePWM);

			// Indication de l'écriture provenant de la caractéristique
			evt.evt_type = BLE_CUS_WRITE;

			// Appel de la fonction de handle pour signalé l'écriture d'un nouveau PWM
			p_cus->evt_handler(p_cus, &evt);
		}
	}

    // Manage the "fonctionnement mode" value
	if ((p_evt_write->handle == p_cus->fonctionnement_mode_value_handles.value_handle)
		&& (p_evt_write->len == 1))
	{
		ble_cus_evt_t evt;

		// TODO : save the value here
		// Save.....()
		initNewSequenceC4(p_evt_write->data[0]);
		printf("Fonct mode = %d\r\n", p_evt_write->data[0]);

		// Indication de l'écriture provenant de la caractéristique
		evt.evt_type = BLE_CUS_WRITE_FONCTIONNEMENT_MODE;

		// Appel de la fonction de handle pour signalé l'écriture d'un nouveau PWM
		p_cus->evt_handler(p_cus, &evt);
	}

	// Manage the "time repartition" value
	if ((p_evt_write->handle == p_cus->time_repartition_value_handles.value_handle)
		&& (p_evt_write->len == 1))
	{
		ble_cus_evt_t evt;

		// TODO : save the value here
		// Save.....()
		printf("Time repartition = %d\r\n", p_evt_write->data[0]);

		// Indication de l'écriture provenant de la caractéristique
		evt.evt_type = BLE_CUS_WRITE_TIME_REPARTITION;

		// Appel de la fonction de handle pour signalé l'écriture d'un nouveau PWM
		p_cus->evt_handler(p_cus, &evt);
	}

	// Manage the sequence number value
	if (p_evt_write->handle == p_cus->sequence_number_value_handles.value_handle)
	{
		if (p_evt_write->len == 1)
		{
			ble_cus_evt_t evt;

			// Sauvegarde du nombre total de séquence en mode C4
			saveNumberSequenceC4(p_evt_write->data[0]);

			printf("Sequence number = %d\r\n", p_evt_write->data[0]);

			// Indication de l'écriture provenant de la caractéristique
			evt.evt_type = BLE_CUS_WRITE_SEQUENCE_NUMBER;

			// Appel de la fonction de handle pour signalé l'écriture d'un nouveau PWM
			p_cus->evt_handler(p_cus, &evt);
		}
	}

	// Manage the write sequence value
	if ((p_evt_write->handle == p_cus->write_sequence_value_handles.value_handle)
		&& (p_evt_write->len == 6))
	{
		ble_cus_evt_t evt;

		saveSequenceC4(p_evt_write->data);

		printf("Write sequence = %d\r\n", p_evt_write->data[0]);

		// Indication de l'écriture provenant de la caractéristique
		evt.evt_type = BLE_CUS_WRITE_WRITE_SEQUENCE;

		// Appel de la fonction de handle pour signalé l'écriture d'un nouveau PWM
		p_cus->evt_handler(p_cus, &evt);
	}

	// Check if the Custom value CCCD is written to and that the value is the appropriate length, i.e 2 bytes.
	if ((p_evt_write->handle == p_cus->custom_value_handles.cccd_handle)
		&& (p_evt_write->len == 2)
	   )
	{
		// CCCD written, call application event handler
		if (p_cus->evt_handler != NULL)
		{
			ble_cus_evt_t evt;

			if (ble_srv_is_notification_enabled(p_evt_write->data))
			{
				evt.evt_type = BLE_CUS_EVT_NOTIFICATION_ENABLED;
			}
			else
			{
				evt.evt_type = BLE_CUS_EVT_NOTIFICATION_DISABLED;
			}
			// Call the application event handler.
			p_cus->evt_handler(p_cus, &evt);
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
void ble_cus_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
//    NRF_LOG_INFO("BLE event received. Event type = %d\r\n", p_ble_evt->header.evt_id); 
    ble_cus_t * p_cus = (ble_cus_t *) p_context;
    
    if (p_cus == NULL || p_ble_evt == NULL)
    {
        return;
    }
    
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_cus, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_cus, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_cus, p_ble_evt);
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

/**@brief Function for adding the Custom Value characteristic.
 *
 * @param[in]   p_cus        Battery Service structure.
 * @param[in]   p_cus_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t custom_value_char_add(ble_cus_t * p_cus, const ble_cus_init_t * p_cus_init)
{
    static char user_desc[] = "LED PWM Color";
	
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
    
    cccd_md.write_perm = p_cus_init->custom_value_char_attr_md.cccd_write_perm;
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
		
    ble_uuid.type = p_cus->uuid_type;
    ble_uuid.uuid = CUSTOM_VALUE_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_cus_init->custom_value_char_attr_md.read_perm;
    attr_md.write_perm = p_cus_init->custom_value_char_attr_md.write_perm;
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
	attr_char_value.p_value = (uint8_t *)&p_cus_init->initial_custom_value;

    err_code = sd_ble_gatts_characteristic_add(p_cus->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_cus->custom_value_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

/**@brief Function for adding the red current Value characteristic.
 *
 * @param[in]   p_cus        Battery Service structure.
 * @param[in]   p_cus_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t red_current_value_char_add(ble_cus_t * p_cus, const ble_cus_init_t * p_cus_init)
{
    static char user_desc[] = "RED Current";

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

    cccd_md.write_perm = p_cus_init->custom_value_char_attr_md.cccd_write_perm;
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

    ble_uuid.type = p_cus->uuid_type;
    ble_uuid.uuid = RED_CURRENT_VALUE_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_cus_init->custom_value_char_attr_md.read_perm;
    attr_md.write_perm = p_cus_init->custom_value_char_attr_md.write_perm;
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
	attr_char_value.p_value = (uint8_t *)&p_cus_init->initial_custom_value;

    err_code = sd_ble_gatts_characteristic_add(p_cus->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_cus->red_current_value_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

/**@brief Function for adding the green current Value characteristic.
 *
 * @param[in]   p_cus        Battery Service structure.
 * @param[in]   p_cus_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t green_current_value_char_add(ble_cus_t * p_cus, const ble_cus_init_t * p_cus_init)
{
    static char user_desc[] = "GREEN Current";

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

    cccd_md.write_perm = p_cus_init->custom_value_char_attr_md.cccd_write_perm;
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

    ble_uuid.type = p_cus->uuid_type;
    ble_uuid.uuid = GREEN_CURRENT_VALUE_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_cus_init->custom_value_char_attr_md.read_perm;
    attr_md.write_perm = p_cus_init->custom_value_char_attr_md.write_perm;
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
	attr_char_value.p_value = (uint8_t *)&p_cus_init->initial_custom_value;

    err_code = sd_ble_gatts_characteristic_add(p_cus->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_cus->green_current_value_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

/**@brief Function for adding the blue current Value characteristic.
 *
 * @param[in]   p_cus        Battery Service structure.
 * @param[in]   p_cus_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t blue_current_value_char_add(ble_cus_t * p_cus, const ble_cus_init_t * p_cus_init)
{
    static char user_desc[] = "BLUE Current";

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

    cccd_md.write_perm = p_cus_init->custom_value_char_attr_md.cccd_write_perm;
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

    ble_uuid.type = p_cus->uuid_type;
    ble_uuid.uuid = BLUE_CURRENT_VALUE_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_cus_init->custom_value_char_attr_md.read_perm;
    attr_md.write_perm = p_cus_init->custom_value_char_attr_md.write_perm;
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
	attr_char_value.p_value = (uint8_t *)&p_cus_init->initial_custom_value;

    err_code = sd_ble_gatts_characteristic_add(p_cus->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_cus->blue_current_value_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

/**@brief Function for adding the white current Value characteristic.
 *
 * @param[in]   p_cus        Battery Service structure.
 * @param[in]   p_cus_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t white_current_value_char_add(ble_cus_t * p_cus, const ble_cus_init_t * p_cus_init)
{
    static char user_desc[] = "WHITE Current";

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

    cccd_md.write_perm = p_cus_init->custom_value_char_attr_md.cccd_write_perm;
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

    ble_uuid.type = p_cus->uuid_type;
    ble_uuid.uuid = WHITE_CURRENT_VALUE_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_cus_init->custom_value_char_attr_md.read_perm;
    attr_md.write_perm = p_cus_init->custom_value_char_attr_md.write_perm;
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
	attr_char_value.p_value = (uint8_t *)&p_cus_init->initial_custom_value;

    err_code = sd_ble_gatts_characteristic_add(p_cus->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_cus->white_current_value_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

/**@brief Function for adding the fonctionnement mode Value characteristic.
 *
 * @param[in]   p_cus        Battery Service structure.
 * @param[in]   p_cus_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t fonctionnement_mode_value_char_add(ble_cus_t * p_cus, const ble_cus_init_t * p_cus_init)
{
    static char user_desc[] = "Fonctionnement mode";

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

    cccd_md.write_perm = p_cus_init->custom_value_char_attr_md.cccd_write_perm;
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

    ble_uuid.type = p_cus->uuid_type;
    ble_uuid.uuid = FONCTIONNEMENT_MODE_VALUE_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_cus_init->custom_value_char_attr_md.read_perm;
    attr_md.write_perm = p_cus_init->custom_value_char_attr_md.write_perm;
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
	attr_char_value.p_value = (uint8_t *)&p_cus_init->initial_custom_value;

    err_code = sd_ble_gatts_characteristic_add(p_cus->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_cus->fonctionnement_mode_value_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

/**@brief Function for adding the time repartition Value characteristic.
 *
 * @param[in]   p_cus        Battery Service structure.
 * @param[in]   p_cus_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t time_repartition_value_char_add(ble_cus_t * p_cus, const ble_cus_init_t * p_cus_init)
{
    static char user_desc[] = "Time repartition";

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

    cccd_md.write_perm = p_cus_init->custom_value_char_attr_md.cccd_write_perm;
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

    ble_uuid.type = p_cus->uuid_type;
    ble_uuid.uuid = TIME_REPARTITION_VALUE_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_cus_init->custom_value_char_attr_md.read_perm;
    attr_md.write_perm = p_cus_init->custom_value_char_attr_md.write_perm;
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
	attr_char_value.p_value = (uint8_t *)&p_cus_init->initial_custom_value;

    err_code = sd_ble_gatts_characteristic_add(p_cus->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_cus->time_repartition_value_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

/**@brief Function for adding the sequence number Value characteristic.
 *
 * @param[in]   p_cus        Battery Service structure.
 * @param[in]   p_cus_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t sequence_number_value_char_add(ble_cus_t * p_cus, const ble_cus_init_t * p_cus_init)
{
    static char user_desc[] = "Sequence number";

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

    cccd_md.write_perm = p_cus_init->custom_value_char_attr_md.cccd_write_perm;
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

    ble_uuid.type = p_cus->uuid_type;
    ble_uuid.uuid = SEQUENCE_NUMBER_VALUE_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_cus_init->custom_value_char_attr_md.read_perm;
    attr_md.write_perm = p_cus_init->custom_value_char_attr_md.write_perm;
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
	attr_char_value.p_value = (uint8_t *)&p_cus_init->initial_custom_value;

    err_code = sd_ble_gatts_characteristic_add(p_cus->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_cus->sequence_number_value_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

/**@brief Function for adding the write sequence Value characteristic.
 *
 * @param[in]   p_cus        Battery Service structure.
 * @param[in]   p_cus_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t write_sequence_value_char_add(ble_cus_t * p_cus, const ble_cus_init_t * p_cus_init)
{
    static char user_desc[] = "Write sequence";

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

    cccd_md.write_perm = p_cus_init->custom_value_char_attr_md.cccd_write_perm;
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

    ble_uuid.type = p_cus->uuid_type;
    ble_uuid.uuid = SEQUENCE_WRITE_VALUE_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_cus_init->custom_value_char_attr_md.read_perm;
    attr_md.write_perm = p_cus_init->custom_value_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint8_t) * 6;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = sizeof(uint8_t) * 6;
	attr_char_value.p_value = (uint8_t *)&p_cus_init->initial_custom_value;

    err_code = sd_ble_gatts_characteristic_add(p_cus->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_cus->write_sequence_value_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

/**@brief Function for adding the read sequence Value characteristic.
 *
 * @param[in]   p_cus        Battery Service structure.
 * @param[in]   p_cus_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t read_sequence_value_char_add(ble_cus_t * p_cus, const ble_cus_init_t * p_cus_init)
{
    static char user_desc[] = "Read sequence";

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

    cccd_md.write_perm = p_cus_init->custom_value_char_attr_md.cccd_write_perm;
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.write  = 0;
    char_md.char_props.notify = 1;
    char_md.p_char_user_desc  = (uint8_t *) user_desc;	// Ici, c'est la valeur de la description
    char_md.char_user_desc_size = strlen(user_desc);
    char_md.char_user_desc_max_size = strlen(user_desc);

    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_cus->uuid_type;
    ble_uuid.uuid = SEQUENCE_READ_VALUE_CHAR_UUID;

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_cus_init->custom_value_char_attr_md.read_perm;
    attr_md.write_perm = p_cus_init->custom_value_char_attr_md.write_perm;
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
	attr_char_value.p_value = (uint8_t *)&p_cus_init->initial_custom_value;

    err_code = sd_ble_gatts_characteristic_add(p_cus->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_cus->read_sequence_value_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

uint32_t ble_cus_init(ble_cus_t * p_cus, const ble_cus_init_t * p_cus_init)
{
    if (p_cus == NULL || p_cus_init == NULL)
    {
        return NRF_ERROR_NULL;
    }

    uint32_t   err_code;
    ble_uuid_t ble_uuid;

    // Initialize service structure
    p_cus->evt_handler               = p_cus_init->evt_handler;
    p_cus->conn_handle               = BLE_CONN_HANDLE_INVALID;

    // Add Custom Service UUID
    ble_uuid128_t base_uuid = {CUSTOM_SERVICE_UUID_BASE};
    err_code =  sd_ble_uuid_vs_add(&base_uuid, &p_cus->uuid_type);
    VERIFY_SUCCESS(err_code);
    
    ble_uuid.type = p_cus->uuid_type;
    ble_uuid.uuid = CUSTOM_SERVICE_UUID;

    // Add the Custom Service
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_cus->service_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Custom Value characteristic
    err_code = custom_value_char_add(p_cus, p_cus_init);
    VERIFY_SUCCESS(err_code);

    err_code = red_current_value_char_add(p_cus, p_cus_init);
    VERIFY_SUCCESS(err_code);

    err_code = green_current_value_char_add(p_cus, p_cus_init);
	VERIFY_SUCCESS(err_code);

	err_code = blue_current_value_char_add(p_cus, p_cus_init);
	VERIFY_SUCCESS(err_code);

	err_code = white_current_value_char_add(p_cus, p_cus_init);
	VERIFY_SUCCESS(err_code);

	err_code = fonctionnement_mode_value_char_add(p_cus, p_cus_init);
	VERIFY_SUCCESS(err_code);

	err_code = time_repartition_value_char_add(p_cus, p_cus_init);
	VERIFY_SUCCESS(err_code);

	err_code = sequence_number_value_char_add(p_cus, p_cus_init);
	VERIFY_SUCCESS(err_code);

	err_code = write_sequence_value_char_add(p_cus, p_cus_init);
	VERIFY_SUCCESS(err_code);

	err_code = read_sequence_value_char_add(p_cus, p_cus_init);
	VERIFY_SUCCESS(err_code);

    return NRF_SUCCESS;
}

uint32_t ble_cus_custom_value_update(ble_cus_t * p_cus, uint32_t custom_value)
{
//    NRF_LOG_INFO("In ble_cus_custom_value_update. \r\n"); 
    if (p_cus == NULL)
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
    err_code = sd_ble_gatts_value_set(p_cus->conn_handle,
                                      p_cus->custom_value_handles.value_handle,
                                      &gatts_value);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
		
		//nrf_gpio_pin_set(20);

    // Send value if connected and notifying.
    if ((p_cus->conn_handle != BLE_CONN_HANDLE_INVALID)) 
    {
        ble_gatts_hvx_params_t hvx_params;

        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_cus->custom_value_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = gatts_value.offset;
        hvx_params.p_len  = &gatts_value.len;
        hvx_params.p_data = gatts_value.p_value;

        err_code = sd_ble_gatts_hvx(p_cus->conn_handle, &hvx_params);
//        NRF_LOG_INFO("sd_ble_gatts_hvx result: %x. \r\n", err_code); 
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
//        NRF_LOG_INFO("sd_ble_gatts_hvx result: NRF_ERROR_INVALID_STATE. \r\n"); 
    }


    return err_code;
}


uint32_t read_sequence_value_update(ble_cus_t * p_cus, uint8_t custom_value)
{
//    NRF_LOG_INFO("In ble_cus_custom_value_update. \r\n");
    if (p_cus == NULL)
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
    err_code = sd_ble_gatts_value_set(p_cus->conn_handle,
                                      p_cus->read_sequence_value_handles.value_handle,
                                      &gatts_value);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Send value if connected and notifying.
    if ((p_cus->conn_handle != BLE_CONN_HANDLE_INVALID))
    {
        ble_gatts_hvx_params_t hvx_params;

        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_cus->read_sequence_value_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = gatts_value.offset;
        hvx_params.p_len  = &gatts_value.len;
        hvx_params.p_data = gatts_value.p_value;

        err_code = sd_ble_gatts_hvx(p_cus->conn_handle, &hvx_params);
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }


    return err_code;
}

void ble_cus_savePWMValue(uint32_t u32Value)
{
    u32PWMValue = u32Value;
}

uint32_t ble_cus_readPWMValue(void)
{
    return u32PWMValue;
}

uint32_t manageCurrentTable(nameOfLedColor color, unsigned int u16Value)
{
	uint32_t u32ValuePWM = 0;

	// On lit les valeurs actuelles des PWM
	u32ValuePWM = ble_cus_readPWMValue();

	switch(color)
	{
		case RED:
			u32ValuePWM = (u32ValuePWM & 0x00FFFFFF) + ((unsigned long)findPWMWithCurrent(0, u16Value) << 24);
			break;
		case GREEN:
			u32ValuePWM = (u32ValuePWM & 0xFF00FFFF) + ((unsigned long)findPWMWithCurrent(1, u16Value) << 16);
			break;
		case BLUE:
			u32ValuePWM = (u32ValuePWM & 0xFFFF00FF) + ((unsigned long)findPWMWithCurrent(2, u16Value) << 8);
			break;
		case WHITE:
			u32ValuePWM = (u32ValuePWM & 0xFFFFFF00) + ((unsigned long)findPWMWithCurrent(3, u16Value));
			break;
	}

	return u32ValuePWM;
}

unsigned char findPWMWithCurrent(unsigned char color, unsigned int u16Value)
{
	unsigned char u08PWMValue = 0xFF;
	unsigned char u08Index = 0;

	// On parcours toutes les valeurs du tableau
	for (u08Index = 0; u08Index < NB_TABLE_PWM; u08Index++)
	{	// On recherche une valeur de courant qui soit plus grande que la consigne
		if (u16TableOfPWM[color][u08Index] > u16Value)
		{	// Si on trouve une valeur plus grande que la consigne, on a trouver le PWM supérieur
			// On prends la valeur du PWM inférieur, et on la multiplie par le pas de chaque PWM, c'est à dire 255 / 10
			u08PWMValue = u08Index * (255 / NB_TABLE_PWM);
			break;
		}
	}

	return u08PWMValue;
}

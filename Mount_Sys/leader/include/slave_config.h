/* Master 0, Slave 1, "EL2809"
 * Vendor ID:       0x00000002
 * Product code:    0x0af93052
 * Revision number: 0x00120000
 */

ec_pdo_entry_info_t slave_1_pdo_entries[] = {
    {0x7000, 0x01, 1}, /* Output */
    {0x7010, 0x01, 1}, /* Output */
    {0x7020, 0x01, 1}, /* Output */
    {0x7030, 0x01, 1}, /* Output */
    {0x7040, 0x01, 1}, /* Output */
    {0x7050, 0x01, 1}, /* Output */
    {0x7060, 0x01, 1}, /* Output */
    {0x7070, 0x01, 1}, /* Output */
    {0x7080, 0x01, 1}, /* Output */
    {0x7090, 0x01, 1}, /* Output */
    {0x70a0, 0x01, 1}, /* Output */
    {0x70b0, 0x01, 1}, /* Output */
    {0x70c0, 0x01, 1}, /* Output */
    {0x70d0, 0x01, 1}, /* Output */
    {0x70e0, 0x01, 1}, /* Output */
    {0x70f0, 0x01, 1}, /* Output */
};

ec_pdo_info_t slave_1_pdos[] = {
    {0x1600, 1, slave_1_pdo_entries + 0}, /* Channel 1 */
    {0x1601, 1, slave_1_pdo_entries + 1}, /* Channel 2 */
    {0x1602, 1, slave_1_pdo_entries + 2}, /* Channel 3 */
    {0x1603, 1, slave_1_pdo_entries + 3}, /* Channel 4 */
    {0x1604, 1, slave_1_pdo_entries + 4}, /* Channel 5 */
    {0x1605, 1, slave_1_pdo_entries + 5}, /* Channel 6 */
    {0x1606, 1, slave_1_pdo_entries + 6}, /* Channel 7 */
    {0x1607, 1, slave_1_pdo_entries + 7}, /* Channel 8 */
    {0x1608, 1, slave_1_pdo_entries + 8}, /* Channel 9 */
    {0x1609, 1, slave_1_pdo_entries + 9}, /* Channel 10 */
    {0x160a, 1, slave_1_pdo_entries + 10}, /* Channel 11 */
    {0x160b, 1, slave_1_pdo_entries + 11}, /* Channel 12 */
    {0x160c, 1, slave_1_pdo_entries + 12}, /* Channel 13 */
    {0x160d, 1, slave_1_pdo_entries + 13}, /* Channel 14 */
    {0x160e, 1, slave_1_pdo_entries + 14}, /* Channel 15 */
    {0x160f, 1, slave_1_pdo_entries + 15}, /* Channel 16 */
};

ec_sync_info_t slave_1_syncs[] = {
    {0, EC_DIR_OUTPUT, 8, slave_1_pdos + 0, EC_WD_ENABLE},
    {1, EC_DIR_OUTPUT, 8, slave_1_pdos + 8, EC_WD_ENABLE},
    {0xff}
};

/* Master 0, Slave 2, "EL6022"
 * Vendor ID:       0x00000002
 * Product code:    0x17863052
 * Revision number: 0x00130000
 */

ec_pdo_entry_info_t slave_2_pdo_entries[] = {
    {0x7001, 0x01, 16},
    {0x7000, 0x11, 8},
    {0x7000, 0x12, 8},
    {0x7000, 0x13, 8},
    {0x7000, 0x14, 8},
    {0x7000, 0x15, 8},
    {0x7000, 0x16, 8},
    {0x7000, 0x17, 8},
    {0x7000, 0x18, 8},
    {0x7000, 0x19, 8},
    {0x7000, 0x1a, 8},
    {0x7000, 0x1b, 8},
    {0x7000, 0x1c, 8},
    {0x7000, 0x1d, 8},
    {0x7000, 0x1e, 8},
    {0x7000, 0x1f, 8},
    {0x7000, 0x20, 8},
    {0x7000, 0x21, 8},
    {0x7000, 0x22, 8},
    {0x7000, 0x23, 8},
    {0x7000, 0x24, 8},
    {0x7000, 0x25, 8},
    {0x7000, 0x26, 8},
    {0x7011, 0x01, 16},
    {0x7010, 0x11, 8},
    {0x7010, 0x12, 8},
    {0x7010, 0x13, 8},
    {0x7010, 0x14, 8},
    {0x7010, 0x15, 8},
    {0x7010, 0x16, 8},
    {0x7010, 0x17, 8},
    {0x7010, 0x18, 8},
    {0x7010, 0x19, 8},
    {0x7010, 0x1a, 8},
    {0x7010, 0x1b, 8},
    {0x7010, 0x1c, 8},
    {0x7010, 0x1d, 8},
    {0x7010, 0x1e, 8},
    {0x7010, 0x1f, 8},
    {0x7010, 0x20, 8},
    {0x7010, 0x21, 8},
    {0x7010, 0x22, 8},
    {0x7010, 0x23, 8},
    {0x7010, 0x24, 8},
    {0x7010, 0x25, 8},
    {0x7010, 0x26, 8},
    {0x6001, 0x01, 16},
    {0x6000, 0x11, 8},
    {0x6000, 0x12, 8},
    {0x6000, 0x13, 8},
    {0x6000, 0x14, 8},
    {0x6000, 0x15, 8},
    {0x6000, 0x16, 8},
    {0x6000, 0x17, 8},
    {0x6000, 0x18, 8},
    {0x6000, 0x19, 8},
    {0x6000, 0x1a, 8},
    {0x6000, 0x1b, 8},
    {0x6000, 0x1c, 8},
    {0x6000, 0x1d, 8},
    {0x6000, 0x1e, 8},
    {0x6000, 0x1f, 8},
    {0x6000, 0x20, 8},
    {0x6000, 0x21, 8},
    {0x6000, 0x22, 8},
    {0x6000, 0x23, 8},
    {0x6000, 0x24, 8},
    {0x6000, 0x25, 8},
    {0x6000, 0x26, 8},
    {0x6011, 0x01, 16},
    {0x6010, 0x11, 8},
    {0x6010, 0x12, 8},
    {0x6010, 0x13, 8},
    {0x6010, 0x14, 8},
    {0x6010, 0x15, 8},
    {0x6010, 0x16, 8},
    {0x6010, 0x17, 8},
    {0x6010, 0x18, 8},
    {0x6010, 0x19, 8},
    {0x6010, 0x1a, 8},
    {0x6010, 0x1b, 8},
    {0x6010, 0x1c, 8},
    {0x6010, 0x1d, 8},
    {0x6010, 0x1e, 8},
    {0x6010, 0x1f, 8},
    {0x6010, 0x20, 8},
    {0x6010, 0x21, 8},
    {0x6010, 0x22, 8},
    {0x6010, 0x23, 8},
    {0x6010, 0x24, 8},
    {0x6010, 0x25, 8},
    {0x6010, 0x26, 8},
};

ec_pdo_info_t slave_2_pdos[] = {
    {0x1604, 23, slave_2_pdo_entries + 0}, /* COM RxPDO-Map Outputs Ch.1 */
    {0x1605, 23, slave_2_pdo_entries + 23}, /* COM RxPDO-Map Outputs Ch.2 */
    {0x1a04, 23, slave_2_pdo_entries + 46}, /* COM TxPDO-Map Inputs Ch.1 */
    {0x1a05, 23, slave_2_pdo_entries + 69}, /* COM TxPDO-Map Inputs Ch.2 */
};

ec_sync_info_t slave_2_syncs[] = {
    {0, EC_DIR_OUTPUT, 0, NULL, EC_WD_DISABLE},
    {1, EC_DIR_INPUT, 0, NULL, EC_WD_DISABLE},
    {2, EC_DIR_OUTPUT, 2, slave_2_pdos + 0, EC_WD_DISABLE},
    {3, EC_DIR_INPUT, 2, slave_2_pdos + 2, EC_WD_DISABLE},
    {0xff}
};

/* Master 0, Slave 5, "EL2809"
 * Vendor ID:       0x00000002
 * Product code:    0x0af93052
 * Revision number: 0x00120000
 */

ec_pdo_entry_info_t slave_5_pdo_entries[] = {
    {0x7000, 0x01, 1}, /* Output */
    {0x7010, 0x01, 1}, /* Output */
    {0x7020, 0x01, 1}, /* Output */
    {0x7030, 0x01, 1}, /* Output */
    {0x7040, 0x01, 1}, /* Output */
    {0x7050, 0x01, 1}, /* Output */
    {0x7060, 0x01, 1}, /* Output */
    {0x7070, 0x01, 1}, /* Output */
    {0x7080, 0x01, 1}, /* Output */
    {0x7090, 0x01, 1}, /* Output */
    {0x70a0, 0x01, 1}, /* Output */
    {0x70b0, 0x01, 1}, /* Output */
    {0x70c0, 0x01, 1}, /* Output */
    {0x70d0, 0x01, 1}, /* Output */
    {0x70e0, 0x01, 1}, /* Output */
    {0x70f0, 0x01, 1}, /* Output */
};

ec_pdo_info_t slave_5_pdos[] = {
    {0x1600, 1, slave_5_pdo_entries + 0}, /* Channel 1 */
    {0x1601, 1, slave_5_pdo_entries + 1}, /* Channel 2 */
    {0x1602, 1, slave_5_pdo_entries + 2}, /* Channel 3 */
    {0x1603, 1, slave_5_pdo_entries + 3}, /* Channel 4 */
    {0x1604, 1, slave_5_pdo_entries + 4}, /* Channel 5 */
    {0x1605, 1, slave_5_pdo_entries + 5}, /* Channel 6 */
    {0x1606, 1, slave_5_pdo_entries + 6}, /* Channel 7 */
    {0x1607, 1, slave_5_pdo_entries + 7}, /* Channel 8 */
    {0x1608, 1, slave_5_pdo_entries + 8}, /* Channel 9 */
    {0x1609, 1, slave_5_pdo_entries + 9}, /* Channel 10 */
    {0x160a, 1, slave_5_pdo_entries + 10}, /* Channel 11 */
    {0x160b, 1, slave_5_pdo_entries + 11}, /* Channel 12 */
    {0x160c, 1, slave_5_pdo_entries + 12}, /* Channel 13 */
    {0x160d, 1, slave_5_pdo_entries + 13}, /* Channel 14 */
    {0x160e, 1, slave_5_pdo_entries + 14}, /* Channel 15 */
    {0x160f, 1, slave_5_pdo_entries + 15}, /* Channel 16 */
};

ec_sync_info_t slave_5_syncs[] = {
    {0, EC_DIR_OUTPUT, 8, slave_5_pdos + 0, EC_WD_ENABLE},
    {1, EC_DIR_OUTPUT, 8, slave_5_pdos + 8, EC_WD_ENABLE},
    {0xff}
};

/* Master 0, Slave 6, "EL6022"
 * Vendor ID:       0x00000002
 * Product code:    0x17863052
 * Revision number: 0x00130000
 */

ec_pdo_entry_info_t slave_6_pdo_entries[] = {
    {0x7001, 0x01, 16},
    {0x7000, 0x11, 8},
    {0x7000, 0x12, 8},
    {0x7000, 0x13, 8},
    {0x7000, 0x14, 8},
    {0x7000, 0x15, 8},
    {0x7000, 0x16, 8},
    {0x7000, 0x17, 8},
    {0x7000, 0x18, 8},
    {0x7000, 0x19, 8},
    {0x7000, 0x1a, 8},
    {0x7000, 0x1b, 8},
    {0x7000, 0x1c, 8},
    {0x7000, 0x1d, 8},
    {0x7000, 0x1e, 8},
    {0x7000, 0x1f, 8},
    {0x7000, 0x20, 8},
    {0x7000, 0x21, 8},
    {0x7000, 0x22, 8},
    {0x7000, 0x23, 8},
    {0x7000, 0x24, 8},
    {0x7000, 0x25, 8},
    {0x7000, 0x26, 8},
    {0x7011, 0x01, 16},
    {0x7010, 0x11, 8},
    {0x7010, 0x12, 8},
    {0x7010, 0x13, 8},
    {0x7010, 0x14, 8},
    {0x7010, 0x15, 8},
    {0x7010, 0x16, 8},
    {0x7010, 0x17, 8},
    {0x7010, 0x18, 8},
    {0x7010, 0x19, 8},
    {0x7010, 0x1a, 8},
    {0x7010, 0x1b, 8},
    {0x7010, 0x1c, 8},
    {0x7010, 0x1d, 8},
    {0x7010, 0x1e, 8},
    {0x7010, 0x1f, 8},
    {0x7010, 0x20, 8},
    {0x7010, 0x21, 8},
    {0x7010, 0x22, 8},
    {0x7010, 0x23, 8},
    {0x7010, 0x24, 8},
    {0x7010, 0x25, 8},
    {0x7010, 0x26, 8},
    {0x6001, 0x01, 16},
    {0x6000, 0x11, 8},
    {0x6000, 0x12, 8},
    {0x6000, 0x13, 8},
    {0x6000, 0x14, 8},
    {0x6000, 0x15, 8},
    {0x6000, 0x16, 8},
    {0x6000, 0x17, 8},
    {0x6000, 0x18, 8},
    {0x6000, 0x19, 8},
    {0x6000, 0x1a, 8},
    {0x6000, 0x1b, 8},
    {0x6000, 0x1c, 8},
    {0x6000, 0x1d, 8},
    {0x6000, 0x1e, 8},
    {0x6000, 0x1f, 8},
    {0x6000, 0x20, 8},
    {0x6000, 0x21, 8},
    {0x6000, 0x22, 8},
    {0x6000, 0x23, 8},
    {0x6000, 0x24, 8},
    {0x6000, 0x25, 8},
    {0x6000, 0x26, 8},
    {0x6011, 0x01, 16},
    {0x6010, 0x11, 8},
    {0x6010, 0x12, 8},
    {0x6010, 0x13, 8},
    {0x6010, 0x14, 8},
    {0x6010, 0x15, 8},
    {0x6010, 0x16, 8},
    {0x6010, 0x17, 8},
    {0x6010, 0x18, 8},
    {0x6010, 0x19, 8},
    {0x6010, 0x1a, 8},
    {0x6010, 0x1b, 8},
    {0x6010, 0x1c, 8},
    {0x6010, 0x1d, 8},
    {0x6010, 0x1e, 8},
    {0x6010, 0x1f, 8},
    {0x6010, 0x20, 8},
    {0x6010, 0x21, 8},
    {0x6010, 0x22, 8},
    {0x6010, 0x23, 8},
    {0x6010, 0x24, 8},
    {0x6010, 0x25, 8},
    {0x6010, 0x26, 8},
};

ec_pdo_info_t slave_6_pdos[] = {
    {0x1604, 23, slave_6_pdo_entries + 0}, /* COM RxPDO-Map Outputs Ch.1 */
    {0x1605, 23, slave_6_pdo_entries + 23}, /* COM RxPDO-Map Outputs Ch.2 */
    {0x1a04, 23, slave_6_pdo_entries + 46}, /* COM TxPDO-Map Inputs Ch.1 */
    {0x1a05, 23, slave_6_pdo_entries + 69}, /* COM TxPDO-Map Inputs Ch.2 */
};

ec_sync_info_t slave_6_syncs[] = {
    {0, EC_DIR_OUTPUT, 0, NULL, EC_WD_DISABLE},
    {1, EC_DIR_INPUT, 0, NULL, EC_WD_DISABLE},
    {2, EC_DIR_OUTPUT, 2, slave_6_pdos + 0, EC_WD_DISABLE},
    {3, EC_DIR_INPUT, 2, slave_6_pdos + 2, EC_WD_DISABLE},
    {0xff}
};

/* Master 1, Slave 0, "AMSAMOTION_EC_MB"
 * Vendor ID:       0x00000b95
 * Product code:    0x00001400
 * Revision number: 0x00010004
 */

ec_pdo_entry_info_t slave_0_pdo_entries[] = {
    {0x7001, 0x01, 8}, /* slave Address */
    {0x7001, 0x02, 8}, /* Data Valid */
    {0x7001, 0x03, 32}, /* coil_1 */
    {0x7001, 0x04, 32}, /* coil_2 */
    {0x7001, 0x05, 16}, /* hold_reg data_1 */
    {0x7001, 0x06, 16}, /* hold_reg data_2 */
    {0x7001, 0x07, 16}, /* hold_reg data_3 */
    {0x7001, 0x08, 16}, /* hold_reg data_4 */
    {0x7001, 0x09, 16}, /* hold_reg data_5 */
    {0x7001, 0x0a, 16}, /* hold_reg data_6 */
    {0x7001, 0x0b, 16}, /* hold_reg data_7 */
    {0x7001, 0x0c, 16}, /* hold_reg data_8 */
    {0x7001, 0x0d, 16}, /* hold_reg data_9 */
    {0x7001, 0x0e, 16}, /* hold_reg data_10 */
    {0x7001, 0x0f, 16}, /* hold_reg data_11 */
    {0x7001, 0x10, 16}, /* hold_reg data_12 */
    {0x7001, 0x11, 16}, /* hold_reg data_13 */
    {0x7001, 0x12, 16}, /* hold_reg data_14 */
    {0x7001, 0x13, 16}, /* hold_reg data_15 */
    {0x7001, 0x14, 16}, /* hold_reg data_16 */
    {0x6001, 0x01, 8}, /* slave Address */
    {0x6001, 0x02, 8}, /* Data Valid */
    {0x6001, 0x03, 32}, /* D_Input_1 */
    {0x6001, 0x04, 32}, /* D_Input_2 */
    {0x6001, 0x05, 16}, /* Input reg data_1 */
    {0x6001, 0x06, 16}, /* Input reg data_2 */
    {0x6001, 0x07, 16}, /* Input reg data_3 */
    {0x6001, 0x08, 16}, /* Input reg data_4 */
    {0x6001, 0x09, 16}, /* Input reg data_5 */
    {0x6001, 0x0a, 16}, /* Input reg data_6 */
    {0x6001, 0x0b, 16}, /* Input reg data_7 */
    {0x6001, 0x0c, 16}, /* Input reg data_8 */
    {0x6001, 0x0d, 16}, /* Input reg data_9 */
    {0x6001, 0x0e, 16}, /* Input reg data_10 */
    {0x6001, 0x0f, 16}, /* Input reg data_11 */
    {0x6001, 0x10, 16}, /* Input reg data_12 */
    {0x6001, 0x11, 16}, /* Input reg data_13 */
    {0x6001, 0x12, 16}, /* Input reg data_14 */
    {0x6001, 0x13, 16}, /* Input reg data_15 */
    {0x6001, 0x14, 16}, /* Input reg data_16 */
    {0x6002, 0x01, 8}, /* slave Address */
    {0x6002, 0x02, 8}, /* Data Valid */
    {0x6002, 0x03, 32}, /* coil_1 */
    {0x6002, 0x04, 32}, /* coil_2 */
    {0x6002, 0x05, 16}, /* hold_reg data_1 */
    {0x6002, 0x06, 16}, /* hold_reg data_2 */
    {0x6002, 0x07, 16}, /* hold_reg data_3 */
    {0x6002, 0x08, 16}, /* hold_reg data_4 */
    {0x6002, 0x09, 16}, /* hold_reg data_5 */
    {0x6002, 0x0a, 16}, /* hold_reg data_6 */
    {0x6002, 0x0b, 16}, /* hold_reg data_7 */
    {0x6002, 0x0c, 16}, /* hold_reg data_8 */
    {0x6002, 0x0d, 16}, /* hold_reg data_9 */
    {0x6002, 0x0e, 16}, /* hold_reg data_10 */
    {0x6002, 0x0f, 16}, /* hold_reg data_11 */
    {0x6002, 0x10, 16}, /* hold_reg data_12 */
    {0x6002, 0x11, 16}, /* hold_reg data_13 */
    {0x6002, 0x12, 16}, /* hold_reg data_14 */
    {0x6002, 0x13, 16}, /* hold_reg data_15 */
    {0x6002, 0x14, 16}, /* hold_reg data_16 */
};

ec_pdo_info_t slave_0_pdos[] = {
    {0x1601, 20, slave_0_pdo_entries + 0}, /* Rx-Mapping Master data to slave */
    {0x1a01, 20, slave_0_pdo_entries + 20}, /* Tx-Mapping Master data from slave */
    {0x1a02, 20, slave_0_pdo_entries + 40}, /* Tx-Mapping Master data from slave */
};

ec_sync_info_t slave_0_syncs[] = {
    {0, EC_DIR_OUTPUT, 0, NULL, EC_WD_DISABLE},
    {1, EC_DIR_INPUT, 0, NULL, EC_WD_DISABLE},
    {2, EC_DIR_OUTPUT, 1, slave_0_pdos + 0, EC_WD_ENABLE},
    {3, EC_DIR_INPUT, 2, slave_0_pdos + 1, EC_WD_DISABLE},
    {0xff}
};

/* Master 1, Slave 1, "Kaiserdrive_ECAT"
 * Vendor ID:       0x00010203
 * Product code:    0x00000402
 * Revision number: 0x00050012
 */

ec_pdo_entry_info_t slave_1_pdo_entries[] = {
    {0x6040, 0x00, 16}, /* C */
    {0x607a, 0x00, 32}, /* T */
    {0x60ff, 0x00, 32}, /* T */
    {0x6071, 0x00, 16}, /* T */
    {0x6041, 0x00, 16}, /* S */
    {0x6064, 0x00, 32}, /* P */
    {0x606c, 0x00, 32}, /* V */
    {0x6077, 0x00, 16}, /* T */
    {0x60f4, 0x00, 32}, /* F */
    {0x603f, 0x00, 16}, /* E */
};

ec_pdo_info_t slave_1_pdos[] = {
    {0x1601, 4, slave_1_pdo_entries + 0}, /* 2 */
    {0x1a01, 6, slave_1_pdo_entries + 4}, /* 2 */
};

ec_sync_info_t slave_1_syncs[] = {
    {0, EC_DIR_OUTPUT, 0, NULL, EC_WD_DISABLE},
    {1, EC_DIR_INPUT, 0, NULL, EC_WD_DISABLE},
    {2, EC_DIR_OUTPUT, 1, slave_1_pdos + 0, EC_WD_ENABLE},
    {3, EC_DIR_INPUT, 1, slave_1_pdos + 1, EC_WD_DISABLE},
    {0xff}
};


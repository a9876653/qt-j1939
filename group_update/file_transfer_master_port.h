#pragma once
#include "file_transfer_master.h"
#include "comm_j1939.h"
#include "QDebug"
#include "middle_signal.h"
#include "QObject"
#include "QMessageBox"
#include "QString"
#include "QFile"
#include "QFileDialog"

class FileTransferMaster : public QObject
{
    Q_OBJECT
public:
    FileTransferMaster();
    ~FileTransferMaster();

    int               start_transfer(uint8_t *obj_addr, uint8_t obj_num, file_data_info_t *data_info);
    file_data_info_t *set_file_data_info(QString file_path);

    void transfer_task();

    float get_update_process();
    void  app_info_request();
    void  verify_request();
    void  send_handshake_cmd(uint8_t *obj_addr, uint8_t obj_num);

signals:
    void sig_dbg_info(QString s);
    void sig_app_info_update(app_info_str_t &str);
    void sig_update_failed(uint8_t result);
    void sig_update_state(uint8_t src, int result, uint32_t offset);

private:
    int      send(uint32_t msg_id, uint8_t *data, uint16_t len);
    void     update_failed_cb(uint8_t result);
    uint32_t get_file_data(uint32_t offset, uint8_t *data, uint32_t len);
    void     transfer_state_cb(uint8_t src, int result, uint32_t offset);

    void dbg_info(const char *msg, ...);
    void slot_recv_pgn_handle(uint32_t pgn, uint8_t src, QVector<uint8_t> array);

private:
    file_transfer_master_t m_file_transfer;
    file_master_if_t       m_file_transfer_if;
    file_data_info_t       m_file_data_info;
    uint8_t                m_dest_addr = ADDRESS_GLOBAL;
};

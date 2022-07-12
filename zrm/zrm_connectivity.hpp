/*
 * Class connectivity with zrm module
 * Ostapenko D.V. NIKTES 2019-03-11
 *
 */



#ifndef ZRMCONNECTIVITY_H
#define ZRMCONNECTIVITY_H

#include "zrmproto.hpp"
#include "zrmchannel.hpp"
#include <miodevworker.h>
#include <qsharedpointer.h>
#include <qevent.h>
#include <qtimer.h>
#include <tuple>

namespace zrm {



using ZrmChannelAttributes = zrm::ZrmChannel::Attributes;

using    ZrmChannelSharedPointer =  QSharedPointer<ZrmChannel>       ;
using    ZrmTextMap       =  QMap<uint32_t, QString>         ;
using    ZrmChannelsMap   =  QMap<uint16_t, ZrmChannelSharedPointer> ;
using    ZrmChannelsKeys  =  QList<ZrmChannelsMap::key_type>     ;

class ZrmConnectivity : public QMultioDevWorker
{
    Q_OBJECT
    Q_DISABLE_COPY(ZrmConnectivity)
public:

    typedef           QList<ZrmConnectivity*>   connectivity_list_t;

    explicit           ZrmConnectivity(const QString& conn_name = QString(), QObject* parent = Q_NULLPTR);
    virtual           ~ZrmConnectivity() override;
    virtual bool       set_connection_string(const QString& conn_str) override;

    void channel_send_packet   (uint16_t channel, uint8_t type, size_t data_size, const void* data = Q_NULLPTR);

    void               channels_clear        ();
    void               channel_add           ( uint16_t ch_num, zrm_work_mode_t work_mode);
    bool               channel_remove        ( uint16_t ch_num);
    bool               channel_exists        ( uint16_t ch_num) const;
    zrm_work_mode_t    channel_work_mode     ( uint16_t ch_num);
    void               channel_set_work_mode ( uint16_t ch_num, zrm_work_mode_t);

    void               channel_subscribe_param ( uint16_t ch_num, zrm_param_t param, bool add);
    void               channel_subscribe_params( uint16_t ch_num, const params_t& params, bool add);

    void               channel_query_params   ( uint16_t chan, const params_t& params);
    void               channel_query_params   (uint16_t  ch_num, const char* params, size_t psize)  ;
    void               channel_query_param    ( uint16_t chan, const zrm_param_t  param);
    void               channel_refresh_info   (uint16_t  channel);

    void               chanel_clear_changes  ( uint16_t     channel);
    zrm_cells_t        channel_cell_info     ( uint16_t     channel);
    void               channel_read_eprom_method(uint16_t     ch_num, uint8_t met_number);
    qint64             channelRespondTime(uint16_t     ch_num);

    ZrmChannelsKeys     get_changed_channels();
    ZrmChannelSharedPointer get_channel   ( uint16_t   channel) const;

    unsigned long      send_period    () const             { return m_send_period ;}
    void               set_send_period(unsigned long value);

    bool               channel_is_executing  (uint16_t ch_num) const;
    bool               channel_is_stopped  (uint16_t ch_num) const;
    bool               channel_is_paused   (uint16_t chan) const;

    void               channel_start       (uint16_t ch_num);
    void               channel_stop        (uint16_t chan);
    void               channel_pause       (uint16_t chan);
    void               channel_reset_error (uint16_t ch_num);
    void               channel_set_method  (uint16_t ch_num, const zrm_method_t& method);
    const zrm_method_t  channel_get_method  (uint16_t ch_num, bool eprom) const;
    const session_t    channel_session     (uint16_t ch_num ) const;
    void               channel_mark_changed(uint16_t chan);
    void               channel_write_param (uint16_t ch_num,  param_write_mode_t wr_mode, zrm_param_t param, const void* data, size_t sz);
    size_t             channel_write_method(uint16_t ch_num, const zrm_method_t& method, param_write_mode_t wr_mode = WM_PROCESS  );
    zrm_maskab_param_t channel_masakb_param(uint16_t ch_num);
    void               channel_set_masakb_param(uint16_t ch_num, const zrm_maskab_param_t& map);


    ZrmChannelAttributes channelAttributes(uint16_t ch_num) const ;
    bool  setChannelAttributes(uint16_t ch_num, const ZrmChannelAttributes& attrs);

    zrm::params_list_t channel_params(uint16_t channel);
    ZrmChannelsKeys     channels();
    int                channels_count();

    zrm::param_variant getParameter(uint16_t channel, zrm::zrm_param_t param);
    method_exec_results_t results_get(uint16_t channel);
    method_exec_results_sensors_t results_sensors_get(uint16_t channel);
    QString           get_stage_type_name(uint16_t ch_num, zrm::stage_type_t type);
    QString           name();
    void              set_name(const QString& cname);

    static QString      zrm_work_mode_name   ( zrm_work_mode_t  wm);
    static QString      zrm_mode_text   (uint32_t code);
    static QString      zrm_error_text  (uint32_t code);
    static QString      zrm_warning_text(uint32_t code);

    static int                 connectivity_count();
    static connectivity_list_t connectivities();
    static ZrmConnectivity*    connectivity  (int idx);
    static int                 channels_total();
    static void                start_all();
    static void                stop_all ();
    static void                make_changed() { ++m_connectivities_changed;}
    static bool                connectivities_changed() {return m_connectivities_changed;}

    static int                 read_from_json(QString path_to_file);
    static bool                write_to_json (QString path_to_file);

signals:
    // Сигнал о получении пакета каналов
    void sig_recv_packet(QByteArray packet_data);
    void sig_send_packet(QByteArray packet_data);
    // Сигнал об изменении канала
    void sig_channel_change(unsigned channel, zrm::params_list_t params_list);
    // Сигнал изменения цвета
    void sig_change_color(unsigned channel, QString color);

protected slots:
    void     sl_ping_timer ();
    void     sl_wcdg_timer ();
    void     send_next_packet();

protected:


    void    notifyRecv(const recv_header_t& recvHeader);
    virtual  void    handle_recv     (const QByteArray& recv_data) override;
    virtual  void    handle_connect  (bool connected  ) override;
    virtual  void    handle_write    (qint64 wr_bytes ) override;
    virtual  void    handle_thread_start  () override;
    //virtual  void    handle_thread_finish () override;

    virtual  ZrmChannel* create_zrm_module(uint16_t number, zrm_work_mode_t work_mode);

    void    handle_recv_channel (const recv_header_t& recv_hdr);
    int     channels_start      ();
    void    channels_stop       (bool silent = false);

    void    ping_module         (ZrmChannel* mod);

    void    on_channels_changed  ();
    void    module_state_changed (ZrmChannelSharedPointer& mod, bool* pneed_request_method);
    void    send_timer_ctrl      (bool start);
    size_t          channel_write_method (uint16_t ch_num);

    void writeToDevice(const void* data, size_t size);
    void writeToDevice(const QByteArray& data);

    void     writeToJson(QJsonObject& jobj);
    void     readFromJson (const QJsonObject& jobj);

    unsigned long   m_send_period = 0;
    int             m_currentSendChannel = 0;

    uint32_t        m_recv_kadr_number;
    recv_buffer_t   m_recv_buffer;
    QTimer          m_send_timer ;
    QTimer          m_ping_timer ;
    QTimer          m_wcdg_timer ;

    mutable QMutex  m_zrm_mutex;

    //send_buffer_t   m_send_buffer;
    ZrmChannelsMap   m_channels;//Список каналов
    ZrmChannelsKeys  m_changed_channels;
    QString          m_name;

    static bool     meta_types_inited;
    static ZrmTextMap m_mode_text   ;
    static ZrmTextMap m_error_text  ;
    static ZrmTextMap m_warning_text;
    static int      m_connectivities_changed;

private:
    static connectivity_list_t m_connectivity_list;
    static void register_connectivity  (ZrmConnectivity* instance);
    static void unregister_connectivity(ZrmConnectivity* instance);

};

} // namespace zrm

Q_DECLARE_METATYPE(zrm::params_list_t);


#endif // ZRMCONNECTIVITY_H

/*
 * Class connectivity with zrm module
 * Ostapenko D.V. NIKTES 2019-03-11
 *
 */



#ifndef ZRMCONNECTIVITY_H
#define ZRMCONNECTIVITY_H

#include "zrmproto.hpp"
#include "zrmmodule.hpp"
#include <miodevworker.h>
#include <qsharedpointer.h>
#include <qevent.h>
#include <qtimer.h>
#include <tuple>

namespace zrm {


enum channel_ctrl_t
{ctrl_request_param, ctrl_write_param };




class QChannelControlEvent : public QEvent
{
    QChannelControlEvent(): QEvent(QEvent::User) {}
public:
    explicit QChannelControlEvent(uint16_t channel, channel_ctrl_t ctrl)
        : QEvent(QEvent::User), m_control(ctrl), m_channel(channel) {}
    explicit QChannelControlEvent(channel_ctrl_t ctrl, uint16_t channel, param_write_mode_t wr_mode, zrm_param_t param, const void* data, size_t sz );
    virtual ~QChannelControlEvent() override;
    uint16_t channel() const {return m_channel;}
    uint32_t control() const {return m_control;}
    param_write_mode_t wr_mode() {return m_wr_mode;}
    zrm_param_t        param () {return m_param;}
    const QByteArray&  data  () const    {return m_data;}
    size_t             data_size() const {return size_t(m_data.size());}

private:
    uint32_t           m_control;
    uint16_t           m_channel;
    param_write_mode_t m_wr_mode;
    zrm_param_t        m_param;
    QByteArray         m_data ;

};

using    zrm_module_ptr_t =  QSharedPointer<ZrmModule>       ;
using    channels_t       =  QMap<uint16_t, zrm_module_ptr_t> ;
using    channels_key_t   =  QList<channels_t::key_type>     ;
using    idtext_t         =  QMap<uint32_t, QString>         ;

class ZrmConnectivity : public QMultioDevWorker
{
    Q_OBJECT
    Q_DISABLE_COPY(ZrmConnectivity)
public:

    typedef           QList<ZrmConnectivity*>   connectivity_list_t;

    explicit           ZrmConnectivity(const QString& conn_name = QString(), QObject* parent = Q_NULLPTR);
    virtual           ~ZrmConnectivity() override;
    virtual bool       set_connection_string(const QString& conn_str) override;

    uint16_t           session_id            ();
    void               set_session_id        (uint16_t ssid);


    size_t             send_packet           (uint16_t channel, uint8_t type, size_t data_size, const void* data = Q_NULLPTR);
    size_t             send_packet           (uint16_t channel, uint8_t type, const devproto::storage_t& data );
    size_t             send_session_start    (uint16_t channel, session_types_t session_type);
    size_t             send_session_stop     (uint16_t channel  );

    void               channels_clear        ();
    void               channel_add           ( uint16_t ch_num, zrm_work_mode_t work_mode);
    bool               channel_remove        ( uint16_t ch_num);
    bool               channel_exists        ( uint16_t ch_num);
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

    channels_key_t     get_changed_channels();
    zrm_module_ptr_t   get_channel   ( uint16_t   channel) const;

    unsigned long      send_period    () const             { return m_send_period ;}
    void               set_send_period(unsigned long value) { m_send_period = value;}

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
    int                channel_box_number(uint16_t ch_num);
    void               channel_set_box_number(uint16_t ch_num, int n);
    int                channel_device_number(uint16_t ch_num);
    void               channel_set_device_number(uint16_t ch_num, int n);
    QString            channel_color(uint16_t ch_num);
    void               channel_set_color(uint16_t ch_num, QString c);

    zrm::params_list_t channel_params(uint16_t channel);
    channels_key_t     channels();
    int                channels_count();

    QVariant          param_get(uint16_t channel, zrm::zrm_param_t param);
    QVariant          param_get(zrm::zrm_param_t param, const zrm::param_variant& pv);
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
    static QString             hms2string(const zrm::method_hms& hms);
    static zrm::method_hms     string2hms(const QString& str);

signals:
    // Сигнал о получении пакета каналов
    void sig_recv_packet(QByteArray packet_data);
    void sig_send_packet(QByteArray packet_data);
    // Сигнал об изменении канала
    void sig_channel_change(unsigned channel, zrm::params_list_t params_list);
    // Сигнал изменения цвета
    void sig_change_color(unsigned channel, QString color);

protected slots:
    void     sl_send_timer ();
    void     sl_ping_timer ();
    void     sl_wcdg_timer ();

protected:

    void    send_next_packet();
    virtual  void    handle_recv     (const QByteArray& recv_data) override;
    virtual  void    handle_connect  (bool connected  ) override;
    virtual  void    handle_write    (qint64 wr_bytes ) override;
    virtual  void    handle_thread_start  () override;
    //virtual  void    handle_thread_finish () override;
    void    recv_check_sequence(uint16_t kadr_number);

    virtual  ZrmModule* create_zrm_module(uint16_t number, zrm_work_mode_t work_mode);

    void    handle_recv_channel (const recv_header_t& recv_hdr);
    int     channels_start      ();
    void    channels_stop       (bool silent = false);

    void    ping_module         (const ZrmModule* mod);

    void    on_channels_changed  ();
    void    module_state_changed (zrm_module_ptr_t& mod, bool* pneed_request_method, bool* pneed_ping);
    void    send_timer_ctrl      (bool start);
    virtual bool    event(QEvent* ev) override;
    virtual void    channel_control_event(QChannelControlEvent* ctrl_event);
    size_t          channel_write_method (uint16_t ch_num);

    virtual void     write(QJsonObject& jobj);
    virtual void     read (const QJsonObject& jobj);


#ifndef PROTOCOL_PT_LINE
    unsigned long   m_send_period = 20;
#else
    unsigned long   m_send_period = 100;
#endif
    bool            m_enable_send = false;
    uint32_t        m_recv_kadr_number;
    recv_buffer_t   m_recv_buffer;
    QTimer          m_send_timer ;
    QTimer          m_ping_timer ;
    QTimer          m_wcdg_timer ;

    mutable QMutex  m_zrm_mutex;

    send_buffer_t   m_send_buffer;
    channels_t      m_channels;//Список каналов
    channels_key_t  m_changed_channels;
    QString         m_name;

    static bool     meta_types_inited;
    static idtext_t m_mode_text   ;
    static idtext_t m_error_text  ;
    static idtext_t m_warning_text;
    static int      m_connectivities_changed;

private:
    static connectivity_list_t m_connectivity_list;
    static void register_connectivity  (ZrmConnectivity* instance);
    static void unregister_connectivity(ZrmConnectivity* instance);

};


inline uint16_t  ZrmConnectivity::session_id    ()
{
    return m_send_buffer.session_id();
}

inline void  ZrmConnectivity::set_session_id(uint16_t ssid)
{
    m_send_buffer.set_sesion_id(ssid);
}

inline size_t    ZrmConnectivity::send_session_start         (uint16_t channel, session_types_t  session_type)
{
    uint8_t st = session_type;
    if (!session_id())
        set_session_id(555);

    return send_packet(channel, PT_CONREQ, sizeof(st), &st);
}

inline size_t    ZrmConnectivity::send_session_stop          (uint16_t channel  )
{
    return send_session_start(channel, ST_FINISH);
}

inline size_t   ZrmConnectivity::send_packet           (uint16_t channel, uint8_t type, const devproto::storage_t& data )
{
    return data.size() ? send_packet(channel, type, data.size(), &data.at(0)) : 0;
}


} // namespace zrm

Q_DECLARE_METATYPE(zrm::params_list_t);


#endif // ZRMCONNECTIVITY_H

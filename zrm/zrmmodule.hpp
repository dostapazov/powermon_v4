#ifndef ZRMMODULE_H
#define ZRMMODULE_H

#include "zrmproto.hpp"
#include <tuple>
#include <algorithm>

namespace zrm {


struct zrm_maskab_param_t
{
    double dU = .0;
    double dT = .0;
};


class ZrmModule
{
public:
// typedef  std::recursive_mutex                  mutex_t;
// typedef  std::lock_guard<mutex_t>              locker_t;

    struct Attributes
    {
        int box_number = 0;
        int device_number = 0;
        uint32_t color = 0x4682b4;
    };


    ZrmModule(uint16_t channel = uint16_t(-1), zrm_work_mode_t work_mode = zrm_work_mode_t::as_charger);
    virtual ~ZrmModule();

    virtual int      handle_recv (const zrm::recv_header_t& recv_data);
    zrm_work_mode_t work_mode   () const { return m_work_mode;}
    void     set_work_mode(zrm_work_mode_t wm) {m_work_mode = wm;}
    uint16_t channel     () const;
    void     set_channel (uint16_t channel);
    int      ping_period() const;
    void     set_ping_period(int value);
    bool     ping_check     (int timer_value);
    void     ping_reset     () const;


    session_types_t  session_request       () const;
    void      set_session_request   (session_types_t sess_type);
    session_t session               () const;
    bool      session_active        () const;
    bool      session_readonly      () const;
    void      session_reset         ();


    void     param_request_add    (zrm_param_t param);
    void     params_request_add   (const params_t&   params);
    void     param_request_remove (const zrm_param_t param);
    void     params_request_remove(const params_t&   params);
    bool     param_is_requested   (zrm_param_t param);
    const params_t&   params_list          () const;
    param_variant     param_get            (zrm_param_t param) const;
    bool     params_is_changed    (zrm_param_t param) const;
    const params_list_t& params_current     () const;

    oper_state_t  get_state(bool prev = false) const;

    const params_list_t& changes()   const;
    void          clear_changes();

    int           results_get(method_exec_results_t&) const;
    void          results_clear();

    void          method_clear();
    void          method_set  (const zrm_method_t& method);
    const zrm_method_t& method_get (bool eprom = false) const    { return eprom ? m_eprom_method : m_current_method;}
    zrm_method_t& method_get (bool eprom = false)          { return eprom ? m_eprom_method : m_current_method;}
    bool          method_is_auto() const { return m_current_method.m_method.m_id > 0 ; }

    bool          is_executing () const;
    bool          is_paused  () const;
    bool          is_stopped () const;

    const method_exec_results_t&          results_get() const ;
    const method_exec_results_sensors_t&  results_sensor_get() const ;
    zrm_cells_t         cells_get() const;
    zrm_maskab_param_t  masakb_param();
    void                set_masakb_param(const zrm_maskab_param_t& map);

    void  setAttributes(const Attributes& attrs) {m_Attributes = attrs;}
    const Attributes& getAttributes() const {return m_Attributes;}

    static uint16_t     handle_method_stages(zrm_method_t& method, uint16_t data_size, const uint8_t* beg, const uint8_t* end);
    static std::string  time_param (const param_variant& pv);
    static std::string  trect_param(const param_variant& pv);
    static std::string  fan_param(const param_variant& pv);



protected:
    void     param_set           (zrm_param_t param, const param_variant& pv);
    void     handle_conreq       (const session_t* session);
    void     handle_data         (const uint8_t*    data, size_t size);
    uint16_t handle_results      (uint16_t data_size, const uint8_t* beg, const uint8_t* end);
    uint16_t handle_results_sensor(uint16_t data_size, const uint8_t* beg, const uint8_t* end);
    uint16_t handle_cells        (uint16_t data_size, const uint8_t* beg, const uint8_t* end);
    uint16_t handle_eprom_method (uint16_t data_size, const uint8_t* beg, const uint8_t* end);
    uint16_t handle_method_stages(uint16_t data_size, const uint8_t* beg, const uint8_t* end);

protected:
//mutable mutex_t          m_mut;
    int                   m_ping_period     =  1000;
    mutable int           m_ping_timeout    =  0;
    uint16_t              m_channel         =  uint16_t(-1);
    session_types_t       m_session_request = ST_CONTROL;
    params_list_t         m_curr_params;   //Список полученных   параметров

    mutable params_list_t m_chg_params;   //Список изменившихся параметров
    params_t              m_ctrl_params;  //Список параметров которые хочется контролировать
    oper_state_t          m_prev_state;

    zrm_cells_t           m_cells;
    zrm_work_mode_t       m_work_mode = as_charger;
    zrm_maskab_param_t    m_maskab_param;

    zrm_method_t          m_eprom_method;   //Метод запрошенный из памяти устройства
    zrm_method_t          m_current_method; //Текущий метод
    method_exec_results_t m_exec_results;
    method_exec_results_sensors_t m_exec_results_sensor;

    send_buffer_t         m_SendBuffer;
    Attributes   m_Attributes;

};

//********* inline implementations

inline void   ZrmModule::ping_reset     () const
{
    m_ping_timeout = m_ping_period;
}

inline uint16_t ZrmModule::channel() const
{
    return m_channel;
}

inline void     ZrmModule::set_channel(uint16_t channel)
{
    m_channel = channel;
}

inline int      ZrmModule::ping_period() const
{
    return m_ping_period;
}

inline void     ZrmModule::set_ping_period(int value)
{
    m_ping_period = value;
}

inline session_types_t  ZrmModule::session_request    () const
{
    return m_session_request;
}

inline         void     ZrmModule::set_session_request(session_types_t sess_type)
{
    m_session_request =  sess_type == ST_FINISH ? ST_READONLY : sess_type;
}

inline const params_t&   ZrmModule::params_list         () const
{
    return m_ctrl_params;
}

inline const params_list_t&  ZrmModule::changes() const
{
    //locker_t l(m_mut);
    return m_chg_params;
}

inline void   ZrmModule::clear_changes()
{
    //locker_t l(m_mut);
    m_chg_params.clear();
}

inline bool       ZrmModule::is_executing() const
{
    return get_state().state_bits.auto_on;
}

inline bool       ZrmModule::is_paused () const
{
    return get_state().state_bits.start_pause;
}

inline bool       ZrmModule::is_stopped() const
{
    auto   st = get_state();
    return !(st.state_bits.auto_on | st.state_bits.start_pause);
}

inline const params_list_t& ZrmModule::params_current() const
{
    //locker_t l(m_mut);
    return m_curr_params;
}

inline bool      ZrmModule::session_active        () const
{
    return session().is_active();
}

inline bool      ZrmModule::session_readonly      () const
{
    return session().is_read_only();
}

inline zrm_maskab_param_t ZrmModule::masakb_param()
{
    return m_maskab_param;
}

inline void               ZrmModule::set_masakb_param(const zrm_maskab_param_t& map)
{
    m_maskab_param = map;
}



inline const method_exec_results_t& ZrmModule::results_get() const
{return  m_exec_results;}

inline const method_exec_results_sensors_t& ZrmModule::results_sensor_get() const {return  m_exec_results_sensor; }

inline bool operator == (const param_variant& pv1, const param_variant& pv2)
{
    return pv1.size == pv2.size && pv1.uqword == pv2.uqword;
}

inline bool operator != (const param_variant& pv1, const param_variant& pv2)
{
    return !(pv1 == pv2);
}



} // namespace zrm

#endif // ZRMMODULE_H

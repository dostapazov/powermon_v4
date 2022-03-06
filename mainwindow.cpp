#include <qglobal.h>
#include "mainwindow.h"
#include <crc_unit.hpp>
#include <qdatetime.h>
#include <QTextCodec>
#include <qfiledialog.h>
#include <qstylefactory.h>
#include <qactiongroup.h>
#include <qdesktopwidget.h>


#include <zrmdatasource.h>
#include <zrmmethodeditor.h>
#include <zrmconnectivityparam.h>
#include <zrmmethodchoose.h>
#include <signal_bloker.hpp>

#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qjsonarray.h>
#include <qscreen.h>
#include <ui_constraints.hpp>


enum actions_id_t {act_unknown, act_ready_view, act_zrm_view, act_method_editor, act_configure, act_style,
                  act_dev_method, act_zrm_report, act_params, act_close};
constexpr const char * const act_id_prop = "action_id";

QtMessageHandler   MainWindow::prev_msg_handler = Q_NULLPTR;
void MainWindow::msg_handler   (QtMsgType msg_type, const QMessageLogContext & msg_context, const QString & msg_text)
{
  if(prev_msg_handler)   prev_msg_handler(msg_type, msg_context, msg_text);
  #ifdef QT_DEBUG
  // TODO Add interceptors
  #endif
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    prev_msg_handler = qInstallMessageHandler(msg_handler);
    //setLocale(QLocale::C);
    setupUi(this);



    QScreen * screen = qApp->primaryScreen();
    connect(screen, &QScreen::primaryOrientationChanged, this, &MainWindow::orientation_changed);

    write_log(QtInfoMsg, "Application started");
    QCoreApplication::setApplicationVersion(QString("4.7"));
    QString wtitle = QString("%1  v:%2").arg(qApp->applicationName()).arg(qApp->applicationVersion());
#ifdef QT_DEBUG
    wtitle+= QString(" [debug version]");
#endif
    setWindowTitle(wtitle);
    QStatusBar* sb = QMainWindow::statusBar();
    sb->showMessage(wtitle);
    ZrmBaseWidget::set_codec_name(QLatin1String("CP1251"));
    ZrmDatabase::set_codec(ZrmBaseWidget::codec());
    init_actions();
    init_styles();
    read_config();

    zrm::ZrmConnectivity::read_from_json(connectivity_file_name());
    zrm::ZrmConnectivity::start_all();

    init_slots();
    install_event_filers();

    zrm_ready->update_ready();
    zrm_ready->ready_accum()->setButton(buttonReadyView);
    zrm_widget->update_ready();
    if(zrm::ZrmConnectivity::channels_total() < 2)
        actZrmView->setChecked(true);
    else
        actReadyView->setChecked (true);
    QIcon icon(QLatin1String(":/icons/icons/powermon.png"));
    setWindowIcon(icon);
    qApp->setWindowIcon(icon);
    setupActions();
    setupStyleSheet();
    update_ui();
}

MainWindow::~MainWindow()
{
  write_config();
  for(auto zrm : findChildren<ZrmBaseWidget*>())
        zrm->bind(Q_NULLPTR,0);
  zrm::ZrmConnectivity::stop_all();
  qApp->processEvents();
  for(auto c : zrm::ZrmConnectivity::connectivities())
      delete c;
}

void MainWindow::setupActions()
{
    buttonReadyView->setDefaultAction(actReadyView);
    buttonZrmView->setDefaultAction(actZrmView);
    buttonMethodEditor->setDefaultAction(actMethod_Editor);
    buttonConfigure->setDefaultAction(actConfigure);
    buttonStyle->setDefaultAction(actStyle);
    buttonExit->setDefaultAction(actExit);

    buttonDevMeth->setDefaultAction(actDevMethod);
    buttonZrmReport->setDefaultAction(actZrmReport);
    buttonParams->setDefaultAction(actParams);
}


#ifdef Q_OS_ANDROID
     void MainWindow::update_android_ui()
     {
         setFixedSize(qApp->desktop()->size());

         for(auto  && sb : findChildren<QAbstractSpinBox*>())
           sb->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);

         QSize icon_size(MAIN_WIDOW_ICON_WIDTH,MAIN_WIDOW_ICON_HEIGHT);
         QSize btn_size(MAIN_WIDOW_BUTTON_WIDTH,MAIN_WIDOW_BUTTON_HEIGHT);
         for(auto && btn : frameMenu->findChildren<QToolButton*>())
         {
            btn->setIconSize(icon_size);
            btn->setMinimumSize(btn_size);
            btn->setMaximumSize(btn_size);
            //btn->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
         }

     }
#else
    void MainWindow::update_desktop_ui()
    {

        QSize icon_size(DESKTOP_MAIN_WIDOW_ICON_WIDTH,DESKTOP_MAIN_WIDOW_ICON_WIDTH);
        QSize btn_size (DESKTOP_MAIN_WIDOW_BUTTON_WIDTH,DESKTOP_MAIN_WIDOW_BUTTON_WIDTH);
        for(auto && btn : frameMenu->findChildren<QToolButton*>())
        {
           btn->setIconSize(icon_size);
           btn->setMinimumSize(btn_size);
           btn->setMaximumSize(btn_size);
        }

    }
#endif


  void MainWindow::update_ui()
  {
    #ifdef Q_OS_ANDROID
      update_android_ui();
#else
      update_desktop_ui();
#endif
    for(auto && zrm_widget : findChildren<ZrmBaseWidget*>())
    {
      zrm_widget->update_ui();
    }
      adjustSize();
  }

void MainWindow::init_actions()
{
    m_action_grp = new QActionGroup(this);

    auto initAction = [this](QAction* act, actions_id_t id)
    {
        act->setProperty(act_id_prop, id);
        connect(act, &QAction::toggled, this, &MainWindow::action_toggled);
        m_action_grp->addAction(act);
    };
    initAction(actReadyView, act_ready_view);
    initAction(actZrmView, act_zrm_view);
    initAction(actMethod_Editor, act_method_editor);
    initAction(actConfigure, act_configure);
    initAction(actStyle, act_style);
    initAction(actDevMethod, act_dev_method);
    initAction(actZrmReport, act_zrm_report);
    initAction(actParams, act_params);
    initAction(actExit, act_close);

    m_action_grp->setExclusive(true);
    for(auto act : m_action_grp->actions())
    {
        act->setCheckable(true);
        act->setChecked(false );
    }
}

void MainWindow::init_slots  ()
{
    connect(zrm_ready, &ZrmReadyWidget::channel_activated , this, &MainWindow::channel_activated);

    connect(style_select, &QComboBox::currentTextChanged, this, &MainWindow::set_style);
    connect(font_bold, &QCheckBox::clicked, this, &MainWindow::edit_font_changed_props);
    connect(font_italic, &QCheckBox::clicked, this, &MainWindow::edit_font_changed_props);
    connect(font_size, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::edit_font_changed_props);
    connect(fontComboBox, &QFontComboBox::currentFontChanged, this, &MainWindow::edit_font_changed);

    connect(zrm_widget, &ZrmWidget::channel_activated , zrm_ready, &ZrmReadyWidget::selectChannel);

    connect(conn_params, SIGNAL(configureApply()), this, SLOT(configure_apply()));

}
void MainWindow::install_event_filers  ()
{
    for(auto sb : findChildren<QDoubleSpinBox*>())
    {
      sb->installEventFilter(this);
    }
}




void MainWindow::init_styles()
{
    QStringList stl = QStyleFactory::keys();
    foreach(QString st, stl)
    {
        style_select->addItem(st);
    }
}


void MainWindow::set_style(const QString & styleName)
{
  if (QStyleFactory::keys().contains(styleName))
  {
     qApp->setStyle(QStyleFactory::create(styleName));
  }
}



QString MainWindow::connectivity_file_name()
{
  return ZrmDataSource::config_file_name(".conn");
}

QString MainWindow::window_param_file_name()
{
 return ZrmDataSource::config_file_name("-config");
}

void MainWindow::write_log(QtMsgType msg_type, QString log_string)
{
#ifdef POWERMON_LOG
    static const char * dtfmt = "yyyy-MM-dd hh::mm:ss.zzz";
    if(!log_file.isOpen())
    {
      log_file.setFileName(qApp->applicationName()+QLatin1String(".log"));
      if(log_file.open(QFile::ReadWrite | QFile::Append ))
          {
            log_stream.setDevice(&log_file);
            write_log(QtMsgType::QtInfoMsg, " open log");
          }
          else return;
    }
  QString dts = QDateTime::currentDateTime().toString(dtfmt);
  log_stream<<dts<< QString(" level %1").arg(msg_type) <<" :> " <<log_string;
  endl(log_stream);
#else
    Q_UNUSED(msg_type)
    Q_UNUSED(log_string)
#endif
}

void MainWindow::slot_dev_error(QString error_string)
{
    write_log(QtMsgType::QtCriticalMsg, error_string);
}

void MainWindow::configure_apply()
{
    if (zrm::ZrmConnectivity::connectivities_changed())
    {
        zrm::ZrmConnectivity::write_to_json(connectivity_file_name());
        zrm::ZrmConnectivity::start_all();
        zrm_ready->update_ready();
        zrm_widget->update_ready();
    }

    QTreeWidgetItem* item = conn_params->current_item();
    zrm::ZrmConnectivity * conn_obj = nullptr;
    uint16_t cnumber = 0;
    if (item)
    {
        QTreeWidgetItem * itemConn = item->parent() ? item->parent() : item;
        if (itemConn && itemConn->treeWidget())
        {
            QVariant varConn = itemConn->data(0, Qt::UserRole);
            int64_t i = varConn.value<int64_t>();
            conn_obj = reinterpret_cast<zrm::ZrmConnectivity*>(reinterpret_cast<QObject*>(i));
        }
        QTreeWidgetItem * itemChan = item->childCount() > 0 ? item->child(0) : item;
        if (itemChan && itemChan->treeWidget())
        {
            QVariant varChan = itemChan->data(1, Qt::UserRole);
            cnumber = uint16_t(varChan.toUInt());
        }
    }
    zrm_ready->selectChannel(conn_obj, cnumber);
}

void MainWindow::style_apply()
{
     qApp->setStyle(QStyleFactory::create(style_select->currentText()));
     qApp->processEvents();
     QFont font = edit_font(fontComboBox->currentFont());
     qApp->setFont(font);
     setFont(font);
     for(auto w : findChildren<QWidget*>())
          w->setFont(font);
     layout()->update();
}


void MainWindow::set_font_for_edit()
{
  SignalBlocker sb(style_frame->findChildren<QWidget*>());
  QFontInfo     font_info = fontInfo();
  font_bold->setChecked  (font_info.bold());
  font_italic->setChecked(font_info.italic());
  font_size->setValue(font_info.pointSize());
  fontComboBox->setCurrentFont(font());
}


QFont MainWindow::edit_font(const QFont & f)
{
    QFont font    = f;
    font.setBold   (font_bold->isChecked());
    font.setItalic (font_italic->isChecked());
    font.setPointSize(font_size->value());
    return font;
}

void MainWindow::edit_font_changed_props()
{
    edit_font_changed(fontComboBox->currentFont());
    update_ui();
}

void MainWindow::edit_font_changed(const QFont & font)
{
    QFont f = edit_font(font);
    gb_ctrls->setFont(f);
    for(auto && w : gb_ctrls->findChildren<QWidget*>())
          w->setFont(f);
    gb_ctrls->layout()->update();
    update_ui();
}

constexpr const char * cfg_style        = "style";
constexpr const char * cfg_font_name    = "font-name";
constexpr const char * cfg_font_size    = "font-size";
constexpr const char * cfg_font_bold    = "font-bold";
constexpr const char * cfg_font_italic  = "font-italic";
/*constexpr const char * cfg_xpos         = "x_pos";
constexpr const char * cfg_ypos         = "y_pos";
constexpr const char * cfg_width        = "width";
constexpr const char * cfg_height       = "height";
constexpr const char * cfg_full_screen  = "full-screen";*/
constexpr const char * cfg_zrm_splitter = "zrm_splitter_sizes";
constexpr const char * cfg_params_splitter = "params_splitter_sizes";
constexpr const char * cfg_stages_splitter = "stages_splitter_sizes";

void MainWindow::write_config       ()
{
     QString cname = window_param_file_name();
     QJsonObject jobj;
     QFontInfo fi(this->font());


     jobj[cfg_style]          = style_select->currentText();
     jobj[cfg_font_name   ]   = fi.family();
     jobj[cfg_font_size   ]   = fi.pixelSize();
     jobj[cfg_font_bold   ]   = fi.bold();
     jobj[cfg_font_italic ]   = fi.italic();


     /*jobj[cfg_xpos        ]   = x();
     jobj[cfg_ypos        ]   = y();
     jobj[cfg_width       ]   = width();
     jobj[cfg_height      ]   = height();

     jobj[cfg_full_screen ]   = isFullScreen();*/

     QJsonArray jarr;
     for (int& s : zrm_widget->getSplitterSizes())
        jarr.append(s);
     jobj[cfg_zrm_splitter] = jarr;

     QJsonArray jarrParams;
     for (int& s : zrm_params->getSplitterSizes())
        jarrParams.append(s);
     jobj[cfg_params_splitter] = jarrParams;

     QJsonArray jarrStages;
     for (int& s : method_editor->getSplitterSizes())
        jarrStages.append(s);
     jobj[cfg_stages_splitter] = jarrStages;

     QFile file(cname);
     if(file.open(QFile::WriteOnly|QFile::Truncate))
     {

        QJsonDocument jdoc(jobj);
        file.write(jdoc.toJson());
        file.close();
     }
}

void MainWindow::updateFont(const QFont &fnt)
{
    qApp->setFont(fnt);
    setFont(fnt);
    for(auto w : findChildren<QWidget*>())
           w->setFont(fnt);
}

void MainWindow::read_config()
{

 QString cname = window_param_file_name();
 QFile file(cname);

 if(file.exists() && file.open(QFile::ReadOnly))
 {

     QJsonDocument jdoc = QJsonDocument::fromJson(file.readAll());
     QJsonObject jobj(jdoc.object());
     if(jobj.contains(cfg_style))
        {
         QString style_name = jobj[cfg_style].toString("Fusion");
         style_select->setCurrentText(style_name);
        }


     if(jobj.contains(cfg_font_name))
     {
       QFont  fnt(jobj[cfg_font_name].toString());
       fnt.setPixelSize(jobj[cfg_font_size].toInt( DEFAULT_FONT_SIZE));
       fnt.setBold(jobj[cfg_font_bold   ].toBool(true));
       fnt.setItalic(jobj[cfg_font_italic ].toBool(false));
       updateFont(fnt);

     }


     QJsonArray jarr = jobj[cfg_zrm_splitter].toArray();
     QList<int> list;
     for (QJsonValueRef&& v : jarr)
         list.append(v.toInt());
     zrm_widget->setSplitterSizes(list);

     QJsonArray jarrParams = jobj[cfg_params_splitter].toArray();
     QList<int> listParams;

     for (QJsonValueRef && v : jarrParams)
         listParams.append(v.toInt());
     zrm_params->setSplitterSizes(listParams);

     QJsonArray jarrStages = jobj[cfg_stages_splitter].toArray();
     QList<int> listStages;

     for (QJsonValueRef && v : jarrStages)
         listStages.append(v.toInt());
     method_editor->setSplitterSizes(listStages);

     layout()->update();
 }
 else
 {
   set_default_config();
 }
}

void MainWindow::set_default_config()
{
    QFont  fnt = this->font();
    fnt.setPixelSize(DEFAULT_FONT_SIZE);
    qDebug()<<"PixelSize " << fnt.pixelSize();
    fnt.setBold(true);
    fnt.setItalic(false);
    style_select->setCurrentText(QString("Fusion"));
    updateFont(fnt);
}


bool MainWindow::eventFilter(QObject * target,QEvent * event)
{
  if(event->type() == QEvent::KeyPress)
  {
    //Замена точки или запятой в разделителе на соответсвующий в текущей локали
    QKeyEvent * key_event = dynamic_cast<QKeyEvent*>(event);
    if(key_event )
    {
      QChar c(key_event->key());
      if((c == QChar(',') || c == QChar('.')) && c != locale().decimalPoint())
      {
        //qDebug()<<"decimal point is "<<locale().decimalPoint() << "text is "<<c;
        c = locale().decimalPoint();
        event = new QKeyEvent (key_event->type(),c.toLatin1() ,key_event->modifiers(),QString(c) );
        key_event->accept();
        qApp->postEvent(target,event);
        return true;
      }
    }
  }
 return QObject::eventFilter(target,event);
}


void MainWindow::orientation_changed(Qt::ScreenOrientation screen_orient)
{
      Q_UNUSED(screen_orient)
}

void MainWindow::action_toggled(bool checked)
{
    QObject * src = sender();
    int act_id = src ? src->property(act_id_prop).toInt() : int(actions_id_t::act_unknown);
    if (method_editor->isEdit() && act_id != act_method_editor)
    {
        // при редактировании методов не переключаемся на другие вкладки
        checked = false;
        actMethod_Editor->setChecked(true);
    }
    switch(act_id)
    {
    case act_ready_view:
        if (checked)
            stackedWidget->setCurrentWidget(zrm_ready);
        break;
    case act_zrm_view:
        if (checked)
            stackedWidget->setCurrentWidget(zrm_view);
        break;
    case act_method_editor:
        if (checked)
        {
            stackedWidget->setCurrentWidget(method_editor_page);
            method_editor->setWorkMode(zrm_ready->current_ready()->work_mode());//, false);
        }
        else
        {
            method_editor->save_user_values();
        }
        method_editor->setVisible(checked);
        break;
    case act_configure:
        if (checked)
        {
            stackedWidget->setCurrentWidget(conn_params_page);
            ZrmChannelMimimal* chan = zrm_ready->current_ready();
            if (chan)
                conn_params->setCurrentItem(chan->connectivity(), 0 /*chan->channel()*/);
        }
        else if (!method_editor->isEdit())
            configure_apply();
        break;
    case act_style:
        if (checked)
        {
            set_font_for_edit();
            stackedWidget->setCurrentWidget(style_page);
        }
        else if (!method_editor->isEdit())
            style_apply();
        break;
    case act_dev_method :
        if (checked)
        {
            stackedWidget->setCurrentWidget(zrm_devmeth);
            zrm_devmeth->updateData();
        }
        break;
    case act_zrm_report :
        if (checked)
            stackedWidget->setCurrentWidget(zrm_report);
        break;
    case act_params :
        if (checked)
            stackedWidget->setCurrentWidget(zrm_params);
        break;
    case act_close :
    {
        configure_apply();
        close();
    }
        break;
    default:
           break;
    }
}

void MainWindow::channel_activated(ZrmChannelMimimal *cm, bool bSelect)
{
    zrm::ZrmConnectivity* conn = nullptr;
    uint16_t channel = 0;
    if (cm)
    {
        conn = cm->connectivity();
        channel = cm->channel();
    }
    QStatusBar* sb = QMainWindow::statusBar();
    if (sb)
    {
        QString str = "Не выбрано";
        if (conn && channel)
        {
            auto sess = conn->channel_session(channel);
            str = cm->channel_name(channel);
            str += sess.is_active() ? QString(" [ ID %1 ]").arg(sess.session_param.ssID, 4, 16, QLatin1Char('0')).toUpper() : tr(" [ нет соединения ]");
        }
        sb->showMessage(str);
    }

    zrm_widget->bind(conn, channel);
    zrm_devmeth->bind(conn, channel);
    zrm_report->bind(conn, channel);
    zrm_params->bind(conn, channel);
    actMethod_Editor->setEnabled(conn && channel);
    actZrmView->setEnabled(conn && channel);
    if (bSelect)
        actZrmView->setChecked(true);

    bool visible = conn && channel && conn->channel_work_mode(channel) == zrm::as_charger;
    buttonZrmReport->setVisible(visible);
    if (!visible && buttonZrmReport->isChecked())
        buttonZrmView->setChecked(true);
}

void MainWindow::setupStyleSheet()
{
    QFile file(":/powermon.qss");
    QString stylesheet;
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Cannot open stylesheet file powermon.qss";
        return;
    }
    else
    {
        stylesheet = QString::fromUtf8(file.readAll());
    }
    setStyleSheet(stylesheet);
}


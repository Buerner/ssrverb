#include "fdnwindow.h"
#include "ui_fdnwindow.h"
#include <math.h>
#include <QCloseEvent>

const unsigned n_rev_sources = 8;

const float LOWEST_FREQ = 100.f;
const float HIGHEST_FREQ = 12000.f;
const float REVERB_MAX = 8.f;

FDNwindow::FDNwindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::FDNwindow),
    _dfdn( n_rev_sources )
{
    ui->setupUi(this);

    ui->t60_band1_slider->setValue( ui->t60_band1_slider->value() );
    ui->t60_band2_slider->setValue( ui->t60_band2_slider->value() );
    ui->t60_band3_slider->setValue( ui->t60_band3_slider->value() );

    _dfdn.set_new_source_callback( FDNwindow::update_source_list, ui->source_selector );
    //_dfdn.set_update_callback( FDNwindow::update_source_list, ui->source_selector );

    // Compute variables for exponatial dial.
    _b = log10f( (HIGHEST_FREQ/LOWEST_FREQ) ) / (ui->low_freq_dial->maximum() - ui->low_freq_dial->minimum() );
    _a = LOWEST_FREQ / powf( 10.f, ui->low_freq_dial->minimum() * _b );

    // Set default values of parameters.
    ui->low_freq_dial->setValue( ui->low_freq_dial->maximum() * 0.2 );
    ui->high_freq_dial->setValue( ui->high_freq_dial->maximum() * 0.6 );

    ui->t60_band1_slider->setValue( ui->t60_band1_slider->maximum() * 0.3f );
    ui->t60_band2_slider->setValue( ui->t60_band1_slider->maximum() * 0.15f );
    ui->t60_band3_slider->setValue( ui->t60_band1_slider->maximum() * 0.05f );

    ui->gain_slider->setValue( ui->gain_slider->maximum() / 2 );
    ui->mix_slider->setValue( ui->gain_slider->maximum() / 2 );


    _dfdn.set_rec_pos( SSRverb::Vector3D{2.f, 2.f, 1.7f} );
    _dfdn.set_radius(1.2);
    _dfdn.activate();
}

FDNwindow::~FDNwindow()
{
    _dfdn.deactivate();
    _dfdn.stop();
    delete ui;
}

void FDNwindow::closeEvent( QCloseEvent* event )
{
    // Disconnect from SSR if still connected.
    if ( _dfdn.is_connected() ) on_connect_button_clicked();

    _dfdn.deactivate();
    _dfdn.stop();
    ui->source_selector->clear();
}

void FDNwindow::on_connect_button_clicked()
{

    if( !_dfdn.is_connected() )
    {
        _dfdn.set_ssr_address( ui->ip_edit->text().toUtf8().data(), ui->port_edit->text().toUInt() );

        if( _dfdn.connect() )
        {
            _dfdn.run();
            _dfdn.setup_rev_sources();

            // Disable IP and port inputs
            ui->ip_edit->setDisabled( true );
            ui->port_edit->setDisabled( true );
            ui->connect_button->setText("disconnect");

            usleep(500000);
            _dfdn.connect_to_ssr();
        }
    }

    else
    {
        _dfdn.remove_rev_sources();
        _dfdn.disconnect();
        ui->source_selector->clear();


        ui->ip_edit->setDisabled( false );
        ui->port_edit->setDisabled( false );
        ui->connect_button->setText("connect");
    }

}

void FDNwindow::on_t60_band1_slider_valueChanged( int value )
{
    float t60_value = float(value) / float(ui->t60_band1_slider->maximum()) * REVERB_MAX;
    _dfdn.set_t60( t60_value, 0 );
}

void FDNwindow::on_t60_band2_slider_valueChanged( int value )
{
    float t60_value = float(value) / float(ui->t60_band2_slider->maximum()) * REVERB_MAX;
    _dfdn.set_t60( t60_value, 1 );
}

void FDNwindow::on_t60_band3_slider_valueChanged( int value )
{
    float t60_value = float(value) / float(ui->t60_band3_slider->maximum()) * REVERB_MAX;
    _dfdn.set_t60( t60_value, 2 );
}

void FDNwindow::on_low_freq_dial_valueChanged( int value )
{
    // Map values.
    float low_freq_value =  _a * powf(10.f, _b * float(value) );
    float high_freq_value = _a * powf(10.f, _b * float(ui->high_freq_dial->value()) );

    // Update label.
    ui->low_freq_label->setText( QString::number(low_freq_value) );

    // Update audio core.
    _dfdn.set_co_freqs( std::vector<float>{low_freq_value, high_freq_value} );
}

void FDNwindow::on_high_freq_dial_valueChanged(int value)
{
    // Map values.
    float high_freq_value = _a * powf(10.f, _b * float(value) );
    float low_freq_value =  _a * powf(10.f, _b * float(ui->low_freq_dial->value()) );

    // Update label.
    ui->high_freq_label->setText( QString::number(high_freq_value) );

    // Update audio core.
    _dfdn.set_co_freqs( std::vector<float>{low_freq_value, high_freq_value} );
}

void FDNwindow::on_source_selector_currentIndexChanged(int index)
{

    std::vector< unsigned short > ids = _dfdn.get_scene()->get_source_ids();

    if( index < ids.size() ) {
        ssrface::Source* source = _dfdn.get_scene()->get_source( ids[index] );

        // Set index and position in audio core.
        if ( source != nullptr )
        {
            _dfdn.set_tracked_source( ids[index], source->x, source->y );
        }
    }
}

void FDNwindow::on_source_selector_highlighted(int index)
{
    on_source_selector_currentIndexChanged( index );
}

void FDNwindow::on_gain_slider_valueChanged(int value)
{
    _dfdn.set_gain( float(value) / float(ui->gain_slider->maximum()) * 2.f );
}

void FDNwindow::on_mix_slider_valueChanged(int value)
{
    // MBs JDN reached at around 0.5 so devide by 2.
    _dfdn.set_fdn_ism_mix( float(value) / float(ui->mix_slider->maximum() ) );
}

void FDNwindow::on_room_x_box_valueChanged(double arg1)
{
    _dfdn.set_room_size( float(arg1), float(ui->room_y_box->value()), float(ui->room_z_box->value()) );
}

void FDNwindow::on_room_y_box_valueChanged(double arg1)
{
    _dfdn.set_room_size( float(ui->room_x_box->value()), float(arg1), float(ui->room_z_box->value()) );
}

void FDNwindow::on_room_z_box_valueChanged(double arg1)
{
    _dfdn.set_room_size( float(ui->room_x_box->value()), float(ui->room_y_box->value()), float(arg1) );
}

void FDNwindow::on_tracking_checkbox_stateChanged(int state )
{
    bool tracking_state;
    state == 0 ? tracking_state = false : tracking_state = true;
    _dfdn.set_tracking( tracking_state );
}



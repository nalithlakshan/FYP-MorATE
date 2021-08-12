#ifndef USB_INTERFACE_H
#define USB_INTERFACE_H

#include <QObject>
#include <QSerialPort>
#include <QByteArray>
#include <QFile>
#include <QFileInfo>
#include <QElapsedTimer>
#include <QString>
#include <QVector>
#include "databaseHandler.h"
#include <QVector>
#include <QVariant>

//Defines
//#define YASARA_MODE 1

#define  FIFO_SIZE 16384  // must be 2^N
#define FIFO_INCR(x) (((x)+1)&((FIFO_SIZE)-1))

class usb_interface : public QObject
{
Q_OBJECT
public:
    explicit usb_interface(QObject *parent = nullptr);

//    databaseHandler db; // QSqlDatabase handler object

    //ENUMs
    enum Device { pincardA, pincardB, pincardC,pincardD, pincardE, pincardF, pincardG, pincardH,
                pincardI, pincardJ, pincardK, pincardL,pincardM, pincardN, pincardO, pincardP,
                io1,io2,io3,io4,io5,io6,io7,io8,io9,io10,io11,io12,io13,io14,io15,io16,
                io17,io18,io19,io20,io21,io22,io23,io24,io25,io26,io27,io28,io29,io30,io31,io32,
                io33,io34,io35,io36,io37,io38,io39,io40,io41,io42,io43,io44,io45,io46,io47,io48,io,pincards
                };
    enum Command {set_V_VS1, set_V_VS2, set_V_VS3, set_I_VS1, set_I_VS2, set_I_VS3,
                  enable_VS1, enable_VS2, enable_VS3, disable_VS1, disable_VS2, disable_VS3,
                  VS1_pin_FB_on,VS1_pin_FB_off, VS2_pin_FB_on, VS2_pin_FB_off,
                  VS3_pin_FB_on,VS3_pin_FB_off,
                  set_load_dac, shift_config, measure_V, measure_I, new_IC_loaded, start_reading_pincards, test_over,
                  load_source_vector,read_capture_vector, execute_sourcing,reset_io_drivers, start_digital_testing,
                  set_fpga_frequency, set_io_voltage, set_source_mode, set_capture_mode, clk_bypass_mode};

public slots:
    int connectICTester();
    void sendString(QString command);
    void receiveString();
    void writeFile(QString input);
    void sendByte(const char* b, int size);
    void composeHeader(Device device, Command command, uint32_t value, uint16_t payloadSize);

    //Excecution window function headers
    void pg3_start_clicked(
            QString designator_name,
            int ID_of_1st_iteration,
            int no_of_iterations,
            QString directory_path,
            QString folder_name,
            QString operator_name,
            QString current_project,
            int taskflow_row_count,
            QVariant tableWidget_state_vr,
            QVariant tableWidget_taskname_vr
    );
    void pg3_stop_clicked();

    void waiting_till_IC_loaded();
    void electrical_testing_function_1(int tab, QString table_name, QTextStream* out);
    void electrical_testing_function_2(int tab, QString table_name, QTextStream* out);
    void electrical_testing_function_3( QTextStream* out);
    void electrical_testing_function_4(int tab, QString table_name, QTextStream* out);
    void electrical_testing_function_5();
    void electrical_testing_function_6( QTextStream* out);
    void print_header();
    QString command_text(uint8_t );

    void request_to_start_reading_pincards();
    void notify_test_over();

    //Digital_testing
    void digital_testing_function(QString current_project, QString task_name,  QTextStream* out);
    void digital_testing_function2(QString current_project, QString task_name,  QTextStream* out);
    uint8_t io_driver_deviceID(QString driver_string);


private:
    //QSerialPort Basic Variables
    QSerialPort *mainMCU;
    static const quint16 mainMCU_vendor_id = 9025; //1155
    static const quint16 mainMCU_product_id = 66; //22336
    QString mainMCU_port_name;
    bool mainMCU_is_available;
    QByteArray serialData;
    QString SerialBuffer;
    QFile file;
    QElapsedTimer myTimer;
//    QElapsedTimer testTimer;
    QString sendText;
    QString receivedText;
    qint64 timeout;
    qint64 no_of_bits_received = 0;
    bool configFlag = 0;

    QList<QString> pincard_names = {"A","B"};


    //Receiving header details
    typedef struct FIFO
        {
            uint32_t head;
            uint32_t tail;
            uint8_t data[FIFO_SIZE];
        } FIFO;
    FIFO usb_rx_fifo = {.head=0, .tail=0};
    uint8_t rx_packet[4096];

    //Packet Related Variables
    QVector<uint8_t> header = QVector<uint8_t>(10);
    QVector<uint8_t> shift_array_pincards = QVector<uint8_t>(336);

    //digital testing
    QVector<uint8_t> data_source_vector = QVector<uint8_t>(0);
    QVector<QString> data_capture_vector = QVector<QString>(48);
    int capture_vectors_count = 0;


    bool iteration_pass_flag = 1;
    int pass_count=0;
    QStringList failed_ICs;
    QStringList passed_ICs;
    QStringList all_ICs;
    bool task_pass_flag = 1;
//    int tempory_random = 0;
    int request_count = 0;


    //V_A_start_timer_dynamic_load variables
    QVector<float> voltage_source_1_V = QVector<float>(2);
    QVector<float> voltage_source_2_V = QVector<float>(2);
    QVector<float> voltage_source_3_V = QVector<float>(2);
    QVector<float> voltage_source_1_A = QVector<float>(2);
    QVector<float> voltage_source_2_A = QVector<float>(2);
    QVector<float> voltage_source_3_A = QVector<float>(2);
    QVector<float> voltage_source_1_start_timer = QVector<float>(2);
    QVector<float> voltage_source_2_start_timer = QVector<float>(2);
    QVector<float> voltage_source_3_start_timer = QVector<float>(2);
    QVector<int> voltage_source_1_V_DAC = QVector<int>(2);
    QVector<int> voltage_source_2_V_DAC = QVector<int>(2);
    QVector<int> voltage_source_3_V_DAC = QVector<int>(2);
    QVector<int> voltage_source_1_A_DAC = QVector<int>(2);
    QVector<int> voltage_source_2_A_DAC = QVector<int>(2);
    QVector<int> voltage_source_3_A_DAC = QVector<int>(2);
    QVector<int> VS1_timer_check = QVector<int>(2);
    QVector<int> VS2_timer_check = QVector<int>(2);
    QVector<int> VS3_timer_check = QVector<int>(2);
    QVector<int> sink_selected = QVector<int>(2);
    QVector<int> source_selected = QVector<int>(2);
    QVector<QString> ammeter_range = QVector<QString>(2);
    QVector<float> I_sink = QVector<float>(2);
    QVector<float> I_source = QVector<float>(2);


    //measure voltage / measure current variables
//    QVector<int> check_box_measure_voltage = QVector<int>(2);
//    QVector<int> check_box_measure_current = QVector<int>(2);
    QVector<QVector<int>> dynamic_load_selected = QVector<QVector<int>>(16);
    QVector<QVector<int>> measure_voltage_checked = QVector<QVector<int>>(16);
    QVector<QVector<int>> measure_current_checked = QVector<QVector<int>>(16);
    QVector<QVector<float>> V_min_value = QVector<QVector<float>>(16);
    QVector<QVector<float>> V_max_value = QVector<QVector<float>>(16);
    QVector<QVector<float>> A_min_value = QVector<QVector<float>>(16);
    QVector<QVector<float>> A_max_value = QVector<QVector<float>>(16);






signals:
    void sendError(QString message);
    void sendWarning(QString message);
    void sendInfo(QString message);
    void send_current_iteration(QString);
    void send_status_icon_update_row(int, int);
    void send_summary_activation_signal(QStringList,QStringList,QStringList);
    void send_print_to_review_text(QString );
};

#endif // USB_INTERFACE_H

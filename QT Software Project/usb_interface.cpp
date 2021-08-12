#include "usb_interface.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QtWidgets>
#include <QMessageBox>


usb_interface::usb_interface(QObject *parent) : QObject(parent)
{

}

//#####################################################################################################
int usb_interface::connectICTester()
{
    if(configFlag == 0){
        mainMCU_port_name = "";
        mainMCU = new QSerialPort;
        file.setFileName("OutputData.txt");
        configFlag = 1;
    }
    mainMCU_is_available = false;
    if(mainMCU->isOpen()){
            mainMCU->close();
    }

    foreach(const QSerialPortInfo &port, QSerialPortInfo::availablePorts())
    {
        if(port.hasVendorIdentifier() && port.hasProductIdentifier())
        {
            if(port.vendorIdentifier() == 1155){
                mainMCU_port_name = port.portName();
                mainMCU_is_available = true;
            }
        }
    }
    if(mainMCU_is_available)
    {
        //Open and configure the serial port
        mainMCU->setPortName(mainMCU_port_name);
        mainMCU->open(QSerialPort::ReadWrite);
        mainMCU->setDataBits(QSerialPort::Data8);
        mainMCU->setParity(QSerialPort::NoParity);
        mainMCU->setStopBits(QSerialPort::OneStop);
        mainMCU->setFlowControl(QSerialPort::NoFlowControl);
        mainMCU->flush();
        return 1;
     }
    else{
        emit sendError("IC Tester is not connected!!!");
        return 0;
    }
}


void usb_interface::writeFile( QString input)
{
    QTextStream out(&file);
    QString text = input;
    out << text;
    file.flush();
}


void usb_interface::sendByte(const char* b, int size)
{
    if(mainMCU->isWritable())
    {
        mainMCU->write(b,size);
        mainMCU->waitForBytesWritten(-1);
    }
    else
    {
        qDebug()<<"Couldn't write to serial!";
    }
}


void usb_interface::sendString(QString command)
{
    int it = command.size()/255;
    int last = command.size()-it*255;
    QString str;
    if(mainMCU->isWritable())
    {
        for(int i =0; i<it; i++){
            str = command.mid(255*i,255);
            mainMCU->write(str.toStdString().c_str());
            mainMCU->waitForBytesWritten(-1);
        }
        if(last > 0){
          str = command.mid(255*it,last);
          mainMCU->write(str.toStdString().c_str());
          mainMCU->waitForBytesWritten(-1);
        }
    }
    else
    {
        qDebug()<<"Couldn't write to serial!";
    }
}


void usb_interface::receiveString()
{
    serialData = mainMCU->readAll();
    SerialBuffer = QString::fromStdString(serialData.toStdString());
    serialData.clear();
    qDebug() << SerialBuffer<<Qt::endl;
    for(int i=0; i<SerialBuffer.length();i++)
    {
        receivedText.append(SerialBuffer.at(i));
        no_of_bits_received += 1;
    }
}

//#####################################################################################################

void usb_interface::composeHeader(Device device, Command command, uint32_t value, uint16_t payloadSize){
    header[0] = 'S';
    header[1] = device;
    header[2] = command;

    //unsigned long val = *(unsigned long *)&value; //if value is float
    header[3] = (char)(value >> 24);
    header[4] = (char)(value >> 16);
    header[5] = (char)(value >> 8);
    header[6] = (char)value;

    header[7] = (char)(payloadSize >> 8);
    header[8] = (char)payloadSize;

    header[9] = 'E';

    /*-----------------------------------------------------------------------
       How to convert a array with a recorded float to a float value again:
                      unsigned long g;
                      g =  (array[0] << 24) | (array[1] << 16)
                          | (array[2] << 8) | (array[3]);
                      float revalue = *(float *)&g;
    ------------------------------------------------------------------------*/
}

//**************************** Execution window ******************************

void usb_interface::pg3_start_clicked(
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
)
{
    if(connectICTester())
    {

        QVector<int>tableWidget_state = tableWidget_state_vr.value<QVector<int>>();
        QVector<QString>tableWidget_taskname = tableWidget_taskname_vr.value<QVector<QString>>();

        databaseHandler db; // QSqlDatabase handler object
        db.connopen();

        qDebug()<<"usb thread: "<<QThread::currentThread();
        db.query_execution("BEGIN TRANSACTION", "transaction was not begun");

        //Initial states
        failed_ICs.clear();
        passed_ICs.clear();
        all_ICs.clear();
        pass_count = 0;
        for(int x=0; x<336; x++){
           shift_array_pincards[x]=0;
        }

        //LOOP AMONG ITERATIONS AKA ICs
        for (int i = ID_of_1st_iteration;i<ID_of_1st_iteration+no_of_iterations;i++ )
        {
            auto iteration_starting_time = std::chrono::high_resolution_clock::now();

            send_print_to_review_text("Tester Ready!, Load the IC and press the button!");
            waiting_till_IC_loaded();


            for (int task =0; task<taskflow_row_count;task++){
                emit send_status_icon_update_row(task, -1);
            }

            iteration_pass_flag = 1;
            emit send_current_iteration(QString::number(i));

            send_print_to_review_text("iteration no: " + QString::number(i));

            QFile file(directory_path+"/"+folder_name+"/"+designator_name+QString::number(i)+".txt");

            if(!file.open(QFile::WriteOnly | QFile::Text)){
                emit sendWarning("File not open");
            }
            QTextStream out(&file);
            //PRINTS REPORT HEADER TO FILE
            out << "#######################MorATE TEST REPORT##############################" << Qt::endl;
            out << "  Project  : " << current_project << Qt::endl;
            out << "  Operator : " << operator_name <<Qt::endl;
            out << "  Date     : " << QDate::currentDate().toString()<<Qt::endl;
            out << "  Time     : "<<QTime::currentTime().toString()<<Qt::endl;
            out << "###################################################################" << Qt::endl;

            send_print_to_review_text("  Project  : " + current_project);
            send_print_to_review_text("  Operator : " + operator_name);
            send_print_to_review_text("  Date     : " + QDate::currentDate().toString());
            send_print_to_review_text("  Time     : " + QTime::currentTime().toString());

            //LOOP AMONG TASKS IN TASK FLOW
            for(int i = 0; i < taskflow_row_count; ++i)
            {                
                task_pass_flag = 1;

                if(tableWidget_state[i]){
                    auto task_starting_time = std::chrono::high_resolution_clock::now();

                    out<<Qt::endl;
                    out << "-------------------------------------------------------------------" << Qt::endl;
                    out<<"Task Name: "<<tableWidget_taskname[i]<<Qt::endl; //PRINTS TASK NAMES TO FILE
                    out << "-------------------------------------------------------------------" << Qt::endl;

                    send_print_to_review_text("\n Current task: "+ tableWidget_taskname[i] + "\n");

                    dynamic_load_selected = QVector<QVector<int>>(16);
                    measure_voltage_checked = QVector<QVector<int>>(16);
                    measure_current_checked = QVector<QVector<int>>(16);
                    V_min_value = QVector<QVector<float>>(16);
                    V_max_value = QVector<QVector<float>>(16);
                    A_min_value = QVector<QVector<float>>(16);
                    A_max_value = QVector<QVector<float>>(16);


                    for (int i=0; i<16; i++){
                        measure_voltage_checked[i].resize(2);
                        measure_current_checked[i].resize(2);
                        V_min_value[i].resize(2);
                        V_max_value[i].resize(2);
                        A_min_value[i].resize(2);
                        A_max_value[i].resize(2);
                        dynamic_load_selected[i].resize(2);
                    }

                    //LOOP AMONG TABS (PIN CARDS) FROM A->P
                    QStringList tab_list={"A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P"};
                    for(int pin_card = 0; pin_card < 2; pin_card++)
                    {
                        QString table_name = current_project+"/"+tableWidget_taskname[i]+"/"+tab_list[pin_card];

                        electrical_testing_function_1(pin_card,table_name, &out);
                    }
                    for(int pin_card = 0; pin_card < 2; pin_card++)
                    {
                        QString table_name = current_project+"/"+tableWidget_taskname[i]+"/"+tab_list[pin_card];
                        electrical_testing_function_2(pin_card,table_name, &out);
                    }

                    electrical_testing_function_3( &out);


                    for(int pin_card = 0; pin_card < 2; pin_card++)
                    {
                        QString table_name = current_project+"/"+tableWidget_taskname[i]+"/"+tab_list[pin_card];
                        electrical_testing_function_4(pin_card,table_name, &out);

                    }
                    electrical_testing_function_5();
                    request_to_start_reading_pincards();
                    electrical_testing_function_6( &out);


              //DIGITAL TESTING PHASE
                    QSqlQuery query;
                    QString table_name = current_project+"/"+tableWidget_taskname[i]+"/Digital IO";
                    int digital_testing_enable_checkbox = 0;
                    query.prepare("select digital_testing_enabled from '"+table_name+"' where ID = '"+QString::number(1)+"' ");
                    if(query.exec())
                    {
                        if (query.next()) {
                            digital_testing_enable_checkbox = query.value(0).toInt();
                        }
                    }
                    else{
                        qDebug()<<"query for digital testing function didnt excecute";
                    }
                    if(digital_testing_enable_checkbox == 1){
                        digital_testing_function(current_project,tableWidget_taskname[i], &out);
                        digital_testing_function2(current_project,tableWidget_taskname[i], &out);
                        mainMCU->flush();
                    }


                    if(task_pass_flag == 1){
                        emit send_status_icon_update_row(i, 1);

                    }
                    else if(task_pass_flag == 0){
                        iteration_pass_flag = 0;
                        emit send_status_icon_update_row(i, 0);

                    }
                    auto task_end_time = std::chrono::high_resolution_clock::now();
                    int task_elapsed_time_ms = std::chrono::duration<double, std::milli>(task_end_time-task_starting_time).count();
                    out<<"Task Duration : "<< task_elapsed_time_ms << "ms" <<Qt::endl;
                    send_print_to_review_text("Task Duration : " + QString::number(task_elapsed_time_ms));
                }


             }
            all_ICs.append(designator_name+QString::number(i));
            if(iteration_pass_flag==1){
                //PRINTS REPORT FOOTER TO FILE
                out << "###################################################################" << Qt::endl;
                out << "  Overall result  : PASS" << Qt::endl;
                out << "###################################################################" << Qt::endl;
                pass_count +=1;
                passed_ICs.append(designator_name+QString::number(i));
            }
            else{
                out << "###################################################################" << Qt::endl;
                out << "  Overall result  : FAIL" << Qt::endl;
                out << "###################################################################" << Qt::endl;
                failed_ICs.append(designator_name+QString::number(i));
            }
            auto iteration_end_time = std::chrono::high_resolution_clock::now();
            int elapsed_time_ms = std::chrono::duration<double, std::milli>(iteration_end_time-iteration_starting_time).count();

            out << "Total Testing Time : " << elapsed_time_ms << "ms" << Qt::endl;
            file.flush();
            file.close();

            //Sleep(1000);

            send_print_to_review_text("Testing the IC is done!!!");
            notify_test_over();


        }
        QFile file(directory_path+"/"+folder_name+"/"+"SUMMARY.txt");

        if(!file.open(QFile::WriteOnly | QFile::Text)){
            emit sendWarning("File not open");
        }
        QTextStream out(&file);
        //PRINTS REPORT SUMMARY TO FILE
        out << "##################MorATE TEST REPORT SUMMARY#########################" << Qt::endl;
        out << "  Project  : " << current_project << Qt::endl;
        out << "  Operator : " << operator_name <<Qt::endl;
        out << "  Date     : " << QDate::currentDate().toString()<<Qt::endl;
        out << "  Time     : "<<QTime::currentTime().toString()<<Qt::endl;
        out << "#####################################################################" << Qt::endl;
        out << Qt::endl;
        out << "  Total ICs tested     : "<< no_of_iterations <<Qt::endl;
        out << "ID of the starting IC  : "<<ID_of_1st_iteration<<Qt::endl;
        out << "ID of the last IC      : "<<ID_of_1st_iteration+no_of_iterations-1<<Qt::endl;
        out << "  No. of ICs passed    : "<< pass_count <<Qt::endl;
        out << "  No. of ICs failed    : "<< no_of_iterations - pass_count <<Qt::endl;
        out << Qt::endl;
        out << "  List of failed ICs   : "<<Qt::endl;

        for(int i=0;i<failed_ICs.length();i++){
            out << failed_ICs[i]<<Qt::endl;
        }
        out << "##########################################################" << Qt::endl;

        file.flush();
        file.close();

        db.query_execution("COMMIT", "commit unsuccessful");
        db.connclose();
        emit sendInfo("reports are successfully generated as text files");
        emit send_summary_activation_signal(failed_ICs,passed_ICs,all_ICs);
        if(mainMCU->isOpen()){
                mainMCU->close();
        }

    }

}


void usb_interface::electrical_testing_function_1(int tab, QString table_name, QTextStream* out)
{
    shift_array_pincards[21*tab+0] = 0;
    shift_array_pincards[21*tab+1] = 0;
    shift_array_pincards[21*tab+2] = 0;
    shift_array_pincards[21*tab+11] = 0;
    shift_array_pincards[21*tab+12] = 0;

    //LOOP AMONG THE PINS 1->16
    for(int pin_no = 1; pin_no < 17; pin_no++)
    {
        QSqlQuery query;
        QString row = "A" + QString::number(pin_no);
        query.prepare("select radio_button_column from '"+table_name+"' where row_name = '"+row+"' ");
        if(query.exec())
        {
            if (query.next()) {
                int radio_button_column_value = query.value(0).toInt();

                //******************************* basic pin connections setting ******************************
                int index = 0;
                if(pin_no<9){
                    index = tab*21 + 21 - pin_no;
                }
                else{
                    index = tab*21 + 19 - pin_no;
                }
                shift_array_pincards[index] = 0;
                QMap<int,int> map_rb;
                map_rb[1] = 6;
                map_rb[2] = 5;
                map_rb[3] = 4;
                map_rb[5] = 7;
                if((radio_button_column_value != 4)&&(radio_button_column_value != 6)){
                    shift_array_pincards[index] |= 1<<map_rb[radio_button_column_value];
                    shift_array_pincards[index] |= 0b00000100;
                }
                else if(radio_button_column_value == 4){
                    dynamic_load_selected[pin_no-1][tab] = 1;
                }
                else{
                    dynamic_load_selected[pin_no-1][tab] = 0;
                }

            }
        }
        else{
            qDebug()<<"query not selected from DB";
        }
    }
    qDebug()<<"dynamid load: "<<dynamic_load_selected;
    //shift config packet sending
    Device pinCard = static_cast<Device>(tab);
    composeHeader(pinCard,shift_config,0,21);
    sendByte((const char *)&header[0], 10);
    sendByte((const char *)&shift_array_pincards[21*tab], 21);
    print_header();

}

















void usb_interface::electrical_testing_function_2(int tab, QString table_name, QTextStream* out)
{

    //feedback variables
    int voltage_source_1_FB;
    int voltage_source_2_FB;
    int voltage_source_3_FB;

            
    QSqlQuery query;
    query.prepare("select V_A_start_timer_dynamic_load_value from '"+table_name+"' where ID between "+QString::number(17)+" and "+QString::number(25)+" or ID="+QString::number(29)+" or ID="+QString::number(30)+" ");
    QList<float> tab_header_data_list;
    if(query.exec())
    {
        while (query.next()) {
            tab_header_data_list << query.value(0).toFloat();
        }
        voltage_source_1_V[tab] = tab_header_data_list[0];
        voltage_source_2_V[tab] = tab_header_data_list[1];
        voltage_source_3_V[tab] = tab_header_data_list[2];
        voltage_source_1_A[tab] = tab_header_data_list[3];
        voltage_source_2_A[tab] = tab_header_data_list[4];
        voltage_source_3_A[tab] = tab_header_data_list[5];
        voltage_source_1_start_timer[tab] = tab_header_data_list[6];
        voltage_source_2_start_timer[tab] = tab_header_data_list[7];
        voltage_source_3_start_timer[tab] = tab_header_data_list[8];
        I_sink[tab] = tab_header_data_list[9];
        I_source[tab] = tab_header_data_list[10];

    }
    else
        qDebug()<<"query not selected from DB";

    QSqlQuery query_FB;
    query_FB.prepare("select FB from '"+table_name+"' where ID between "+QString::number(26)+" and "+QString::number(28)+" or ID ="+QString::number(33)+" ");
    QStringList tab_VS_FeedBack_data_list;
    if(query_FB.exec())
    {
        while (query_FB.next()) {
            tab_VS_FeedBack_data_list << query_FB.value(0).toString();
        }
        voltage_source_1_FB = tab_VS_FeedBack_data_list[0].mid(1).toInt();
        voltage_source_2_FB = tab_VS_FeedBack_data_list[1].mid(1).toInt();
        voltage_source_3_FB = tab_VS_FeedBack_data_list[2].mid(1).toInt();
        ammeter_range[tab]  = tab_VS_FeedBack_data_list[3];

    }
    else
        qDebug()<<"query not selected from DB";

    QSqlQuery query_sink_source;
    query_sink_source.prepare("select sink_source from '"+table_name+"' where ID = "+QString::number(31)+" or ID = "+QString::number(32)+" ");
    QList<int> tab_sink_source_data_list;
    if(query_sink_source.exec())
    {
        while (query_sink_source.next()) {
            tab_sink_source_data_list << query_sink_source.value(0).toInt();
        }
        sink_selected[tab] = tab_sink_source_data_list[0];
        source_selected[tab] = tab_sink_source_data_list[1];


    }
    else
        qDebug()<<"query not selected from DB";

    //*********************************** sources configuration *****************************************
    Device pinCard = static_cast<Device>(tab);

    //::::::::setting feedbacks::::::::
    int A =0, B =0, C =0, ones =0, zeros =0; //variables used to set A,B,C input bits of the muxes

    //FB pin for VS1
    if(voltage_source_1_FB==0){
        shift_array_pincards[1 + 21*tab] &= 0b11111110;
        shift_array_pincards[11 + 21*tab]&= 0b11111110;
        composeHeader(pinCard,VS1_pin_FB_off,0,0);
        sendByte((const char *)&header[0], 10);
        print_header();

    }
    else if(voltage_source_1_FB<9){
        shift_array_pincards[11 + 21*tab] |= 0b00000001;
        shift_array_pincards[1 + 21*tab]  &= 0b11111110;
        composeHeader(pinCard,VS1_pin_FB_on,0,0);
        sendByte((const char *)&header[0], 10);
        print_header();

        switch (voltage_source_1_FB)
        {
        case 1: C=1; B=1; A=1;break;
        case 3: C=1; B=1; A=0;break;
        case 4: C=1; B=0; A=1;break;
        case 2: C=1; B=0; A=0;break;
        case 7: C=0; B=1; A=1;break;
        case 5: C=0; B=1; A=0;break;
        case 8: C=0; B=0; A=1;break;
        case 6: C=0; B=0; A=0;break;
        }
        ones  = 0;
        zeros = 0;
        ones  = (C<<3|B<<2|A<<1);
        zeros = ~((!C)<<3|(!B)<<2|(!A)<<1);
        shift_array_pincards[12 + 21*tab] |= ones;
        shift_array_pincards[12 + 21*tab] &= zeros;
    }
    else{
        shift_array_pincards[11 + 21*tab] &= 0b11111110;
        shift_array_pincards[1 + 21*tab]  |= 0b00000001;
        composeHeader(pinCard,VS1_pin_FB_on,0,0);
        sendByte((const char *)&header[0], 10);
        print_header();
        switch (voltage_source_1_FB)
        {
        case 9 : C=1; B=1; A=1;break;
        case 11: C=1; B=1; A=0;break;
        case 12: C=1; B=0; A=1;break;
        case 10: C=1; B=0; A=0;break;
        case 15: C=0; B=1; A=1;break;
        case 13: C=0; B=1; A=0;break;
        case 16: C=0; B=0; A=1;break;
        case 14: C=0; B=0; A=0;break;
        }
        ones  = 0;
        zeros = 0;
        ones  = (C<<3|B<<2|A<<1);
        zeros = ~((!C)<<3|(!B)<<2|(!A)<<1);
        shift_array_pincards[2 + 21*tab] |= ones;
        shift_array_pincards[2 + 21*tab] &= zeros;
    }

    //FB pin for VS2
    if(voltage_source_2_FB==0){
        shift_array_pincards[11 + 21*tab]&= 0b11101111;
        shift_array_pincards[1 + 21*tab] &= 0b11101111;
        composeHeader(pinCard,VS2_pin_FB_off,0,0);
        sendByte((const char *)&header[0], 10);
        print_header();
    }
    else if(voltage_source_2_FB<9){
        shift_array_pincards[11 + 21*tab] |= 0b00010000;
        shift_array_pincards[1 + 21*tab]  &= 0b11101111;
        composeHeader(pinCard,VS2_pin_FB_on,0,0); 
        sendByte((const char *)&header[0], 10);   
        print_header();
        switch (voltage_source_2_FB)
        {
        case 4: C=1; B=1; A=1;break;
        case 2: C=1; B=1; A=0;break;
        case 3: C=1; B=0; A=1;break;
        case 1: C=1; B=0; A=0;break;
        case 8: C=0; B=1; A=1;break;
        case 6: C=0; B=1; A=0;break;
        case 5: C=0; B=0; A=1;break;
        case 7: C=0; B=0; A=0;break;
        }
        ones  = 0;
        zeros = 0;
        ones  = (C<<1|B<<2|A<<3);
        zeros = ~((!C)<<1|(!B)<<2|(!A)<<3);
        shift_array_pincards[11 + 21*tab] |= ones;
        shift_array_pincards[11 + 21*tab] &= zeros;

    }
    else{
        shift_array_pincards[11 + 21*tab] &= 0b11101111;
        shift_array_pincards[1 + 21*tab]  |= 0b00010000;
        composeHeader(pinCard,VS2_pin_FB_on,0,0);
        sendByte((const char *)&header[0], 10);
        print_header();
        switch (voltage_source_2_FB)
        {
        case 12: C=1; B=1; A=1;break;
        case 10: C=1; B=1; A=0;break;
        case 11: C=1; B=0; A=1;break;
        case 9 : C=1; B=0; A=0;break;
        case 16: C=0; B=1; A=1;break;
        case 14: C=0; B=1; A=0;break;
        case 13: C=0; B=0; A=1;break;
        case 15: C=0; B=0; A=0;break;
        }
        ones  = 0;
        zeros = 0;
        ones  = (C<<1|B<<2|A<<3);
        zeros = ~((!C)<<1|(!B)<<2|(!A)<<3);
        shift_array_pincards[1 + 21*tab] |= ones;
        shift_array_pincards[1 + 21*tab] &= zeros;
    }

    //FB pin for VS3
    if(voltage_source_3_FB==0){
        shift_array_pincards[12 + 21*tab] &= 0b11101111;
        shift_array_pincards[2 + 21*tab]  &= 0b11101111;
        composeHeader(pinCard,VS3_pin_FB_off,0,0);
        sendByte((const char *)&header[0], 10);
        print_header();
    }
    else if(voltage_source_3_FB<9){
        shift_array_pincards[12 + 21*tab] |= 0b00010000;
        shift_array_pincards[2 + 21*tab]  &= 0b11101111;
        composeHeader(pinCard,VS3_pin_FB_on,0,0);
        sendByte((const char *)&header[0], 10);
        print_header();
        switch (voltage_source_3_FB)
        {
        case 4: C=1; B=1; A=1;break;
        case 2: C=1; B=1; A=0;break;
        case 1: C=1; B=0; A=1;break;
        case 3: C=1; B=0; A=0;break;
        case 8: C=0; B=1; A=1;break;
        case 5: C=0; B=1; A=0;break;
        case 7: C=0; B=0; A=1;break;
        case 6: C=0; B=0; A=0;break;
        }
        ones  = 0;
        zeros = 0;
        ones  = (C<<0|B<<6|A<<5);
        zeros = ~((!C)<<0|(!B)<<6|(!A)<<5);
        shift_array_pincards[12 + 21*tab] |= ones;
        shift_array_pincards[12 + 21*tab] &= zeros;
    }
    else{
        shift_array_pincards[12 + 21*tab] &= 0b11101111;
        shift_array_pincards[2 + 21*tab]  |= 0b00010000;
        composeHeader(pinCard,VS3_pin_FB_on,0,0);
        sendByte((const char *)&header[0], 10);
        print_header();
        switch (voltage_source_3_FB)
        {
        case 12: C=1; B=1; A=1;break;
        case 10: C=1; B=1; A=0;break;
        case 9 : C=1; B=0; A=1;break;
        case 11: C=1; B=0; A=0;break;
        case 16: C=0; B=1; A=1;break;
        case 13: C=0; B=1; A=0;break;
        case 15: C=0; B=0; A=1;break;
        case 14: C=0; B=0; A=0;break;
        }
        ones  = 0;
        zeros = 0;
        ones  = (C<<0|B<<6|A<<5);
        zeros = ~((!C)<<0|(!B)<<6|(!A)<<5);
        shift_array_pincards[2 + 21*tab] |= ones;
        shift_array_pincards[2 + 21*tab] &= zeros;
    }

    composeHeader(pinCard,shift_config,0,21);
    sendByte((const char *)&header[0], 10);
    sendByte((const char *)&shift_array_pincards[21*tab], 21);
    print_header();
}


void usb_interface::electrical_testing_function_3( QTextStream* out)
{
    for(int tab = 0;tab<2;tab++){
        Device pinCard = static_cast<Device>(tab);

        //::::::::setting V,I DAC values::::::::
        /*------------------------------------------------------------
        |          BEGIN OFFUNCTIONS TO CALCULATE DAC VALUES         |
        ------------------------------------------------------------*/
           voltage_source_1_V_DAC[tab] = 1000 * voltage_source_1_V[tab] ;
           voltage_source_2_V_DAC[tab] = 1000 * voltage_source_2_V[tab] ;
           voltage_source_3_V_DAC[tab] = 1000 * voltage_source_3_V[tab] ;
           voltage_source_1_A_DAC[tab] = 1000 * voltage_source_1_A[tab] ;
           voltage_source_2_A_DAC[tab] = 1000 * voltage_source_2_A[tab] ;
           voltage_source_3_A_DAC[tab] = 1000 * voltage_source_3_A[tab] ;
        /*------------------------------------------------------------
        |          END OF FUNCTIONS TO CALCULATE DAC VALUES          |
        ------------------------------------------------------------*/

        VS1_timer_check[tab]= 0;
        VS2_timer_check[tab]= 0;
        VS3_timer_check[tab]= 0;

        //VS1
        if(voltage_source_1_V[tab] == -1){
            composeHeader(pinCard,disable_VS1,0,0);
            sendByte((const char *)&header[0], 10);
            print_header();
        }
        else if(voltage_source_1_start_timer[tab] == 0){
            composeHeader(pinCard,set_V_VS1,voltage_source_1_V_DAC[tab],0);
            sendByte((const char *)&header[0], 10);
            print_header();

            composeHeader(pinCard,set_I_VS1,voltage_source_1_A_DAC[tab],0);
            sendByte((const char *)&header[0], 10);
            print_header();

            composeHeader(pinCard,enable_VS1,0,0);
            sendByte((const char *)&header[0], 10);
            print_header();
        }
        else{
            VS1_timer_check[tab] = 1;
        }

        //VS2
        if(voltage_source_2_V[tab] == -1){
            composeHeader(pinCard,disable_VS2,0,0);
            sendByte((const char *)&header[0], 10);
            print_header();
        }
        else if(voltage_source_2_start_timer[tab] == 0){
            composeHeader(pinCard,set_V_VS2,voltage_source_2_V_DAC[tab],0);
            sendByte((const char *)&header[0], 10);
            print_header();
            composeHeader(pinCard,set_I_VS2,voltage_source_2_A_DAC[tab],0);
            sendByte((const char *)&header[0], 10);
            print_header();
            composeHeader(pinCard,enable_VS2,0,0);
            sendByte((const char *)&header[0], 10);
            print_header();
        }
        else{
            VS2_timer_check[tab] = 1;
        }

        //VS3
        if(voltage_source_3_V[tab] == -1){
            composeHeader(pinCard,disable_VS3,0,0);
            sendByte((const char *)&header[0], 10);
            print_header();
        }
        else if(voltage_source_3_start_timer[tab] == 0){
            composeHeader(pinCard,set_V_VS3,voltage_source_3_V_DAC[tab],0);
            sendByte((const char *)&header[0], 10);
            print_header();
            composeHeader(pinCard,set_I_VS3,voltage_source_3_A_DAC[tab],0);
            sendByte((const char *)&header[0], 10);
            print_header();
            composeHeader(pinCard,enable_VS3,0,0);
            sendByte((const char *)&header[0], 10);
            print_header();
        }
        else{
            VS3_timer_check[tab] = 1;
        }


    }

    //::::::::Start Timers to Enable::::::::
    //timer start and send 3 enables at respective times

    myTimer.start();

    Device pinCard_A = static_cast<Device>(0);
    Device pinCard_B = static_cast<Device>(1);

    while((VS1_timer_check[0] == 1)||(VS2_timer_check[0] == 1)||(VS3_timer_check[0] == 1)||
          (VS1_timer_check[1] == 1)||(VS2_timer_check[1] == 1)||(VS3_timer_check[1] == 1))
    {
        //VS1 timer check
        if((VS1_timer_check[0] == 1) && (voltage_source_1_start_timer[0] <= myTimer.elapsed())){
            composeHeader(pinCard_A,set_V_VS1,voltage_source_1_V_DAC[0],0);
            sendByte((const char *)&header[0], 10);
            print_header();
            composeHeader(pinCard_A,set_I_VS1,voltage_source_1_A_DAC[0],0);
            sendByte((const char *)&header[0], 10);
            print_header();
            composeHeader(pinCard_A,enable_VS1,0,0);
            sendByte((const char *)&header[0], 10);
            print_header();
            VS1_timer_check[0] = 0;
        }

        if((VS1_timer_check[1] == 1) && (voltage_source_1_start_timer[1] <= myTimer.elapsed())){
            composeHeader(pinCard_B,set_V_VS1,voltage_source_1_V_DAC[1],0);
            sendByte((const char *)&header[0], 10);
            print_header();
            composeHeader(pinCard_B,set_I_VS1,voltage_source_1_A_DAC[1],0);
            sendByte((const char *)&header[0], 10);
            print_header();
            composeHeader(pinCard_B,enable_VS1,0,0);
            sendByte((const char *)&header[0], 10);
            print_header();
            VS1_timer_check[1] = 0;
        }

        //VS2 timer check
        if((VS2_timer_check[0] == 1) && (voltage_source_2_start_timer[0] <= myTimer.elapsed())){
            composeHeader(pinCard_A,set_V_VS2,voltage_source_2_V_DAC[0],0);
            sendByte((const char *)&header[0], 10);
            print_header();
            composeHeader(pinCard_A,set_I_VS2,voltage_source_2_A_DAC[0],0);
            sendByte((const char *)&header[0], 10);
            print_header();
            composeHeader(pinCard_A,enable_VS2,0,0);
            sendByte((const char *)&header[0], 10);
            print_header();
            VS2_timer_check[0] = 0;
        }

        if((VS2_timer_check[1] == 1) && (voltage_source_2_start_timer[1] <= myTimer.elapsed())){
            composeHeader(pinCard_B,set_V_VS2,voltage_source_2_V_DAC[1],0);
            sendByte((const char *)&header[0], 10);
            print_header();
            composeHeader(pinCard_B,set_I_VS2,voltage_source_2_A_DAC[1],0);
            sendByte((const char *)&header[0], 10);
            print_header();
            composeHeader(pinCard_B,enable_VS2,0,0);
            sendByte((const char *)&header[0], 10);
            print_header();
            VS2_timer_check[1] = 0;
        }

        //VS3 timer check
        if((VS3_timer_check[0] == 1) && (voltage_source_3_start_timer[0] <= myTimer.elapsed())){
            composeHeader(pinCard_A,set_V_VS3,voltage_source_3_V_DAC[0],0);
            sendByte((const char *)&header[0], 10);
            print_header();
            composeHeader(pinCard_A,set_I_VS3,voltage_source_3_A_DAC[0],0);
            sendByte((const char *)&header[0], 10);
            print_header();
            composeHeader(pinCard_A,enable_VS3,0,0);
            sendByte((const char *)&header[0], 10);
            print_header();
            VS3_timer_check[0] = 0;
        }


        if((VS3_timer_check[1] == 1) && (voltage_source_3_start_timer[1] <= myTimer.elapsed())){
            composeHeader(pinCard_B,set_V_VS3,voltage_source_3_V_DAC[1],0);
            sendByte((const char *)&header[0], 10);
            print_header();
            composeHeader(pinCard_B,set_I_VS3,voltage_source_3_A_DAC[1],0);
            sendByte((const char *)&header[0], 10);
            print_header();
            composeHeader(pinCard_B,enable_VS3,0,0);
            sendByte((const char *)&header[0], 10);
            print_header();
            VS3_timer_check[1] = 0;
        }
    }

}














void usb_interface::electrical_testing_function_4(int tab,QString table_name, QTextStream* out )
{
    //LOOP AMONG THE PINS 1->16
    for(int pin_no = 1; pin_no < 17; pin_no++)
    {
        QSqlQuery query;
        QString row = "A" + QString::number(pin_no);
        query.prepare(
                    "select measure_voltage,"
                    "measure_current,"
                    "min_voltage,"
                    "max_voltage,"
                    "min_current,"
                    "max_current"
                    " from '"+table_name+"' where row_name = '"+row+"' ");

        if(query.exec())
        {

            while (query.next()) {

                measure_voltage_checked[pin_no-1][tab] = query.value(0).toInt();
                measure_current_checked[pin_no-1][tab] = query.value(1).toInt();

                V_min_value[pin_no-1][tab] = query.value(2).toFloat();
                V_max_value[pin_no-1][tab] = query.value(3).toFloat();
                A_min_value[pin_no-1][tab] = query.value(4).toFloat();
                A_max_value[pin_no-1][tab] = query.value(5).toFloat();
            }
        }
        else
            qDebug()<<"query not selected from DB";     
    }

}

void usb_interface::electrical_testing_function_5()
{
    bool pincard = 0 ;
    QVector<int> next_pin = {1,1};

    request_count = 0;
    bool A_requested = 0;
    bool B_requested = 0;
    bool A_done = 0;
    bool B_done = 0;

    //ammeter range selection for pincards for one particular task
    if(ammeter_range[0] == "mA") shift_array_pincards[0] = 0b01000010;
    else if(ammeter_range[0] == "uA") shift_array_pincards[0] = 0b00100001;
    else if(ammeter_range[0] == "nA") shift_array_pincards[0] = 0b00000100;
    else if(ammeter_range[0] == "A")  shift_array_pincards[0] = 0b10001000;

    if(ammeter_range[1] == "mA") shift_array_pincards[21] = 0b01000010;
    else if(ammeter_range[1] == "uA") shift_array_pincards[21] = 0b00100001;
    else if(ammeter_range[1] == "nA") shift_array_pincards[21] = 0b00000100;
    else if(ammeter_range[1] == "A")  shift_array_pincards[21] = 0b10001000;



    while(next_pin[0] <=16 || next_pin[1] <=16)
    {
        if(measure_voltage_checked[next_pin[pincard]-1] [pincard]==1 ||
                measure_current_checked[next_pin[pincard]-1] [pincard]==1)
        {
            //------------------------------------------------------------------------------------//
            //              START- send packets after retrieving data from 7 vectors              //
            //------------------------------------------------------------------------------------//
            int pin = next_pin[pincard]-1;

            int index = 0;
            if(pin<8){
                index = pincard*21 + 20 - pin;
            }
            else{
                index = pincard*21 + 18 - pin;
            }

            //RESET
            //write reset values to shiftarraypincard (reset every A- A+ V bit to 0)
            for(int i =0; i< 16 ; i++){
                int temp;
                if(i<8){
                    temp = pincard*21 + 20 - i;
                }
                else{
                    temp = pincard*21 + 18 - i;
                }

                shift_array_pincards[temp] &= 0b11110100;
            }
            shift_array_pincards[0+21*pincard] &= 0b11101111;  //load unselect for reseting
            //send shift_config to reset A- A+ V values to 0
            Device pinCard = static_cast<Device>(pincard);
            //connect V
            composeHeader(pinCard,shift_config,0,21);
            sendByte((const char *)&header[0], 10);
            sendByte((const char *)&shift_array_pincards[21*pincard], 21);
            print_header();



            if(dynamic_load_selected[pin][pincard] == 1)
            {
                //Set dynamic load current
                int load_dac_value;
                if(sink_selected[pinCard]==1){
                    load_dac_value = 2500-I_sink[pincard];

                }
                else{
                    load_dac_value = 2500+I_source[pincard];

                }
                composeHeader(pinCard,set_load_dac,load_dac_value,0);
                sendByte((const char *)&header[0], 10);
                print_header();

                //A- A+ bits to 1
                shift_array_pincards[index] = 0b00001010;
                shift_array_pincards[0+21*pincard] |= 0b00010000;
                if(measure_voltage_checked[pin][pincard] == 1 ){
                    //V bit to 1
                    shift_array_pincards[index] |= 0b00000001;
                }

                //send shiftconfig to record shift_array_pincards
                Device pinCard = static_cast<Device>(pincard);
                composeHeader(pinCard,shift_config,0,21);
                sendByte((const char *)&header[0], 10);
                sendByte((const char *)&shift_array_pincards[21*pincard], 21);
                print_header();

                //measure V
                if(measure_voltage_checked[pin][pincard] == 1 ){
                    composeHeader(pinCard,measure_V,next_pin[pincard],0);
                    sendByte((const char *)&header[0], 10);
                    request_count += 1;
                    print_header();
                }

                //measure I
                if(measure_current_checked[pin][pincard] == 1 ){
                    composeHeader(pinCard,measure_I,next_pin[pincard],0);
                    sendByte((const char *)&header[0], 10);
                    request_count += 1;
                    print_header();
                }

            }

            else{
                if(measure_voltage_checked[pin][pincard] == 1 && measure_current_checked[pin][pincard] == 0){

                    shift_array_pincards[index] |= 0b00000001;

                    Device pinCard = static_cast<Device>(pincard);
                    //connect V
                    composeHeader(pinCard,shift_config,0,21);
                    sendByte((const char *)&header[0], 10);
                    sendByte((const char *)&shift_array_pincards[21*pincard], 21);
                    print_header();
                    //measure V
                    composeHeader(pinCard,measure_V,next_pin[pincard],0);
                    sendByte((const char *)&header[0], 10);
                    request_count += 1;
                    print_header();
                }
                else{
                    if(measure_voltage_checked[pin][pincard] == 1 ){
                        shift_array_pincards[index] |= 0b00000001;

                    }
                    //connect A+ A-
                    shift_array_pincards[index] |= 0b00001010;

                    Device pinCard = static_cast<Device>(pincard);
                    composeHeader(pinCard,shift_config,0,21);
                    sendByte((const char *)&header[0], 10);
                    sendByte((const char *)&shift_array_pincards[21*pincard], 21);
                    print_header();

                    //Disconnect direct relay
                    shift_array_pincards[index] &= 0b11111011;

                    composeHeader(pinCard,shift_config,0,21);
                    sendByte((const char *)&header[0], 10);
                    sendByte((const char *)&shift_array_pincards[21*pincard], 21);
                    print_header();

                    //measure V
                    if(measure_voltage_checked[pin][pincard] == 1 ){

                        composeHeader(pinCard,measure_V,next_pin[pincard],0);
                        sendByte((const char *)&header[0], 10);
                        request_count += 1;
                        print_header();
                    }
                    //measure I
                    if(measure_current_checked[pin][pincard] == 1 ){

                        composeHeader(pinCard,measure_I,next_pin[pincard],0);
                        sendByte((const char *)&header[0], 10);
                        request_count += 1;
                        print_header();
                    }


                    //connect direct relay
                    shift_array_pincards[index] |= 0b00000100;

                    composeHeader(pinCard,shift_config,0,21);
                    sendByte((const char *)&header[0], 10);
                    sendByte((const char *)&shift_array_pincards[21*pincard], 21);
                    print_header();

                }
            }


            next_pin[pincard] += 1;

            if(next_pin[pincard] > 16){
                if(pincard == 0){
                    A_done = 1;
                }
                else{
                    B_done = 1;
                }
            }


            if(pincard == 1 && next_pin[0] <= 16){
                pincard = 0;
            }
            else if(pincard == 0 && next_pin[1] <= 16){
                pincard = 1;
            }
            else if(next_pin[pincard] > 16 ){
                A_done = 1;
                B_done = 1;
                break;
            }


        }

        else
        {   
            next_pin[pincard] += 1;

            if(next_pin[pincard] > 16)
            {
                if(pincard == 1 && next_pin[0] <= 16){
                    pincard = 0;
                    B_done = 1;
                }
                else if(pincard == 0 && next_pin[1] <= 16){

                    pincard = 1;
                    A_done = 1;
                }
                else{
                    A_done = 1;
                    B_done = 1;
                    break;
                }
            }
        }
   }
}


void usb_interface::electrical_testing_function_6( QTextStream* out)
{
    int read_id = 0;
    int packet_size =10;
    int rx_device;
    int rx_command;
    int16_t rx_value;
    int payload_size;
    QByteArray requestData;

    QVector<int> measure_voltage_checked_ones_A;
    QVector<int> measure_current_checked_ones_A;
    QVector<int> measure_voltage_checked_ones_B;
    QVector<int> measure_current_checked_ones_B;

    for(int pin = 1 ; pin<17;pin++)
    {
            if(measure_voltage_checked[pin-1][0] == 1){
                measure_voltage_checked_ones_A.append(pin);
            }
            if(measure_current_checked[pin-1][0] == 1){
                measure_current_checked_ones_A.append(pin);
            }
            if(measure_voltage_checked[pin-1][1] == 1){
                measure_voltage_checked_ones_B.append(pin);
            }
            if(measure_current_checked[pin-1][1] == 1){
                measure_current_checked_ones_B.append(pin);
            }
    }

    while (request_count !=0)
    {

        //reading from usb buffer
        requestData = mainMCU->readAll();
        while (mainMCU->waitForReadyRead(5));
            requestData += mainMCU->readAll();


        //store buffer details to or fifo
        for(int i = 0; i<requestData.size(); i++){
            usb_rx_fifo.data[usb_rx_fifo.head] = requestData[i];
            usb_rx_fifo.head = FIFO_INCR(usb_rx_fifo.head );
        }


        //if a packet reading is ongoing
        if(read_id < packet_size)
        {
            //If data is available in the fifo
            if(usb_rx_fifo.tail != usb_rx_fifo.head)
            {
              rx_packet[read_id] = usb_rx_fifo.data[usb_rx_fifo.tail];
              usb_rx_fifo.tail = FIFO_INCR(usb_rx_fifo.tail);

              //finished reading the whole header(10 bytes)
              if(read_id == 9) // header is read
              {
                rx_device  = rx_packet[1];
                rx_command = rx_packet[2];
                rx_value   = (rx_packet[5]<<8)|(rx_packet[6]);

                payload_size = (rx_packet[7]<<8) | (rx_packet[8]); // read the payload_size

                if(payload_size != 0)
                {
                  packet_size = packet_size + payload_size; //extend the read length
                }
              }
              read_id++;
            }
        }


        //If a packet(header) reading has just successfully completed
        else
        {
            request_count -= 1;
            read_id = 0;
            packet_size = 10;

            int current_pin;

            if(rx_command==20 && rx_device==0){
                current_pin= measure_voltage_checked_ones_A[0];
                measure_voltage_checked_ones_A.pop_front();
            }
            else if(rx_command==21 && rx_device==0){
                current_pin= measure_current_checked_ones_A[0];
                measure_current_checked_ones_A.pop_front();
            }
            else if(rx_command==20 && rx_device==1){
                current_pin= measure_voltage_checked_ones_B[0];
                measure_voltage_checked_ones_B.pop_front();
            }
            else if(rx_command==21 && rx_device==1){
                current_pin= measure_current_checked_ones_B[0];
                measure_current_checked_ones_B.pop_front();
            }

            if(rx_command==20){
                float v_result = roundf(((float) rx_value )/pow(2,15) * 4.096 * 5 * 100)/100.0;

                if(v_result >= V_min_value[current_pin-1][rx_device] &
                   v_result <= V_max_value[current_pin-1][rx_device] )
                {
                    *out<<"Pincard: "<<pincard_names[rx_device] << "\tPin: "<< current_pin<< "   \tV_result: " <<QString::number(v_result, 'f', 2) <<"     \tV_min: "<<V_min_value[current_pin-1][rx_device]<<"\tV_max: "<<V_max_value[current_pin-1][rx_device] <<"\tStatus: PASS" <<Qt::endl;
                    send_print_to_review_text("Pincard: "+ pincard_names[rx_device] + "\tPin: "+ QString::number(current_pin)+ "\tV_result: " +QString::number(v_result, 'f', 2) +"     \tV_min: "+QString::number(V_min_value[current_pin-1][rx_device])+"\tV_max: "+ QString::number(V_max_value[current_pin-1][rx_device]) +"\tStatus: PASS");
                }
                else{
                    *out<<"Pincard: "<<pincard_names[rx_device] << "\tPin: "<< current_pin<< "   \tV_result: " <<QString::number(v_result, 'f', 2) <<"     \tV_min: "<<V_min_value[current_pin-1][rx_device]<<"\tV_max: "<<V_max_value[current_pin-1][rx_device] <<"\tStatus: FAIL" <<Qt::endl;
                    send_print_to_review_text("Pincard: "+ pincard_names[rx_device] + "\tPin: "+ QString::number(current_pin)+ "\tV_result: " +QString::number(v_result, 'f', 2) +"     \tV_min: "+QString::number(V_min_value[current_pin-1][rx_device])+"\tV_max: "+ QString::number(V_max_value[current_pin-1][rx_device]) +"\tStatus: FAIL");
                    task_pass_flag = 0;
                }
            }

            else if(rx_command==21){
                float i_result = roundf(((float) rx_value)/pow(2,15) * 4.096 * 1000 * 100)/100.0;
                if(ammeter_range[rx_device] == "uA") i_result += 11.6;
                else if(ammeter_range[rx_device] == "mA") i_result += 11.75;


                if(i_result >= A_min_value[current_pin-1][rx_device] &
                   i_result <= A_max_value[current_pin-1][rx_device] )
                {
                    *out <<"Pincard: "<<pincard_names[rx_device] << "\tPin: "<< current_pin<< "   \tI_result: " <<QString::number(i_result, 'f', 2) <<"     \tI_min: "<<A_min_value[current_pin-1][rx_device]<<"\tI_max: "<<A_max_value[current_pin-1][rx_device] <<"\tStatus: PASS" <<Qt::endl;
                    send_print_to_review_text("Pincard: "+ pincard_names[rx_device] + "\tPin: "+ QString::number(current_pin)+ "\tI_result: " +QString::number(i_result, 'f', 2) +"     \tI_min: "+QString::number(A_min_value[current_pin-1][rx_device])+"\tI_max: "+ QString::number(A_max_value[current_pin-1][rx_device]) +"\tStatus: PASS");

                }
                else{
                    *out<<"Pincard: "<<pincard_names[rx_device] << "\tPin: "<< current_pin<< "   \tI_result: " <<QString::number(i_result, 'f', 2) <<"     \tI_min: "<<A_min_value[current_pin-1][rx_device]<<"\tI_max: "<<A_max_value[current_pin-1][rx_device] <<"\tStatus: FAIL" <<Qt::endl;
                    send_print_to_review_text("Pincard: "+ pincard_names[rx_device] + "\tPin: "+ QString::number(current_pin)+ "\tI_result: " +QString::number(i_result, 'f', 2) +"     \tI_min: "+QString::number(A_min_value[current_pin-1][rx_device])+"\tI_max: "+ QString::number(A_max_value[current_pin-1][rx_device]) +"\tStatus: FAIL");
                    task_pass_flag = 0;

                }
            }
        }
    }

}


void usb_interface:: pg3_stop_clicked(){
    qDebug()<< "Main thread was interrupted!!!";
}


void usb_interface::print_header(){

    send_print_to_review_text("Device: "+ QString::number(header[1])+
                              "\t Command: "+ command_text(header[2])+
                              "\t Value: " + QString::number(header[3]<< 24 |header[4]<<16 |header[5]<<8 |header[6])+
                              "\t Payload Size: "+ QString::number(header[7]<<8 | header[8])
                             );
}


QString usb_interface::command_text(uint8_t command){
    switch (command) {
    case set_V_VS1: return "set_V_VS1" ;
    case set_V_VS2: return "set_V_VS2";
    case set_V_VS3:return "set_V_VS3";
    case set_I_VS1:return "set_I_VS1" ;
    case set_I_VS2:return "set_I_VS2";
    case set_I_VS3:return "set_I_VS3";
    case enable_VS1:return "enable_VS1" ;
    case enable_VS2:return "enable_VS2";
    case enable_VS3:return "enable_VS3";
    case disable_VS1:return "disable_VS1";
    case disable_VS2:return "disable_VS2";
    case disable_VS3:return "disable_VS3";
    case VS1_pin_FB_on:return "VS1_pin_FB_on";
    case VS1_pin_FB_off:return "VS1_pin_FB_off";
    case VS2_pin_FB_on:return "VS2_pin_FB_on";
    case VS2_pin_FB_off:return "VS2_pin_FB_off";
    case VS3_pin_FB_on:return "VS3_pin_FB_on";
    case VS3_pin_FB_off:return "VS3_pin_FB_off";
    case set_load_dac:return "set_load_dac";
    case shift_config:return "shift_config";
    case measure_V:return "measure_V";
    case measure_I:return "measure_I";
    case load_source_vector:return "load_source_vector";
    case read_capture_vector:return "read_capture_vector";
    case execute_sourcing:return "execute_sourcing";
    case reset_io_drivers:return "reset_io_drivers";
    case start_digital_testing: return "start_digital_testing";
    case set_fpga_frequency: return "set_fpga_frequency";
    case set_io_voltage: return "set_io_voltage";
    case set_source_mode: return "set_source_mode";
    case set_capture_mode: return "set_capture_mode";
    case clk_bypass_mode: return "clk_bypass_mode";
    }
}


uint8_t usb_interface::io_driver_deviceID(QString driver_string){
    return driver_string.mid(2).toInt()+15;
}


void usb_interface::waiting_till_IC_loaded()
{
    int read_id = 0;
    int rx_command;
    QByteArray requestData;

    while(1)
    {
        //reading from usb buffer
        requestData = mainMCU->readAll();
        while (mainMCU->waitForReadyRead(10))
            requestData += mainMCU->readAll();

        //store buffer details to or fifo
        for(int i = 0; i<requestData.size(); i++){
            usb_rx_fifo.data[usb_rx_fifo.head] = requestData[i];
            usb_rx_fifo.head = FIFO_INCR(usb_rx_fifo.head );
        }

        //if a packet reading is ongoing
        if(read_id < 10)
        {
            //If data is available in the fifo
            if(usb_rx_fifo.tail != usb_rx_fifo.head)
            {
              rx_packet[read_id] = usb_rx_fifo.data[usb_rx_fifo.tail];
              usb_rx_fifo.tail = FIFO_INCR(usb_rx_fifo.tail);

              //finished reading the whole header(10 bytes)
              if(read_id == 9) // header is read
              {
                rx_command = rx_packet[2];
              }
              read_id++;
            }
        }
        else if(rx_command == new_IC_loaded){
                break;
        }
        else{
            read_id = 0;
        }
    }
}


void usb_interface::request_to_start_reading_pincards()
{
    composeHeader(pincards,start_reading_pincards,0,0);
    sendByte((const char *)&header[0], 10);
}


void usb_interface::notify_test_over()
{
    composeHeader(pincards,test_over,0,0);
    sendByte((const char *)&header[0], 10);
}


//----------------------------------------------------------------------------------

void usb_interface::digital_testing_function(QString current_project, QString task_name, QTextStream* out){
    QSqlQuery query;
    QString table_name = current_project+"/"+task_name+"/Digital IO";
    QString code_string ;

    query.prepare("select code from '"+table_name+"' where ID = '"+QString::number(1)+"' ");
    if(query.exec())
    {
        if (query.next()) {
            code_string = query.value(0).toString();
        }
    }
    else{
        qDebug()<<"query for digital testing function didnt excecute";
    }
    QStringList list = code_string.trimmed().split(QRegExp(";"), QString::SkipEmptyParts);

    /*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/

    for (int i = 0; i<list.length();i++)
    {
        list[i] =list[i].trimmed();

        //-----------------------------------------------------------------------------------------------------//
        //                                    command type 1 <<                                                //
        //-----------------------------------------------------------------------------------------------------//
        if (list[i].contains("<<"))
        {
            QStringList command_string_line = list[i].split(QRegExp("<<"),QString::SkipEmptyParts);
            for(int j=0 ; j<command_string_line.length();j++){
                command_string_line[j] = command_string_line[j].trimmed().simplified().replace( " ", "" );
            }

            QString source_driver = command_string_line[0];

            QString source_vector_string = command_string_line[1];
            int div_by_8_reminder = source_vector_string.size()%8;
            QString zero("0");
            if(div_by_8_reminder != 0){
                QString zero_padding = zero.repeated(8-div_by_8_reminder);
                source_vector_string.append(zero_padding);
            }

            int number_of_bytes = source_vector_string.size()/8;
            data_source_vector.clear();
            for(int byte = 0; byte<number_of_bytes; byte++)
           {
               uint8_t data = 0;
               for(int i=0 ;i<8 ;i++){
                   if(source_vector_string[i + byte*8] == "1"){
                       data |= 1<< (7-i);
                   }

               }

               data_source_vector.append(data);

           }

           Device io_driver=static_cast<Device>(io_driver_deviceID(source_driver));
           composeHeader(io_driver,load_source_vector,0,data_source_vector.size());
           sendByte((const char *)&header[0], 10);
           sendByte((const char *)&data_source_vector[0], data_source_vector.size());
           print_header();

        }


        //-----------------------------------------------------------------------------------------------------//
        //                                    command type 2 execute()                                                //
        //-----------------------------------------------------------------------------------------------------//
        else if(list[i].contains("execute()"))
        {
            composeHeader(io,execute_sourcing,0,0);
            sendByte((const char *)&header[0], 10);
            print_header();
        }

        //-----------------------------------------------------------------------------------------------------//
        //                                    command type 3 >>                                                //
        //-----------------------------------------------------------------------------------------------------//
        else if (list[i].contains(">>")){
            QStringList command_string_line = list[i].split(QRegExp(">>"),QString::SkipEmptyParts);
            for(int j=0 ; j<command_string_line.length();j++){
                command_string_line[j] = command_string_line[j].trimmed().simplified().replace( " ", "" ).replace( ",", "" );
                command_string_line[j] = command_string_line[j].simplified().replace( ",", "" );
            }

            QString capture_driver = command_string_line[0];
            QString capture_vector_string = command_string_line[1];
            data_capture_vector[capture_driver.mid(2).toInt()-1] = capture_vector_string;

            int number_of_bytes = capture_vector_string.size()/8 +1;

            composeHeader(static_cast<Device>(io_driver_deviceID(capture_driver)),read_capture_vector,number_of_bytes,0);
            sendByte((const char *)&header[0], 10);
            print_header();

            capture_vectors_count += 1;
        }

        //-----------------------------------------------------------------------------------------------------//
        //                                    command type 4 reset()                                           //
        //-----------------------------------------------------------------------------------------------------//
        else if (list[i].contains("reset()")){
            composeHeader(io,reset_io_drivers,0,0);
            sendByte((const char *)&header[0], 10);
            print_header();

            for(int i=0; i<48; i++){
                data_capture_vector[i].clear();
            }
            data_source_vector.clear();
        }

        //-----------------------------------------------------------------------------------------------------//
        //                                    command type 5 clock                                            //
        //-----------------------------------------------------------------------------------------------------//
        else if(list[i].contains("clock")){
            QStringList command_string_line = list[i].split(QRegExp("="),QString::SkipEmptyParts);
            for(int j=0 ; j<command_string_line.length();j++){
                command_string_line[j] = command_string_line[j].trimmed().simplified().replace( " ", "" ).replace( ",", "" );
                command_string_line[j] = command_string_line[j].simplified().replace( ",", "" );
            }
            composeHeader(io, set_fpga_frequency, command_string_line[1].toInt(), 0);
            sendByte((const char *)&header[0], 10);
            print_header();

        }

        //-----------------------------------------------------------------------------------------------------//
        //                                    command type 6 io_voltage =                                           //
        //-----------------------------------------------------------------------------------------------------//
        else if(list[i].contains("io_voltage")){
            QStringList command_string_line = list[i].split(QRegExp("="),QString::SkipEmptyParts);
            for(int j=0 ; j<command_string_line.length();j++){
                command_string_line[j] = command_string_line[j].trimmed().simplified().replace( " ", "" ).replace( ",", "" );
                command_string_line[j] = command_string_line[j].simplified().replace( ",", "" ).replace("mV","");
            }
            composeHeader(io, set_io_voltage, command_string_line[1].toInt(), 0);
            sendByte((const char *)&header[0], 10);
            print_header();

        }

        //-----------------------------------------------------------------------------------------------------//
        //                                    command type 7 OUTPUT                                           //
        //-----------------------------------------------------------------------------------------------------//
        else if(list[i].contains("OUTPUT") ||list[i].contains("output")||list[i].contains("Output")){
            QStringList command_string_line = list[i].trimmed().split(QRegExp("="),QString::SkipEmptyParts);
            for(int j=0 ; j<command_string_line.length();j++){
                command_string_line[j] = command_string_line[j].trimmed().simplified().replace( " ", "" ).replace( ",", "" );
                command_string_line[j] = command_string_line[j].simplified().replace( ",", "" );
            }

            QString io_driver = command_string_line[0];

            composeHeader(static_cast<Device>(io_driver_deviceID(io_driver)), set_source_mode, 0, 0);
            sendByte((const char *)&header[0], 10);
            print_header();
        }

        //-----------------------------------------------------------------------------------------------------//
        //                                    command type 8 INPUT                                           //
        //-----------------------------------------------------------------------------------------------------//
        else if(list[i].contains("INPUT") ||list[i].contains("input")||list[i].contains("Input")){
            QStringList command_string_line = list[i].trimmed().split(QRegExp("="),QString::SkipEmptyParts);
            for(int j=0 ; j<command_string_line.length();j++){
                command_string_line[j] = command_string_line[j].trimmed().simplified().replace( " ", "" ).replace( ",", "" );
                command_string_line[j] = command_string_line[j].simplified().replace( ",", "" );
            }

            QString io_driver = command_string_line[0];

            composeHeader(static_cast<Device>(io_driver_deviceID(io_driver)), set_capture_mode, 0, 0);
            sendByte((const char *)&header[0], 10);
            print_header();
        }

        //-----------------------------------------------------------------------------------------------------//
        //                                    command type 8 clk_out                                           //
        //-----------------------------------------------------------------------------------------------------//
        else if(list[i].contains("clk_out") ||list[i].contains("CLK_OUT")||list[i].contains("CLKOUT")){
            QStringList command_string_line = list[i].trimmed().split(QRegExp("="),QString::SkipEmptyParts);
            for(int j=0 ; j<command_string_line.length();j++){
                command_string_line[j] = command_string_line[j].trimmed().simplified().replace( " ", "" ).replace( ",", "" );
                command_string_line[j] = command_string_line[j].simplified().replace( ",", "" );
            }

            QString io_driver = command_string_line[0];

            composeHeader(static_cast<Device>(io_driver_deviceID(io_driver)), clk_bypass_mode, 0, 0);
            sendByte((const char *)&header[0], 10);
            print_header();
        }

    }
}


void usb_interface::digital_testing_function2(QString current_project, QString task_name, QTextStream* out)
{
    int read_id = 0;
    int packet_size =10;
    int rx_device;
    int rx_command;
    int16_t rx_value;
    int payload_size;
    QByteArray requestData;

    while (capture_vectors_count !=0)
    {

        //reading from usb buffer
        requestData = mainMCU->readAll();
        while (mainMCU->waitForReadyRead(0))
            requestData += mainMCU->readAll();


        //store buffer details to or fifo
        for(int i = 0; i<requestData.size(); i++){
            usb_rx_fifo.data[usb_rx_fifo.head] = requestData[i];
            usb_rx_fifo.head = FIFO_INCR(usb_rx_fifo.head );
        }


        //if a packet reading is ongoing
        if(read_id < packet_size){

            //If data is available in the fifo
            if(usb_rx_fifo.tail != usb_rx_fifo.head)
            {
              rx_packet[read_id] = usb_rx_fifo.data[usb_rx_fifo.tail];
              usb_rx_fifo.tail = FIFO_INCR(usb_rx_fifo.tail);

              //finished reading the whole header(10 bytes)
              if(read_id == 9) // header is read
              {
                rx_device  = rx_packet[1];
                rx_command = rx_packet[2];
                rx_value   = (rx_packet[5]<<8)|(rx_packet[6]);
                payload_size = (rx_packet[7]<<8) | (rx_packet[8]); // read the payload_size

                if(payload_size != 0)
                {
                  packet_size = packet_size + payload_size; //extend the read length
                }
              }
              read_id++;
            }
        }


        //If a packet reading has just successfully completed
        else
        {
            capture_vectors_count -= 1;
            read_id = 0;
            packet_size = 10;

            if(rx_command == read_capture_vector)
            {
                int capture_vector_id = rx_device - 16;
                int number_of_bytes = data_capture_vector[capture_vector_id].size()/8 +1;
                QString received_vector = "";
                QString zero("0");

                for(int k=0; k<number_of_bytes; k++){
                    QString binary_number_str = QString::number(rx_packet[10+k],2);
                    QString byte_string = zero.repeated(8-binary_number_str.size());
                    byte_string.append(binary_number_str);

                    received_vector.append(byte_string);
                }
                received_vector = received_vector.mid(1,data_capture_vector[capture_vector_id].size());
                qDebug()<<"received vector: "<<received_vector;

                int fail_flag=0;
                for(int m=0; m<received_vector.size(); m++){
                    if(data_capture_vector[capture_vector_id][m] != 'x' && data_capture_vector[capture_vector_id][m] != 'X' ){
                        if(data_capture_vector[capture_vector_id][m]!=received_vector[m]){
                            fail_flag = 1;
                        }
                    }
                }

                if(fail_flag == 0){
                    *out <<"io"<<(rx_device-15)<< "\nExpected: "<<data_capture_vector[capture_vector_id]<< "\nReceived: " <<received_vector<<"\nStatus: PASS" <<Qt::endl;
                    send_print_to_review_text("io"+QString::number(rx_device-15)+"\nExpected: "+data_capture_vector[capture_vector_id]+"\nReceived: "+received_vector+"\nStatus: PASS");
                }
                else{
                    *out <<"io"<<(rx_device-15)<< "\nExpected: "<<data_capture_vector[capture_vector_id]<< "\nReceived: " <<received_vector<<"\nStatus: FAIL" <<Qt::endl;
                    send_print_to_review_text("io"+QString::number(rx_device-15)+"\nExpected: "+data_capture_vector[capture_vector_id]+"\nReceived: "+received_vector+"\nStatus: FAIL");
                    task_pass_flag = 0;
                }
            }
        }
    }
}

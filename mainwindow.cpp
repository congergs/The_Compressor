#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QString>
#include <QDebug>
#include <QFileInfo>
#include "audio-adpcm-cpp\src\adpcm.cpp"
#include "audio-adpcm-cpp\src\wave.cpp"
#include <fstream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushCompress_clicked()
{
    ui->label_3->setText("");

    qDebug() << "Selected file: " << fileSrc;

    QString fileDst = fileSrc;
    fileDst.replace(".wav", ".adp");

    std::ifstream in(fileSrc.toStdString(), std::ios::binary);

    std::cout << fileSrc.toStdString() << std::endl;



    WAVEHeader wav;

    in.read((char*)&wav, sizeof(WAVEHeader));

    if(isCorrectHeader(wav))
    {
        if(wav.bitsPerSample == 16 || wav.numChannels <= 2)
        {
            char* samples = new char[wav.subchunk2Size];
            in.read(samples, wav.subchunk2Size);

            ADPCMHeader adp;
            char* data = new char[ADPCMDataSize(wav)];

            compress(samples, data, wav, adp);

            std::ofstream out(fileDst.toStdString(), std::ios::binary);
            out.write((char*)&adp, sizeof(ADPCMHeader));
            out.write((char*)&wav, sizeof(WAVEHeader));
            out.write(data, adp.dataSize);
            out.close();

            ui->label_3->setText("Done!");
        }
        else
        {   ui->label_3->setText("WAV Format not supported");
            in.close();
            return;
        }
    }
    else
    {
        ui->label_3->setText("File type not supported");
        in.close();
        return;
    }


}


void MainWindow::on_pushDecompress_clicked()
{
    ui->label_3->setText("");

    QString fileDst = fileSrc;
    fileDst.replace(".adp", "-decompressed.wav");



    WAVEHeader  wav;
    ADPCMHeader adp;

    std::ifstream in(fileSrc.toStdString(), std::ios::binary);

    if(!in)
    {   std::cerr << fileSrc.toStdString() << " : No such file or directory\n";
        return;
    }

    in.read((char*)&adp, sizeof(ADPCMHeader));
    in.read((char*)&wav, sizeof(WAVEHeader));

    if(isCorrectHeader(adp))
    {
        if(isCorrectHeader(wav))
        {   char* samples = new char[wav.subchunk2Size];
            char* data    = new char[adp.dataSize];
            in.read((char*)data, adp.dataSize);
            in.close();

            decompress(data, samples, adp);

            std::ofstream out(fileDst.toStdString(), std::ios::binary);
            out.write((char*)&wav, sizeof(WAVEHeader));
            out.write(samples, wav.subchunk2Size);
            out.close();

            ui->label_3->setText("Done!");
        }
        else
        {   ui->label_3->setText("WAV Format not supported");
            in.close();
            return;
        }
    }
    else
    {   ui->label_3->setText("File type not supported");
        in.close();
        return;
    }
}

void MainWindow::on_selectFile_clicked()
{
    ui->label_3->setText("");
    ui->label_2->setText("");
    ui->statusbar->showMessage("");

    fileSrc = QFileDialog::getOpenFileName(this,
                                                   tr("Open Image"), "../../../", tr("Audio Files (*.adp *.wav)"));
    fileSrc.replace('/', '\\');

    if(fileSrc == "")
    {
        ui->statusbar->showMessage("No file selected!");
        return;
    }



    ui->label_2->setText(QFileInfo(QFile(fileSrc)).fileName());
}



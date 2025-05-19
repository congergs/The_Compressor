#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QString>
#include <QDebug>
#include <QFileInfo>
#include "audio-adpcm-cpp\src\adpcm.cpp"
#include "audio-adpcm-cpp\src\wave.cpp"
#include "huffman-compression\huffman-compression.cpp"
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

    QString fileDst = fileSrc;



    std::cout << fileSrc.toStdString() << std::endl;

    if(lossy)
    {
        fileDst.replace(".wav", ".bura");
        std::ifstream in(fileSrc.toStdString(), std::ios::binary);

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

                QString display = QString("Done!\nCompressed and saved as: %1").arg(QFileInfo(QFile(fileDst)).fileName());
                ui->label_3->setText(display);
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
    else
    {
        fileDst.replace(".wav", ".burak");

        ll filesize, predfilesize;

        filesize = Utility::get_file_size(fileSrc.toStdString().c_str());
        auto mapper = CompressUtility::parse_file(fileSrc.toStdString().c_str(), filesize);
        Node *const root = CompressUtility::generate_huffman_tree(mapper);
        std::string buf = "";
        predfilesize = CompressUtility::store_huffman_value(root, buf);

        CompressUtility::compress(fileSrc.toStdString().c_str(), fileDst.toStdString().c_str(), filesize, predfilesize);

        QString display = QString("Done!\nCompressed and saved as: %1").arg(QFileInfo(QFile(fileDst)).fileName());
        ui->label_3->setText(display);
    }



}


void MainWindow::on_pushDecompress_clicked()
{
    ui->label_3->setText("");

    QString fileDst = fileSrc;

    if(lossy)
    {
        fileDst.replace(".bura", "-lossydecompressed.wav");



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

                QString display = QString("Done!\nDecompressed and saved as: %1").arg(QFileInfo(QFile(fileDst)).fileName());
                ui->label_3->setText(display);
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
    else if(fileSrc.contains(".burak"))
    {
        fileDst.replace(".burak", "-losslessdecompressed.wav");

        ll filesize, predfilesize;

        filesize = Utility::get_file_size(fileSrc.toStdString().c_str());
        DecompressUtility::decompress(fileSrc.toStdString().c_str(), fileDst.toStdString().c_str(), filesize, predfilesize);

        QString display = QString("Done!\nDecompressed and saved as: %1").arg(QFileInfo(QFile(fileDst)).fileName());
        ui->label_3->setText(display);
    }
    else
    {
        ui->label_3->setText("File type not supported");
    }


}

void MainWindow::on_selectFile_clicked()
{
    ui->label_3->setText("");
    ui->label_2->setText("");
    ui->statusbar->showMessage("");

    fileSrc = QFileDialog::getOpenFileName(this,
                                                   tr("Open Image"), "../../../", tr("Audio Files (*.bura *.wav *.burak)"));
    fileSrc.replace('/', '\\');

    if(fileSrc == "")
    {
        ui->statusbar->showMessage("No file selected!");
        return;
    }



    ui->label_2->setText(QFileInfo(QFile(fileSrc)).fileName());
}



void MainWindow::on_selectMethod_currentIndexChanged(int index)
{
    if(!ui->selectMethod->currentIndex()) lossy = true;
    else lossy = false;
}

